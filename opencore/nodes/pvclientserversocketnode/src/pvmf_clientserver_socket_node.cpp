/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include "pvmf_clientserver_socket_node.h"
#include "oscl_string_utils.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pvmf_errorinfomessage_extension.h"
#include "oscl_byte_order.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "oscl_tickcount.h"
#include "oscl_rand.h"
#include "oscl_time.h"
#include "oscl_bin_stream.h"

// Use default DLL entry point for Symbian
#include "oscl_dll.h"

//////////////////////////////////////////////////
// Standard Node APIs
//////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFClientServerSocketNode::PVMFClientServerSocketNode(OsclTCPSocket* aSocketHandle, int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFClientServerSocketNode")
        , iSockServ(NULL)
        , TIMEOUT_CONNECT(-1)
        , TIMEOUT_SEND(-1)
        , TIMEOUT_RECV(-1)
        , TIMEOUT_SHUTDOWN(-1)
        , iSocketHandle(aSocketHandle)
        , iSockConfig(NULL)
{
    iLogger = NULL;
    iDataPathLogger = NULL;
    iOsclErrorTrapImp = NULL;
    iExtensionRefCount = 0;
    iMaxTcpRecvBufferSize = SNODE_DEFAULT_SOCKET_TCP_BUFFER_SIZE;
    iMaxTcpRecvBufferCount = SNODE_DEFAULT_SOCKET_TCP_BUFFER_COUNT;
    iSocketID = 0;
    iCommandErrorCode = PVMFSocketNodeErrorEventStart;
    iErrorEventErrorCode = PVMFSocketNodeErrorEventStart;

    iPVMFPort = NULL;
    iNumStopPortActivityPending = (-1);//inactive.

    int32 err;
    OSCL_TRY(err,

             iPendingCmdQueue.Construct(PVMF_SOCKET_NODE_COMMAND_ID_START,
                                        PVMF_SOCKET_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCmdQueue.Construct(0, 1);
             iCancelCmdQueue.Construct(0, 1);
            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iPendingCmdQueue.clear();
        iCurrentCmdQueue.clear();
        iCancelCmdQueue.clear();
        if (iPVMFPort)
        {
            iPVMFPort->Disconnect();
            delete iPVMFPort;
            iPVMFPort = NULL;
        }
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclActiveObject);
        OSCL_LEAVE(err);
    }
}


OSCL_EXPORT_REF PVMFClientServerSocketNode::~PVMFClientServerSocketNode()
{
    Cancel();

    //thread logoff
    if (IsAdded())
        RemoveFromScheduler();

    /* Cleanup allocated ports */
    CleanupPorts();

    //Cleanup commands
    while (!iCurrentCmdQueue.empty())
    {
        CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFFailure);
    }
    while (!iPendingCmdQueue.empty())
    {
        CommandComplete(iPendingCmdQueue, iPendingCmdQueue.front(), PVMFFailure);
    }
    while (!iCancelCmdQueue.empty())
    {
        CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFFailure);
    }
}


OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::ThreadLogon()
{
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFClientServerSocketNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.socketnode");
            iOsclErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
        default:
            return PVMFErrInvalidState;
    }
}


OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:ThreadLogoff"));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            if (IsAdded())
                RemoveFromScheduler();
            iLogger = NULL;
            iDataPathLogger = NULL;
            iOsclErrorTrapImp = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;
        }
        default:
            return PVMFErrInvalidState;
    }
}


OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    OSCL_UNUSED_ARG(aNodeCapability);
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFPortIter* PVMFClientServerSocketNode::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);
    //retrive a port iterator
    return (PVMFPortIter*)iPVMFPort;
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:RequestPort"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:ReleasePort"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Init"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Init"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Start"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Stop"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Flush"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Pause"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:Reset"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:CancelAllCommands"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFClientServerSocketNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:CancelCommand"));
    PVMFSocketNodeCommand cmd;
    cmd.PVMFSocketNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

//Port activity handler
void PVMFClientServerSocketNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVMFClientServerSocketPort* sockPort = NULL;
    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CONNECT:
        {
            sockPort = OSCL_STATIC_CAST(PVMFClientServerSocketPort*, aActivity.iPort);
            OSCL_ASSERT(sockPort && sockPort->iConfig);
            *iSockConfig = *sockPort->iConfig;

            setSocketPortMemAllocator(aActivity.iPort, sockPort->iAllocSharedPtr);

            //Receives may have been blocked waiting on the port to be connected, so check here.
            if (CanReceive())
                StartRecvOperation();
        }
        break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
        {
            sockPort = (PVMFClientServerSocketPort*)(aActivity.iPort);
            OSCL_ASSERT(sockPort && sockPort->iConfig);

            //Try to process this message now.
            if (CanProcessIncomingMsg())
                ProcessIncomingMsg();
            //Otherwise, ignore this event now.  Other code will check
            //the input queue as needed.
        }
        break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
        {
            sockPort = (PVMFClientServerSocketPort*)(aActivity.iPort);
            OSCL_ASSERT(sockPort && sockPort->iConfig);
            SocketPortConfig& aSockConfig = *sockPort->iConfig;

            if (aSockConfig.iState.iRecvOperation == EPVSocketPortRecvOperation_WaitOnConnectedPort)
            {
                RecvOperationComplete(PVMFSuccess, NULL);
            }

            //Otherwise ignore this event now.  Other code will check connected
            //port status as needed.
        }
        break;

        default:
            //all other events can be ignored.
            break;
    }
}


OSCL_EXPORT_REF void PVMFClientServerSocketNode::addRef()
{
    ++iExtensionRefCount;
}

OSCL_EXPORT_REF void PVMFClientServerSocketNode::removeRef()
{
    --iExtensionRefCount;
}

OSCL_EXPORT_REF bool PVMFClientServerSocketNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    OSCL_UNUSED_ARG(uuid);
    OSCL_UNUSED_ARG(iface);
    return false;
}

void PVMFClientServerSocketNode::Run()
{
    //Process node commands
    while (CanProcessCommand())
        ProcessCommand(iPendingCmdQueue, iPendingCmdQueue.front());
}


bool PVMFClientServerSocketNode::CanProcessCommand()
{
    return (!iPendingCmdQueue.empty()
            && (iCurrentCmdQueue.empty()
                || (iPendingCmdQueue.front().hipri() && iCancelCmdQueue.empty())));
}

//Process an input command.
void PVMFClientServerSocketNode::ProcessCommand(PVMFSocketNodeCmdQ& aCmdQ, PVMFSocketNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFSocketNodeCommand::ProcessCommand() in"));

    PVMFStatus status = PVMFFailure;
    iCommandErrorCode = PVMFSocketNodeErrorEventStart; //no error

    if (aCmd.hipri())
    {
        //calling logic should prevent multiple cancels in progress.
        OSCL_ASSERT(iCancelCmdQueue.empty());

        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
                status = DoCancelAllCommands(aCmd);
                break;

            case PVMF_GENERIC_NODE_CANCELCOMMAND:
                status = DoCancelCommand(aCmd);
                break;

            default://unknown command type
                status = PVMFFailure;
                break;
        }

        //If the command was not finished in this call, then move it to the
        //"cancel cmd" queue where it will remain until it is completed.
        if (status == PVMFPending)
        {
            //move the new cmd to the "cancel cmd" queue where it will
            //remain until complete.
            int32 err;
            OSCL_TRY(err, iCancelCmdQueue.StoreL(aCmd););

            //we reserved space in the CancelCmdQueue already, so
            //it should not be possible to fail here.
            OSCL_ASSERT(err == OsclErrNone);

            //erase the original command
            aCmdQ.Erase(&aCmd);
        }
        else
        {
            CommandComplete(aCmdQ, aCmd, status, NULL);
        }
    }
    else
    {
        //calling logic should prevent multiple commands in progress.
        OSCL_ASSERT(iCurrentCmdQueue.empty());

        OsclAny* eventData = NULL;

        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_QUERYUUID:
                status = DoQueryUuid(aCmd);
                break;

            case PVMF_GENERIC_NODE_QUERYINTERFACE:
                status = DoQueryInterface(aCmd);
                break;

            case PVMF_GENERIC_NODE_REQUESTPORT:
            {
                status = DoRequestPort(aCmd);
                eventData = iPVMFPort;
                break;
            }

            case PVMF_GENERIC_NODE_RELEASEPORT:
                status = DoReleasePort(aCmd);
                break;

            case PVMF_GENERIC_NODE_INIT:
                status = DoInit(aCmd);
                break;

            case PVMF_GENERIC_NODE_PREPARE:
                status = DoPrepare(aCmd);
                if (status == PVMFSuccess)
                {
                    ChangeExternalState(EPVMFNodePrepared);
                }
                break;

            case PVMF_GENERIC_NODE_START:
                status = DoStart(aCmd);
                break;

            case PVMF_GENERIC_NODE_STOP:
                status = DoStop(aCmd);
                break;

            case PVMF_GENERIC_NODE_FLUSH:
                status = DoFlush(aCmd);
                break;

            case PVMF_GENERIC_NODE_PAUSE:
                status = DoPause(aCmd);
                break;

            case PVMF_GENERIC_NODE_RESET:
                status = DoReset(aCmd);
                break;

            default://unknown command type
                status = PVMFFailure;
                break;
        }

        if (status == PVMFPending)
        {
            int32 err;
            OSCL_TRY(err, iCurrentCmdQueue.StoreL(aCmd););
            OSCL_ASSERT(err == OsclErrNone);

            //erase the original command
            aCmdQ.Erase(&aCmd);
        }
        else
        {
            CommandComplete(aCmdQ, aCmd, status, eventData);
        }
    }
}

//Called to complete a node command.
void PVMFClientServerSocketNode::CommandComplete(PVMFSocketNodeCmdQ& aCmdQ,
        PVMFSocketNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)

{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    bool canProcess = CanProcessCommand();

    PVMFStatus status = aStatus;

    if (aStatus == PVMFSuccess)
    {
        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_INIT:
                ChangeExternalState(EPVMFNodeInitialized);
                break;

            case PVMF_GENERIC_NODE_PREPARE:
                ChangeExternalState(EPVMFNodePrepared);
                break;

            case PVMF_GENERIC_NODE_START:
                ChangeExternalState(EPVMFNodeStarted);
                break;

            case PVMF_GENERIC_NODE_STOP:
                ChangeExternalState(EPVMFNodePrepared);
                break;

            case PVMF_GENERIC_NODE_PAUSE:
                ChangeExternalState(EPVMFNodePaused);
                break;

            case PVMF_GENERIC_NODE_RESET:
                ChangeExternalState(EPVMFNodeCreated);

                //Complete the reset command.
                {
                    //cleanup all ports.
                    CleanupPorts();
                    SetState(EPVMFNodeIdle);
                }
                break;

            case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
                //Complete the reset command.
            {
                //Since "cancel all" is effectively used as a Reset,
                //go ahead and cleanup all ports.
                CleanupPorts();
            }
            break;
            default:
                break;
        }
    }

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }
    //else if no error input, see if "command error code" was set.
    else if (iCommandErrorCode != PVMFSocketNodeErrorEventStart)
    {
        PVUuid eventuuid = PVMFSocketNodeEventTypeUUID;
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (iCommandErrorCode, eventuuid, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, status, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }
    iCommandErrorCode = PVMFSocketNodeErrorEventStart;//reset.

    //See if there was a pending cancel waiting on the current command to
    //complete.
    if (!iCancelCmdQueue.empty()
            && iCancelCmdQueue.front().iCmd == PVMF_GENERIC_NODE_CANCELCOMMAND)
    {
        CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFSuccess);
    }

    //May need to resume command handling if the AO was blocked
    //waiting on asynchronous command completion, but it's unblocked now.
    if (!canProcess
            && CanProcessCommand()
            && IsAdded())
    {
        RunIfNotReady();
    }
}

PVMFCommandId PVMFClientServerSocketNode::QueueCommandL(PVMFSocketNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::QueueCommandL()"));
    PVMFCommandId id;

    id = iPendingCmdQueue.AddL(aCmd);

    //This may be a processing trigger.
    //Wakeup the AO if needed.
    if (IsAdded()
            && CanProcessCommand())
    {
        RunIfNotReady();
    }

    return id;
}

//This is the callback from Oscl Sockets for socket operation completion.
OSCL_EXPORT_REF  void PVMFClientServerSocketNode::HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::HandleSocketEvent() In aId=%d, aFxn=%d, aEvent=%d, aError=%d", aId, aFxn, aEvent, aError));

    if (iSockConfig && aEvent == EPVSocketSuccess)
    {
        PVMFSocketActivity activity((aEvent == EPVSocketSuccess) ? PVMFSuccess : PVMFFailure, aId, aFxn, aEvent, aError);

        //Call the appropriate handler
        switch (aFxn)
        {
            case EPVSocketRecv:
                OSCL_ASSERT(iSockConfig->iState.iRecvOperation == EPVSocketPortRecvOperation_Recv);

                RecvOperationComplete(activity.iStatus, &activity);
                break;

            case EPVSocketSend:
                OSCL_ASSERT(iSockConfig->iState.iSendOperation == EPVSocketPortSendOperation_Send);

                SendOperationComplete(activity.iStatus, &activity);
                break;

            case EPVSocketShutdown:
                //OSCL_ASSERT(iSockConfig->iState.iConnectOperation==EPVSocketPortConnectOperation_Shutdown);
                SequenceComplete(PVMFSuccess);
                //ConnectOperationComplete(activity.iStatus,&activity);
                break;

            default:
                OSCL_ASSERT(0);//unexpected
                break;
        }
    }
    else if (aEvent == EPVSocketCancel)
    {
        switch (aFxn)
        {
            case EPVSocketRecv:
                if (iSockConfig->iState.iSendOperation == EPVSocketPortSendOperation_Send
                        && iSockConfig->iState.iSendOperationCanceled == false)
                {//if does hv anything to send, cancel it
                    CancelSendOperation();
                }
                else
                {	//shutdown
                    CloseSocketConnection();
                }
                break;

            case EPVSocketSend:
                if (iSockConfig->iState.iRecvOperation == EPVSocketPortRecvOperation_Recv
                        && iSockConfig->iState.iRecvOperationCanceled == false)
                {
                    CancelRecvOperation();
                }
                else
                {
                    //shutdown
                    CloseSocketConnection();
                }
                break;

            case EPVSocketShutdown:
                //quite difficult to cancel shutdown
                break;

            default:
                OSCL_ASSERT(0);//unexpected
                break;
        }
    }
    else
    {//failure
        switch (aFxn)
        {
            case EPVSocketRecv:
                iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_None;
                iSockConfig->iState.iRecvOperationCanceled = true;
                break;

            case EPVSocketSend:
                iSockConfig->iState.iSendOperation = EPVSocketPortSendOperation_None;
                iSockConfig->iState.iSendOperationCanceled = true;
                break;

            case EPVSocketShutdown:
            default:
                OSCL_ASSERT(0);//unexpected
                break;
        }
    }
}


//Allocate a node port and add it to the port vector.
PVMFStatus PVMFClientServerSocketNode::AddPort(int32 tag)
{
    iPVMFPort = new PVMFClientServerSocketPort((int32)tag,
            this,
            DEFAULT_DATA_QUEUE_CAPACITY,
            DEFAULT_DATA_QUEUE_CAPACITY,
            DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT,
            //the output queue is empty because
            //this node pushes data directly to the
            //connecte port's input.
            0, 0, 0);

    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::AllocatePortMemPool(int32 tag, PVMFSocketNodeMemPool* & aMemPool)
{
    aMemPool = NULL;
    OsclAny *MemPtr = NULL;
    MemPtr = iAlloc.ALLOCATE(sizeof(PVMFSocketNodeMemPool));
    if (MemPtr == NULL)
    {
        return PVMFErrNoMemory;
    }

    int32 errcode = 0;
    OSCL_TRY(errcode, aMemPool = OSCL_PLACEMENT_NEW(MemPtr, PVMFSocketNodeMemPool(SNODE_DEFAULT_NUMBER_MEDIADATA_IN_MEMPOOL);));
    if (errcode != OsclErrNone)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::DoRequestPort: PVMFSocketNodeMemPool Construct Failed Ln %d", errcode, __LINE__));
        iAlloc.deallocate(MemPtr);
        return PVMFErrNoMemory;
    }

    aMemPool->iPortTag = tag;
    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::CancelSendOperation()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::CancelSendOperation()"));

    PVMFStatus status = PVMFSuccess;

    switch (iSockConfig->iState.iSendOperation)
    {
        case EPVSocketPortSendOperation_None:
            break;

        case EPVSocketPortSendOperation_Send:
            if (iSockConfig->iTCPSocket)
            {
                if (!iSockConfig->iState.iSendOperationCanceled)
                {
                    iSockConfig->iState.iSendOperationCanceled = true;
                    iSockConfig->iTCPSocket->CancelSend();
                }
                status = PVMFPending;
                //wait on send to complete in HandleSocketEvent
            }
            break;

        default:
            OSCL_ASSERT(0);
            status = PVMFFailure;
            break;
    }
    return status;
}


PVMFStatus PVMFClientServerSocketNode::DoStopNodeActivity()
{
    if (iNumStopPortActivityPending > 0)
        return PVMFPending;

    //Stop socket activity on all ports.
    uint32 nPortsPending = 0;
    if (iSockConfig)
    {
        // discard any saved socket activity events
        iSockConfig->iSocketRecvActivity.iValid = false;

        //if a request port is going on, be sure to complete the command,
        //although we will interrupt the current processing.
        if (iSockConfig->iState.iSequence == EPVSocketPortSequence_RequestPort
                && iCurrentCmdQueue.size()
                && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_REQUESTPORT)
        {
            CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFErrCancelled, NULL);
        }

        //if a cleanup is already underway, just keep waiting for it to complete,
        //else start a new sequence
        if (iSockConfig->iState.iSequence == EPVSocketPortSequence_SocketCleanup)
        {
            nPortsPending++;
        }
        else if (StartSequence(iSockConfig->iState.iSequence = EPVSocketPortSequence_SocketCleanup) == PVMFPending)
        {
            nPortsPending++;
        }
    }

    if (nPortsPending > 0)
        iNumStopPortActivityPending = nPortsPending;

    if (iNumStopPortActivityPending > 0)
        return PVMFPending; //wait on completion in SequenceComplete.

    return PVMFSuccess;
}

//Flush is implemented for this node, but hasn't been tested.
PVMFStatus PVMFClientServerSocketNode::DoFlush(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::DoFlush() In"));

    if ((iInterfaceState != EPVMFNodeStarted) && (iInterfaceState != EPVMFNodePaused))
    {
        return PVMFErrInvalidState;
    }

    PVMFStatus status = PVMFSuccess;
    {
        iPVMFPort->SuspendInput();
        if (status != PVMFPending
                && iPVMFPort->IncomingMsgQueueSize() > 0)
        {
            status = PVMFPending;//Wait on this queue to empty.
            //Completion is detected in SequenceComplete.
        }
    }
    return status;
}

//Pause is a do-nothing for this node.
PVMFStatus PVMFClientServerSocketNode::DoPause(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::DoPause() In"));

    if (iInterfaceState == EPVMFNodePaused)
    {
        return PVMFSuccess;
    }

    if (iInterfaceState != EPVMFNodeStarted)
    {
        return PVMFErrInvalidState;
    }

    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::DoReset(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::DoReset()"));

    /* This node allows a reset from any state */

    return DoStopNodeActivity();
}

PVMFStatus PVMFClientServerSocketNode::DoQueryUuid(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    /*	//This node supports Query UUID from any state

    	OSCL_String* mimetype;
    	Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    	bool exactmatch;
    	aCmd.PVMFSocketNodeCommandBase::Parse(mimetype,uuidvec,exactmatch);

    	//Try to match the input mimetype against any of
    	//the custom interfaces for this node

    	//Match against custom interface1...
    	if (*mimetype==PVMF_SOCKET_NODE_EXTENSION_INTERFACE_MIMETYPE
    		//also match against base mimetypes for custom interface1,
    		//unless exactmatch is set.
    		|| (!exactmatch && *mimetype==PVMF_SOCKET_NODE_MIMETYPE)
    		|| (!exactmatch && *mimetype==PVMF_SOCKET_NODE_BASEMIMETYPE))
    	{

    		PVUuid uuid(PVMF_SOCKET_NODE_EXTENSION_INTERFACE_UUID);
    		uuidvec->push_back(uuid);
    	}*/
    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::DoQueryInterface(PVMFSocketNodeCommand&  aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    return PVMFSuccess;
}

PVMFStatus PVMFClientServerSocketNode::DoRequestPort(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;
    aCmd.PVMFSocketNodeCommandBase::Parse(tag, portconfig);

    PVMFStatus status = AddPort(tag);
    if (status != PVMFSuccess)
        return status;

    //create the socket config
    iSockConfig = OSCL_NEW(SocketPortConfig, ());
    iPVMFPort->iConfig = iSockConfig;
    iSockConfig->iPVMFPort = iPVMFPort;

    iSockConfig->iContainer = this;
    iSockConfig->iSockId = iSocketID++;
    iSockConfig->iTag = tag;
    iSockConfig->iTCPSocket = iSocketHandle;

    //create the mem pool
    PVMFSocketNodeMemPool* memPool;
    status = AllocatePortMemPool(tag, memPool);
    if (status == PVMFSuccess)
        iSockConfig->iMemPool = memPool;
    return status;
}


//Release ports is a do-nothing for this node.
PVMFStatus PVMFClientServerSocketNode::DoReleasePort(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::DoInit(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    if (iInterfaceState != EPVMFNodeIdle)
    {
        return PVMFErrInvalidState;
    }
    return PVMFSuccess;
}


//Prepare is a do-nothing for this node.
PVMFStatus PVMFClientServerSocketNode::DoPrepare(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::DoPrepare() In"));

    if (iInterfaceState != EPVMFNodeInitialized)
    {
        return PVMFErrInvalidState;
    }
    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::DoStart(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    if (iInterfaceState == EPVMFNodeStarted)
    {
        return PVMFSuccess;//already started.
    }

    if (iInterfaceState != EPVMFNodePrepared &&
            iInterfaceState != EPVMFNodePaused)
    {
        return PVMFErrInvalidState;
    }

    TPVMFNodeInterfaceState curState = iInterfaceState;
    ChangeExternalState(EPVMFNodeStarted);

    PVMFStatus status = PVMFSuccess;
    if (iSockConfig)
    {
        //Start handling incoming messages
        if (CanProcessIncomingMsg())
            ProcessIncomingMsg();

        //Start the receives.
        if (CanReceive())
        {
            status = StartRecvOperation();
            if (status == PVMFPending)
            {
                status = PVMFSuccess;
            }
            else if (status != PVMFSuccess)
            {
                ChangeExternalState(curState);
            }
        }
    }
    return status;
}



PVMFStatus PVMFClientServerSocketNode::DoStop(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::DoStop() In"));

    if ((iInterfaceState != EPVMFNodeStarted) && (iInterfaceState != EPVMFNodePaused))
    {
        return PVMFErrInvalidState;
    }
    return PVMFSuccess;
}


PVMFStatus PVMFClientServerSocketNode::DoCancelCommand(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::DoCancelCommand() Called"));

    // extract the command ID from the parameters.
    PVMFCommandId cmdId;
    aCmd.PVMFSocketNodeCommandBase::Parse(cmdId);

    if (!iCurrentCmdQueue.empty()
            && iCurrentCmdQueue.front().iId == cmdId)
    {
        return DoCancelCurrentCommand(iCurrentCmdQueue, iCurrentCmdQueue.front());
        //wait on current command to complete.  The cancel command
        //will ultimately be completed in the "CommandComplete" for the current command.
    }

    {
        // start at element 1 since this cancel command is element 0
        PVMFSocketNodeCommand* cmd = iPendingCmdQueue.FindById(cmdId, 1);
        if (cmd)
        {
            // cancel the queued command.  Note this will complete out-of-order.
            CommandComplete(iPendingCmdQueue, *cmd, PVMFErrCancelled);
            // no further action is required.
            return PVMFSuccess;
        }
    }

    //this command fails if the given command is not queued or in progress.
    return PVMFErrArgument;
}

PVMFStatus PVMFClientServerSocketNode::DoCancelAllCommands(PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    PVMFStatus status = DoStopNodeActivity();

    if (!iCurrentCmdQueue.empty())
        CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFErrCancelled);

    //Cancel all other pending commands, except for this one which is
    //element 0 in the queue
    for (uint32 i = 1;i < iPendingCmdQueue.size();i++)
        CommandComplete(iPendingCmdQueue, iPendingCmdQueue[i], PVMFErrCancelled);

    //May need to wait on completion of StopNodeActivity.
    return status;
}


PVMFStatus PVMFClientServerSocketNode::DoCancelCurrentCommand(PVMFSocketNodeCmdQ& aCmdQ, PVMFSocketNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmdQ);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::DoCancelCurrentCommand()"));
    switch (aCmd.iCmd)
    {

        case PVMF_GENERIC_NODE_REQUESTPORT:
            //there may be a connect operation pending-- cancel it.
        {
            if (iPVMFPort)
            {
                //SocketPortConfig* iSockConfig=iRequestedPort->iConfig;
                if (iSockConfig)
                {
                    //	CancelConnectOperation();
                    return PVMFPending;//wait on the operation to complete
                    //in HandleSocketEvent or HandleDNSEvent
                }
            }
            //shouldn't get here...
            return PVMFFailure;
        }
        break;

        case PVMF_GENERIC_NODE_RESET:
            //it's too complicated to cancel a reset, so just wait on completion
            return PVMFPending;

        case PVMF_GENERIC_NODE_FLUSH:
            //to cancel a flush, just discard all remaining port messages
            //and keep waiting on completion of current message.
        {
            iPVMFPort->ClearMsgQueues();
        }
        return PVMFPending;//keep waiting on flush completion in SequenceComplete.

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            //these have asynchronous completion, but the command handling prevents
            //processing a cancel during a cancel, so we shouldn't get here.
            OSCL_ASSERT(0);
            return PVMFFailure;

        default:
            //no other node commands have asynchronous completion, so this is unexpected.
            OSCL_ASSERT(0);
            return PVMFFailure;
    }
}

PVMFStatus PVMFClientServerSocketNode::CancelRecvOperation()
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::CancelRecvOperation() In"));

    PVMFStatus status = PVMFSuccess;

    switch (iSockConfig->iState.iRecvOperation)
    {
        case EPVSocketPortRecvOperation_None:
            break;

        case EPVSocketPortRecvOperation_Recv:
            if (iSockConfig->iTCPSocket)
            {
                if (!iSockConfig->iState.iRecvOperationCanceled)
                {
                    iSockConfig->iState.iRecvOperationCanceled = true;
                    iSockConfig->iTCPSocket->CancelRecv();
                }
                status = PVMFPending;
                //wait on recv to complete in HandleSocketEvent
            }
            break;

        case EPVSocketPortRecvOperation_WaitOnConnectedPort:
            //just clear the state
            iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_None;
            //also discard the received data and the associated recv activity
            if (iSockConfig->iSocketRecvActivity.iValid)
                iSockConfig->iSocketRecvActivity.iValid = false;
            if (iSockConfig->iPendingRecvMediaData.GetRep())
                iSockConfig->iPendingRecvMediaData.Unbind();
            break;

        case EPVSocketPortRecvOperation_WaitOnMemory:
            if (iSockConfig->iMemPool != NULL)
                iSockConfig->iMemPool->CancelFreeChunkAvailableCallback();
            //clear the state
            iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_None;
            break;

        default:
            OSCL_ASSERT(0);//add code for this case
            status = PVMFFailure;
            break;
    }
    return status;
}

//a memory pool callback
void SocketPortConfig::freechunkavailable(OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(aContextData);
    //complete the "wait on memory" state
    if (iState.iRecvOperation == EPVSocketPortRecvOperation_WaitOnMemory)
    {
        iContainer->RecvOperationComplete(PVMFSuccess, NULL);	//*this,
    }
}


void PVMFClientServerSocketNode::CleanupTCP()
{
    //cleanup media messages
    if (iSockConfig->iPendingRecvMediaData.GetRep() != NULL)
    {
        iSockConfig->iPendingRecvMediaData.Unbind();

    }
    if (iSockConfig->iPendingSendMediaData.GetRep() != NULL)
    {
        iSockConfig->iPendingSendMediaData.Unbind();
    }
}


void PVMFClientServerSocketNode::CleanupPorts()
{
    if (iPVMFPort)
    {
        SocketPortConfig* it = iPVMFPort->iConfig;
        if (it)
        {
            //unlink the PVMFPort so we won't try to send any EOS msg
            //during the TCP cleanup.
            it->iPVMFPort = NULL;
            it->CleanupMemPools();
            OSCL_DELETE(it);
            iPVMFPort->iConfig = NULL;
        }

        iPVMFPort->Disconnect();
        delete iPVMFPort;
        iPVMFPort = NULL;
    }
}


PVMFSocketNodeMemPool::PVMFSocketNodeMemPool(uint32 aMemPoolNumBufs)
        : iMediaDataMemPool(NULL)
{
    iMediaDataMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (aMemPoolNumBufs, MEDIA_DATA_CLASS_SIZE));
    iInternalAlloc = NULL;
}


void SocketPortConfig::DoSetSocketPortMemAllocator(PVLogger* aLogger, OsclSharedPtr<PVMFSharedSocketDataBufferAlloc> aAlloc)
{
    if (aAlloc.GetRep())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, aLogger, PVLOGMSG_INFO, (0, "SocketPortConfig::DoSetSocketPortMemAllocator() Using input mem allocator"));
        /*
         * Deletion of any previously created allocators is handled as part of
         * reset / node delete. So just re-assign the shared ptr here.
         */
        if (iMemPool->iSocketAllocSharedPtr.GetRep() != NULL)
        {
            iMemPool->iSocketAllocSharedPtr.Unbind();
        }
        iMemPool->iSocketAllocSharedPtr = aAlloc;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, aLogger, PVLOGMSG_INFO, (0, "SocketPortConfig::DoSetSocketPortMemAllocator() no mem allocator. Create one"));

        uint aligned_socket_alloc_size = oscl_mem_aligned_size(sizeof(PVMFSMSharedBufferAllocWithReSize));

        uint aligned_refcnt_size = 	oscl_mem_aligned_size(sizeof(OsclRefCounterSA<PVMFSharedSocketDataBufferAllocCleanupSA>));

        OsclMemAllocator my_alloc;
        uint8 *my_ptr = (uint8*) my_alloc.ALLOCATE(aligned_refcnt_size + aligned_socket_alloc_size);
        OsclRefCounter *my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA<PVMFSharedSocketDataBufferAllocCleanupSA>(my_ptr));
        my_ptr += aligned_refcnt_size;

        // allow one resize for a maximum mempool size of TCP_BUFFER_SIZE*TCP_BUFFER_IN_MEMPOOL
        iMemPool->iInternalAlloc = OSCL_NEW(PVMFSMSharedBufferAllocWithReSize, (
                                                iContainer->iMaxTcpRecvBufferSize * (iContainer->iMaxTcpRecvBufferCount - 1), "TCPsocketBuffer",
                                                1, iContainer->iMaxTcpRecvBufferSize));

        PVMFSharedSocketDataBufferAlloc *alloc_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFSharedSocketDataBufferAlloc(iMemPool->iInternalAlloc));

        OsclSharedPtr<PVMFSharedSocketDataBufferAlloc> shared_alloc(alloc_ptr, my_refcnt);
        iMemPool->iSocketAllocSharedPtr = shared_alloc;
    }
}

void SocketPortConfig::CleanupMemPools()
{
    if (iMemPool->iInternalAlloc != NULL)
    {
        iMemPool->iInternalAlloc->DecrementKeepAliveCount();
        uint32 numOutStandingBuffers =
            iMemPool->iInternalAlloc->getNumOutStandingBuffers();
        if (numOutStandingBuffers == 0)
        {
            OSCL_DELETE((iMemPool->iInternalAlloc));
            iMemPool->iInternalAlloc = NULL;
        }
    }
    PVMFSocketNodeAllocator alloc;
    iMemPool->~PVMFSocketNodeMemPool();
    alloc.deallocate((OsclAny*)(iMemPool));
    iMemPool = NULL;
}


void PVMFClientServerSocketNode::ReportErrorEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg =
            OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        PVMFAsyncEvent asyncevent(PVMFErrorEvent,
                                  aEventType,
                                  NULL,
                                  OSCL_STATIC_CAST(PVInterface*, eventmsg),
                                  aEventData,
                                  NULL,
                                  0);
        PVMFNodeInterface::ReportErrorEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
    {
        PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
    }
}

void PVMFClientServerSocketNode::ReportInfoEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg =
            OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        PVMFAsyncEvent asyncevent(PVMFInfoEvent,
                                  aEventType,
                                  NULL,
                                  OSCL_STATIC_CAST(PVInterface*, eventmsg),
                                  aEventData,
                                  NULL,
                                  0);
        PVMFNodeInterface::ReportInfoEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
    {
        PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
    }
}

OSCL_EXPORT_REF bool PVMFClientServerSocketNode::setSocketPortMemAllocator(PVMFPortInterface* aInPort,
        OsclSharedPtr<PVMFSharedSocketDataBufferAlloc> aAlloc)
{

    PVMFClientServerSocketPort* aPort = OSCL_STATIC_CAST(PVMFClientServerSocketPort*, aInPort);
    SocketPortConfig* tmpSockConfig = (aPort) ? aPort->iConfig : NULL;
    if (NULL != tmpSockConfig)
    {
        tmpSockConfig->DoSetSocketPortMemAllocator(iLogger, aAlloc);
        return true;
    }
    return false;
}


bool PVMFClientServerSocketNode::CanProcessIncomingMsg()
{
    return
        //node is started
        iInterfaceState == EPVMFNodeStarted
        //port has input messages
        && iSockConfig->iPVMFPort && iSockConfig->iPVMFPort->IncomingMsgQueueSize() > 0
        //port is not busy with any sequence
        && iSockConfig->iState.iSequence == EPVSocketPortSequence_None;
}


void PVMFClientServerSocketNode::ProcessIncomingMsg()
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::ProcessIncomingMsg: aPort=0x%x", iSockConfig->iPVMFPort));

    //Dequeue the incoming message
    PVMFSharedMediaMsgPtr msg;
    PVMFStatus status = iSockConfig->iPVMFPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::ProcessIncomingDataMsg: Error - DequeueIncomingMsg failed"));
        ReportErrorEvent(PVMFErrPortProcessing);
        return ;
    }

    OSCL_ASSERT(iSockConfig->iState.iSequence == EPVSocketPortSequence_None);

    //Handle 3 types of messages
    if (msg->getFormatID() == PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID)
    {
        //Disconnect message
        StartSequence(EPVSocketPortSequence_InputDisconnectMsg);
    }
    else if (msg->getFormatID() == PVMF_MEDIA_MSG_DATA_FORMAT_ID)
    {
        //Start the sequence
        StartSequence(EPVSocketPortSequence_InputDataMsg, (OsclAny*)&msg);
    }
    else
    {
        //unexpected message type
        ReportErrorEvent(PVMFErrPortProcessing);
    }
}


PVMFStatus PVMFClientServerSocketNode::StartSendOperation(PVMFSharedMediaMsgPtr& aMsg)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::StartSendOperation() In"));

    //caller should have verified we can send now.
    OSCL_ASSERT(iSockConfig->iState.iSendOperation == EPVSocketPortSendOperation_None);

    //caller should provide a media data message as input
    OSCL_ASSERT(aMsg->getFormatID() == PVMF_MEDIA_MSG_DATA_FORMAT_ID);

    //there should be either a UDP or TCP socket on this port
    if (!iSockConfig->iTCPSocket)
    {
        return PVMFFailure;
    }

    // Retrieve memory fragment to write to
    if (iSockConfig->iPendingSendMediaData.GetRep())
        iSockConfig->iPendingSendMediaData.Unbind();

    convertToPVMFMediaData(iSockConfig->iPendingSendMediaData, aMsg);
    OsclRefCounterMemFrag refCtrMemFragOut;
    iSockConfig->iPendingSendMediaData->getMediaFragment(0, refCtrMemFragOut);

    PVMFStatus status = PVMFFailure;

    if (iSockConfig->iTCPSocket)
    {
        iSockConfig->iState.iSendOperation = EPVSocketPortSendOperation_Send;

        TPVSocketEvent retVal = iSockConfig->iTCPSocket->Send((uint8*)refCtrMemFragOut.getMemFragPtr(),
                                refCtrMemFragOut.getMemFragSize(),
                                TIMEOUT_SEND);

        if (retVal == EPVSocketPending)
            status = PVMFPending;//wait on HandleSocketEvent callback
        else
            status = PVMFFailure;

        if (PVMFPending != status)
        {
            PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::StartSendOperation: TCP - Error. status=%d", status));
        }
    }

    //Handle synchronous completion or failures
    if (status != PVMFPending)
        status = SendOperationComplete(status, NULL);

    return status;
}


PVMFStatus PVMFClientServerSocketNode::SendOperationComplete(PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::SendOperationComplete()"));

    OSCL_ASSERT(aStatus != PVMFPending);

    PVMFStatus status = aStatus;

    //Update the send state
    TPVSocketPortSendOperation curOp = iSockConfig->iState.iSendOperation;
    iSockConfig->iState.iSendOperation = EPVSocketPortSendOperation_None;
    iSockConfig->iState.iSendOperationStatus = aStatus;
    iSockConfig->iState.iSendOperationCanceled = false;

    //Release the media buffer after each send operation, regardless of success/fail.
    if (iSockConfig->iPendingSendMediaData.GetRep() != NULL)
        iSockConfig->iPendingSendMediaData.Unbind();

    if (aSocketActivity)
    {
        //Datapath logging
        switch (aSocketActivity->iEvent)
        {
            case EPVSocketSuccess:
                //PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0,"PVMFClientServerSocketNode::SendOperationComplete - Success - SockId=%d, Mime=%s", iSockConfig->iSockId, iSockConfig->iMime.get_str()));
                break;
            case EPVSocketTimeout:
                //PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0,"PVMFClientServerSocketNode::SendOperationComplete - TimeOut - SockId=%d, Mime=%s", iSockConfig->iSockId, iSockConfig->iMime.get_str()));
                break;
            case EPVSocketFailure:
                //PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0,"PVMFClientServerSocketNode::SendOperationComplete - Failed - SockId=%d, Mime=%s", iSockConfig->iSockId, iSockConfig->iMime.get_str()));
                break;
            case EPVSocketCancel:
                //PVMF_SOCKETNODE_LOGDATATRAFFIC_E((0,"PVMFClientServerSocketNode::SendOperationComplete - Cancelled - SockId=%d, Mime=%s", iSockConfig->iSockId, iSockConfig->iMime.get_str()));
                break;
            default:
                OSCL_ASSERT(0);
                break;
        }
    }

    //report TCP errors.
    if (aStatus != PVMFSuccess
            && curOp == EPVSocketPortSendOperation_Send)
    {
        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorTCPSocketSendError);
    }

    //This completes an input data message sequence
    if (iSockConfig->iState.iSequence == EPVSocketPortSequence_InputDataMsg)
        SequenceComplete(aStatus);
    return status;
}


bool PVMFClientServerSocketNode::CanReceive()
{
    return
        //node is started
        iInterfaceState == EPVMFNodeStarted
        //port is connected
        && iSockConfig->iPVMFPort && iSockConfig->iPVMFPort->IsConnected()
        //socket exists (gets created during request port or connect sequence)
        && (iSockConfig->iTCPSocket)
        //port is not busy with any sequence other than sending data
        && (iSockConfig->iState.iSequence == EPVSocketPortSequence_None
            || iSockConfig->iState.iSequence == EPVSocketPortSequence_InputDataMsg)
        //port is not busy with any receive operation.
        && iSockConfig->iState.iRecvOperation == EPVSocketPortRecvOperation_None
        //there's no node stop going on.
        && iNumStopPortActivityPending < 0;
}

//Enter the "wait on memory" state
void PVMFClientServerSocketNode::StartRecvWaitOnMemory(int32 aSize)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::StartRecvWaitOnMemory() In"));

    iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_WaitOnMemory;

    if (aSize)
    {//wait on data buffer
        iSockConfig->iMemPool->notifyfreechunkavailable(*iSockConfig, aSize, NULL);
    }
    else
    {//wait on media data wrapper
        iSockConfig->iMemPool->iMediaDataMemPool->notifyfreechunkavailable(*iSockConfig, NULL);
    }
}

//Enter the "wait on connected port" state
void PVMFClientServerSocketNode::StartRecvWaitOnConnectedPort(PVMFSocketActivity& aSocketActivity)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::StartRecvWaitOnConnectedPort() In"));

    //outgoing queue is busy-- must queue this event for later processing
    switch (aSocketActivity.iFxn)
    {
        case EPVSocketRecv:
        case EPVSocketRecvFrom:
            iSockConfig->iSocketRecvActivity.Set(aSocketActivity.iStatus
                                                 , aSocketActivity.iId
                                                 , aSocketActivity.iFxn
                                                 , aSocketActivity.iEvent
                                                 , aSocketActivity.iError);
            break;
        default:
            OSCL_ASSERT(false);//invalid input arg.
            break;
    }

    //current state shoudl be idle
    OSCL_ASSERT(iSockConfig->iState.iRecvOperation == EPVSocketPortRecvOperation_None);

    iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_WaitOnConnectedPort;
}


PVMFStatus PVMFClientServerSocketNode::StartRecvOperation()
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::StartRecvOperation() In"));

    //caller should have verified we can receive data right now.
    OSCL_ASSERT(iSockConfig->iState.iRecvOperation == EPVSocketPortRecvOperation_None);

    //there should be either a UDP or TCP socket on this port and a memory pool.
    if (!iSockConfig->iTCPSocket)
    {
        return PVMFFailure;
    }
    if (!iSockConfig->iMemPool)
    {
        return PVMFFailure;
    }

    PVMFStatus status = PVMFFailure;

    if (iSockConfig->iTCPSocket)
    {
        iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_Recv;

        //Allocate memory
        int32 err;
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
        OSCL_TRY(err,
                 mediaDataImpl = iSockConfig->iMemPool->getMediaDataImpl(iMaxTcpRecvBufferSize););
        if (err != OsclErrNone)
        {
            StartRecvWaitOnMemory(iMaxTcpRecvBufferSize);
            status = PVMFPending;
            //wait on memory pool callback "notifyfreechunkavailable"
        }

        else
        {
            OSCL_TRY(err, iSockConfig->iPendingRecvMediaData = PVMFMediaData::createMediaData(
                         mediaDataImpl, iSockConfig->iMemPool->iMediaDataMemPool););
            if (err != OsclErrNone)
            {
                StartRecvWaitOnMemory();
                status = PVMFPending;
                //wait on memory pool callback "notifyfreechunkavailable"
            }

            else
            {
                // Retrieve memory fragment to write to
                OsclRefCounterMemFrag refCtrMemFragOut;
                iSockConfig->iPendingRecvMediaData->getMediaFragment(0, refCtrMemFragOut);

                //Issue the Oscl socket request
                TPVSocketEvent retVal = iSockConfig->iTCPSocket->Recv((uint8*)refCtrMemFragOut.getMemFragPtr()
                                        , refCtrMemFragOut.getCapacity()
                                        , TIMEOUT_RECV);
                if (EPVSocketPending != retVal)
                {
                    status = PVMFFailure;
                }
                else
                {
                    status = PVMFPending;
                    //wait on HandleSocketEvent callback.
                }
            }
        }
    }
    else	//for (iTCPSocket==NULL)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::StartRecvOperation() Unexpected error, no socket"));
    }

    //Handle synchronous completion or failure.
    if (status != PVMFPending
            && status != PVMFFailure)
    {
        PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::StartRecvOperation: Error. status=%d", status));
    }

    if (status != PVMFPending)
        status = RecvOperationComplete(status, NULL);

    return status;
}

PVMFStatus PVMFClientServerSocketNode::RecvOperationComplete(PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity)
{
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::RecvOperationComplete() In"));

    //status should never be pending here
    OSCL_ASSERT(aStatus != PVMFPending);

    PVMFStatus status = aStatus;
    bool recvOperationCanceled = iSockConfig->iState.iRecvOperationCanceled;

    //check the condition of reset here
    if (iSockConfig->iState.iSequence == EPVSocketPortSequence_SocketCleanup
            || recvOperationCanceled)
    {
        return PVMFSuccess;
    }

    //Update the state
    TPVSocketPortRecvOperation curOp = iSockConfig->iState.iRecvOperation;
    iSockConfig->iState.iRecvOperation = EPVSocketPortRecvOperation_None;
    iSockConfig->iState.iRecvOperationStatus = aStatus;
    iSockConfig->iState.iRecvOperationCanceled = false;

    switch (curOp)
    {
        case EPVSocketPortRecvOperation_WaitOnMemory:
            //a memory wait is complete.
            //PVMF_SOCKETNODE_LOGDATATRAFFIC_I((0, "PVMFClientServerSocketNode::RecvOperationComplete WaitOnMemory - SockId=%d, Mime=%s ", iSockConfig->iSockId, iSockConfig->iMime.get_str()));
            break;

        case EPVSocketPortRecvOperation_WaitOnConnectedPort:
            //a port wait is complete
            if (iSockConfig->iSocketRecvActivity.iValid)
            {
                iSockConfig->iSocketRecvActivity.iValid = false;

                switch (iSockConfig->iSocketRecvActivity.iFxn)
                {
                    case EPVSocketRecv:
                        HandleRecvComplete(iSockConfig->iSocketRecvActivity.iStatus
                                           , &iSockConfig->iSocketRecvActivity, recvOperationCanceled);
                        break;
                    default:
                        OSCL_ASSERT(0);//invalid arg
                        break;
                }
            }
            break;

        case EPVSocketPortRecvOperation_Recv:
            HandleRecvComplete(aStatus, aSocketActivity, recvOperationCanceled);
            break;

        default:
            OSCL_ASSERT(0);//add code for this case
            break;
    }

    if (CanReceive())
        status = StartRecvOperation();

    return status;
}


void PVMFClientServerSocketNode::HandleRecvComplete(PVMFStatus aStatus, PVMFSocketActivity* aSocketActivity, bool aRecvOperationCanceled)
{
    OSCL_UNUSED_ARG(aRecvOperationCanceled);
    PVMF_SOCKETNODE_LOGSTACKTRACE((0, "PVMFClientServerSocketNode::HandleRecvComplete() In"));

    //operation should be complete when this is called.
    OSCL_ASSERT(aStatus != PVMFPending);

    //If there's no socket activity input, then this must be a failure in initiating
    //a Recv operation.
    if (!aSocketActivity || !iSockConfig->iPVMFPort
            || !iSockConfig->iPVMFPort->IsConnected())
    {
        OSCL_ASSERT(aStatus != PVMFSuccess);
        ReportSocketNodeError(PVMFErrResource, PVMFSocketNodeErrorSocketFailure);
        //release media data
        if (iSockConfig->iPendingRecvMediaData.GetRep())
            iSockConfig->iPendingRecvMediaData.Unbind();
        return;
    }

    if (aSocketActivity->iEvent == EPVSocketSuccess)
    {
        if (iSockConfig->iPVMFPort->IsOutgoingQueueBusy())
        {
            //wait on port so we can send recv data.
            StartRecvWaitOnConnectedPort(*aSocketActivity);
            return;
        }
    }

    //If we get here then it's time to process the recv result.

    //Release media data on failure
    if (aStatus != PVMFSuccess)
    {
        if (iSockConfig->iPendingRecvMediaData.GetRep())
            iSockConfig->iPendingRecvMediaData.Unbind();
    }

    switch (aSocketActivity->iEvent)
    {
        case EPVSocketSuccess:
        {
            //Get data length and set media buffer size
            int32 len;
            iSockConfig->iTCPSocket->GetRecvData(&len);
            if (len <= 0)
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::HandleRecvComplete - Sucessful Recv With Zero Length"));
                OSCL_ASSERT(false);
            }
            iSockConfig->iPendingRecvMediaData->setMediaFragFilledLen(0, len);

            // Resize the buffer
            if (iSockConfig->iMemPool)
            {
                OsclSharedPtr<PVMFMediaDataImpl> mediaMsgImpl;
                iSockConfig->iPendingRecvMediaData->getMediaDataImpl(mediaMsgImpl);
                iSockConfig->iMemPool->resizeSocketDataBuffer(mediaMsgImpl);
            }
            else
            {
                PVMF_SOCKETNODE_LOGERROR((0, "PVMFClientServerSocketNode::HandleRecvComplete() ERROR:mempool not found"));
                OSCL_ASSERT(0);
                return ;
            }

            //queue to next port
            PVMFSharedMediaMsgPtr aMediaMsgPtr;
            convertToPVMFMediaMsg(aMediaMsgPtr, iSockConfig->iPendingRecvMediaData);

            PVMFStatus status = iSockConfig->iPVMFPort->QueueOutgoingMsg(aMediaMsgPtr);
            if (status != PVMFSuccess)
            {
                ReportErrorEvent(PVMFErrPortProcessing);
                return ;
            }
        }
        break;

        case EPVSocketTimeout:
        {
            ReportSocketNodeError(PVMFErrTimeout, PVMFSocketNodeErrorSocketTimeOut);
        }
        break;

        case EPVSocketCancel:
            break;

        case EPVSocketFailure:
        {
            //After a receive failure, we may need to do a TCP shutdown.
            //Check what else is currently happening on the port.
            switch (iSockConfig->iState.iSequence)
            {
                case EPVSocketPortSequence_RequestPort:
                case EPVSocketPortSequence_InputDisconnectMsg:
                case EPVSocketPortSequence_SocketCleanup:
                    break;

                case EPVSocketPortSequence_None:
                case EPVSocketPortSequence_InputDataMsg:
                    //for these cases, start a shutdown sequence
                    //start the sequence
                    iSockConfig->iState.iSequence = EPVSocketPortSequence_SocketCleanup;
                    StartSequence(EPVSocketPortSequence_SocketCleanup);
                    break;

                default:
                    //need code to handle this case.
                    OSCL_ASSERT(0);
                    break;
            }
        }
        break;

        default:
            OSCL_ASSERT(0);
            break;
    }
}



OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::SetMaxTCPRecvBufferSize(uint32 aBufferSize)
{
    if ((aBufferSize > 0) && (aBufferSize < SNODE_DEFAULT_MAX_TCP_RECV_BUFFER_SIZE))
    {
        iMaxTcpRecvBufferSize = aBufferSize;
        return PVMFSuccess;
    }
    return PVMFErrArgument;
}

OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::GetMaxTCPRecvBufferSize(uint32& aSize)
{
    aSize = iMaxTcpRecvBufferSize;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::SetMaxTCPRecvBufferCount(uint32 aBufferSize)
{
    iMaxTcpRecvBufferCount = aBufferSize;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVMFClientServerSocketNode::GetMaxTCPRecvBufferCount(uint32& aSize)
{
    aSize = iMaxTcpRecvBufferCount;
    return PVMFSuccess;
}

PVMFStatus PVMFClientServerSocketNode::StartSequence(TPVSocketPortSequence aSequence, OsclAny* aParam)
{
    PVMFStatus status = PVMFFailure;
    //Set the sequence.
    iSockConfig->iState.iSequence = aSequence;

    switch (aSequence)
    {
        case EPVSocketPortSequence_InputDataMsg:
        {
            OSCL_ASSERT(aParam);
            return StartSendOperation(*((PVMFSharedMediaMsgPtr*)aParam));
        }

        case EPVSocketPortSequence_InputDisconnectMsg:
        {
            status = CloseSocketConnection();
        }
        break;

        case EPVSocketPortSequence_SocketCleanup:
            if (iSockConfig->iTCPSocket)
            {
                //CancelRecvOperation();
                //CancelSendOperation();
                status = CloseSocketConnection();
            }
            break;

        default:
        {
            OSCL_ASSERT(0);
            return PVMFFailure;
        }
    }

    if (status != PVMFPending)
    {
        //nothing needed.
        SequenceComplete(PVMFSuccess);
        return PVMFSuccess;
    }
    return status;
}



PVMFStatus PVMFClientServerSocketNode::CloseSocketConnection()
{
    PVMFStatus status;
    iSockConfig->iState.iSequence = EPVSocketPortSequence_SocketCleanup;
    if (!iSockConfig->iTCPSocket)
    {
        status = PVMFFailure;//unexpected
        return status;
    }
    //Initiate a socket shutdown.
    TPVSocketEvent ret = iSockConfig->iTCPSocket->Shutdown(EPVSocketBothShutdown, TIMEOUT_SHUTDOWN);
    if (ret == EPVSocketPending)
    {
        status = PVMFPending;
        //wait on the socket shutdown to complete in HandleSocketEvent callback
    }
    else
    {
        status = PVMFFailure;
    }
    return status;
}


void PVMFClientServerSocketNode::SequenceComplete(PVMFStatus aStatus)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFClientServerSocketNode::SequenceComplete() Sequence %d Status %d ", iSockConfig->iState.iSequence, aStatus));

    TPVSocketPortSequence curSequence = iSockConfig->iState.iSequence;
    iSockConfig->iState.iSequence = EPVSocketPortSequence_None;
    iSockConfig->iState.iSequenceStatus = aStatus;

    switch (curSequence)
    {
        case EPVSocketPortSequence_RequestPort:
            //may need to complete the node command
            if (iCurrentCmdQueue.size()
                    && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_REQUESTPORT)
            {
                CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), aStatus, iPVMFPort);
            }
            break;

        case EPVSocketPortSequence_InputDisconnectMsg:
        case EPVSocketPortSequence_InputDataMsg:
            //If we've just completed an input port message, this may complete a node Flush command
            if (!iCurrentCmdQueue.empty()
                    && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_FLUSH)
            {
                //Flush is done when all input ports are empty.
                if (iPVMFPort->IncomingMsgQueueSize() > 0)
                    return;//keep waiting
                CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFSuccess);
            }
            break;


        case EPVSocketPortSequence_SocketCleanup:
        {
            iPVMFPort->SuspendInput();
            iPVMFPort->ClearMsgQueues();

            CleanupPorts();
            //This port is done-- decrement the counter
            iNumStopPortActivityPending--;

            //When counter reaches zero, all ports are done and the sequence is complete.
            if (iNumStopPortActivityPending == 0)
            {
                //Reset the counter to the "idle" value.
                iNumStopPortActivityPending = (-1);

                //There may be a Reset or CancelAll command waiting on
                //this to complete.
                if (!iCurrentCmdQueue.empty()
                        && iCurrentCmdQueue.front().iCmd == PVMF_GENERIC_NODE_RESET)
                {
                    CommandComplete(iCurrentCmdQueue, iCurrentCmdQueue.front(), PVMFSuccess);
                }
                else if (!iCancelCmdQueue.empty()
                         && iCancelCmdQueue.front().iCmd == PVMF_GENERIC_NODE_CANCELALLCOMMANDS)
                {
                    CommandComplete(iCancelCmdQueue, iCancelCmdQueue.front(), PVMFSuccess);
                }
                return;
            }
        }
        break;

        default:
            break;
    }

    if (curSequence != EPVSocketPortSequence_InputDataMsg
            && CanReceive())
    {
        StartRecvOperation();
    }

    if (CanProcessIncomingMsg())
        ProcessIncomingMsg();
}
