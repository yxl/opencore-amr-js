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
 * @file pvmf_media_input_node.cpp
 * @brief
 */

#include "oscl_base.h"
#include "pvmf_media_input_node_factory.h"
#include "pvmf_media_input_node.h"
#include "pvmf_media_input_node_outport.h"
#include "oscl_dll.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pvmf_media_input_node_events.h"

#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif
#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif
#include "oscl_assert.h"

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

#ifdef _TEST_AE_ERROR_HANDLING
#define FAIL_NODE_CMD_START 2
#define FAIL_NODE_CMD_STOP 3
#define FAIL_NODE_CMD_FLUSH 4
#define FAIL_NODE_CMD_PAUSE 5
#define FAIL_NODE_CMD_RELEASE_PORT 7
#endif


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PvmfMediaInputNodeFactory::Create(PvmiMIOControl* aMIOControl)
{
    return PvmfMediaInputNode::Create(aMIOControl);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNodeFactory::Delete(PVMFNodeInterface* aNode)
{
    PvmfMediaInputNode::Release(aNode);
}

////////////////////////////////////////////////////////////////////////////
PVMFNodeInterface* PvmfMediaInputNode::Create(PvmiMIOControl* aIOInterfacePtr)
{
    PvmfMediaInputNode* node = OSCL_NEW(PvmfMediaInputNode, ());
    if (node)
    {
        OSCL_TRAPSTACK_PUSH(node);
        node->ConstructL(aIOInterfacePtr);
        OSCL_TRAPSTACK_POP();
    }
    return (PVMFNodeInterface*)node;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::Release(PVMFNodeInterface* aNode)
{
    OSCL_DELETE(((PvmfMediaInputNode*)aNode));
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNode::ThreadLogon()
{
    if (iInterfaceState != EPVMFNodeCreated)
        return PVMFErrInvalidState;

    iLogger = PVLogger::GetLoggerObject("PvmfMediaInputNode");

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::ThreadLogon"));

    if (!IsAdded())
        AddToScheduler();

    if (iMediaIOControl)

        iMediaIOControl->ThreadLogon();

    SetState(EPVMFNodeIdle);
#ifdef _TEST_AE_ERROR_HANDLING
    iErrorHandlingStartFailed = false;
    iErrorHandlingStopFailed = false;
    iError_No_Memory = false;
    iError_Out_Queue_Busy = false;
#endif
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::ThreadLogoff"));

    if (iInterfaceState != EPVMFNodeIdle)
        return PVMFErrInvalidState;

    if (IsAdded())
        RemoveFromScheduler();

    iLogger = NULL;

    if (iMediaIOControl)
    {
        iMediaIOControl->ThreadLogoff();
        iMediaIOControl->disconnect(iMediaIOSession);
        //ignore any returned error.

        iMediaIOState = PvmfMediaInputNode::MIO_STATE_IDLE;
    }

    SetState(EPVMFNodeCreated);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PvmfMediaInputNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    if (!iMediaIOConfig)
        return PVMFFailure;

    aNodeCapability.iCanSupportMultipleInputPorts = false;
    aNodeCapability.iCanSupportMultipleOutputPorts = false;
    aNodeCapability.iHasMaxNumberOfPorts = true;
    aNodeCapability.iMaxNumberOfPorts = 1;

    PvmiKvp* kvp ;
    int numParams ;
    int32 err ;
    int32 i ;
    PVMFStatus status;

    // Get input formats capability from media IO
    kvp = NULL;
    numParams = 0;
    status = iMediaIOConfig->getParametersSync(NULL, (PvmiKeyType)INPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
    if (status == PVMFSuccess)
    {
        OSCL_TRY(err,
                 for (i = 0; i < numParams; i++)
                 aNodeCapability.iInputFormatCapability.push_back(PVMFFormatType(kvp[i].value.pChar_value));
                );
        if (kvp)
            iMediaIOConfig->releaseParameters(0, kvp, numParams);
    }
    //else ignore errors.

    // Get output formats capability from media IO
    kvp = NULL;
    numParams = 0;
    status = iMediaIOConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
    if (status == PVMFSuccess)
    {
        OSCL_TRY(err,
                 for (i = 0; i < numParams; i++)
                 aNodeCapability.iOutputFormatCapability.push_back(PVMFFormatType(kvp[i].value.pChar_value));
                );
        if (kvp)
            iMediaIOConfig->releaseParameters(0, kvp, numParams);
    }
    //else ignore errors.

    if (aNodeCapability.iInputFormatCapability.empty() && aNodeCapability.iOutputFormatCapability.empty())
        return PVMFFailure;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PvmfMediaInputNode::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iOutPortVector.Reset();
    return &iOutPortVector;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::QueryUUID() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::QueryInterface() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::RequestPort() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::ReleasePort() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Init() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Prepare() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Start() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Stop() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Flush() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Pause() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::Reset() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::CancelAllCommands() called"));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PvmfMediaInputNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::CancelCommands() called cmdId=%d", aCmdId));
    PvmfMediaInputNodeCmd cmd;
    cmd.PvmfMediaInputNodeCmdBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNode::removeRef()
{
    if (iExtensionRefCount > 0)
        --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PvmfMediaInputNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PvmfNodesSyncControlUuid)
    {
        PvmfNodesSyncControlInterface* myInterface = OSCL_STATIC_CAST(PvmfNodesSyncControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
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
OSCL_EXPORT_REF void PvmfMediaInputNode::RequestCompleted(const PVMFCmdResp& aResponse)
//callback from the MIO module.
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::RequestCompleted: Cmd ID=%d", aResponse.GetCmdId()));

    //look for cancel completion.
    if (iMediaIOCancelPending
            && aResponse.GetCmdId() == iMediaIOCancelCmdId)
    {
        iMediaIOCancelPending = false;

        OSCL_ASSERT(!iCancelCommand.empty());

        //Current cancel command is now complete.
        CommandComplete(iCancelCommand, iCancelCommand.front(), PVMFSuccess);
    }
    //look for non-cancel completion
    else if (iMediaIORequest != ENone
             && aResponse.GetCmdId() == iMediaIOCmdId)
    {
        OSCL_ASSERT(!iCurrentCommand.empty());
        PvmfMediaInputNodeCmd& cmd = iCurrentCommand.front();

        switch (iMediaIORequest)
        {
            case EQueryCapability:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PvmfMediaInputNodeErr_MediaIOQueryCapConfigInterface;
                }
                else
                {
                    if (iMediaIOConfigPVI)
                    {

                        iMediaIOConfig = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, iMediaIOConfigPVI);
                        iMediaIOConfigPVI = NULL;
                    }
                    else
                    {
                        LOGERROR((0, "PvmfMediaInputNode:RequestComplete Error:Not setting Queryinterface ,iTempCapConfigInterface is NULL"));
                    }
                }
                break;

            case EInit:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PvmfMediaInputNodeErr_MediaIOInit;
                }
                else
                {
                    iMediaIOState = MIO_STATE_INITIALIZED;
                }
                break;

            case EStart:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PvmfMediaInputNodeErr_MediaIOStart;
                }
                else
                {
                    iMediaIOState = MIO_STATE_STARTED;
                    //tell all the ports that I/O is started.
                    {
                        uint32 i;
                        for (i = 0; i < iOutPortVector.size(); i++)
                            iOutPortVector[i]->MediaIOStarted();
                    }
                }
                break;

            case EPause:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PvmfMediaInputNodeErr_MediaIOPause;
                }
                else
                {
                    iMediaIOState = MIO_STATE_PAUSED;
                }
                break;

            case EStop:
                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PvmfMediaInputNodeErr_MediaIOStop;
                }
                else
                {
                    iMediaIOState = MIO_STATE_INITIALIZED;
                }
                break;
            case EReset:

                if (aResponse.GetCmdStatus() != PVMFSuccess)
                {
                    cmd.iEventCode = PvmfMediaInputNodeErr_MediaIOReset;
                }
                else
                {
                    iMediaIOState = MIO_STATE_IDLE;
                    SetState(EPVMFNodeIdle);
                }
                break;
            default:
                OSCL_ASSERT(false);
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
        LOGERROR((0, "PvmfMediaInputNode:RequestComplete Warning! Unexpected command ID %d"
                  , aResponse.GetCmdId()));
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNode::ReportErrorEvent(PVMFEventType aEventType, PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aEventType);
    OSCL_UNUSED_ARG(aExtMsg);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PvmfMediaInputNode::ReportInfoEvent(PVMFEventType aEventType, PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aEventType);
    OSCL_UNUSED_ARG(aExtMsg);
}

////////////////////////////////////////////////////////////////////////////
PvmfMediaInputNode::PvmfMediaInputNode()
        : OsclActiveObject(OsclActiveObject::EPriorityNominal, "PvmfMediaInputNode")
        , iMediaIOControl(NULL)
        , iMediaIOSession(NULL)
        , iMediaIOConfig(NULL)
        , iMediaIOConfigPVI(NULL)
        , iMediaIOState(PvmfMediaInputNode::MIO_STATE_IDLE)
        , iEventUuid(PvmfMediaInputNodeEventTypesUUID)
        , iExtensionRefCount(0)
        , iLogger(NULL)
{
#ifdef _TEST_AE_ERROR_HANDLING
    iChunkCount = 0;
    iErrorTrackID = -1;
    iTrackID = 0;
    iErrorCancelMioRequest = false;
    iErrorSendMioRequest = 0;
    iErrorNodeCmd = 0;
#endif
}

////////////////////////////////////////////////////////////////////////////
PvmfMediaInputNode::~PvmfMediaInputNode()
{
    Cancel();
    if (IsAdded())
        RemoveFromScheduler();

    //if any MIO commands are outstanding, there will be
    //a crash when they callback-- so panic here instead.
    if (!iCancelCommand.empty()
            || iMediaIORequest != ENone)
        OSCL_ASSERT(0);

    //Cleanup allocated ports
    while (!iOutPortVector.empty())
        iOutPortVector.Erase(&iOutPortVector.front());

    //Cleanup commands
    //The command queues are self-deleting, but we want to
    //notify the observer of unprocessed commands.
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
//		iCurrentCommand.Erase(&iCurrentCommand.front());
    }
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
//		iInputCommands.Erase(&iInputCommands.front());
    }
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::ConstructL(PvmiMIOControl* aIOInterfacePtr)
{
    iLogger = NULL;
    iMediaIOControl = aIOInterfacePtr;
    iPortActivity = 0;
    iInputCommands.Construct(1, 10);//reserve 10
    iCurrentCommand.Construct(1, 1);//reserve 1.
    iCancelCommand.Construct(1, 1);//reserve 1.
    iOutPortVector.Construct(0);//reserve zero.
    iMediaIORequest = ENone;
    iMediaIOCancelPending = false;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::Run()
{
    //Process async node commands.
    if (!iInputCommands.empty())
    {
        ProcessCommand();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PvmfMediaInputNode::QueueCommandL(PvmfMediaInputNodeCmd& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::ProcessCommand()
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
    OSCL_ASSERT(!iInputCommands.empty());
    PvmfMediaInputNodeCmd& aCmd = iInputCommands.front();

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
                    OSCL_ASSERT(false);
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
                    OSCL_ASSERT(false);
                    cmdstatus = PVMFFailure;
                    break;
            }
        }

        //erase the input command.
        if (cmdstatus != PVMFPending)
        {
            //Request already failed-- erase from Current Command.
            //Node command remains in Input Commands.
            iCurrentCommand.Erase(&iCurrentCommand.front());
        }
        else
        {
            //Node command is now stored in Current Command, so erase from Input Commands.
            iInputCommands.Erase(&aCmd);
        }
    }

    if (cmdstatus != PVMFPending)
        CommandComplete(iInputCommands, aCmd, cmdstatus, aEventData);

}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::CommandComplete(PvmfMediaInputNodeCmdQ& aCmdQ, PvmfMediaInputNodeCmd& aCmd, PVMFStatus aStatus, OsclAny*aEventData)
{
    if (aStatus == PVMFSuccess)
    {
        LOGINFO((0, "PvmfMediaInputNode:CommandComplete Id %d Cmd %d Status %d Context %d EVData %d EVCode %d"
                 , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData, aCmd.iEventCode));
    }
    else
    {
        LOGERROR((0, "PvmfMediaInputNode:CommandComplete Error! Id %d Cmd %d Status %d Context %d EVData %d EVCode %d"
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
                    for (i = 0; i < iOutPortVector.size(); i++)
                        iOutPortVector[i]->ClearMsgQueues();
                }
                break;
            case PVMF_GENERIC_NODE_FLUSH:
                SetState(EPVMFNodePrepared);
                //Stop the ports
                {
                    for (uint32 i = 0; i < iOutPortVector.size(); i++)
                        iOutPortVector[i]->Stop();
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
    if (aCmd.iCmd == PVMF_MEDIAIONODE_STARTMIO)
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

        if (eventCode != PvmfMediaInputNodeErr_First)
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
PVMFStatus PvmfMediaInputNode::DoQueryUuid(PvmfMediaInputNodeCmd& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PvmfMediaInputNodeCmdBase::Parse(mimetype, uuidvec, exactmatch);

    uuidvec->push_back(PvmfNodesSyncControlUuid);

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoQueryInterface(PvmfMediaInputNodeCmd& aCmd)
{
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PvmfMediaInputNodeCmdBase::Parse(uuid, ptr);
    if (uuid && ptr)
    {
        if (queryInterface(*uuid, *ptr))
            return PVMFSuccess;
    }
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoRequestPort(PvmfMediaInputNodeCmd& aCmd, OsclAny*&aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::DoRequestPort"));
    //This node supports port request from any state

    //retrieve port tag & mimetype
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PvmfMediaInputNodeCmdBase::Parse(tag, mimetype);

    switch (tag)
    {

        case PVMF_MEDIAIO_NODE_OUTPUT_PORT_TAG:
        {
            //Allocate a new port
            OsclAny *ptr = NULL;
            int32 err;
            OSCL_TRY(err, ptr = iOutPortVector.Allocate(););
            if (err != OsclErrNone || !ptr)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PvmfMediaInputNode::DoRequestPort: Error - iOutPortVector Out of memory"));
                return PVMFErrNoMemory;
            }

            OSCL_StackString<20> portname;
            portname = "PVMFMediaIpNodeOut";

            PvmfMediaInputNodeOutPort *port = OSCL_PLACEMENT_NEW(ptr, PvmfMediaInputNodeOutPort(this, portname.get_cstr()));

            //set the format from the mimestring.
            if (mimetype)
            {
                PVMFFormatType fmt = mimetype->get_str();
                if (fmt != PVMF_MIME_FORMAT_UNKNOWN)
                    port->Configure(fmt, mimetype);
            }

            //Add the port to the port vector.
            OSCL_TRY(err, iOutPortVector.AddL(port););
            if (err != OsclErrNone)
            {
                iOutPortVector.DestructAndDealloc(port);
                return PVMFErrNoMemory;
            }
            aEventData = (OsclAny*)port;

            //cast the port to the generic interface before returning.
            PVMFPortInterface*retval = port;
            aEventData = (OsclAny*)retval;
            return PVMFSuccess;
        }

        default:
        {
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PvmfMediaInputNode::DoRequestPort: Error - Invalid port tag"));
            return PVMFFailure;
        }
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoReleasePort(PvmfMediaInputNodeCmd& aCmd)
{
    //This node supports release port from any state

    PVMFPortInterface* p;
    aCmd.PvmfMediaInputNodeCmdBase::Parse(p);
    //search the output port vector
    {
        PvmfMediaInputNodeOutPort* port = (PvmfMediaInputNodeOutPort*)p;
        PvmfMediaInputNodeOutPort** portPtr = iOutPortVector.FindByValue(port);

#ifdef _TEST_AE_ERROR_HANDLING
        if (FAIL_NODE_CMD_RELEASE_PORT == iErrorNodeCmd)
        {
            portPtr = NULL;
        }
#endif
        if (portPtr)
        {
            (*portPtr)->Disconnect();
            iOutPortVector.Erase(portPtr);
            return PVMFSuccess;
        }
    }
    aCmd.iEventCode = PvmfMediaInputNodeErr_PortNotExist;
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoInit(PvmfMediaInputNodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
    if (EPVMFNodeInitialized == iInterfaceState)
    {
        return PVMFSuccess;
    }

    if (iInterfaceState != EPVMFNodeIdle)
        return PVMFErrInvalidState;

    if (!iMediaIOControl)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl is NULL"));
        aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIONotExist;
        return PVMFFailure;
    }

    if (iMediaIOControl->connect(iMediaIOSession, this) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl->connect failed"));
        aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOConnect;
        return PVMFFailure;
    }

    //Query for MIO capability and config interface
    return SendMioRequest(aCmd, EQueryCapability);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoPrepare(PvmfMediaInputNodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::DoPrepare"));

    if (EPVMFNodePrepared == iInterfaceState)
    {
        return PVMFSuccess;
    }
    if (iInterfaceState != EPVMFNodeInitialized)
        return PVMFErrInvalidState;

    return SendMioRequest(aCmd, EInit);

}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoStart(PvmfMediaInputNodeCmd& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::DoStart"));
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_START == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    if (EPVMFNodeStarted == iInterfaceState)
    {
        return PVMFSuccess;
    }

    if (iInterfaceState != EPVMFNodePrepared
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Start the MIO
    PVMFStatus status = SendMioRequest(aCmd, EStart);
    if (status != PVMFPending)
        return status;


    //Start the ports
    uint32 i = 0;
    for (i = 0; i < iOutPortVector.size(); i++)
        iOutPortVector[i]->Start();

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoStop(PvmfMediaInputNodeCmd& aCmd)
{
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_STOP == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    if (EPVMFNodePrepared == iInterfaceState)
    {
        return PVMFSuccess;
    }
    if (iInterfaceState != EPVMFNodeStarted
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Stop the MIO
    PVMFStatus status = SendMioRequest(aCmd, EStop);

    //Stop the ports
    for (uint32 i = 0; i < iOutPortVector.size(); i++)
        iOutPortVector[i]->Stop();

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoFlush(PvmfMediaInputNodeCmd& aCmd)
{
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_FLUSH == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    if (iInterfaceState != EPVMFNodeStarted
            && iInterfaceState != EPVMFNodePaused)
        return PVMFErrInvalidState;

    //Disable the input.
    if (iOutPortVector.size() > 0)
    {
        //Stop the MIO input
        SendMioRequest(aCmd, EStop);
        //ignore returned error.
    }
    //wait for all data to be consumed
    return PVMFPending;
}

////////////////////////////////////////////////////////////////////////////
bool PvmfMediaInputNode::IsFlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::FlushComplete()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::FlushComplete"));

    if (iMediaIORequest != ENone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfMediaInputNode::FlushComplete: Media IO request is pending"));
        return;
    }

    if (!PortQueuesEmpty())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfMediaInputNode::FlushComplete: Port queues are not empty"));
        return;
    }

    //Flush is complete.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PvmfMediaInputNode::FlushComplete: Done"));
    CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoPause(PvmfMediaInputNodeCmd& aCmd)
{
#ifdef _TEST_AE_ERROR_HANDLING
    if (FAIL_NODE_CMD_PAUSE == iErrorNodeCmd)
    {
        iInterfaceState = EPVMFNodeError;
    }
#endif
    if (EPVMFNodePaused == iInterfaceState)
    {
        return PVMFSuccess;
    }
    if (iInterfaceState != EPVMFNodeStarted)
        return PVMFErrInvalidState;

    return SendMioRequest(aCmd, EPause);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoReset(PvmfMediaInputNodeCmd& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    if (IsAdded())
    {
        //delete all ports and notify observer.
        while (!iOutPortVector.empty())
            iOutPortVector.Erase(&iOutPortVector.front());

        //restore original port vector reserve.
        iOutPortVector.Reconstruct();
        //Reset the MIO
        PVMFStatus status = SendMioRequest(aCmd, EReset);
        return status;
    }
    else
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return 0;
    }

}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::DoCancelAllCommands(PvmfMediaInputNodeCmd& aCmd)
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
PVMFStatus PvmfMediaInputNode::DoCancelCommand(PvmfMediaInputNodeCmd& aCmd)
{
    //tbd, need to cancel the MIO

    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PvmfMediaInputNodeCmdBase::Parse(id);

    //first check "current" command if any
    {
        PvmfMediaInputNodeCmd* cmd = iCurrentCommand.FindById(id);
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
        PvmfMediaInputNodeCmd* cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
            //report cancel success
            return PVMFSuccess;
        }
    }
    //if we get here the command isn't queued so the cancel fails.
    aCmd.iEventCode = PvmfMediaInputNodeErr_CmdNotQueued;
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::SendMioRequest(PvmfMediaInputNodeCmd& aCmd, EMioRequest aRequest)
{
    //Make an asynchronous request to MIO component
    //and save the request in iMediaIORequest.


    //there should not be a MIO command in progress..
    OSCL_ASSERT(iMediaIORequest == ENone);


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
                     iMediaIOCmdId = iMediaIOControl->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                                     iMediaIOConfigPVI, NULL);
                    );

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl->QueryInterface failed"));
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOQueryCapConfigInterface;
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
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = iMediaIOControl->Init(););

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl->Init failed"));
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOInit;
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
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = iMediaIOControl->Start(););
#ifdef _TEST_AE_ERROR_HANDLING
            if (iErrorHandlingStartFailed)
            {
                err = OsclErrGeneral;
            }
#endif

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl->Start failed"));
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOStart;
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
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err = 0 ;
            OSCL_TRY(err, iMediaIOCmdId = iMediaIOControl->Pause(););

#ifdef _TEST_AE_ERROR_HANDLING

            if (iErrorSendMioRequest == 2)
            {
                err = OsclErrGeneral;
            }
#endif
            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl->Pause failed"));
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOPause;
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
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOWrongState;
                status = PVMFFailure;
                break;
            }

            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = iMediaIOControl->Stop(););
#ifdef _TEST_AE_ERROR_HANDLING
            if (iErrorHandlingStopFailed)
            {
                err = OsclErrGeneral;
            }
#endif

            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PvmfMediaInputNode::DoInit: Error - iMediaIOControl->Stop failed"));
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOStop;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;
        case EReset:
        {
            int32 err ;
            OSCL_TRY(err, iMediaIOCmdId = iMediaIOControl->Reset(););
            if (err != OsclErrNone)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMediaInputNode::SendMioRequest: Error - iMIOControl->Reset failed"));
                aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOReset;
                status = PVMFFailure;
            }
            else
            {
                status = PVMFPending;
            }
        }
        break;

        default:
            status = PVMFFailure;
            OSCL_ASSERT(false);//unrecognized command.
    }

    if (status == PVMFPending)
    {
        LOGINFOHI((0, "PvmfMediaInputNode:SendMIORequest: Command Issued to MIO component, waiting on response..."));
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfMediaInputNode::CancelMioRequest(PvmfMediaInputNodeCmd& aCmd)
{
    OSCL_ASSERT(iMediaIORequest != ENone);

    OSCL_ASSERT(!iMediaIOCancelPending);

    //Issue the cancel to the MIO.
    iMediaIOCancelPending = true;
    int32 err;
    OSCL_TRY(err, iMediaIOCancelCmdId = iMediaIOControl->CancelCommand(iMediaIOCmdId););

#ifdef _TEST_AE_ERROR_HANDLING
    if (iErrorCancelMioRequest)
    {
        err = OsclErrGeneral;
    }
#endif
    if (err != OsclErrNone)
    {
        aCmd.iEventCode = PvmfMediaInputNodeErr_MediaIOCancelCommand;
        iMediaIOCancelPending = false;
        return PVMFFailure;
    }
    LOGINFOHI((0, "PvmfMediaInputNode:SendMIORequest: Cancel Command Issued to MIO component, waiting on response..."));
    return PVMFPending;//wait on request to cancel.
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::MioConfigured()
//called by the port when the MIO was just configured.
//only called for ports that need to wait on config before starting
//the MIO.
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfMediaInputNode::MioConfigured() called"));
    if (iInterfaceState == EPVMFNodeStarted)
    {
        PvmfMediaInputNodeCmd cmd;
        cmd.PvmfMediaInputNodeCmdBase::Construct(0, PVMF_MEDIAIONODE_STARTMIO, NULL);
        QueueCommandL(cmd);
    }
}

////////////////////////////////////////////////////////////////////////////
//                  Event reporting routines.
////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::SetState(TPVMFNodeInterfaceState s)
{
    LOGINFO((0, "PvmfMediaInputNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}

////////////////////////////////////////////////////////////////////////////
void PvmfMediaInputNode::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData, PVMFStatus aEventCode)
{
    LOGERROR((0, "PvmfMediaInputNode:NodeErrorEvent Type %d EVData %d EVCode %d"
              , aEventType, aEventData, aEventCode));

    //create the extension message if any.
    if (aEventCode != PvmfMediaInputNodeErr_First)
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
void PvmfMediaInputNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData, PVMFStatus aEventCode)
{
    LOGINFO((0, "PvmfMediaInputNode:NodeInfoEvent Type %d EVData %d EVCode %d"
             , aEventType, aEventData, aEventCode));

    //create the extension message if any.
    if (aEventCode != PvmfMediaInputNodeErr_First)
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
bool PvmfMediaInputNode::PortQueuesEmpty()
{
    uint32 i;
    for (i = 0;i < iOutPortVector.size();i++)
    {
        if (iOutPortVector[i]->IncomingMsgQueueSize() > 0
                || iOutPortVector[i]->OutgoingMsgQueueSize() > 0)
        {
            return false;
        }
    }
    return true;
}










