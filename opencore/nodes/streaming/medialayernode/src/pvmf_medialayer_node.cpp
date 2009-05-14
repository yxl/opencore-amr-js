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
#ifndef  OSCL_MIME_STRING_UTILS_H_INCLUDED
#include "pv_mime_string_utils.h"
#endif
#ifndef  OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif
#ifndef  OSCL_ASSERT_H_INCLUDED
#include "oscl_assert.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif
#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif
#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif
#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
#endif
#ifndef MEDIAINFO_H_INCLUDED
#include "media_info.h"
#endif
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif
#ifndef PVMF_MEDIALAYER_NODE_H_INCLUDED
#include "pvmf_medialayer_node.h"
#endif
#ifndef PVMF_MEDIA_MSG_HEADER_H_INCLUDED
#include "pvmf_media_msg_header.h"
#endif
#ifndef PVMF_SM_CONFIG_H_INCLUDED
#include "pvmf_sm_config.h"
#endif

#define RETURN_ERROR_WHEN_MINUS_TIMESTAMP
// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

/**
 * Node Constructor & Destructor
 */
OSCL_EXPORT_REF PVMFMediaLayerNode::PVMFMediaLayerNode(int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFMediaLayerNode")
{
    iLogger = NULL;
    iDataPathLogger = NULL;
    iDataPathLoggerFlowCtrl = NULL;
    iClockLogger = NULL;
    iOsclErrorTrapImp = NULL;
    iExtensionInterface = NULL;
    iPayLoadParserRegistry = NULL;
    oPortDataLog = false;
    iLogFileIndex = '0';
    iClientPlayBackClock = NULL;
    iDecryptionInterface = NULL;
    srcPtr = NULL;
    diffAudioVideoTS = 0;
    iAdjustTimeReady = false;
    oEOSsendunits = false;
    iTimerNoDataTrack = 10 * 1000;
    iReposTime = 0;
    preroll64 = 0;
    iStreamID = 0;
    iExtensionInterface = NULL;
    iNumRunL = 0;
    iDiagnosticsLogged = false;

    int32 err;
    OSCL_TRY(err,
             /*
              * Create the input command queue.Use a reserve to avoid lots of
              * dynamic memory allocation.
              */
             iInputCommands.Construct(MEDIA_LAYER_NODE_CMD_START,
                                      MEDIA_LAYER_NODE_CMD_QUE_RESERVE);

             /*
              * Create the "current command" queue.  It will only contain one
              * command at a time, so use a reserve of 1.
              */
             iCurrentCommand.Construct(0, 1);

             /* Create the port vector */
             iPortVector.Construct(MEDIA_LAYER_NODE_VECTOR_RESERVE);

             /*
              * Set the node capability data.
              * This node can support an unlimited number of ports.
              */
             iCapability.iCanSupportMultipleInputPorts = true;
             iCapability.iCanSupportMultipleOutputPorts = true;
             iCapability.iHasMaxNumberOfPorts = false;
             iCapability.iMaxNumberOfPorts = 0; /* no maximum */

             iCapability.iInputFormatCapability.push_back(PVMF_MIME_RTP);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_M4V);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_AMR_IETF);

            );



    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iInputCommands.clear();
        iCurrentCommand.clear();
        iPortVector.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclActiveObject);
        OSCL_LEAVE(err);
    }
}

OSCL_EXPORT_REF PVMFMediaLayerNode::~PVMFMediaLayerNode()
{
    LogSessionDiagnostics();

    Cancel();
    if (iExtensionInterface)
    {
        iExtensionInterface->removeRef();
    }

    /* delete related decryption */
    iDecryptionInterface = NULL;
    if (srcPtr != NULL)
        oscl_free((uint8*)srcPtr);
    srcPtr = NULL;

    /* thread logoff */
    if (IsAdded())
        RemoveFromScheduler();

    /*
     * Cleanup allocated ports
     * The port vector is self-deleting, but we want to notify
     * observers that ports are being deleted
     */
    while (!iPortVector.empty())
    {
        /* delete corresponding port params */
        Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;

        for (it = iPortParamsQueue.begin();
                it != iPortParamsQueue.end();
                it++)
        {
            if (it->iPort == iPortVector.front())
            {
                if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
                {
                    DestroyPayLoadParser(&(it->iMimeType), it->iPayLoadParser);
                    if (oPortDataLog)
                    {
                        if (it->iBinAppenderPtr.GetRep() != NULL)
                        {
                            it->iPortLogger->RemoveAppender(it->iBinAppenderPtr);
                            it->iBinAppenderPtr.Unbind();
                        }
                    }
                }
                if (it->ipFragGroupAllocator != NULL)
                {
                    it->ipFragGroupAllocator->CancelFreeChunkAvailableCallback();
                    it->ipFragGroupAllocator->removeRef();
                }
                if (it->ipFragGroupMemPool != NULL)
                {
                    it->ipFragGroupMemPool->removeRef();
                }
                it->CleanUp();
                iPortParamsQueue.erase(it);
                break;
            }
        }
        iPortVector.Erase(&iPortVector.front());
    }

    /*
     * Cleanup commands
     * The command queues are self-deleting, but we want to
     * notify the observer of unprocessed commands.
     */
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
    }
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }
}

/**
 * Public Node API implementation
 */

/**
 * Do thread-specific node creation and go to "Idle" state.
 */
OSCL_EXPORT_REF PVMFStatus PVMFMediaLayerNode::ThreadLogon()
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:ThreadLogon"));
    PVMFStatus status;
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        {
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFMediaLayerNode");
            iRunlLogger = PVLogger::GetLoggerObject("Run.PVMFMediaLayerNode");
            iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.medialayer");
            iDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.medialayer.in");
            iDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.medialayer.out");
            iDataPathLoggerFlowCtrl = PVLogger::GetLoggerObject("datapath.sourcenode.medialayer.portflowcontrol");
            if (iDataPathLoggerFlowCtrl != NULL)
            {
                iDataPathLoggerFlowCtrl->DisableAppenderInheritance();
            }
            iClockLogger = PVLogger::GetLoggerObject("clock");
            iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.streamingmanager.medialayer");
            iDiagnosticsLogged = false;
            iReposLogger = PVLogger::GetLoggerObject("pvplayerrepos.sourcenode.streamingmanager.medialayer");
            iOsclErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
            SetState(EPVMFNodeIdle);
            status = PVMFSuccess;
        }
        break;
        default:
            status = PVMFErrInvalidState;
            break;
    }
    return status;
}

/**
 * Do thread-specific node cleanup and go to "Created" state.
 */
OSCL_EXPORT_REF PVMFStatus PVMFMediaLayerNode::ThreadLogoff()
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:ThreadLogoff"));
    PVMFStatus status;
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            if (IsAdded())
            {
                RemoveFromScheduler();
            }
            iLogger = NULL;
            iDataPathLogger = NULL;
            iDataPathLoggerFlowCtrl = NULL;
            iClockLogger = NULL;
            iDiagnosticsLogger = NULL;
            iOsclErrorTrapImp = NULL;
            SetState(EPVMFNodeCreated);
            status = PVMFSuccess;
        }
        break;

        default:
            status = PVMFErrInvalidState;
            break;
    }
    return status;
}

/**
 * retrieve node capabilities.
 */
OSCL_EXPORT_REF
PVMFStatus PVMFMediaLayerNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:GetCapability"));
    aNodeCapability = iCapability;
    return PVMFSuccess;
}

/**
 * retrieve a port iterator.
 */
OSCL_EXPORT_REF
PVMFPortIter* PVMFMediaLayerNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:GetPorts"));
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::QueryUUID(PVMFSessionId s,
        const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:QueryUUID"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_QUERYUUID,
            aMimeType,
            aUuids,
            aExactUuidsOnly,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::QueryInterface(PVMFSessionId s,
        const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:QueryInterface"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_QUERYINTERFACE,
            aUuid,
            aInterfacePtr,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::RequestPort(PVMFSessionId aSession,
        int32 aPortTag,
        const PvmfMimeString* aPortConfig,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:RequestPort"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(aSession,
            PVMF_MEDIA_LAYER_NODE_REQUESTPORT,
            aPortTag,
            aPortConfig,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::ReleasePort(PVMFSessionId s,
        PVMFPortInterface& aPort,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:ReleasePort"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_RELEASEPORT,
            aPort,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Init(PVMFSessionId s,
                                       const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Init"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_INIT,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Prepare(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Prepare"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_PREPARE,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Start(PVMFSessionId s,
                                        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Start"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_START,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Stop(PVMFSessionId s,
                                       const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Stop"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_STOP,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Flush(PVMFSessionId s,
                                        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Flush"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_FLUSH,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Pause(PVMFSessionId s,
                                        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Pause"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_PAUSE,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::Reset(PVMFSessionId s,
                                        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:Reset"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_RESET,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::CancelAllCommands(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:CancelAllCommands"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_CANCELALLCOMMANDS,
            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command
 */
OSCL_EXPORT_REF
PVMFCommandId PVMFMediaLayerNode::CancelCommand(PVMFSessionId s,
        PVMFCommandId aCmdId,
        const OsclAny* aContext)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:CancelCommand"));
    PVMFMediaLayerNodeCommand cmd;
    cmd.PVMFMediaLayerNodeCommandBase::Construct(s,
            PVMF_MEDIA_LAYER_NODE_CANCELCOMMAND,
            aCmdId,
            aContext);
    return QueueCommandL(cmd);
}


/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to
 * memory allocation failure.
 */
PVMFCommandId PVMFMediaLayerNode::QueueCommandL(PVMFMediaLayerNodeCommand& aCmd)
{
    PVMFCommandId id;
    id = iInputCommands.AddL(aCmd);
    if (IsAdded())
    {
        //wakeup the AO
        RunIfNotReady();
    }
    return id;
}

void
PVMFMediaLayerNode::MoveCmdToCurrentQueue(PVMFMediaLayerNodeCommand& aCmd)
{
    int32 err;
    OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
    if (err != OsclErrNone)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }
    iInputCommands.Erase(&aCmd);
    return;
}

/**
 * Asynchronous Command processing routines.
 * These routines are all called under the AO.
 */

/**
 * Called by the command handler AO to process a command from
 * the input queue.
 * Return true if a command was processed, false if the command
 * processor is busy and can't process another command now.
 */
bool PVMFMediaLayerNode::ProcessCommand(PVMFMediaLayerNodeCommand& aCmd)
{
    /*
     * normally this node will not start processing one command
     * until the prior one is finished.  However, a hi priority
     * command such as Cancel must be able to interrupt a command
     * in progress.
     */
    if (!iCurrentCommand.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {
        case PVMF_MEDIA_LAYER_NODE_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_INIT:
            DoInit(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_START:
            DoStart(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_STOP:
            DoStop(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_PAUSE:
            DoPause(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_RESET:
            DoReset(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_MEDIA_LAYER_NODE_CANCELCOMMAND:
            DoCancelCommand(aCmd);
            break;

        default:
        {
            /* unknown command type */
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
        }
        break;
    }

    return true;
}

/**
 * The various command handlers call this when a command is complete.
 */
void PVMFMediaLayerNode::CommandComplete(MediaLayerNodeCmdQ& aCmdQ,
        PVMFMediaLayerNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                         , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        PVMF_MEDIA_LAYER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), errormsg);
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    /* Erase the command from the queue. */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }
    /*
     * Transition to error state in case of select errors only, viz.
     * PVMFFailure, PVMFErrNoMemory, PVMFErrNoResources
     * Any other status implies that the node is probably in a recoverable
     * state
     */
    if ((aStatus == PVMFFailure) ||
            (aStatus == PVMFErrNoMemory) ||
            (aStatus == PVMFErrNoResources))
    {
        SetState(EPVMFNodeError);
    }
}

/**
 * Called by the command handler AO to do the node Reset.
 */
void PVMFMediaLayerNode::DoReset(PVMFMediaLayerNodeCommand& aCmd)
{
    LogSessionDiagnostics();

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            /* Clear queued messages in ports */
            uint32 i;
            for (i = 0; i < iPortVector.size(); i++)
            {
                iPortVector[i]->ClearMsgQueues();
                PVMFMediaLayerPortContainer* portContainerPtr = NULL;
                GetPortContainer(iPortVector[i], portContainerPtr);
                if (portContainerPtr)
                    portContainerPtr->ResetParams();
            }
        }
        /* Intentional fall thru */
        case EPVMFNodeCreated:
        case EPVMFNodeIdle:
        case EPVMFNodeInitialized:
        case EPVMFNodePrepared:
        case EPVMFNodeError:
        {
            /* delete related decryption */
            iDecryptionInterface = NULL;
            if (srcPtr != NULL)
                oscl_free((uint8*)srcPtr);
            srcPtr = NULL;

            Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;
            for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
            {
                if (it->ipFragGroupAllocator != NULL)
                    it->ipFragGroupAllocator->CancelFreeChunkAvailableCallback();
            }

            /* delete all ports and notify observer */
            while (!iPortVector.empty())
            {
                iPortVector.Erase(&iPortVector.front());
            }

            /* delete port params */
            while (!iPortParamsQueue.empty())
            {
                it = iPortParamsQueue.begin();
                if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
                {
                    DestroyPayLoadParser(&(it->iMimeType), it->iPayLoadParser);
                    if (oPortDataLog)
                    {
                        if (it->iBinAppenderPtr.GetRep() != NULL)
                        {
                            it->iPortLogger->RemoveAppender(it->iBinAppenderPtr);
                            it->iBinAppenderPtr.Unbind();
                        }
                    }
                }
                if (it->ipFragGroupAllocator != NULL)
                {
                    it->ipFragGroupAllocator->CancelFreeChunkAvailableCallback();
                    it->ipFragGroupAllocator->removeRef();
                }
                if (it->ipFragGroupMemPool != NULL)
                {
                    it->ipFragGroupMemPool->removeRef();
                }
                it->CleanUp();
                iPortParamsQueue.erase(it);
            }
            /* restore original port vector reserve */
            iPortVector.Reconstruct();

            SetState(EPVMFNodeIdle);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
 * Called by the command handler AO to do the Query UUID
 */
void PVMFMediaLayerNode::DoQueryUuid(PVMFMediaLayerNodeCommand& aCmd)
{
    /* This node supports Query UUID from any state */
    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFMediaLayerNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    /*
     * Try to match the input mimetype against any of
     * the custom interfaces for this node
     */

    /*
     * Match against custom interface1...
     * also match against base mimetypes for custom interface1,
     * unless exactmatch is set.
     */
    if (*mimetype == PVMF_MEDIALAYER_CUSTOMINTERFACE_MIMETYPE
            || (!exactmatch && *mimetype == PVMF_MEDIALAYER_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_MEDIALAYER_BASEMIMETYPE))
    {
        PVUuid uuid(PVMF_MEDIALAYERNODE_EXTENSIONINTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
 * Called by the command handler AO to do the Query Interface.
 */
void PVMFMediaLayerNode::DoQueryInterface(PVMFMediaLayerNodeCommand& aCmd)
{
    /* This node supports Query Interface from any state */
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFMediaLayerNodeCommandBase::Parse(uuid, ptr);

    if (*uuid == PVUuid(PVMF_MEDIALAYERNODE_EXTENSIONINTERFACE_UUID))
    {
        if (!iExtensionInterface)
        {
            PVMFMediaLayerNodeAllocator alloc;
            int32 err;
            OsclAny*ptr = NULL;
            OSCL_TRY(err,
                     ptr = alloc.ALLOCATE(sizeof(PVMFMediaLayerNodeExtensionInterfaceImpl));
                    );
            if (err != OsclErrNone || !ptr)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iExtensionInterface =
                OSCL_PLACEMENT_NEW(ptr, PVMFMediaLayerNodeExtensionInterfaceImpl(this));
        }

        if (iExtensionInterface->queryInterface(*uuid, *ptr))
        {
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        else
        {
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
        }
    }
    else
    {
        //not supported
        *ptr = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
    }
}

/**
 * Called by the command handler AO to do the port request
 */
void PVMFMediaLayerNode::DoRequestPort(PVMFMediaLayerNodeCommand& aCmd)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::DoRequestPort"));

    /* retrieve port tag and mime string */
    int32 tag;
    OSCL_String* portConfig;
    aCmd.PVMFMediaLayerNodeCommandBase::Parse(tag, portConfig);

    /* Allocate a new port */
    OsclAny *ptr = NULL;
    bool retVal;

    retVal = Allocate(ptr);

    if (retVal == false || !ptr)
    {
        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::DoRequestPort: Error - iPortVector Out of memory"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    OsclExclusivePtr<PVMFMediaLayerPort> portAutoPtr;
    PVMFMediaLayerPort* port = NULL;
    PVMFMediaLayerPortContainer portParams;
    OSCL_StackString<8> asf(_STRLIT_CHAR("asf"));
    OSCL_StackString<8> rtp(_STRLIT_CHAR("rtp"));

    /*
     * Odd numbered ports are output
     */
    if (tag % 2)
    {
        portParams.tag = PVMF_MEDIALAYER_PORT_TYPE_OUTPUT;
        port = OSCL_PLACEMENT_NEW(ptr, PVMFMediaLayerPort(tag, this, "MediaLayerOut"));
        portAutoPtr.set(port);
        portParams.iPort = port;
        portParams.id = tag;
        portParams.iMimeType = portConfig->get_cstr();

        // parse the mime string to find out the associated input port
        uint inputPort;
        if ((portConfig == NULL) ||
                (oscl_strncmp(portConfig->get_cstr(), asf.get_cstr(), 3) != 0))
        {
            // no mime string -> use the old mapping from output port to input port
            //   input port tag = output port tag - 1

            inputPort = tag - 1;
            portParams.iTransportType = rtp;
        }
        else
        {
            portParams.iTransportType = asf;

            if (!parseOutputPortMime(portConfig, inputPort))
            {
                PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error"
                                      " - cannot parse mime string: %s for output port %d",
                                      this, portConfig, tag));
            }

            // according to our tag scheme, the input port tag should be even
            if (inputPort % 2)
            {
                PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error"
                                      " - invalid input port tag %d in output port mime %s",
                                      this, inputPort, portConfig));
                CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
                return;
            }
        }

        // now, we have the tag for the input port - try to find it in the queue
        uint index;
        for (index = 0; index < iPortParamsQueue.size(); index++)
        {
            if ((uint)iPortParamsQueue[index].id == inputPort)
            {
                break;
            }
        }

        if (index == iPortParamsQueue.size())
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error"
                                  " - input port %d not yet commissioned (output port %d setup)",
                                  this, inputPort, tag));
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
            return;
        }

        // add the index of the input port in the counter port vector for the output port
        portParams.vCounterPorts.push_back(index);

        // create frag group allocator
        portParams.ipFragGroupMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (MEDIALAYERNODE_MAXNUM_MEDIA_DATA));
        if (portParams.ipFragGroupMemPool == NULL)
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error - Unable to allocate mempool", this));
            CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
            portAutoPtr.release();
            return;
        }
        portParams.ipFragGroupAllocator = new PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>(
            MEDIALAYERNODE_MAXNUM_MEDIA_DATA,
            MEDIALAYERNODE_MAXNUM_MEDIA_DATA,
            portParams.ipFragGroupMemPool);
        if (portParams.ipFragGroupAllocator == NULL)
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error - Unable to create frag group allocator", this));
            CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
            return;
        }
        portParams.ipFragGroupAllocator->create();

        // add the output port to the queue
        int newIndex = iPortParamsQueue.size();

        retVal = Push(portParams);
        if (retVal == false)
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error - iPortParamsQueue.push_back() failed", this));
            CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
            return;
        }

        // now, add the new output port's index to the input port's counter port vector
        iPortParamsQueue[index].vCounterPorts.push_back(newIndex);
    }
    /*
     * Even numbered ports are input
     */
    else
    {
        portParams.tag = PVMF_MEDIALAYER_PORT_TYPE_INPUT;
        port = OSCL_PLACEMENT_NEW(ptr, PVMFMediaLayerPort(tag, this, "MediaLayerIn"));
        portAutoPtr.set(port);
        portParams.iPort = port;
        portParams.id = tag;

        {
            portParams.iTransportType = rtp;
        }

        IPayloadParser* parser = CreatePayLoadParser(portConfig);
        if (parser != NULL)
        {
            portParams.iPayLoadParser = parser;
            portParams.iMimeType = portConfig->get_cstr();
        }
        else
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error - CreatePayLoadParser() failed", this));
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
            portAutoPtr.release();
            return;
        }

        if (oPortDataLog)
        {
            OSCL_StackString<32> iMLPortLoggerTag("PVMFMLNode");
            iMLPortLoggerTag += iLogFileIndex;
            iLogFileIndex += 1;
            iMLPortLoggerTag += portConfig->get_cstr();
            portParams.iPortLogger = PVLogger::GetLoggerObject(iMLPortLoggerTag.get_cstr());
            OSCL_StackString<32> MLPortLogFile;
            MLPortLogFile = portLogPath;
            MLPortLogFile += iMLPortLoggerTag.get_cstr();
            portParams.iLogFile = MLPortLogFile;

            PVLoggerAppender *binAppender =
                BinaryFileAppender::CreateAppender((char*)(portParams.iLogFile.get_cstr()));

            if (binAppender == NULL)
            {
                PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error - Binary Appender Create failed", this));
                CommandComplete(iInputCommands, aCmd, PVMFErrNoResources);
                portAutoPtr.release();
                return;
            }
            OsclRefCounterSA<PVMFMediaLayerNodeLoggerDestructDealloc>* binAppenderRefCounter =
                new OsclRefCounterSA<PVMFMediaLayerNodeLoggerDestructDealloc>(binAppender);

            OsclSharedPtr<PVLoggerAppender> appenderSharedPtr(binAppender, binAppenderRefCounter);
            portParams.iBinAppenderPtr = appenderSharedPtr;
            portParams.iPortLogger->AddAppender(portParams.iBinAppenderPtr);
        }

        retVal = Push(portParams);
        if (retVal == false)
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DoRequestPort: Error - iPortParamsQueue.push_back() failed", this));
            CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
            return;
        }
    }

    /* Add the port to the port vector. */
    retVal = AddPort(port);

    if (retVal == false)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        portAutoPtr.release();
        return;
    }

    portAutoPtr.release();

    /* Return the port pointer to the caller. */
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port);
}

bool PVMFMediaLayerNode::Allocate(OsclAny*& ptr)
{
    int32 err;
    OSCL_TRY(err, ptr = iPortVector.Allocate(););
    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}

bool PVMFMediaLayerNode::Push(PVMFMediaLayerPortContainer portParams)
{
    int32 err;
    OSCL_TRY(err, iPortParamsQueue.push_back(portParams););
    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}

bool PVMFMediaLayerNode::AddPort(PVMFMediaLayerPort* port)
{
    int32 err;
    OSCL_TRY(err, iPortVector.AddL(port););
    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}
/**
 * Called by the command handler AO to do the port release
 */
void PVMFMediaLayerNode::DoReleasePort(PVMFMediaLayerNodeCommand& aCmd)
{
    /* This node supports release port from any state */

    /* Find the port in the port vector */
    PVMFPortInterface* p = NULL;
    aCmd.PVMFMediaLayerNodeCommandBase::Parse(p);

    PVMFMediaLayerPort* port = (PVMFMediaLayerPort*)p;

    PVMFMediaLayerPort** portPtr = iPortVector.FindByValue(port);
    if (portPtr)
    {
        /* delete corresponding port params */
        Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;

        for (it = iPortParamsQueue.begin();
                it != iPortParamsQueue.end();
                it++)
        {
            if (it->iPort == port)
            {
                if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
                {
                    DestroyPayLoadParser(&(it->iMimeType), it->iPayLoadParser);
                    if (oPortDataLog)
                    {
                        if (it->iBinAppenderPtr.GetRep() != NULL)
                        {
                            it->iPortLogger->RemoveAppender(it->iBinAppenderPtr);
                            it->iBinAppenderPtr.Unbind();
                        }
                    }
                }
                if (it->ipFragGroupAllocator != NULL)
                {
                    it->ipFragGroupAllocator->CancelFreeChunkAvailableCallback();
                    it->ipFragGroupAllocator->removeRef();
                }
                if (it->ipFragGroupMemPool != NULL)
                {
                    it->ipFragGroupMemPool->removeRef();
                }
                it->CleanUp();
                iPortParamsQueue.erase(it);
                break;
            }
        }
        iPortVector.Erase(portPtr);
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        /* port not found */
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
    }
}

/**
 * Called by the command handler AO to do the node Init
 */
void PVMFMediaLayerNode::DoInit(PVMFMediaLayerNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            SetState(EPVMFNodeInitialized);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}
/**

 * Called by the command handler AO to do the node Prepare
 */
void PVMFMediaLayerNode::DoPrepare(PVMFMediaLayerNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        {
            SetState(EPVMFNodePrepared);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

IPayloadParser*
PVMFMediaLayerNode::CreatePayLoadParser(PvmfMimeString* aPortConfig)
{
    PayloadParserRegistry* registry = iPayLoadParserRegistry;

    if (registry == NULL)
    {
        PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::CreatePayLoadParser: Error - Invalid Registry", this));
        return NULL;
    }

    OsclMemoryFragment memFrag;
    memFrag.ptr = (OsclAny*)(aPortConfig->get_cstr());
    memFrag.len = aPortConfig->get_size();
    IPayloadParserFactory* factory = registry->lookupPayloadParserFactory(memFrag);
    if (factory == NULL)
    {
        PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::CreatePayLoadParser: Error - Invalid Factory", this));
        return NULL;
    }
    return (factory->createPayloadParser());
}

void
PVMFMediaLayerNode::DestroyPayLoadParser(PvmfMimeString* aPortConfig,
        IPayloadParser* aParser)
{
    PayloadParserRegistry* registry = iPayLoadParserRegistry;

    if (registry == NULL)
    {
        PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DestroyPayLoadParser: Error - Invalid Registry", this));
        OSCL_LEAVE(OsclErrBadHandle);
    }

    OsclMemoryFragment memFrag;
    memFrag.ptr = (OsclAny*)(aPortConfig->get_cstr());
    memFrag.len = aPortConfig->get_size();
    IPayloadParserFactory* factory =
        registry->lookupPayloadParserFactory(memFrag);
    if (factory == NULL)
    {
        PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::DestroyPayLoadParser: Error - Invalid Factory", this));
        OSCL_LEAVE(OsclErrBadHandle);
    }
    factory->destroyPayloadParser(aParser);
}

/**
 * Called by the command handler AO to do the node Start
 */
void PVMFMediaLayerNode::DoStart(PVMFMediaLayerNodeCommand& aCmd)
{
    iDiagnosticsLogged = false;
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
        {
            /* transition to Started */
            SetState(EPVMFNodeStarted);
        }
        break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}

/**
 * Called by the command handler AO to do the node Stop
 */
void PVMFMediaLayerNode::DoStop(PVMFMediaLayerNodeCommand& aCmd)
{
    LogSessionDiagnostics();

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            /* Clear queued messages in ports */
            uint32 i;
            for (i = 0; i < iPortVector.size(); i++)
            {
                iPortVector[i]->ClearMsgQueues();

                PVMFMediaLayerPortContainer* portContainerPtr = NULL;
                bool bRet = GetPortContainer(iPortVector[i], portContainerPtr);
                if (bRet)
                {
                    portContainerPtr->ResetParams();
                    portContainerPtr->vAccessUnits.clear();
                }
            }
            /* transition to Prepared state */
            Oscl_Int64_Utils::set_uint64(preroll64, 0, 0);
            oEOSsendunits = false;
            SetState(EPVMFNodePrepared);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
 * Called by the command handler AO to do the node Flush
 */
void PVMFMediaLayerNode::DoFlush(PVMFMediaLayerNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            /*
             * the flush is asynchronous.  move the command from
             * the input command queue to the current command, where
             * it will remain until the flush completes.
             */
            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iInputCommands.Erase(&aCmd);
            /* Notify all ports to suspend their input */
            for (uint32 i = 0;i < iPortVector.size();i++)
                iPortVector[i]->SuspendInput();
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
 * A routine to tell if a flush operation is in progress.
 */
bool PVMFMediaLayerNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_MEDIA_LAYER_NODE_FLUSH);
}


/**
 * Called by the command handler AO to do the node Pause
 */
void PVMFMediaLayerNode::DoPause(PVMFMediaLayerNodeCommand& aCmd)
{
    PVMFStatus status;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        {
            /* transition to paused state */
            SetState(EPVMFNodePaused);
            status = PVMFSuccess;
        }
        break;

        case EPVMFNodePaused:
            /* Ignore Pause if already paused */
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }
    CommandComplete(iInputCommands, aCmd, status);
    return;
}

/**
 * Called by the command handler AO to do the Cancel All
 */
void PVMFMediaLayerNode::DoCancelAllCommands(PVMFMediaLayerNodeCommand& aCmd)
{
    /* first cancel the current command if any */
    {
        while (!iCurrentCommand.empty())
            CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFErrCancelled);
    }

    /* next cancel all queued commands */
    {
        /* start at element 1 since this cancel command is element 0 */
        while (iInputCommands.size() > 1)
            CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
    }

    /* finally, report cancel complete */
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
 * Called by the command handler AO to do the Cancel single command
 */
void PVMFMediaLayerNode::DoCancelCommand(PVMFMediaLayerNodeCommand& aCmd)
{
    /* extract the command ID from the parameters */
    PVMFCommandId id;
    aCmd.PVMFMediaLayerNodeCommandBase::Parse(id);

    /* first check "current" command if any */
    {
        PVMFMediaLayerNodeCommand* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {
            /* cancel the queued command */
            CommandComplete(iCurrentCommand, *cmd, PVMFErrCancelled);
            /* report cancel success */
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            return;
        }
    }

    /* next check input queue */
    {
        /* start at element 1 since this cancel command is element 0 */
        PVMFMediaLayerNodeCommand* cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            /* cancel the queued command */
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
            /* report cancel success */
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            return;
        }
    }
    /* if we get here the command isn't queued so the cancel fails */
    CommandComplete(iInputCommands, aCmd, PVMFFailure);
}


/////////////////////////////////////////////////////
// Event reporting routines.
/////////////////////////////////////////////////////
void PVMFMediaLayerNode::SetState(TPVMFNodeInterfaceState s)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}

void PVMFMediaLayerNode::ReportErrorEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode:NodeErrorEvent Type %d Data %d"
                          , aEventType, aEventData));

    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg;
        PVMF_MEDIA_LAYER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), eventmsg);
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

void PVMFMediaLayerNode::ReportInfoEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode:NodeInfoEvent Type %d Data %d"
                         , aEventType, aEventData));

    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg;
        PVMF_MEDIA_LAYER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), eventmsg);
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

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////
void PVMFMediaLayerNode::QueuePortActivity(PVMFMediaLayerPortContainer* aPortContainer,
        const PVMFPortActivity &aActivity)
{
    OSCL_UNUSED_ARG(aPortContainer);
    OSCL_UNUSED_ARG(aActivity);
    if (IsAdded())
    {
        RunIfNotReady();
    }
}

void PVMFMediaLayerNode::freechunkavailable(PVMFPortInterface* aPort)
{
    PVMF_MLNODE_LOGINFO((0, "0x%x PVMFMediaLayerNode::freechunkavailable: port=0x%x",
                         this, aPort));

    PVMFMediaLayerPortContainer* portContainerPtr = NULL;

    if (!GetPortContainer(aPort, portContainerPtr))
    {
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aPort));
        PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::freechunkavailable: Error - GetPortContainer failed", this));
        return;
    }

    portContainerPtr->oProcessIncomingMessages = true;
    PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                         "PVMFMediaLayerNode::freechunkavailable: Mime=%s, QSize =%d", portContainerPtr->iMimeType.get_cstr(), aPort->IncomingMsgQueueSize()));
    /*
     * An input port will call this when memory is available in the rtp
     * payload parser.  We may need to wake up the input data processing.
     */
    if (IsAdded())
    {
        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                             "PVMFMediaLayerNode::freechunkavailable: - Calling RunIfNotReady"));
        RunIfNotReady();
    }
}

void PVMFMediaLayerNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::PortActivity: port=0x%x, type=%d",
                         aActivity.iPort, aActivity.iType));

    PVMFMediaLayerPortContainer* portContainerPtr = NULL;

    if (aActivity.iType != PVMF_PORT_ACTIVITY_DELETED)
    {
        if (!GetPortContainer(aActivity.iPort, portContainerPtr))
        {
            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
            PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HPA: Error - GetPortContainer failed"));
            return;
        }
    }

    /*
     * A port is reporting some activity or state change.  This code
     * figures out whether we need to queue a processing event
     * for the AO, and/or report a node event to the observer.
     */

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CREATED:
            /*
             * Report port created info event to the node.
             */
            ReportInfoEvent(PVMFInfoPortCreated, (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_DELETED:
        {
            /*
             * Report port deleted info event to the node.
             */
            ReportInfoEvent(PVMFInfoPortDeleted, (OsclAny*)aActivity.iPort);
        }
        break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
        {
            /*
             * An outgoing message was queued on this port.
             * Only output ports have outgoing messages in
             * this node
             */
            int32 portTag = portContainerPtr->tag;
            switch (portTag)
            {
                case PVMF_MEDIALAYER_PORT_TYPE_OUTPUT:
                    if (portContainerPtr->oProcessOutgoingMessages)
                        QueuePortActivity(portContainerPtr, aActivity);
                    break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
        {
            /*
             * An incoming message was queued on this port.
             * Input ports only get incoming messages in this
             * node
             */
            int32 portTag = portContainerPtr->tag;

            switch (portTag)
            {
                case PVMF_MEDIALAYER_PORT_TYPE_INPUT:
                    if (portContainerPtr->oProcessIncomingMessages)
                        QueuePortActivity(portContainerPtr, aActivity);
                    break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
        {
            int32 portTag = portContainerPtr->tag;
            switch (portTag)
            {
                case PVMF_MEDIALAYER_PORT_TYPE_INPUT:
                    /*
                     * Input ports do not use their outgoing queues
                     * in this node.
                     */
                    OSCL_ASSERT(false);
                    break;

                case PVMF_MEDIALAYER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port cannot accept any more
                     * msgs on its outgoing queue. This implies that the corresponding
                     * input port must stop processing messages.
                     */
                    PVMFPortInterface* inPort = getPortCounterpart(portContainerPtr);
                    if (inPort != NULL)
                    {
                        PVMFMediaLayerPortContainer* inPortContainerPtr = NULL;
                        if (!GetPortContainer(inPort, inPortContainerPtr))
                        {
                            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                            PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - GetPortContainer failed"));
                            return;
                        }
                        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::HPA: Output Port Queue Busy - Mime=%s",
                                                             inPortContainerPtr->iMimeType.get_cstr()));
                        inPortContainerPtr->oProcessIncomingMessages = false;
                    }
                    else
                    {
                        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - getPortCounterpart failed"));
                        return;
                    }
                }
                break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
        {
            /*
             * Outgoing queue was previously busy, but is now ready.
             * We may need to schedule new processing events depending
             * on the port type.
             */
            int32 portTag = portContainerPtr->tag;
            switch (portTag)
            {
                case PVMF_MEDIALAYER_PORT_TYPE_INPUT:
                    /*
                     * Input ports do not use their outgoing queues
                     * in this node.
                     */
                    OSCL_ASSERT(false);
                    break;

                case PVMF_MEDIALAYER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port can accept more
                     * msgs on its outgoing queue. This implies that the corresponding
                     * input port can start processing messages again.
                     */
                    PVMFPortInterface* inPort = getPortCounterpart(portContainerPtr);
                    if (inPort != NULL)
                    {
                        PVMFMediaLayerPortContainer* inPortContainerPtr = NULL;
                        if (!GetPortContainer(inPort, inPortContainerPtr))
                        {
                            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                            PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - GetPortContainer failed"));
                            return;
                        }
                        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::HPA: Output Port Queue Ready - Mime=%s",
                                                             inPortContainerPtr->iMimeType.get_cstr()));
                        inPortContainerPtr->oProcessIncomingMessages = true;
                    }
                    else
                    {
                        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - getPortCounterpart failed"));
                        return;
                    }
                }
                break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
            if (IsAdded())
            {
                RunIfNotReady();
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
        {
            /*
             * The connected port has become busy (its incoming queue is
             * busy).
             */
            int32 portTag = portContainerPtr->tag;
            switch (portTag)
            {
                case PVMF_MEDIALAYER_PORT_TYPE_INPUT:
                    /*
                     * Input ports do not use their outgoing queues
                     * in this node.
                     */
                    OSCL_ASSERT(false);
                    break;

                case PVMF_MEDIALAYER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port cannot send any more
                     * msgs from its outgoing queue. It should stop processing
                     * messages till the connect port is ready.
                     */
                    portContainerPtr->oProcessOutgoingMessages = false;
                    PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::HPA: Connected port busy - Mime=%s",
                                                         portContainerPtr->iMimeType.get_cstr()));
                    PVMFPortInterface* inPort = getPortCounterpart(portContainerPtr);
                    if (inPort != NULL)
                    {
                        PVMFMediaLayerPortContainer* inPortContainerPtr = NULL;
                        if (!GetPortContainer(inPort, inPortContainerPtr))
                        {
                            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                            PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - GetPortContainer failed"));
                            return;
                        }
                        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::HPA: Connected port busy - Stop Input - Mime=%s",
                                                             inPortContainerPtr->iMimeType.get_cstr()));
                        if (checkOutputPortsBusy(inPortContainerPtr))
                        {
                            inPortContainerPtr->oProcessIncomingMessages = false;
                        }
                    }
                    else
                    {
                        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - getPortCounterpart failed"));
                        return;
                    }
                }
                break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
        {
            /*
             * The connected port has transitioned from Busy to Ready.
             * It's time to start processing messages outgoing again.
             */
            int32 portTag = portContainerPtr->tag;
            switch (portTag)
            {
                case PVMF_MEDIALAYER_PORT_TYPE_INPUT:
                    /*
                     * Input ports do not use their outgoing queues
                     * in this node.
                     */
                    OSCL_ASSERT(false);
                    break;

                case PVMF_MEDIALAYER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port can now send
                     * msgs from its outgoing queue. It can start processing
                     * messages now.
                     */
                    portContainerPtr->oProcessOutgoingMessages = true;
                    PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::HPA: Connected port ready - Mime=%s",
                                                         portContainerPtr->iMimeType.get_cstr()));
                    PVMFPortInterface* inPort = getPortCounterpart(portContainerPtr);
                    if (inPort != NULL)
                    {
                        PVMFMediaLayerPortContainer* inPortContainerPtr = NULL;
                        if (!GetPortContainer(inPort, inPortContainerPtr))
                        {
                            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                            PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - GetPortContainer failed"));
                            return;
                        }
                        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::HPA: Connected port busy - Resume Input - Mime=%s",
                                                             inPortContainerPtr->iMimeType.get_cstr()));
                        inPortContainerPtr->oProcessIncomingMessages = true;
                    }
                    else
                    {
                        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
                        PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::HandlePortActivity: Error - getPortCounterpart failed"));
                        return;
                    }
                }
                break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
            if (IsAdded())
            {
                RunIfNotReady();
            }
        }
        break;


        default:
            break;
    }
}

/*
 * called by the AO to process a port activity message
 */
bool PVMFMediaLayerNode::ProcessPortActivity(PVMFMediaLayerPortContainer* aPortContainer)
{
    PVMFStatus status = PVMFSuccess;
    switch (aPortContainer->tag)
    {
        case PVMF_MEDIALAYER_PORT_TYPE_OUTPUT:
        {
            if ((aPortContainer->oProcessOutgoingMessages) &&
                    (aPortContainer->iPort->OutgoingMsgQueueSize() > 0))
            {
                status = ProcessOutgoingMsg(aPortContainer);
            }
            else
            {
                status = PVMFErrBusy;
            }
        }
        break;

        case PVMF_MEDIALAYER_PORT_TYPE_INPUT:
        {
            /*
             * attempt to send any left over payloads first
             * If both output port payload msg is not available or outgoing queue is busy,
             * oProcessIncomingMessages is set to false and PVMFErrBusy is replied.
             */
            status = sendAccessUnits(aPortContainer);
            /*
             * Process any incoming messages if the output ports are not busy
             * and if there are media msgs available
             */
            if (status == PVMFSuccess && aPortContainer->iPort->IncomingMsgQueueSize() > 0)
            {
                if (aPortContainer->oProcessIncomingMessages)
                {
                    status = ProcessIncomingMsg(aPortContainer);
                }
                else
                {
                    status = PVMFErrBusy;
                }
            }
        }
        break;

        default:
            OSCL_ASSERT(false);
            break;
    }

    /* report a failure in port processing */
    if (status != PVMFErrBusy && status != PVMFSuccess)
    {
        PVMF_MLNODE_LOGERROR((0,
                              "PVMFMediaLayerNode::ProcessPortActivity: Error - ProcessPortActivity failed. port=0x%x",
                              aPortContainer->iPort));
        ReportErrorEvent(PVMFErrPortProcessing);
    }

    /* return true if we processed an activity */
    return (status != PVMFErrBusy);
}

PVMFStatus PVMFMediaLayerNode::ProcessIncomingMsg(PVMFMediaLayerPortContainer* pinputPort)
{
    PVMFStatus status;

    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::ProcessIncomingMsg: aPort=0x%x",
                         pinputPort->iPort));

    if (pinputPort->tag != PVMF_MEDIALAYER_PORT_TYPE_INPUT)
    {
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(pinputPort->iPort));
        PVMF_MLNODE_LOGERROR((0,
                              "PVMFMediaLayerNode::ProcessIncomingMsg: Error - Not an Input Port"));
        return PVMFFailure;
    }

    //
    // Check if the output port is busy.
    //
    if (checkOutputPortsBusy(pinputPort))
    {
        PVUuid eventuuid = PVMFMediaLayerNodeEventTypeUUID;

        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                             "PVMFMediaLayerNode::ProcessIncomingMsg: Cant Send Data - Output Queue Busy"));
        /*
         * Processing will resume when we get outgoing queue ready notification
         * in the port activity
         */
        pinputPort->oProcessIncomingMessages = false;
        return PVMFErrBusy;
    }

    //
    // dequeue the message
    // if it is EOS leave it on the port queue until we are
    // done sending all payloads
    //
    PVMFSharedMediaMsgPtr msgIn;
    PVMFSharedMediaDataPtr peekDataPtr;
    if (!oEOSsendunits)
    {
        bool oIsEOSCmd = false;

        PVMFMediaLayerPort* mlPort =
            OSCL_STATIC_CAST(PVMFMediaLayerPort*, pinputPort->iPort);
        status = mlPort->peekHead(peekDataPtr, oIsEOSCmd);
        if (status != PVMFSuccess)
        {
            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(pinputPort->iPort));
            PVMF_MLNODE_LOGERROR((0,
                                  "0x%x PVMFMediaLayerNode::ProcessIncomingMsg: Error - peekHead failed", this));
            return status;
        }
        if (oIsEOSCmd == true)
        {
            PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::ProcessIncomingMsg() Detect EOS message"));

            //EOS msg is received. Before sending EOS, we send left payload msgs.
            bool IsAccessUnitsEmpty = false;
            status = checkPortCounterpartAccessUnitQueue(pinputPort, &IsAccessUnitsEmpty);
            if (status != PVMFSuccess)
            {
                ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(pinputPort->iPort));
                PVMF_MLNODE_LOGERROR((0,
                                      "0x%x PVMFMediaLayerNode::ProcessIncomingMsg: Error - checkPortCounterpartAccessUnitQueue failed", this));
                return status;
            }
            if (IsAccessUnitsEmpty)
            {
                oEOSsendunits = true;
                return sendAccessUnits(pinputPort);
            }
        }
    }
    else
    {
        bool IsAccessUnitsEmpty = false;
        status = checkPortCounterpartAccessUnitQueue(pinputPort, &IsAccessUnitsEmpty);
        if (status != PVMFSuccess)
        {
            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(pinputPort->iPort));
            PVMF_MLNODE_LOGERROR((0,
                                  "0x%x PVMFMediaLayerNode::ProcessIncomingMsg: Error - checkPortCounterpartAccessUnitQueue failed", this));
            return status;
        }
        if (IsAccessUnitsEmpty)
        {
            return sendAccessUnits(pinputPort);
        }
        else
        {
            oEOSsendunits = false;
        }
    }
    status = pinputPort->iPort->DequeueIncomingMsg(msgIn);
    if (status != PVMFSuccess)
    {
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(pinputPort->iPort));
        PVMF_MLNODE_LOGERROR((0,
                              "0x%x PVMFMediaLayerNode::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this));
        return status;
    }
    //
    // what kind of demuxing are we doing?
    //
    bool isOneToN = pinputPort->iIsOneToN;

    PVUid32 msgFormatID = msgIn->getFormatID();

    if (msgFormatID > PVMF_MEDIA_CMD_FORMAT_IDS_START)
    {
        if (msgFormatID == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
        {
            iStreamID = msgIn->getStreamID();
            PVMF_MLNODE_LOGDATATRAFFIC_IN((0, "PVMFMediaLayerNode::ProcessInputMsg_OneToN: Sending MediaCmd - BOS Received"));
            PVMF_MLNODE_LOG_REPOS((0, "PVMFMediaLayerNode::ProcessInputMsg_OneToN: Sending MediaCmd - BOS Received"));
            // send BOS on every output port
            for (uint i = 0; i < pinputPort->vCounterPorts.size(); i++)
            {
                PVMFMediaLayerPortContainer* poutPort = &this->iPortParamsQueue[pinputPort->vCounterPorts[i]];
                // set BOS Timestamp to last media ts sent on this port
                msgIn->setTimestamp(poutPort->iContinuousTimeStamp);
                status = poutPort->iPort->QueueOutgoingMsg(msgIn);

                if (status != PVMFSuccess)
                {
                    PVMF_MLNODE_LOGERROR((0,
                                          "0x%x PVMFMediaLayerNode::ProcessInputMsg_OneToN: "
                                          "Error - QueueOutgoingMsg for BOS failed", this));
                    return status;
                }
                else
                {
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                    uint32 timebase32 = 0;
                    uint32 clientClock32 = 0;
                    bool overflowFlag = false;
                    iClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag,
                                                           PVMF_MEDIA_CLOCK_MSEC, timebase32);
                    PVMF_MLNODE_LOGDATATRAFFIC_OUT((0,
                                                    "PVMFMediaLayerNode::ProcessInputMsg_OneToN: Sending BOS - MimeType=%s Clock=%d",
                                                    poutPort->iMimeType.get_cstr(), clientClock32));
                    PVMF_MLNODE_LOG_REPOS((0,
                                           "PVMFMediaLayerNode::ProcessInputMsg_OneToN: Sending BOS - MimeType=%s Clock=%d",
                                           poutPort->iMimeType.get_cstr(), clientClock32));
#endif
                }
            }

        }
        else if (msgFormatID == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
        {
            //
            // check for EOS
            //
            // Upstream EOS recvd
            pinputPort->oUpStreamEOSRecvd = true;
            pinputPort->oEOSReached = true;

            // send EOS on every output port
            for (uint i = 0; i < pinputPort->vCounterPorts.size(); i++)
            {
                PVMFMediaLayerPortContainer* poutPort = &this->iPortParamsQueue[pinputPort->vCounterPorts[i]];
                // set EOS Timestamp to last media ts sent on this port
                if (poutPort->oDisableTrack == false && poutPort->oEOSReached == false)
                {
                    poutPort->iContinuousTimeStamp += PVMF_MEDIA_LAYER_NODE_ASF_REPOS_TIME_OFFSET_IN_MS;
                    msgIn->setTimestamp(poutPort->iContinuousTimeStamp);
                    msgIn->setStreamID(iStreamID);
                    status = poutPort->iPort->QueueOutgoingMsg(msgIn);

                    if (status != PVMFSuccess)
                    {
                        PVMF_MLNODE_LOGERROR((0,
                                              "0x%x PVMFMediaLayerNode::ProcessInputMsg_OneToN: "
                                              "Error - QueueOutgoingMsg for EOS failed", this));
                        return status;
                    }
                    else
                    {
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                        uint32 timebase32 = 0;
                        uint32 clientClock32 = 0;
                        bool overflowFlag = false;
                        iClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag,
                                                               PVMF_MEDIA_CLOCK_MSEC, timebase32);
                        PVMF_MLNODE_LOGDATATRAFFIC_OUT((0,
                                                        "PVMFMediaLayerNode::ProcessInputMsg_OneToN: Sending EOS - MimeType=%s, StreamId=%d, Clock=%d",
                                                        pinputPort->iMimeType.get_cstr(),
                                                        msgIn->getStreamID(),
                                                        clientClock32));
#endif
                    }
                }
            }
        }
        else
        {

            /* unknown command - pass it along */
            for (uint i = 0; i < pinputPort->vCounterPorts.size(); i++)
            {
                PVMFMediaLayerPortContainer* poutPort = &this->iPortParamsQueue[pinputPort->vCounterPorts[i]];
                status = poutPort->iPort->QueueOutgoingMsg(msgIn);

                if (status != PVMFSuccess)
                {
                    PVMF_MLNODE_LOGERROR((0,
                                          "0x%x PVMFMediaLayerNode::ProcessInputMsg_OneToN: "
                                          "Error - QueueOutgoingMsg for MediaCmd=%d failed", msgFormatID, this));
                    return status;
                }
                else
                {
                    poutPort->iReConfig = false;
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                    uint32 timebase32 = 0;
                    uint32 clientClock32 = 0;
                    bool overflowFlag = false;
                    iClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
                    PVMF_MLNODE_LOGDATATRAFFIC_OUT((0,
                                                    "PVMFMediaLayerNode::ProcessInputMsg_OneToN: Sending MediaCmd - CmdId=%d, MimeType=%s Clock=%2d",
                                                    msgFormatID, pinputPort->iMimeType.get_cstr(), clientClock32));
#endif
                }
            }
        }
        return status;
    }

    //If error is detected in track config info, EOS should be sent.
    for (uint k = 0; k < pinputPort->vCounterPorts.size(); k++)
    {
        PVMFMediaLayerPortContainer* poutPort = &this->iPortParamsQueue[pinputPort->vCounterPorts[k]];
        if (poutPort->oDisableTrack == true && poutPort->oDetectBrokenTrack == true)
        {
            poutPort->oDetectBrokenTrack = false;
            poutPort->iFirstFrameAfterRepositioning = false;
            poutPort->iContinuousTimeStamp += PVMF_MEDIA_LAYER_NODE_ASF_REPOS_TIME_OFFSET_IN_MS;
            status = sendEndOfTrackCommand(poutPort);
            if (status != PVMFSuccess)
            {
                PVMF_MLNODE_LOGERROR((0,
                                      "0x%x PVMFMediaLayerNode::ProcessInputMsg_OneToN: "
                                      "Error - QueueOutgoingMsg for EOS failed", this));
                return status;
            }
        }
    }

    if (pinputPort->oEOSReached == true)
    {
        PVMF_MLNODE_LOGWARNING((0,
                                "PVMFMediaLayerNode::ProcessInputMsg_OneToN: "
                                "Msg too late, discard - EOS already sent"));
        return PVMFSuccess;
    }

    //
    // unpack the payload data
    //
    PVMFSharedMediaDataPtr dataIn;
    convertToPVMFMediaData(dataIn, msgIn);
    iPayLoad.vfragments.clear();
    iPayLoad.stream = dataIn->getStreamID();
    iPayLoad.marker = (dataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT);
    if (dataIn->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT)
        iPayLoad.randAccessPt = true;
    iPayLoad.sequence = dataIn->getSeqNum();
    iPayLoad.timestamp = dataIn->getTimestamp();
    pinputPort->iCurrFormatId = dataIn->getFormatID();
    OsclRefCounterMemFrag fsi;
    dataIn->getFormatSpecificInfo(fsi);

    pinputPort->iCurrFormatSpecInfo = fsi;

    for (uint f = 0; f < dataIn->getNumFragments(); f++)
    {
        OsclRefCounterMemFrag memFrag;
        dataIn->getMediaFragment(f, memFrag);
        iPayLoad.vfragments.push_back(memFrag);
    }

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
    uint32 inputDataSize = 0;
    /* Get input size for logging */
    uint32 inputNumFrags = dataIn->getNumFragments();
    for (uint32 i = 0; i < inputNumFrags; i++)
    {
        OsclRefCounterMemFrag memFrag;
        dataIn->getMediaFragment(i, memFrag);
        inputDataSize += memFrag.getMemFragSize();
    }

    uint32 timebase32 = 0;
    uint32 clientClock32 = 0;
    bool overflowFlag = false;
    iClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    PVMF_MLNODE_LOGDATATRAFFIC_IN((0,
                                   "PVMFMediaLayerNode::ProcessInputMsg_OneToN - Input: MimeType=%s, TS=%d, SEQNUM=%d, SIZE=%d, Clock=%d",
                                   pinputPort->iMimeType.get_cstr(), msgIn->getTimestamp(), msgIn->getSeqNum(), inputDataSize, clientClock32));
#endif

    PVMFMediaLayerPortContainer* poutPort = NULL;
    bool bRet = false;
    if (isOneToN == false)
    {
        bRet = GetPortContainer(getPortCounterpart(pinputPort), poutPort);
    }

    //
    // parse the payload data until all payloads are extracted.
    // in case of http streaming, we parse audio & video data to each output port access units.
    //
    PayloadParserStatus retVal = PayloadParserStatus_InputNotExhausted;
    while (retVal == PayloadParserStatus_InputNotExhausted)
    {
        {
            if (bRet)
            {
                retVal =
                    pinputPort->iPayLoadParser->Parse(iPayLoad,
                                                      poutPort->vAccessUnits);
            }
        }

        if ((retVal == PayloadParserStatus_MemorAllocFail) ||
                (retVal == PayloadParserStatus_Failure))
        {
            PVUuid eventuuid = PVMFMediaLayerNodeEventTypeUUID;
            int32 infocode = PVMFMediaLayerNodePayloadParserError;
            ReportInfoEvent(PVMFErrProcessing, (OsclAny*)(&(pinputPort->iMimeType)), &eventuuid, &infocode);
            PVMF_MLNODE_LOGWARNING((0,
                                    "PVMFMediaLayerNode::ProcessIncomingMsg: Error - parse Payload failed ErrCode = %d", retVal));
            status = PVMFFailure;
        }
    }

    if (retVal != PayloadParserStatus_Success)
    {
        // no data ready yet
        return PVMFSuccess;
    }

    return sendAccessUnits(pinputPort);
}


PVMFStatus PVMFMediaLayerNode::sendAccessUnits(PVMFMediaLayerPortContainer* pinputPort)
{

    PVMFStatus status = PVMFSuccess;
    bool checkAccessUnitsSize = false;

    for (uint i = 0; i < pinputPort->vCounterPorts.size(); i++)
    {
        PVMFMediaLayerPortContainer* poutPort =
            &this->iPortParamsQueue[pinputPort->vCounterPorts[i]];

        if (poutPort->vAccessUnits.size() > 0)
        {
            checkAccessUnitsSize = true;

            if ((poutPort->iPort->IsOutgoingQueueBusy() == false) &&
                    (poutPort->ipFragGroupAllocator->IsMsgAvailable() == true))
            {
                //send each access unit to its respective port
                status = dispatchAccessUnits(pinputPort, poutPort);
            }
            else
            {
                PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                                     "PVMFMediaLayerNode::sendAccessUnits: Port / Mem Pool Busy - Mime=%s vQ=%d",
                                                     poutPort->iMimeType.get_cstr(), poutPort->vAccessUnits.size()));
                status = PVMFErrBusy;
            }
            if (status == PVMFErrBusy)
            {
                // Processing will resume when we get freechunkavailable notice from the port.
                pinputPort->oProcessIncomingMessages = false;
                {
                    if (!poutPort->ipFragGroupAllocator->IsMsgAvailable())
                    {
                        poutPort->ipFragGroupAllocator->notifyfreechunkavailable(*((PVMFMediaLayerPort*)pinputPort->iPort));
                        PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                                             "PVMFMediaLayerNode::sendAccessUnits: Calling notifyfreechunkavailable - Mime=%s vQ=%d",
                                                             poutPort->iMimeType.get_cstr(), poutPort->vAccessUnits.size()));
                    }
                }
                if (pinputPort->oProcessIncomingMessages == false)
                {
                    // no media msgs available on any output port.
                    // then we set oProcessIncomingMessages as false not to call ProcessIncomingMsg.
                    status = PVMFErrBusy;
                    break;
                }
                else
                {
                    //at least, media msgs on one output port is available and we set oProcessIncomingMessages to true again.
                    status = PVMFSuccess;
                    continue;
                }
            }
        }
    }
    //in case that both output port vAccessUnits is empty and oProcessIncomingMessages is set as false,
    //we never set oProcessIncomingMessages to true and cannot call ProcessIncomingMsg.
    //to escape this situation, we check IsMsgAvailable for both output port here.
    if (checkAccessUnitsSize == false)
    {
        if (pinputPort->oProcessIncomingMessages == false)
        {
            if (pinputPort->oProcessIncomingMessages == false)
            {
                // no media msgs available on any output port
                status = PVMFErrBusy;
            }
            else
            {
                //at least, media msgs on one output port is available and we set oProcessIncomingMessages to true again.
                status = PVMFSuccess;
            }
        }
    }
    return status;
}

PVMFStatus PVMFMediaLayerNode::dispatchAccessUnits(PVMFMediaLayerPortContainer* pinputPort,
        PVMFMediaLayerPortContainer* poutPort)
{
    PVMFStatus status = PVMFSuccess;
    //
    // send each access unit to its respective port
    //
    typedef Oscl_Vector<IPayloadParser::Payload, OsclMemAllocator>::iterator iterator_type;
    iterator_type it;
    for (it = poutPort->vAccessUnits.begin(); it != poutPort->vAccessUnits.end(); it++)
    {
        // check out port status
        if (poutPort->iPort->IsOutgoingQueueBusy())
        {
            PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                                 "PVMFMediaLayerNode::dispatchAccessUnits: Output Port Busy - Mime=%s", poutPort->iMimeType.get_cstr()));
            status = PVMFErrBusy;
            break;
        }
        //
        // check if a media message is available
        //
        if (!poutPort->ipFragGroupAllocator->IsMsgAvailable())
        {
            PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0, "PVMFMediaLayerNode::dispatchAccessUnits: Can't send - Waiting for free buffers - Mime=%s vQ=%d", poutPort->iMimeType.get_cstr(), poutPort->vAccessUnits.size()));
            status = PVMFErrBusy;
            break;
        }

        // retrieve a data implementation
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImplOut;
        bool retVal;
        retVal = Allocate(mediaDataImplOut, poutPort);

        if (retVal == false)
        {
            status = PVMFErrNoMemory;
            break;
        }

        // Once we detect received DATA is same as reconfig streamID, we send reconfig msg first.
        // Reconfig streamID is informed from SM node at switchstream.
        if (poutPort->iReConfig == true && poutPort->oReconfigId == it->stream)
        {
            if (poutPort->oMsgReconfig.GetRep() != NULL)
            {
                PVMFSharedMediaDataPtr mediaData;
                convertToPVMFMediaData(mediaData, poutPort->oMsgReconfig);

                status = poutPort->iPort->QueueOutgoingMsg(poutPort->oMsgReconfig);

                if (status != PVMFSuccess)
                {
                    PVMF_MLNODE_LOGERROR((0,
                                          "0x%x PVMFMediaLayerNode::dispatchAccessUnits: "
                                          "Error - QueueOutgoingMsg for ReConfig failed", this));
                    return status;
                }
                else
                {
                    poutPort->iReConfig = false;
                    poutPort->oReconfigId = 0;
                    poutPort->oMsgReconfig.Unbind();
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                    uint32 timebase32 = 0;
                    uint32 clientClock32 = 0;
                    bool overflowFlag = false;
                    iClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
                    PVMF_MLNODE_LOGDATATRAFFIC_OUT((0,
                                                    "PVMFMediaLayerNode::dispatchAccessUnits: Sending ReConfig - MimeType=%s Clock=%2d",
                                                    poutPort->iMimeType.get_cstr(), clientClock32));
#endif
                }
            }
            else
            {
                poutPort->iReConfig = false;
                PVUuid eventuuid = PVMFMediaLayerNodeEventTypeUUID;
                int32 infocode = PVMFMediaLayerNodePayloadParserError;
                ReportErrorEvent(PVMFErrResourceConfiguration, (OsclAny*)(&(poutPort->iMimeType)), &eventuuid, &infocode);

                PVMF_MLNODE_LOGDATATRAFFIC_E((0, "PVMFMediaLayerNode::dispatchAccessUnits: Error - We don't receive Reconfig msg yet - Mime=%s vQ=%d", poutPort->iMimeType.get_cstr(), poutPort->vAccessUnits.size()));
                SetState(EPVMFNodeError);
                status = PVMFFailure;
                break;
            }
        }


        // fill the data implementation with the contents of the payload structure
        uint32 markerInfo = 0;
        if (it->marker == 1)
        {
            markerInfo |= PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;
        }
        if (it->randAccessPt == true)
        {
            markerInfo |= PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT;
        }

        if (it->endOfNAL)
        {
            markerInfo |= PVMF_MEDIA_DATA_MARKER_INFO_END_OF_NAL_BIT;
        }

        mediaDataImplOut->setMarkerInfo(markerInfo);
        mediaDataImplOut->setRandomAccessPoint(it->randAccessPt);
        for (uint j = 0; j < it->vfragments.size(); j++)
        {
            mediaDataImplOut->appendMediaFragment(it->vfragments[j]);
        }

        // create a data message
        PVMFMediaMsgHeader newMsgHeader;
        newMsgHeader.format_id = pinputPort->iCurrFormatId;
        PVMFSharedMediaDataPtr mediaDataOut =
            PVMFMediaData::createMediaData(mediaDataImplOut, &newMsgHeader);
        /*
         * Could happen during bitstream switching,
         * asf payload parser starts seq num for each stream from zero
         */
        if (it->sequence < poutPort->iPrevMsgSeqNum)
        {
            poutPort->iPrevMsgSeqNum++;
            it->sequence = poutPort->iPrevMsgSeqNum;
        }
        else
        {
            poutPort->iPrevMsgSeqNum = it->sequence;
        }
        mediaDataOut->setSeqNum(it->sequence);
        mediaDataOut->setStreamID(it->stream);
        mediaDataOut->setTimestamp(it->timestamp);
        mediaDataOut->setFormatSpecificInfo(pinputPort->iCurrFormatSpecInfo);

        /* Decrypt sample if protected */
        if (iDecryptionInterface != NULL)
        {
            if (iDecryptionInterface->CanDecryptInPlace())
            {
                PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::dispatchAccessUnits() Decryption is needed"));

                /* Retrieve memory fragment to write to */
                OsclRefCounterMemFrag refCtrMemFragOut;
                OsclMemoryFragment memFragOut;
                uint32 num = mediaDataOut->getNumFragments();
                uint32 totalPayloadSize = mediaDataOut->getFilledSize();
                uint8* srcDrmPtr = srcPtr;
                uint32 payloadSize = 0;

                for (uint32 i = 0; i < num; i++)
                {
                    mediaDataOut->getMediaFragment(i, refCtrMemFragOut);
                    memFragOut.ptr = refCtrMemFragOut.getMemFrag().ptr;
                    payloadSize = refCtrMemFragOut.getMemFrag().len;
                    oscl_memcpy(srcDrmPtr, (uint8*)(memFragOut.ptr), (payloadSize*sizeof(uint8)));

                    srcDrmPtr += payloadSize;
                    payloadSize = 0;

                }

                if (totalPayloadSize > maxPacketSize)
                {
                    PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::dispatchAccessUnits() - PayloadSize is bigger than Packet size"));
                    return PVMFFailure;
                }

                bool oDecryptRet =
                    iDecryptionInterface->DecryptAccessUnit(srcPtr,
                                                            totalPayloadSize);
                srcDrmPtr = srcPtr;
                for (uint32 j = 0; j < num; j++)
                {
                    mediaDataOut->getMediaFragment(j, refCtrMemFragOut);
                    memFragOut.ptr = refCtrMemFragOut.getMemFrag().ptr;
                    payloadSize = refCtrMemFragOut.getMemFrag().len;
                    oscl_memcpy((uint8*)(memFragOut.ptr), srcDrmPtr, (payloadSize*sizeof(uint8)));

                    srcDrmPtr += payloadSize;
                    payloadSize = 0;

                }
                if (oDecryptRet != true)
                {
                    PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::dispatchAccessUnits() - Decrypt Sample Failed"));
                    return PVMFFailure;
                }

                PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::dispatchAccessUnits() Decryption completed : payload lenght=%d", totalPayloadSize));
            }
            else
            {
                /* We always decrpt in place for Janus */
                PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::dispatchAccessUnits() - Inplace Decrypt Not available"));
                OSCL_ASSERT(false);
            }
        }

        // queue the message
        PVMFSharedMediaMsgPtr msgOut;
        convertToPVMFMediaMsg(msgOut, mediaDataOut);


        // send the message
        msgOut->setStreamID(iStreamID);
        status = poutPort->iPort->QueueOutgoingMsg(msgOut);

        if (status == PVMFErrBusy)
        {
            PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                                 "PVMFMediaLayerNode::dispatchAccessUnits: Outgoing Queue Busy - MimeType=%s", poutPort->iMimeType.get_cstr()));
            break;
        }
        else if (status != PVMFSuccess)
        {
            PVMF_MLNODE_LOGERROR((0,
                                  "0x%x PVMFMediaLayerNode::sendAccessUnits: "
                                  "Error - QueueOutgoingMsg failed", this));
            break;
        }
        else
        {
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
            if (oPortDataLog)
            {
                LogMediaData(mediaDataOut, pinputPort->iPort);
            }

            uint32 timebase32 = 0;
            uint32 clientClock32 = 0;
            bool overflowFlag = false;
            iClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);

            /* Get size for log purposes */
            uint32 numFrags = mediaDataOut->getNumFragments();
            uint32 size = 0;
            OsclRefCounterMemFrag memFrag;
            for (uint32 sizecount = 0; sizecount < numFrags; sizecount++)
            {
                mediaDataOut->getMediaFragment(sizecount, memFrag);
                size += memFrag.getMemFragSize();
            }
            PVMF_MLNODE_LOGDATATRAFFIC_OUT((0,
                                            "PVMFMediaLayerNode::dispatchAccessUnits: "
                                            "SSRC=%d, MimeType=%s SIZE=%d, TS=%d, SEQNUM=%d, MBIT=%d, KEY=%d, Clock=%d Delta=%d",
                                            msgOut->getStreamID(), poutPort->iMimeType.get_cstr(),
                                            size, msgOut->getTimestamp(), msgOut->getSeqNum(),
                                            (mediaDataOut->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_M_BIT),
                                            (mediaDataOut->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT),
                                            clientClock32, (msgOut->getTimestamp() - clientClock32)));
#endif
        }
    }
    // clear payloads successfully sent
    if (it != poutPort->vAccessUnits.begin())
    {
        // there is something to delete (if it == v.begin() then nothing was processed in the loop)
        it = poutPort->vAccessUnits.erase(poutPort->vAccessUnits.begin(), it);
    }
    return status;
}

bool PVMFMediaLayerNode::Allocate(OsclSharedPtr<PVMFMediaDataImpl>& mediaDataImplOut, PVMFMediaLayerPortContainer* poutPort)
{
    int32 err;
    OSCL_TRY_NO_TLS(iOsclErrorTrapImp, err, mediaDataImplOut = poutPort->ipFragGroupAllocator->allocate());
    OSCL_ASSERT(err == OsclErrNone); // we just checked that a message is available

    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}

bool PVMFMediaLayerNode::checkOutputPortsBusy(PVMFMediaLayerPortContainer* pinputPort)
{
    OSCL_ASSERT(pinputPort->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT);

    // check each output port's queue status
    // return true only if ALL output ports are busy
    for (uint i = 0; i < pinputPort->vCounterPorts.size(); i++)
    {
        if (iPortParamsQueue[pinputPort->vCounterPorts[i]].iPort->IsOutgoingQueueBusy() == false)
        {
            return false;
        }
    }

    return true;
}

PVMFStatus PVMFMediaLayerNode::ProcessOutgoingMsg(PVMFMediaLayerPortContainer* aPortContainer)
{
    PVMFPortInterface* aPort = aPortContainer->iPort;
    PVMF_MLNODE_LOGINFO((0, "0x%x PVMFMediaLayerNode::ProcessOutgoingMsg: aPort=0x%x", this, aPort));

    PVMFStatus status;
    if (aPortContainer->tag == PVMF_MEDIALAYER_PORT_TYPE_OUTPUT)
    {
        /*
         * If connected port is busy, the outgoing message cannot be process. It will be
         * queued to be processed again after receiving PORT_ACTIVITY_CONNECTED_PORT_READY
         * from this port.
         */
        if (aPort->IsConnectedPortBusy())
        {
            PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                                 "0x%x PVMFMediaLayerNode::ProcessOutgoingMsg: Connected port is busy", this));
            aPortContainer->oProcessOutgoingMessages = false;
            return PVMFErrBusy;
        }

        status = aPort->Send();
        if (status == PVMFErrBusy)
        {
            PVMF_MLNODE_LOGDATATRAFFIC_FLOWCTRL((0,
                                                 "0x%x PVMFMediaLayerNode::ProcessOutgoingMsg: Connected port goes into busy state", this));
            aPortContainer->oProcessOutgoingMessages = false;
        }
        PVMF_MLNODE_LOGINFO((0,
                             "PVMFMediaLayerNode::ProcessOutgoingMsg: Send - %s", aPortContainer->iMimeType.get_cstr()));
    }
    else
    {
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aPort));
        PVMF_MLNODE_LOGERROR((0,
                              "0x%x PVMFMediaLayerNode::ProcessOutgoingMsg: Error - Not an Output Port", this));
        status = PVMFFailure;
    }
    return status;
}

bool PVMFMediaLayerNode::CheckForPortRescheduling()
{
    uint32 i;
    for (i = 0; i < iPortVector.size(); i++)
    {
        PVMFMediaLayerPortContainer* portContainerPtr = NULL;

        if (!GetPortContainer(iPortVector[i], portContainerPtr))
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::CheckForPortRescheduling: Error - GetPortContainer failed", this));
            return false;
        }

        if (portContainerPtr->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
        {
            if (portContainerPtr->iPort->IncomingMsgQueueSize() > 0)
            {
                if (portContainerPtr->oProcessIncomingMessages)
                {
                    if (oEOSsendunits)
                        return false;
                    /*
                     * Found a port that has outstanding activity and
                     * is not busy.
                     */
                    return true;
                }
            }
        }
        else if (portContainerPtr->tag == PVMF_MEDIALAYER_PORT_TYPE_OUTPUT)
        {
            /*
             * If there are output msgs waiting and if the connected port
             * is not busy - reschedule
             */
            if ((portContainerPtr->iPort->OutgoingMsgQueueSize() > 0) &&
                    (portContainerPtr->oProcessOutgoingMessages))
            {
                /*
                 * Found a port that has outstanding activity and
                 * is not busy.
                 */
                return true;
            }
            /*
             * If there are payload access units msgs waiting and if the connected port
             * is not busy or message is available - reschedule
             * If another output port is ok and the connected port is busy, we do not reschedule.
             */
            if (portContainerPtr->vAccessUnits.size() > 0)
            {
                if ((portContainerPtr->iPort->IsOutgoingQueueBusy() == false) &&
                        (portContainerPtr->ipFragGroupAllocator->IsMsgAvailable() == true))
                {
                    /*
                     * Found payload access units msgs that is available and outgoing queue
                     * is not busy.
                     */
                    return true;
                }
            }
        }
    }
    /*
     * No port processing needed - either all port activity queues are empty
     * or the ports are backed up due to flow control.
     */
    return false;
}

bool PVMFMediaLayerNode::CheckForPortActivityQueues()
{
    uint32 i;
    for (i = 0; i < iPortVector.size(); i++)
    {
        PVMFMediaLayerPortContainer* portContainerPtr = NULL;

        if (!GetPortContainer(iPortVector[i], portContainerPtr))
        {
            PVMF_MLNODE_LOGERROR((0, "0x%x PVMFMediaLayerNode::CheckForPortRescheduling: Error - GetPortContainer failed", this));
            return false;
        }

        if ((portContainerPtr->iPort->IncomingMsgQueueSize() > 0) ||
                (portContainerPtr->iPort->OutgoingMsgQueueSize() > 0))
        {
            /*
             * Found a port that still has an outstanding activity.
             */
            return true;
        }
    }
    /*
     * All port activity queues are empty
     */
    return false;
}

/**
 * Active object implementation
 */
/**
 * This AO handles both API commands and port activity.
 * The AO will either process one command or service all connected
 * ports once per call.  It will re-schedule itself and run continuously
 * until it runs out of things to do.
 */
void PVMFMediaLayerNode::Run()
{
    iNumRunL++;

    uint32 startticks = OsclTickCount::TickCount();
    uint32 starttime = OsclTickCount::TicksToMsec(startticks);

    uint32 i;
    /* Process commands */
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            /*
             * note: need to check the state before re-scheduling
             * since the node could have been reset in the ProcessCommand
             * call.
             */
            if (iInterfaceState != EPVMFNodeCreated)
            {
                if (IsAdded())
                {
                    RunIfNotReady();
                }
            }
            return;
        }
    }

    /*
     * Process ports activity - This node would run thru the entire
     * list of ports and would try and process atleast one actvity per
     * port.
     */
    if (iInterfaceState == EPVMFNodeStarted || FlushPending())
    {
        bool oRescheduleBasedOnPorts = false;
        bool oContinueToProcess = true;
        uint32 count = 0;
        uint32 timespentinloop = 0;
        while (oContinueToProcess)
        {
            count++;
            for (i = 0; i < iPortVector.size(); i++)
            {
                PVMFMediaLayerPortContainer* portContainerPtr = NULL;

                if (!GetPortContainer(iPortVector[i], portContainerPtr))
                {
                    PVMF_MLNODE_LOGERROR((0, "PVMFMediaLayerNode::Run: Error - GetPortContainer failed"));
                    return;
                }

                ProcessPortActivity(portContainerPtr);
            }

            oRescheduleBasedOnPorts = CheckForPortRescheduling();

            uint32 currticks = OsclTickCount::TickCount();
            uint32 currtime = OsclTickCount::TicksToMsec(currticks);
            timespentinloop += (currtime - starttime);

            if (timespentinloop < MEDIALAYERNODE_MAX_RUNL_TIME_IN_MS)
            {
                if (oRescheduleBasedOnPorts == false)
                {
                    // we have time, but we have nothing to do
                    oContinueToProcess = false;
                }
            }
            else
            {
                //we have exceeded time limit so yield, if we have more stuff to do
                //we will reschedule below
                oContinueToProcess = false;
            }
        }
        if (oRescheduleBasedOnPorts)
        {
            if (IsAdded())
            {
                /*
                 * Re-schedule since there is atleast one port that needs processing
                 */
                RunIfNotReady();
            }
        }
        PVMF_MLNODE_LOG_RUNL((0, "PVMFMediaLayerNode::Run: NumLoops=%d, TotalTime=%d ms", count, timespentinloop));
        return;
    }

    /*
     * If we get here we did not process any ports or commands.
     * Check for completion of a flush command...
     */
    if (FlushPending() && (!CheckForPortActivityQueues()))
    {
        /*
         * Debug check-- all the port queues should be empty at
         * this point.
         */
        for (i = 0; i < iPortVector.size(); i++)
        {
            if (iPortVector[i]->IncomingMsgQueueSize() > 0 ||
                    iPortVector[i]->OutgoingMsgQueueSize() > 0)
            {
                OSCL_ASSERT(false);
            }
        }
        /*
         * Flush is complete.  Go to prepared state.
         */
        SetState(EPVMFNodePrepared);
        /*
         * resume port input so the ports can be re-started.
         */
        for (i = 0; i < iPortVector.size(); i++)
        {
            iPortVector[i]->ResumeInput();
        }
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
}

void PVMFMediaLayerNode::DoCancel()
{
    /* the base class cancel operation is sufficient */
    OsclActiveObject::DoCancel();
}

bool
PVMFMediaLayerNode::GetPortContainer(PVMFPortInterface* aPort,
                                     PVMFMediaLayerPortContainer& aContainer)
{
    uint32 i;
    for (i = 0; i < iPortParamsQueue.size(); i++)
    {
        PVMFMediaLayerPortContainer portParams = iPortParamsQueue[i];

        if (portParams.iPort == aPort)
        {
            aContainer = portParams;
            return true;
        }
    }
    return false;
}

bool
PVMFMediaLayerNode::GetPortContainer(PVMFPortInterface* aPort, int& index)
{
    for (index = 0; index < (int)iPortParamsQueue.size(); index++)
    {
        if (iPortParamsQueue[index].iPort == aPort)
        {
            return true;
        }
    }

    return false;
}
bool
PVMFMediaLayerNode::GetPortContainer(PVMFPortInterface* aPort,
                                     PVMFMediaLayerPortContainer*& aContainerPtr)
{
    Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;
    for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
    {
        if (it->iPort == aPort)
        {
            aContainerPtr = it;
            return true;
        }
    }
    return false;
}

PVMFPortInterface*
PVMFMediaLayerNode::getPortCounterpart(PVMFMediaLayerPortContainer aContainer)
{
    return iPortParamsQueue[aContainer.vCounterPorts[0]].iPort;
}

PVMFPortInterface*
PVMFMediaLayerNode::getPortCounterpart(PVMFMediaLayerPortContainer* aContainerPtr)
{
    return iPortParamsQueue[aContainerPtr->vCounterPorts[0]].iPort;
}

void
PVMFMediaLayerNode::LogMediaData(PVMFSharedMediaDataPtr data,
                                 PVMFPortInterface* aPort)
{
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;
    if (!GetPortContainer(aPort, portContainerPtr))
    {
        return;
    }

    PVMFMediaData* mediaData = data.GetRep();

    if (mediaData != NULL)
    {
        OsclRefCounterMemFrag memFrag;
        /* Get Format Specific Info, if any */
        mediaData->getFormatSpecificInfo(memFrag);
        OsclAny* ptr = memFrag.getMemFragPtr();
        uint32 size = memFrag.getMemFragSize();

        OSCL_StackString<8> h264("H264");
        if ((size > 0) && (portContainerPtr->oFormatSpecificInfoLogged == false))
        {
            portContainerPtr->oFormatSpecificInfoLogged = true;
            if (!oscl_strcmp(portContainerPtr->iMimeType.get_cstr(), h264.get_cstr()))
            {
                PVMF_MLNODE_LOGBIN(portContainerPtr->iPortLogger, (0, 1, sizeof(uint32), &size));
            }
            PVMF_MLNODE_LOGBIN(portContainerPtr->iPortLogger, (0, 1, size, ptr));
        }

        /* Log Media Fragments */
        uint32 numFrags = mediaData->getNumFragments();
        for (uint32 i = 0; i < numFrags; i++)
        {
            mediaData->getMediaFragment(i, memFrag);
            ptr = memFrag.getMemFragPtr();
            size = memFrag.getMemFragSize();
            if (!oscl_strcmp(portContainerPtr->iMimeType.get_cstr(), h264.get_cstr()))
            {
                PVMF_MLNODE_LOGBIN(portContainerPtr->iPortLogger, (0, 1, sizeof(uint32), &size));
            }
            PVMF_MLNODE_LOGBIN(portContainerPtr->iPortLogger, (0, 1, size, ptr));
        }
    }
}


bool PVMFMediaLayerNode::CheckForEOS()
{
    Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;
    for (it = iPortParamsQueue.begin();
            it != iPortParamsQueue.end();
            it++)
    {
        if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
        {
            if (it->oEOSReached == false)
            {
                return false;
            }
        }
    }
    return true;
}

bool
PVMFMediaLayerNode::setPortMediaParams(PVMFPortInterface* aPort,
                                       OsclRefCounterMemFrag& aConfig,
                                       mediaInfo* aMediaInfo)
{
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;

    if (!GetPortContainer(aPort, portContainerPtr))
    {
        return false;
    }

    /*
     * there is a previously set config. make a note of it.
     * in case a re config media cmd comes along we would send out this config
     */
    if (portContainerPtr->iTrackConfig.getMemFragSize() > 0)
    {
        portContainerPtr->iReConfig = true;
    }
    portContainerPtr->iTrackConfig = aConfig;

    // this method is called for input and output ports
    if (portContainerPtr->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
    {
        return portContainerPtr->iPayLoadParser->Init(aMediaInfo);
    }
    else
    {
        return true;
    }

}

PVMFStatus
PVMFMediaLayerNode::verifyPortConfigParams(const char* aFormatValType,
        PVMFPortInterface* aPort,
        OsclAny* aConfig,
        mediaInfo* aMediaInfo)
{
    OSCL_UNUSED_ARG(aMediaInfo);
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;
    if (GetPortContainer(aPort, portContainerPtr))
    {
        if (portContainerPtr->tag == PVMF_MEDIALAYER_PORT_TYPE_OUTPUT)
        {
            PVMFMediaLayerPort* mediaLayerNodePort = OSCL_STATIC_CAST(PVMFMediaLayerPort*, aPort);

            PVMFStatus status =
                mediaLayerNodePort->pvmiVerifyPortFormatSpecificInfoSync(aFormatValType,
                        aConfig);
            return status;
        }
    }
    return PVMFErrArgument;
}

bool
PVMFMediaLayerNode::setTrackDisable(PVMFPortInterface* aPort)
{
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;
    if (GetPortContainer(aPort, portContainerPtr))
    {
        portContainerPtr->oDetectBrokenTrack = true;
        portContainerPtr->oDisableTrack = true;
        return true;
    }
    return false;
}

bool
PVMFMediaLayerNode::setOutPortStreamParams(PVMFPortInterface* aPort,
        uint streamid,
        uint32 aPreroll,
        bool aLiveStream)
{
    OSCL_UNUSED_ARG(streamid);
    OSCL_UNUSED_ARG(aPreroll);
    OSCL_UNUSED_ARG(aLiveStream);
    PVMFMediaLayerPortContainer* outPort = NULL;
    int id;

    if (!GetPortContainer(aPort, id))
    {
        return false;
    }
    else
    {
        outPort = &iPortParamsQueue[id];
    }

    return true;
}

bool PVMFMediaLayerNode::parseOutputPortMime(OSCL_String* pmime,
        uint& inputPort)
{
    char* param = NULL;

    // get the input-port parameter from the mime string
    pv_mime_string_extract_param(0, (char*)pmime->get_cstr(), param);
    if (param == NULL)
    {
        return false;
    }

    // now, find where the parameter value starts
    do
    {
        if (*param++ == '=')
        {
            break;
        }
    }
    while (*param != 0);

    if (*param == 0)
    {
        // end of string
        return false;
    }

    // retrieve the numeric parameter value
    return PV_atoi(param, 'd', (uint32&)inputPort);
}


void PVMFMediaLayerNode::setInPortReposFlag(PVMFPortInterface* aPort, uint32 aSeekTimeInMS)
{
    PVMFMediaLayerPortContainer* portContainerPtr = NULL;

    PVMF_MLNODE_LOGINFO((0, "0x%x PVMFMediaLayerNode::setInPortReposFlag In", this));

    GetPortContainer(aPort, portContainerPtr);

    OSCL_ASSERT(portContainerPtr != NULL);
    OSCL_ASSERT(portContainerPtr->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT);
    OSCL_ASSERT(portContainerPtr->iPayLoadParser != NULL);

    portContainerPtr->oEOSReached = false;

    // sequence numbers down to decoder must be sequential- find out min. packet seq number for each stream
    // before we delete unsent buffer so that subsequent seq numbers can be adjusted and gap closed

    typedef Oscl_Vector<uint32, OsclMemAllocator> OsclUintVector;
    typedef Oscl_Vector<IPayloadParser::Payload, OsclMemAllocator>::iterator payload_iterator;

    // maximum sequence number for given stream
    OsclUintVector vecMinSeqNum;
    // list of all streams that have unsent packets and need to be adjusted
    OsclUintVector vecStreamsRequiringAdjustment;

    for (payload_iterator it = portContainerPtr->vAccessUnits.begin();
            it != portContainerPtr->vAccessUnits.end(); it++)
    {
        const uint32 currStreamNum = it->stream;
        const uint32 currSeq = it->sequence;

        // check whether we have this stream number already in our records
        bool bStreamAlreadyListed = false;
        for (uint32 uiS = 0; uiS < vecStreamsRequiringAdjustment.size(); ++uiS)
        {
            if (vecStreamsRequiringAdjustment[uiS] == currStreamNum)
            {
                bStreamAlreadyListed = true;
                break;
            }
        }
        if (!bStreamAlreadyListed)
            vecStreamsRequiringAdjustment.push_back(currStreamNum);

        // extend vector to cover all streams
        while (currStreamNum + 1 > vecMinSeqNum.size())
            vecMinSeqNum.push_back(0xFFFFFFFF);

        // keep track of smallest value so far
        if (currSeq < vecMinSeqNum[currStreamNum])
            vecMinSeqNum[currStreamNum] = currSeq;
    }

    // sequence generator keeps track of current sequence numbers. update it for all streams

    for (OsclUintVector::iterator itStream = vecStreamsRequiringAdjustment.begin();
            itStream != vecStreamsRequiringAdjustment.end();++itStream)
    {
        portContainerPtr->iPayLoadParser->Reposition(true, *itStream, vecMinSeqNum[*itStream]);
    }

    portContainerPtr->iPayLoadParser->Reposition();
    portContainerPtr->vAccessUnits.clear();
    for (uint i = 0; i < portContainerPtr->vCounterPorts.size(); i++)
    {
        PVMFMediaLayerPortContainer* poutPort = &this->iPortParamsQueue[portContainerPtr->vCounterPorts[i]];
        poutPort->vAccessUnits.clear();
    }
    portContainerPtr->iFirstFrameAfterRepositioning = true;
    portContainerPtr->iFirstFrameAfterReposTimeStamp = aSeekTimeInMS;
    portContainerPtr->iPort->ClearMsgQueues();

    diffAudioVideoTS = 0;
    iAdjustTimeReady = false;
    oEOSsendunits = false;

    PVMF_MLNODE_LOGINFO((0, "0x%x PVMFMediaLayerNode::setInPortReposFlag Out", this));
}

uint32 PVMFMediaLayerNode::getMaxOutPortTimestamp(PVMFPortInterface* aPort,
        bool oPeek)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::getMaxOutPortTimestamp() In"));

    PVMFMediaLayerPortContainer* inportContainerPtr = NULL;

    GetPortContainer(aPort, inportContainerPtr);

    OSCL_ASSERT(inportContainerPtr != NULL);
    OSCL_ASSERT(inportContainerPtr->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT);
    OSCL_ASSERT(inportContainerPtr->iPayLoadParser != NULL);

    inportContainerPtr->oEOSReached = false;

    uint32 max = 0;
    uint i;
    for (i = 0; i < inportContainerPtr->vCounterPorts.size(); i++)
    {
        PVMFMediaLayerPortContainer* outportContainerPtr =
            &this->iPortParamsQueue[inportContainerPtr->vCounterPorts[i]];

        uint64 ts64 = outportContainerPtr->iContinuousTimeStamp;
        uint32 ts32 = Oscl_Int64_Utils::get_uint64_lower32(ts64);
        if (ts32 > max)
        {
            max = ts32;
        }
        outportContainerPtr->oEOSReached = false;
    }
    if (oPeek == false)
    {
        /* reset all continuous ts to max */
        uint64 max64 = 0;
        Oscl_Int64_Utils::set_uint64(max64, 0, max);
        for (i = 0; i < inportContainerPtr->vCounterPorts.size(); i++)
        {
            PVMFMediaLayerPortContainer* outportContainerPtr =
                &this->iPortParamsQueue[inportContainerPtr->vCounterPorts[i]];
            outportContainerPtr->iContinuousTimeStamp = max64;
        }
    }
    return max;
}

PVMFStatus PVMFMediaLayerNode::sendEndOfTrackCommand(PVMFMediaLayerPortContainer* poutputPort)
{
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::sendEndOfTrackCommand() In"));

    PVMFStatus retval;

    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
    // Set the format ID
    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

    //Set stream ID
    sharedMediaCmdPtr->setStreamID(iStreamID);

    // Set the timestamp
    sharedMediaCmdPtr->setTimestamp(poutputPort->iContinuousTimeStamp);

    // Set the sequence number
    sharedMediaCmdPtr->setSeqNum(poutputPort->iPrevMsgSeqNum);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
    retval = poutputPort->iPort->QueueOutgoingMsg(mediaMsgOut);
    if (retval != PVMFSuccess)
    {
        PVMF_MLNODE_LOGERROR((0,
                              "0x%x PVMFMediaLayerNode::sendEndOfTrackCommand: "
                              "Error - QueueOutgoingMsg for EOS failed", this));
        return retval;
    }

    PVMF_MLNODE_LOGDATATRAFFIC_OUT((0, "PVMFMediaLayerNode::sendEndOfTrackCommand() MimeType=%s, StreamID=%d, TS=%d",
                                    poutputPort->iMimeType.get_cstr(),
                                    iStreamID,
                                    poutputPort->iContinuousTimeStamp));
    PVMF_MLNODE_LOGINFO((0, "PVMFMediaLayerNode::sendEndOfTrackCommand() Out"));
    return retval;
}

bool PVMFMediaLayerNode::setPlayRange(int32 aStartTimeInMS,
                                      int32 aStopTimeInMS,
                                      bool oRepositioning)
{
    iPlayStartTime = aStartTimeInMS;
    iPlayStopTime = aStopTimeInMS;
    Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;
    for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
    {
        if (oRepositioning)
        {
            it->iPort->ClearMsgQueues();
            it->vAccessUnits.clear();
        }

        if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
            it->oEOSReached = false;
    }
    return true;
}

PVMFStatus PVMFMediaLayerNode::checkPortCounterpartAccessUnitQueue(PVMFMediaLayerPortContainer* pinputPort, bool* IsAccessUnitsEmpty)
{
    for (uint i = 0; i < pinputPort->vCounterPorts.size(); i++)
    {
        PVMFMediaLayerPortContainer* poutPort =
            &this->iPortParamsQueue[pinputPort->vCounterPorts[i]];

        if (poutPort->vAccessUnits.size() > 0)
        {
            *IsAccessUnitsEmpty = true;
        }
    }
    return PVMFSuccess;
}


PVMFStatus PVMFMediaLayerNode::LogPayLoadParserStats()
{
    return PVMFSuccess;
}

void PVMFMediaLayerNode::LogSessionDiagnostics()
{
    if (iDiagnosticsLogged == false)
    {
        iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.streamingmanager.medialayer");

        PVMF_MLNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
        PVMF_MLNODE_LOGDIAGNOSTICS((0, "PVMFMediaLayerNode - iNumRunL = %d", iNumRunL));

        Oscl_Vector<PVMFMediaLayerPortContainer, PVMFMediaLayerNodeAllocator>::iterator it;
        for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
        {
            PvmfPortBaseImpl* ptr =
                OSCL_STATIC_CAST(PvmfPortBaseImpl*, it->iPort);
            PvmfPortBaseImplStats stats;
            ptr->GetStats(stats);

            if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_INPUT)
            {
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "PVMF_MEDIALAYER_PORT_TYPE_INPUT"));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iIncomingMsgRecv = %d", stats.iIncomingMsgRecv));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iIncomingMsgConsumed = %d", stats.iIncomingMsgConsumed));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iIncomingQueueBusy = %d", stats.iIncomingQueueBusy));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgQueued = %d", stats.iOutgoingMsgQueued));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgSent = %d", stats.iOutgoingMsgSent));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingQueueBusy = %d", stats.iOutgoingQueueBusy));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgDiscarded = %d", stats.iOutgoingMsgDiscarded));
            }
            else if (it->tag == PVMF_MEDIALAYER_PORT_TYPE_OUTPUT)
            {
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "PVMF_MEDIALAYER_PORT_TYPE_OUTPUT"));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iIncomingMsgRecv = %d", stats.iIncomingMsgRecv));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iIncomingMsgConsumed = %d", stats.iIncomingMsgConsumed));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iIncomingQueueBusy = %d", stats.iIncomingQueueBusy));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgQueued = %d", stats.iOutgoingMsgQueued));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgSent = %d", stats.iOutgoingMsgSent));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingQueueBusy = %d", stats.iOutgoingQueueBusy));
                PVMF_MLNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgDiscarded = %d", stats.iOutgoingMsgDiscarded));
            }
        }
        iDiagnosticsLogged = true;
    }
}


/*********************************************************************/
/* Extension interface */
PVMFMediaLayerNodeExtensionInterfaceImpl::PVMFMediaLayerNodeExtensionInterfaceImpl(PVMFMediaLayerNode*c)
        : PVInterfaceImpl<PVMFMediaLayerNodeAllocator>(PVUuid(PVMF_MEDIALAYERNODE_EXTENSIONINTERFACE_UUID))
        , iContainer(c)
{}

PVMFMediaLayerNodeExtensionInterfaceImpl::~PVMFMediaLayerNodeExtensionInterfaceImpl()
{}

PVMFStatus
PVMFMediaLayerNodeExtensionInterfaceImpl::setDRMDecryptionInterface(uint32 maxPacketSize,
        PVMFCPMPluginAccessUnitDecryptionInterface* aDecryptionInterface)
{
    return (iContainer->setDRMDecryptionInterface(maxPacketSize, aDecryptionInterface));
}

PVMFStatus
PVMFMediaLayerNodeExtensionInterfaceImpl::setPayloadParserRegistry(PayloadParserRegistry* registry)
{
    iContainer->setPayloadParserRegistry(registry);
    return PVMFSuccess;
}

PVMFStatus
PVMFMediaLayerNodeExtensionInterfaceImpl::setPortDataLogging(bool logEnable,
        OSCL_String* logPath)
{
    return (iContainer->setPortDataLogging(logEnable, logPath));
}

bool
PVMFMediaLayerNodeExtensionInterfaceImpl::setPlayRange(int32 aStartTimeInMS,
        int32 aStopTimeInMS,
        bool oRepositioning)
{
    return (iContainer->setPlayRange(aStartTimeInMS, aStopTimeInMS, oRepositioning));
}

bool
PVMFMediaLayerNodeExtensionInterfaceImpl::setClientPlayBackClock(PVMFMediaClock* aClientPlayBackClock)
{
    return (iContainer->setClientPlayBackClock(aClientPlayBackClock));
}

bool
PVMFMediaLayerNodeExtensionInterfaceImpl::setPortMediaParams(PVMFPortInterface* aPort,
        OsclRefCounterMemFrag& aConfig,
        mediaInfo* aMediaInfo)
{
    return (iContainer->setPortMediaParams(aPort, aConfig, aMediaInfo));
}

PVMFStatus
PVMFMediaLayerNodeExtensionInterfaceImpl::verifyPortConfigParams(const char* aFormatValType,
        PVMFPortInterface* aPort,
        OsclAny* aConfig,
        mediaInfo* aMediaInfo)
{
    return (iContainer->verifyPortConfigParams(aFormatValType, aPort, aConfig, aMediaInfo));
}

bool
PVMFMediaLayerNodeExtensionInterfaceImpl::setTrackDisable(PVMFPortInterface* aPort)
{
    return (iContainer->setTrackDisable(aPort));
}

void
PVMFMediaLayerNodeExtensionInterfaceImpl::setMediaLayerTimerDurationMS(uint32 aTimer)
{
    iContainer->setMediaLayerTimerDurationMS(aTimer);
}

bool
PVMFMediaLayerNodeExtensionInterfaceImpl::setOutPortStreamParams(PVMFPortInterface* aPort,
        uint streamid,
        uint32 aPreroll,
        bool aLiveStream)
{
    return (iContainer->setOutPortStreamParams(aPort, streamid, aPreroll, aLiveStream));
}

void PVMFMediaLayerNodeExtensionInterfaceImpl::setInPortReposFlag(PVMFPortInterface* aPort, uint32 aSeekTimeInMS)
{
    iContainer->setInPortReposFlag(aPort, aSeekTimeInMS);
}

uint32
PVMFMediaLayerNodeExtensionInterfaceImpl::getMaxOutPortTimestamp(PVMFPortInterface* aPort,
        bool oPeek)
{
    return iContainer->getMaxOutPortTimestamp(aPort, oPeek);
}
