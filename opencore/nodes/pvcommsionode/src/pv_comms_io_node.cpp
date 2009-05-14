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
/**
 * @file pv_comms_io_node.cpp
 * @brief
 */

#include "oscl_base.h"
#include "pv_comms_io_node_factory.h"
#include "pv_comms_io_node.h"
#include "pv_comms_io_node_port.h"
#include "oscl_dll.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pv_comms_io_node_events.h"

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

/**
//Macros for calling PVLogger
*/
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVCommsIONodeFactory::Create(PvmiMIOControl* aMIOInputControl, PvmiMIOControl *aMIOOutputControl, bool logBitstream)
{
    return PVCommsIONode::Create(aMIOInputControl, aMIOOutputControl, logBitstream);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVCommsIONodeFactory::Create(PvmiMIOControl *aMIOControl, bool logBitstream)
{
    return PVCommsIONode::Create(aMIOControl, logBitstream);
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONodeFactory::Delete(PVMFNodeInterface* aNode)
{
    PVCommsIONode::Release(aNode);
}

////////////////////////////////////////////////////////////////////////////
PVMFNodeInterface* PVCommsIONode::Create(PvmiMIOControl* aMIOInputControl, PvmiMIOControl *aMIOOutputControl, bool logBitstream)
{
    PVCommsIONode* node = OSCL_NEW(PVCommsIONode, (logBitstream));
    if (node)
    {
        OSCL_TRAPSTACK_PUSH(node);
        node->ConstructL(aMIOInputControl, aMIOOutputControl);
        OSCL_TRAPSTACK_POP();
    }
    return (PVMFNodeInterface*)node;
}

////////////////////////////////////////////////////////////////////////////
PVMFNodeInterface* PVCommsIONode::Create(PvmiMIOControl* aMIOControl, bool logBitstream)
{
    PVCommsIONode* node = OSCL_NEW(PVCommsIONode, (logBitstream));
    if (node)
    {
        OSCL_TRAPSTACK_PUSH(node);
        node->ConstructL(aMIOControl);
        OSCL_TRAPSTACK_POP();
    }
    return (PVMFNodeInterface*)node;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::Release(PVMFNodeInterface* aNode)
{
    OSCL_DELETE(((PVCommsIONode*)aNode));
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONode::ThreadLogon()
{
    if (iInterfaceState != EPVMFNodeCreated)
        return PVMFErrInvalidState;

    iLogger = PVLogger::GetLoggerObject("PVCommsIONode");

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::ThreadLogon"));

    if (!IsAdded())
        AddToScheduler();

    iMediaInputContext.ThreadLogon();
    if (!(iMediaOutputContext == iMediaInputContext))
        iMediaOutputContext.ThreadLogon();

    SetState(EPVMFNodeIdle);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::ThreadLogoff"));

    if (iInterfaceState != EPVMFNodeIdle)
        return PVMFErrInvalidState;

    if (IsAdded())
        RemoveFromScheduler();

    iLogger = NULL;

    iMediaInputContext.ThreadLogoff();
    if (!(iMediaOutputContext == iMediaInputContext))
        iMediaOutputContext.ThreadLogoff();

    SetState(EPVMFNodeCreated);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVCommsIONode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    if (iInterfaceState == EPVMFNodeIdle ||
            iInterfaceState == EPVMFNodeCreated)
        return PVMFFailure;

    if ((iMediaInputContext.isValid() && !iMediaInputContext.hasConfig()) ||
            (iMediaOutputContext.isValid() && !iMediaOutputContext.hasConfig()))
        return PVMFFailure;

    aNodeCapability.iCanSupportMultipleInputPorts = false;
    aNodeCapability.iCanSupportMultipleOutputPorts = false;
    aNodeCapability.iHasMaxNumberOfPorts = true;
    if (iMediaOutputContext.isValid() && iMediaInputContext.isValid())
        aNodeCapability.iMaxNumberOfPorts = 2;
    else
        aNodeCapability.iMaxNumberOfPorts = 1;

    PvmiKvp* kvp ;
    int numParams ;
    int32 err ;
    int32 i ;
    PVMFStatus status;

    // Get input formats capability from media IO
    kvp = NULL;
    numParams = 0;

    PvmiCapabilityAndConfig *inputConfig, *outputConfig;

    inputConfig = iMediaInputContext.iMediaIOConfig;
    outputConfig = iMediaOutputContext.iMediaIOConfig;

    if (inputConfig)
    {
        status = inputConfig->getParametersSync(NULL,
                                                OSCL_CONST_CAST(char*, INPUT_FORMATS_CAP_QUERY),
                                                kvp, numParams, NULL);
        if (status == PVMFSuccess)
        {
            OSCL_TRY(err,
                     for (i = 0; i < numParams; i++)
                     aNodeCapability.iInputFormatCapability.push_back(kvp[i].value.pChar_value););
            if (kvp)
                inputConfig->releaseParameters(0, kvp, numParams);
        }
        //else ignore errors.
    }
    // Get output formats capability from media IO
    kvp = NULL;
    numParams = 0;

    if (outputConfig)
    {
        status = outputConfig->getParametersSync(NULL,
                 OSCL_CONST_CAST(char*, OUTPUT_FORMATS_CAP_QUERY),
                 kvp, numParams, NULL);
        if (status == PVMFSuccess)
        {
            OSCL_TRY(err,
                     for (i = 0; i < numParams; i++)
                     aNodeCapability.iOutputFormatCapability.push_back(kvp[i].value.pChar_value);
                    );
            if (kvp)
                outputConfig->releaseParameters(0, kvp, numParams);
        }
        //else ignore errors.
    }

    if (aNodeCapability.iInputFormatCapability.empty() && aNodeCapability.iOutputFormatCapability.empty())
        return PVMFFailure;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PVCommsIONode::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::QueryUUID() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::QueryInterface() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::RequestPort() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::ReleasePort() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Init() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Prepare() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Start() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Stop() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Flush() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Pause() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::Reset() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::CancelAllCommands() called"));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVCommsIONode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::CancelCommands() called cmdId=%d", aCmdId));
    PVCommsIONodeCmd cmd;
    cmd.PVCommsIONodeCmdBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONode::removeRef()
{
    if (iExtensionRefCount > 0)
        --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVCommsIONode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PvmfNodesSyncControlUuid)
    {
        PvmfNodesSyncControlInterface* myInterface = OSCL_STATIC_CAST(PvmfNodesSyncControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else
    {
        iface = NULL;
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONode::RequestCompleted(const PVMFCmdResp& aResponse)
//callback from the MIO module.
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::RequestCompleted: Cmd ID=%d", aResponse.GetCmdId()));

    //look for cancel completion.
    if (iMediaIOCancelPending
            && aResponse.GetCmdId() == iMediaIOCancelCmdId)
    {
        iMediaIOCancelPending = false;

        Assert(!iCancelCommand.empty());

        //Current cancel command is now complete.
        CommandComplete(iCancelCommand, iCancelCommand.front(), PVMFSuccess);
    }
    //look for non-cancel completion
    else if (iMediaIORequest != ENone
             && aResponse.GetCmdId() == iMediaIOCmdId)
    {
        // Kind of tricky here.  For each node request, ie Init()
        // up to 2 asynchronous MIO requests will be made.  We need
        // to intercept the first MIO response here and make sure to
        // send another one if necessary.  The actual node command
        // completion message won't be sent until we get the 2nd
        // MIO response.
        Assert(!iCurrentCommand.empty());
        PVCommsIONodeCmd& cmd = iCurrentCommand.front();

        switch (iMediaIORequest)
        {
            case EQueryCapability:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVCommsIONodeErr_MediaIOQueryCapConfigInterface;
                }
                else
                {
                    if ((cmd.iCmd == PVMF_GENERIC_NODE_INIT) && iTempCapConfigInterface)
                    {
                        cmd.iControlContext->iMediaIOConfig = OSCL_STATIC_CAST(PvmiCapabilityAndConfig *, iTempCapConfigInterface);
                        iTempCapConfigInterface = NULL;

                    }
                    else
                    {
                        LOGERROR((0, "PVCommsIONode:RequestComplete Error:Not setting interface iCmd=%d,iTempCapConfigInterface=%x",
                                  cmd.iCmd, iTempCapConfigInterface));


                    }
                    if (ReRunCommandForNextMIO(cmd))
                    {
                        return;
                    }
                    else	if (cmd.iControlContext == &iMediaInputContext &&
                             iMediaInputContext.iControl == iMediaOutputContext.iControl)
                    {
                        // if we have a single object that supports both input and
                        // output, the context's controls will be identical, and
                        // we don't need to query the interface again, so we'll just
                        // copy the session and config interface
                        // to the output context, since they will be the same
                        iMediaOutputContext = iMediaInputContext;
                    }
                }
                break;

            case EInit:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVCommsIONodeErr_MediaIOInit;
                }
                else if (ReRunCommandForNextMIO(cmd))
                {
                    return;
                }
                else
                {
                    iMediaIOState = MIO_STATE_INITIALIZED;
                }
                break;

            case EStart:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVCommsIONodeErr_MediaIOStart;
                }
                else if (ReRunCommandForNextMIO(cmd))
                {
                    return;
                }
                else
                {
                    iMediaIOState = MIO_STATE_STARTED;
                    //tell all the ports that I/O is started.
                    {
                        uint32 i;
                        for (i = 0; i < iPortVector.size(); i++)
                            iPortVector[i]->MediaIOStarted();
                    }
                }
                break;

            case EPause:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVCommsIONodeErr_MediaIOPause;
                }
                else if (ReRunCommandForNextMIO(cmd))
                {
                    return;
                }
                else
                {
                    iMediaIOState = MIO_STATE_PAUSED;
                }
                break;

            case EStop:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PVCommsIONodeErr_MediaIOStop;
                }
                else if (ReRunCommandForNextMIO(cmd))
                {
                    return;
                }
                else
                {
                    iMediaIOState = MIO_STATE_INITIALIZED;
                }
                break;

            default:
                Assert(false);
                break;
        }

        //current node command is now complete.
        if (IsFlushPending())
        {
            iMediaIORequest = ENone;
            FlushComplete();
        }
        else
        {
            CommandComplete(iCurrentCommand, iCurrentCommand.front(), aResponse.GetCmdStatus());
        }
    }
    else
    {
        //unexpected response.
        LOGERROR((0, "PVCommsIONode:RequestComplete Warning! Unexpected command ID %d"
                  , aResponse.GetCmdId()));
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONode::ReportErrorEvent(PVMFEventType aEventType, PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aEventType);
    OSCL_UNUSED_ARG(aExtMsg);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVCommsIONode::ReportInfoEvent(PVMFEventType aEventType, PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aEventType);
    OSCL_UNUSED_ARG(aExtMsg);
}

////////////////////////////////////////////////////////////////////////////
PVCommsIONode::PVCommsIONode(bool logBitstream)
        : OsclActiveObject(OsclActiveObject::EPriorityNominal, "PVCommsIONode")
        , iMediaIOState(PVCommsIONode::MIO_STATE_IDLE)
        , iEventUuid(PVCommsIONodeEventTypesUUID)
        , iExtensionRefCount(0)
        , iLogger(NULL)
        , iLogBitstream(logBitstream)
        , iTempCapConfigInterface(NULL)
{
}

////////////////////////////////////////////////////////////////////////////
PVCommsIONode::~PVCommsIONode()
{
    Cancel();
    if (IsAdded())
        RemoveFromScheduler();

    //if any MIO commands are outstanding, there will be
    //a crash when they callback-- so panic here instead.
    OSCL_ASSERT(!(!iCancelCommand.empty()
                  || iMediaIORequest != ENone));

    //Cleanup allocated ports
    while (!iPortVector.empty())
        iPortVector.Erase(&iPortVector.front());

    //Cleanup commands
    //The command queues are self-deleting, but we want to
    //notify the observer of unprocessed commands.
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
        iCurrentCommand.Erase(&iCurrentCommand.front());
    }
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
        iInputCommands.Erase(&iInputCommands.front());
    }
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::ConstructL(PvmiMIOControl* aMIOInputControl, PvmiMIOControl *aMIOOutputControl)
{
    if (((aMIOInputControl == NULL) && (aMIOOutputControl == NULL)) ||
            aMIOInputControl == aMIOOutputControl)
        OsclError::Leave(OsclErrArgument);
    iLogger = NULL;
    iMediaInputContext.iControl = aMIOInputControl;
    iMediaOutputContext.iControl = aMIOOutputControl;
    iPortActivity = 0;
    iInputCommands.Construct(1, 10);//reserve 10
    iCurrentCommand.Construct(1, 1);//reserve 1.
    iCancelCommand.Construct(1, 1);//reserve 1.
    iPortVector.Construct(0);//reserve zero.
    iMediaIORequest = ENone;
    iMediaIOCancelPending = false;
}

void PVCommsIONode::ConstructL(PvmiMIOControl* aMIOControl)
{
    if (aMIOControl == NULL)
        OsclError::Leave(OsclErrArgument);

    iLogger = NULL;
    iMediaInputContext.iControl = aMIOControl;
    iMediaOutputContext.iControl = aMIOControl;

    iPortActivity = 0;
    iInputCommands.Construct(1, 10);//reserve 10
    iCurrentCommand.Construct(1, 1);//reserve 1.
    iCancelCommand.Construct(1, 1);//reserve 1.
    iPortVector.Construct(0);//reserve zero.
    iMediaIORequest = ENone;
    iMediaIOCancelPending = false;
}


////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::Run()
{
    //Process async node commands.
    if (!iInputCommands.empty())
    {
        ProcessCommand();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVCommsIONode::QueueCommandL(PVCommsIONodeCmd& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::ProcessCommand()
{
    //Can't do anything when an asynchronous cancel is in progress-- just
    //need to wait on completion.
    if (!iCancelCommand.empty())
        return ; //keep waiting.

    //If a command is in progress, only hi pri cancel commands can interrupt it.
    if (!iCurrentCommand.empty()
            && !iInputCommands.front().hipri())
    {
        return ; //keep waiting
    }

    //The newest or highest pri command is in the front of the queue.
    Assert(!iInputCommands.empty());
    PVCommsIONodeCmd& aCmd = iInputCommands.front();

    PVMFStatus cmdstatus;
    OsclAny* aEventData = NULL;
    if (aCmd.hipri())
    {
        //save input command in cancel command
        int32 err;
        OSCL_TRY(err, iCancelCommand.StoreL(aCmd););
        if (err != OsclErrNone)
        {
            cmdstatus = PVMFErrNoMemory;
        }
        else
        {
            //Process the interrupt commands.
            switch (aCmd.iCmd)
            {
                case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
                    cmdstatus = DoCancelAllCommands(aCmd);
                    break;

                case PVMF_GENERIC_NODE_CANCELCOMMAND:
                    cmdstatus = DoCancelCommand(aCmd);
                    break;

                default:
                    Assert(false);
                    cmdstatus = PVMFFailure;
                    break;
            }
        }

        //erase the input command.
        if (cmdstatus != PVMFPending)
        {
            //Request already failed-- erase from Cancel Command.
            //Node command remains in Input Commands.
            iCancelCommand.Erase(&iCurrentCommand.front());
        }
        else
        {
            //Node command is now stored in Cancel Command, so erase from Input Commands.
            iInputCommands.Erase(&aCmd);
        }
    }
    else
    {
        //save input command in current command
        int32 err;
        OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
        if (err != OsclErrNone)
        {
            cmdstatus = PVMFErrNoMemory;
        }
        else
        {
            //Process the normal pri commands.
            switch (aCmd.iCmd)
            {
                case PVMF_GENERIC_NODE_QUERYUUID:
                    cmdstatus = DoQueryUuid(aCmd);
                    break;

                case PVMF_GENERIC_NODE_QUERYINTERFACE:
                    cmdstatus = DoQueryInterface(aCmd);
                    break;

                case PVMF_GENERIC_NODE_REQUESTPORT:
                    cmdstatus = DoRequestPort(aCmd, aEventData);
                    break;

                case PVMF_GENERIC_NODE_RELEASEPORT:
                    cmdstatus = DoReleasePort(aCmd);
                    break;

                case PVMF_GENERIC_NODE_INIT:
                    cmdstatus = DoInit(aCmd);
                    break;

                case PVMF_GENERIC_NODE_PREPARE:
                    cmdstatus = DoPrepare(aCmd);
                    break;

                case PVMF_GENERIC_NODE_START:
                    cmdstatus = DoStart(aCmd);
                    break;

                case PVMF_GENERIC_NODE_STOP:
                    cmdstatus = DoStop(aCmd);
                    break;

                case PVMF_GENERIC_NODE_FLUSH:
                    cmdstatus = DoFlush(aCmd);
                    break;

                case PVMF_GENERIC_NODE_PAUSE:
                    cmdstatus = DoPause(aCmd);
                    break;

                case PVMF_GENERIC_NODE_RESET:
                    cmdstatus = DoReset(aCmd);
                    break;

                default://unknown command type
                    Assert(false);
                    cmdstatus = PVMFFailure;
                    break;
            }
        }

        //erase the input command.
        if (cmdstatus != PVMFPending)
        {
            //Request already failed/succeeded-- erase from Current Command.
            //Node command remains in Input Commands.
            iCurrentCommand.Erase(&iCurrentCommand.front());
        }
        else
        {
            //Node command is now stored in Current Command, so erase from Input Commands.
            //The DoXXX() routines will modify the command object, so we
            //need to update the command object in the current command queue
            iCurrentCommand[iCurrentCommand.size()-1] = aCmd;
            iInputCommands.Erase(&aCmd);
        }
    }

    if (cmdstatus != PVMFPending)
        CommandComplete(iInputCommands, aCmd, cmdstatus, aEventData);

}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::CommandComplete(PVCommsIONodeCmdQ& aCmdQ, PVCommsIONodeCmd& aCmd, PVMFStatus aStatus, OsclAny*aEventData)
{
    if (aStatus == PVMFSuccess)
    {
        LOGINFO((0, "PVCommsIONode:CommandComplete Id %d Cmd %d Status %d Context %d EVData %d EVCode %d"
                 , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData, aCmd.iEventCode));
    }
    else
    {
        LOGERROR((0, "PVCommsIONode:CommandComplete Error! Id %d Cmd %d Status %d Context %d EVData %d EVCode %d"
                  , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData, aCmd.iEventCode));
    }

    //do state transitions and any final command completion.
    if (aStatus == PVMFSuccess)
    {
        switch (aCmd.iCmd)
        {
            case PVMF_GENERIC_NODE_INIT:
                SetState(EPVMFNodeInitialized);
                break;
            case PVMF_GENERIC_NODE_PREPARE:
                SetState(EPVMFNodePrepared);
                break;
            case PVMF_GENERIC_NODE_START:
                SetState(EPVMFNodeStarted);
                break;
            case PVMF_GENERIC_NODE_PAUSE:
                SetState(EPVMFNodePaused);
                break;
            case PVMF_GENERIC_NODE_STOP:
                SetState(EPVMFNodePrepared);
                //clear the message queues of any unprocessed data now.
                {
                    uint32 i;
                    for (i = 0; i < iPortVector.size(); i++)
                        iPortVector[i]->ClearMsgQueues();
                }
                break;
            case PVMF_GENERIC_NODE_FLUSH:
                SetState(EPVMFNodePrepared);
                //Stop the ports
                {
                    for (uint32 i = 0; i < iPortVector.size(); i++)
                        iPortVector[i]->Stop();
                }
                break;
            default:
                break;
        }
    }

    //Reset the media I/O request
    iMediaIORequest = ENone;


    //The "startmio" command originated from port activity,
    //not from the node controller, so no command complete event
    //is needed.
    if (aCmd.iCmd == PVMF_COMMSIONODE_STARTMIO)
    {
        //Erase the command from the queue.
        aCmdQ.Erase(&aCmd);
    }
    else
    {
        //Extract parameters needed for command response.
        PVMFCommandId cmdId = aCmd.iId;
        const OsclAny* cmdContext = aCmd.iContext;
        PVMFSessionId cmdSess = aCmd.iSession;
        PVMFStatus eventCode = aCmd.iEventCode;

        //Erase the command from the queue.
        aCmdQ.Erase(&aCmd);

        if (eventCode != PVCommsIONodeErr_First)
        {
            //create extended response.

            PVMFBasicErrorInfoMessage*eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (eventCode, iEventUuid, NULL));

            PVMFCmdResp resp(cmdId
                             , cmdContext
                             , aStatus
                             , OSCL_STATIC_CAST(PVInterface*, eventmsg)
                             , aEventData);

            //report to the session observers.
            PVMFNodeInterface::ReportCmdCompleteEvent(cmdSess, resp);

            //remove the ref to the extended response
            if (eventmsg)
                eventmsg->removeRef();
        }
        else
        {
            //create basic response
            PVMFCmdResp resp(cmdId, cmdContext, aStatus, aEventData);

            //report to the session observers.
            PVMFNodeInterface::ReportCmdCompleteEvent(cmdSess, resp);
        }
    }
    //re-schedule if there are more commands and node isn't logged off
    if (!iInputCommands.empty()
            && IsAdded())
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoQueryUuid(PVCommsIONodeCmd& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVCommsIONodeCmdBase::Parse(mimetype, uuidvec, exactmatch);

    uuidvec->push_back(PvmfNodesSyncControlUuid);

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoQueryInterface(PVCommsIONodeCmd& aCmd)
{
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVCommsIONodeCmdBase::Parse(uuid, ptr);
    if (uuid && ptr)
    {
        if (queryInterface(*uuid, *ptr))
            return PVMFSuccess;
    }
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoRequestPort(PVCommsIONodeCmd& aCmd, OsclAny*&aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::DoRequestPort"));
    //This node supports port request from any state

    //retrieve port tag & mimetype
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PVCommsIONodeCmdBase::Parse(tag, mimetype);

    switch (tag)
    {

        case PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG:
        case PVMF_COMMSIO_NODE_INPUT_PORT_TAG:
        case PVMF_COMMSIO_NODE_IO_PORT_TAG:
        {
            //Allocate a new port
            OsclAny *ptr = NULL;
            int32 err;
            if (!ValidatePortTagRequest(tag))
                return PVMFErrNotSupported;

            OSCL_TRY(err, ptr = iPortVector.Allocate(););
            if (err != OsclErrNone || !ptr)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVCommsIONode::DoRequestPort: Error - iPortVector Out of memory"));
                return PVMFErrNoMemory;
            }
            PVCommsIONodePort *port = OSCL_PLACEMENT_NEW(ptr, PVCommsIONodePort(tag, this));

            //set the format from the mimestring.
            if (mimetype)
            {
                PVMFFormatType fmt = mimetype->get_str();
                if (fmt != PVMF_MIME_FORMAT_UNKNOWN)
                    port->Configure(fmt);
            }

            //Add the port to the port vector.
            OSCL_TRY(err, iPortVector.AddL(port););
            if (err != OsclErrNone)
            {
                iPortVector.DestructAndDealloc(port);
                return PVMFErrNoMemory;
            }
            aEventData = (OsclAny*)port;

            //cast the port to the generic interface before returning.
            PVMFPortInterface*retval = port;
            aEventData = (OsclAny*)retval;
            return PVMFSuccess;
        }
        break;

        default:
        {
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVCommsIONode::DoRequestPort: Error - Invalid port tag"));
            return PVMFFailure;
        }
        break;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoReleasePort(PVCommsIONodeCmd& aCmd)
{
    //This node supports release port from any state

    PVMFPortInterface* p;
    aCmd.PVCommsIONodeCmdBase::Parse(p);
    //search the output port vector
    {
        PVCommsIONodePort* port = (PVCommsIONodePort*)p;
        PVCommsIONodePort** portPtr = iPortVector.FindByValue(port);
        if (portPtr)
        {
            (*portPtr)->Disconnect();
            iPortVector.Erase(portPtr);
            return PVMFSuccess;
        }
    }
    aCmd.iEventCode = PVCommsIONodeErr_PortNotExist;
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoInit(PVCommsIONodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    bool haveNewContext;

    if (iInterfaceState != EPVMFNodeIdle)
        return PVMFErrInvalidState;

    if (!(haveNewContext = GetNextContextInSequence(aCmd)) && aCmd.iControlContext == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVCommsIONode::DoInit: Error - All media controls are NULL"));
        aCmd.iEventCode = PVCommsIONodeErr_MediaIONotExist;
        return PVMFFailure;
    }
    else if (aCmd.iControlContext->iControl->connect(aCmd.iControlContext->iMediaSession, this) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVCommsIONode::DoInit: Error - iMediaIOControl->connect failed"));
        aCmd.iEventCode = PVCommsIONodeErr_MediaIOConnect;
        return PVMFFailure;
    }

    //Query for MIO capability and config interface
    return SendMioRequest(aCmd, EQueryCapability);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoPrepare(PVCommsIONodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::DoPrepare"));

    if (iInterfaceState != EPVMFNodeInitialized)
        return PVMFErrInvalidState;

    if (aCmd.iControlContext == NULL)
        GetNextContextInSequence(aCmd);
    return SendMioRequest(aCmd, EInit);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoStart(PVCommsIONodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::DoStart"));

    if (iInterfaceState != EPVMFNodePrepared
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Start the MIO
    if (aCmd.iControlContext == NULL)
        GetNextContextInSequence(aCmd);

    PVMFStatus status = SendMioRequest(aCmd, EStart);

    if (status != PVMFPending)
        return status;

    // don't start ports until last MIO start has been sent
    PVCommsIONodeCmd tempCmd = aCmd;
    if (!GetNextContextInSequence(tempCmd))
    {
        uint32 i = 0;
        for (i = 0; i < iPortVector.size(); i++)
            iPortVector[i]->Start();
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoStop(PVCommsIONodeCmd& aCmd)
{
    if (iInterfaceState != EPVMFNodeStarted
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Stop the MIO
    if (aCmd.iControlContext == NULL)
        GetNextContextInSequence(aCmd);

    PVMFStatus status = SendMioRequest(aCmd, EStop);

    //Stop the ports
    for (uint32 i = 0; i < iPortVector.size(); i++)
        iPortVector[i]->Stop();

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoFlush(PVCommsIONodeCmd& aCmd)
{
    if (iInterfaceState != EPVMFNodeStarted
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Disable the input.
    if (iPortVector.size() > 0)
    {
        //Stop the MIO input
        if (aCmd.iControlContext == NULL)
            GetNextContextInSequence(aCmd);

        SendMioRequest(aCmd, EStop);
        //ignore returned error.
    }
    //wait for all data to be consumed
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
bool PVCommsIONode::IsFlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::FlushComplete()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFVideoEncNode::FlushComplete"));

    if (iMediaIORequest != ENone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFVideoEncNode::FlushComplete: Media IO request is pending"));
        return;
    }

    if (!PortQueuesEmpty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFVideoEncNode::FlushComplete: Port queues are not empty"));
        return;
    }

    //Flush is complete.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PVMFVideoEncNode::FlushComplete: Done"));
    CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoPause(PVCommsIONodeCmd& aCmd)
{
    if (iInterfaceState != EPVMFNodeStarted)
        return PVMFErrInvalidState;

    if (aCmd.iControlContext == NULL)
        GetNextContextInSequence(aCmd);

    return SendMioRequest(aCmd, EPause);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoReset(PVCommsIONodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        case EPVMFNodeIdle:
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        {
            //delete all ports and notify observer.
            while (!iPortVector.empty())
                iPortVector.Erase(&iPortVector.front());

            //restore original port vector reserve.
            iPortVector.Reconstruct();

            //logoff & go back to Created state.
            SetState(EPVMFNodeIdle);

            return PVMFSuccess;
        }
        break;

        default:
            return PVMFErrInvalidState;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoCancelAllCommands(PVCommsIONodeCmd& aCmd)
{
    //first cancel any MIO commmand in progress.
    if (iMediaIORequest != ENone)
    {
        return CancelMioRequest(aCmd);
    }

    //first cancel the current command if any
    {
        while (!iCurrentCommand.empty())
            CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFErrCancelled);
    }

    //next cancel all queued commands
    {
        //start at element 1 since this cancel command is element 0.
        while (iInputCommands.size() > 1)
            CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
    }

    //finally, report cancel complete.
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::DoCancelCommand(PVCommsIONodeCmd& aCmd)
{
    //tbd, need to cancel the MIO

    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVCommsIONodeCmdBase::Parse(id);

    //first check "current" command if any
    {
        PVCommsIONodeCmd* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {
            //first cancel any MIO commmand in progress.
            if (iMediaIORequest != ENone)
            {
                return CancelMioRequest(aCmd);
            }
            else
            {
                //cancel the queued command
                CommandComplete(iCurrentCommand, *cmd, PVMFErrCancelled);
                //report cancel success
                return PVMFSuccess;
            }
        }
    }

    //next check input queue.
    {
        //start at element 1 since this cancel command is element 0.
        PVCommsIONodeCmd* cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
            //report cancel success
            return PVMFSuccess;
        }
    }
    //if we get here the command isn't queued so the cancel fails.
    aCmd.iEventCode = PVCommsIONodeErr_CmdNotQueued;
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::SendMioRequest(PVCommsIONodeCmd& aCmd, EMioRequest aRequest)
{
    //Make an asynchronous request to MIO component
    //and save the request in iMediaIORequest.


    //there should not be a MIO command in progress..
    Assert(iMediaIORequest == ENone);

    //save media io request.
    iMediaIORequest = aRequest;

    PVMFStatus status;

    //Issue the command to the MIO.
    switch (aRequest)
    {
        case EQueryCapability:
        {
            int32 err ;

            OSCL_TRY(err,
                     iMediaIOCmdId = aCmd.iControlContext->iControl->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                                     iTempCapConfigInterface , NULL);
                    );
            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVCommsIONode::DoInit: Error - iMediaIOControl->QueryInterface failed"));
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOQueryCapConfigInterface;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EInit:
        {
            if (iMediaIOState != MIO_STATE_IDLE)
            {
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = aCmd.iControlContext->iControl->Init(););

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVCommsIONode::DoInit: Error - iMediaIOControl->Init failed"));
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOInit;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EStart:
        {
            if (iMediaIOState != MIO_STATE_INITIALIZED
                    && iMediaIOState != MIO_STATE_PAUSED)
            {
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = aCmd.iControlContext->iControl->Start(););

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVCommsIONode::DoInit: Error - iMediaIOControl->Start failed"));
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOStart;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EPause:
        {
            if (iMediaIOState != MIO_STATE_STARTED)
            {
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = aCmd.iControlContext->iControl->Pause(););

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVCommsIONode::DoInit: Error - iMediaIOControl->Pause failed"));
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOPause;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        case EStop:
        {
            if (iMediaIOState != MIO_STATE_STARTED
                    && iMediaIOState != MIO_STATE_PAUSED)
            {
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = aCmd.iControlContext->iControl->Stop(););

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVCommsIONode::DoInit: Error - iMediaIOControl->Stop failed"));
                aCmd.iEventCode = PVCommsIONodeErr_MediaIOStop;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        default:
            Assert(false);//unrecognized command.
            status = PVMFFailure;
            break;
    }

    if (status == PVMFPending)
    {
        LOGINFOHI((0, "PVCommsIONode:SendMIORequest: Command Issued to MIO component, waiting on response..."));
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVCommsIONode::CancelMioRequest(PVCommsIONodeCmd& aCmd)
{
    Assert(iMediaIORequest != ENone);

    Assert(!iMediaIOCancelPending);

    //Issue the cancel to the MIO.
    iMediaIOCancelPending = true;
    int32 err;
    OSCL_TRY(err, iMediaIOCancelCmdId = aCmd.iControlContext->iControl->CancelCommand(iMediaIOCmdId););
    if (err != OsclErrNone)
    {
        aCmd.iEventCode = PVCommsIONodeErr_MediaIOCancelCommand;
        iMediaIOCancelPending = false;
        return PVMFFailure;
    }
    LOGINFOHI((0, "PVCommsIONode:SendMIORequest: Cancel Command Issued to MIO component, waiting on response..."));
    return PVMFPending;//wait on request to cancel.
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::MioConfigured(int32 aPortTag)
//called by the port when the MIO was just configured.
//only called for ports that need to wait on config before starting
//the MIO.
//RH Note: probably not need for Comms right now, and the port
//does not call this method.
{
    OSCL_UNUSED_ARG(aPortTag);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVCommsIONode::MioConfigured() called"));
    if (iInterfaceState == EPVMFNodeStarted)
    {
        PVCommsIONodeCmd cmd;
//		cmd.iControlContext = ContextFromTag(aPortTag);
        cmd.PVCommsIONodeCmdBase::Construct(0, PVMF_COMMSIONODE_STARTMIO, NULL);
        QueueCommandL(cmd);
    }
}

////////////////////////////////////////////////////////////////////////////
//                  Event reporting routines.
////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::SetState(TPVMFNodeInterfaceState s)
{
    LOGINFO((0, "PVCommsIONode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData, PVMFStatus aEventCode)
{
    LOGERROR((0, "PVCommsIONode:NodeErrorEvent Type %d EVData %d EVCode %d"
              , aEventType, aEventData, aEventCode));

    //create the extension message if any.
    if (aEventCode != PVCommsIONodeErr_First)
    {
        PVMFBasicErrorInfoMessage* eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aEventCode, iEventUuid, NULL));
        PVMFAsyncEvent asyncevent(PVMFErrorEvent, aEventType, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg), aEventData, NULL, 0);
        PVMFNodeInterface::ReportErrorEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
        PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData, PVMFStatus aEventCode)
{
    LOGINFO((0, "PVCommsIONode:NodeInfoEvent Type %d EVData %d EVCode %d"
             , aEventType, aEventData, aEventCode));

    //create the extension message if any.
    if (aEventCode != PVCommsIONodeErr_First)
    {
        PVMFBasicErrorInfoMessage* eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aEventCode, iEventUuid, NULL));
        PVMFAsyncEvent asyncevent(PVMFErrorEvent, aEventType, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg), aEventData, NULL, 0);
        PVMFNodeInterface::ReportInfoEvent(asyncevent);
        eventmsg->removeRef();
    }
    else
        PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
}

////////////////////////////////////////////////////////////////////////////
bool PVCommsIONode::PortQueuesEmpty()
{
    uint32 i;
    for (i = 0;i < iPortVector.size();i++)
    {
        if (iPortVector[i]->IncomingMsgQueueSize() > 0
                || iPortVector[i]->OutgoingMsgQueueSize() > 0)
        {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////
void PVCommsIONode::Assert(bool condition)
{
    if (!condition)
    {
        LOGERROR((0, "PVCommsIONode::Assert Failed!"));
        OSCL_ASSERT(0);
    }
}

MIOControlContextSet PVCommsIONode::ContextSetFromTag(int32 aTag)
{
    MIOControlContextSet contextSet;

    switch (aTag)
    {
        case PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG:
            if (iMediaInputContext.isValid())
                contextSet.iMediaInputElement = &iMediaInputContext;
            break;
        case PVMF_COMMSIO_NODE_INPUT_PORT_TAG:
            if (iMediaOutputContext.isValid())
                contextSet.iMediaOutputElement = &iMediaOutputContext;
            break;
        case PVMF_COMMSIO_NODE_IO_PORT_TAG:
            if (iMediaOutputContext.isValid() && iMediaInputContext.isValid())
            {
                contextSet.iMediaInputElement = &iMediaInputContext;
                contextSet.iMediaOutputElement = &iMediaOutputContext;
            }
            break;
        default:
            break;
    }
    return contextSet;
}
/*
PvmiCapabilityAndConfig *PVCommsIONode::GetConfig(int32 aPortTag)
{
	MIOControlContextElement *context = ContextSetFromTag(aPortTag);
	if (context)
		return context->iMediaIOConfig;
	return NULL;
}
*/
bool PVCommsIONode::CreateMediaTransfer(int32 aPortTag, PvmiMediaTransfer *&aInputTransfer, PvmiMediaTransfer *&aOutputTransfer)
{
    MIOControlContextSet set = ContextSetFromTag(aPortTag);

    if (set.isEmpty())
        return false;

    if (set.iMediaInputElement)
    {
        aInputTransfer = set.iMediaInputElement->iControl->createMediaTransfer(set.iMediaInputElement->iMediaSession);
        if (!aInputTransfer)
            return false;
    }
    if (set.iMediaOutputElement)
    {
        aOutputTransfer = set.iMediaOutputElement->iControl->createMediaTransfer(set.iMediaOutputElement->iMediaSession);
        if (!aOutputTransfer)
            return false;
    }

    return true;
}

void PVCommsIONode::DeleteMediaTransfer(int32 aPortTag, PvmiMediaTransfer *aInputTransfer, PvmiMediaTransfer *aOutputTransfer)
{
    MIOControlContextSet set = ContextSetFromTag(aPortTag);

    if (set.isEmpty())
        return;

    if (set.iMediaInputElement && aInputTransfer)
    {
        set.iMediaInputElement->iControl->deleteMediaTransfer(set.iMediaInputElement->iMediaSession, aInputTransfer);
    }
    else if (set.iMediaOutputElement && aOutputTransfer)
    {
        set.iMediaOutputElement->iControl->deleteMediaTransfer(set.iMediaOutputElement->iMediaSession, aOutputTransfer);
    }
}

bool PVCommsIONode::GetNextContextInSequence(PVCommsIONodeCmd &aCmd)
{
    // We need to determine which IO controls to start
    // the init with
    if (aCmd.iControlContext == NULL)
    {
        if (iMediaInputContext.isValid())
            aCmd.iControlContext = &iMediaInputContext;
        else if (iMediaOutputContext.isValid())
            aCmd.iControlContext = &iMediaOutputContext;
        else
            return false;
    }
    else if ((aCmd.iControlContext == &iMediaInputContext) &&
             iMediaOutputContext.isValid() &&
             (iMediaOutputContext.iControl != iMediaInputContext.iControl))
    {
        aCmd.iControlContext = &iMediaOutputContext;
    }
    else
        return false;

    return true;
}

bool PVCommsIONode::ReRunCommandForNextMIO(PVCommsIONodeCmd &aCmd)
{
    // This method should only be called from the
    // RequestCompleted method, and allows us to re-use
    // command objects to asynchronously initialize more than
    // one MIO component under the context of a single node
    // command id.

    // The force parameter will force the command to be re run for
    // an output context even if its MIO component is a reference to
    // the input context component.  This is useful for the query interface
    // portion of initialization.

    PVCommsIONodeCmd newCmd = aCmd;

    if (GetNextContextInSequence(newCmd))
    {
        iCurrentCommand.Erase(&aCmd);
        // Don't use AddL because that overwrites the iId
        // which must remain the same.
        iInputCommands.StoreL(newCmd);
        RunIfNotReady();
        iMediaIORequest = ENone;
        return true;
    }
    return false;
}

bool PVCommsIONode::MIOSupportsPortTag(int32 aTag, MIOControlContextSet &aSet)
{
    PvmiKvp* kvp ;
    int numParams ;

    // Get input formats capability from media IO
    kvp = NULL;
    numParams = 0;

    if (aTag & PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG)
    {
        if (!aSet.iMediaInputElement->hasConfig() ||
                aSet.iMediaInputElement->iMediaIOConfig->getParametersSync(NULL,
                        OSCL_CONST_CAST(char*, OUTPUT_FORMATS_CAP_QUERY),
                        kvp, numParams, NULL) != PVMFSuccess)
            return false;
        aSet.iMediaInputElement->iMediaIOConfig->releaseParameters(0, kvp, numParams);
        if (numParams == 0)
            return false;
    }
    if (aTag & PVMF_COMMSIO_NODE_INPUT_PORT_TAG)
    {
        if (!aSet.iMediaOutputElement->hasConfig() ||
                aSet.iMediaOutputElement->iMediaIOConfig->getParametersSync(NULL,
                        OSCL_CONST_CAST(char*, INPUT_FORMATS_CAP_QUERY),
                        kvp, numParams, NULL) != PVMFSuccess)
            return false;
        aSet.iMediaOutputElement->iMediaIOConfig->releaseParameters(0, kvp, numParams);
        if (numParams == 0)
            return false;
    }

    return true;
}

// Check some stuff to determine if we can hand out
// a port of this type...

bool PVCommsIONode::ValidatePortTagRequest(int32 aTag)
{
    uint32 curPorts = iPortVector.size();

    switch (aTag)
    {
        case PVMF_COMMSIO_NODE_IO_PORT_TAG:
            if (curPorts > 0)
                return false; // can have only one I/O port
            break;

        case PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG:
            if (curPorts > 1 ||
                    (curPorts == 1 && iPortVector[0]->GetPortTag() != PVMF_COMMSIO_NODE_INPUT_PORT_TAG))
                return false;
            break;
        case PVMF_COMMSIO_NODE_INPUT_PORT_TAG:
            if (curPorts > 1 ||
                    (curPorts == 1 && iPortVector[0]->GetPortTag() != PVMF_COMMSIO_NODE_OUTPUT_PORT_TAG))
                return false;
            break;
        default:
            return false;
    }

    MIOControlContextSet set = ContextSetFromTag(aTag);

    if (set.isEmpty() || !MIOSupportsPortTag(aTag, set))
        return false;

    return true;
}
