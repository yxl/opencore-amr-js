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
#ifndef PVMF_SM_FSP_BASE_IMPL_H
#include "pvmf_sm_fsp_base_impl.h"
#endif

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#ifndef PVMI_KVP_UTIL_H_INCLUDED
#include "pvmi_kvp_util.h"
#endif

#ifndef PVMI_DRM_KVP_H_INCLUDED
#include "pvmi_drm_kvp.h"
#endif

#ifndef PVMF_SM_FSP_BASE_METADATA_H_INCLUDED
#include "pvmf_sm_fsp_base_metadata.h"
#endif

#ifndef OSCL_SNPRINTF_H_INCLUDED
#include "oscl_snprintf.h"
#endif

#ifndef PVMF_JITTER_BUFFER_FACTORY_H
#include "pvmf_jitter_buffer_factory.h"
#endif

//////////////////////////////////////////////////
// Node Constructor & Destructor
//////////////////////////////////////////////////

PVMFSMFSPBaseNode::PVMFSMFSPBaseNode(int32 aPriority): OsclActiveObject(aPriority, "PVMFSMFSPBaseNode")
        , iPlayListRepositioningSupported(false)
        , iMetaDataInfo(NULL)
        , iUseCPMPluginRegistry(false)
        , iSMBaseLogger(NULL)
        , iChildNodeErrHandler(NULL)
{
    ResetNodeParams(false);
    iCPM = NULL;
}

void PVMFSMFSPBaseNode::Construct()
{
    CreateCommandQueues();
    iSessionSourceInfo = OSCL_NEW(PVMFSMFSPSessionSourceInfo, ());
    iMetaDataInfo = OSCL_NEW(PVMFSMSessionMetaDataInfo, ());
    iChildNodeErrHandler = PVMFSMFSPChildNodeErrorHandler::CreateErrHandler(this);
}

void PVMFSMFSPBaseNode::CreateCommandQueues()
{
    int32 err;
    OSCL_TRY(err,
             /*
              * Create the input command queue.  Use a reserve to avoid lots of
              * dynamic memory allocation.
              */
             iInputCommands.Construct(PVMF_SM_FSP_NODE_COMMAND_ID_START,
                                      PVMF_SM_FSP_VECTOR_RESERVE);

             /*
              * Create the "current command" queue.  It will only contain one
              * command at a time, so use a reserve of 1.
              */
             iCurrentCommand.Construct(0, 1);

             /*
              * Create the "cancel command" queue.  It will only contain one
              * command at a time, so use a reserve of 1.
              */
             iCancelCommand.Construct(0, 1);

             iErrHandlingCommandQ.Construct(0, 2);
            );
    if (err != OsclErrNone)
    {
        CleanUp();
        OSCL_LEAVE(err);
    }
}

PVMFSMFSPBaseNode::~PVMFSMFSPBaseNode()
{
    if (IsAdded())
    {
        Cancel();
        RemoveFromScheduler();
    }
    CleanUp();
}

void PVMFSMFSPBaseNode::CleanUp()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CleanUp - In"));
    ResetNodeParams();
    if (iCPM != NULL)
    {
        iCPM->ThreadLogoff();
        PVMFCPMFactory::DestroyContentPolicyManager(iCPM);
        iCPM = NULL;
    }
    PVMFSMFSPChildNodeErrorHandler::DeleteErrHandler(iChildNodeErrHandler);
    iChildNodeErrHandler = NULL;
    iFSPChildNodeContainerVec.clear();
    OSCL_DELETE(iMetaDataInfo);
    OSCL_DELETE(iSessionSourceInfo);
    iErrHandlingCommandQ.clear();
    iCancelCommand.clear();
    iCurrentCommand.clear();
    iInputCommands.clear();
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CleanUp - Out"));
}

///////////////////////////////////////////////////////////////////////////////
//Implementation of virtuals declared in PVMFNodeInterface
///////////////////////////////////////////////////////////////////////////////
/**
 * Do thread-specific node creation and go to "Idle" state.
 */
PVMFStatus PVMFSMFSPBaseNode::ThreadLogon()
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        {
            if (!IsAdded())
                AddToScheduler();
            iCommandSeqLogger = PVLogger::GetLoggerObject("pvplayercmdseq.streamingmanager");
            iSMBaseLogger = PVLogger::GetLoggerObject("PVMFSMFSPBaseNode");
            /*
             * Call thread logon for all the children nodes
             */
            for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
            {
                if (iFSPChildNodeContainerVec[i].iNode->ThreadLogon() != PVMFSuccess)
                {
                    PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode - Child Node:ThreadLogon Failed, Node Tag %d", iFSPChildNodeContainerVec[i].iNodeTag));
                    status = PVMFFailure;
                    break;
                }
            }
            if (PVMFSuccess == status)
            {
                PVMF_SM_FSP_BASE_LOGDEBUG((0, "PVMFSMFSPBaseNode::ThreadLogon() - State - EPVMFNodeIdle"));
                SetState(EPVMFNodeIdle);
            }
        }
        break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::ThreadLogon - Out %d status", status));
    return (status);
}

/**
 * Do thread-specific node cleanup and go to "Created" state.
 */
PVMFStatus PVMFSMFSPBaseNode::ThreadLogoff()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::ThreadLogoff - In"));

    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            /* Reset the metadata key list */
            /* Clean up CPM related variables */
            /* Call thread logoff for all the children nodes */
            for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
            {
                PVMFNodeInterface* node = iFSPChildNodeContainerVec[i].iNode;
                if (node->GetState() != EPVMFNodeCreated)
                {
                    if ((status = node->ThreadLogoff()) != PVMFSuccess)
                    {
                        PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode - Child Node:ThreadLogoff Failed, Node Tag %d status %d", iFSPChildNodeContainerVec[i].iNodeTag, status));
                    }
                }
            }
            ResetNodeParams();
            SetState(EPVMFNodeCreated);
            PVMF_SM_FSP_BASE_LOGDEBUG((0, "PVMFSMFSPBaseNode::ThreadLogoff() - State - EPVMFNodeIdle"));
            iSMBaseLogger = NULL;
            iLogger = NULL;
            if (IsAdded())
                RemoveFromScheduler();
        }
        break;

        case EPVMFNodeCreated:
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }
    return (status);
}

/**
 * Makes session with PVMFSMFSPBaseNode node.
 * The purpose of this node(or of the concrete implementation of the node
 * derived from this node) is to handle most of the commands, which otherwise
 * would have been handled by the source node or various extension interfaces
 * that would have been exposed by it [PVMFStreamingManagerNode].
 * SM node and FSP will handle the command in mutually exclusive manner.
 * Whatever command will be issued to source node. Firstly, Streaming Manager Node
 * will try to service it. If SM node could not service it, it will route the
 * command to the FSP.
 * In order to send the response of the command completion (of the command
 * serviced by the FSP) directly to the node connected to the SM Node,
 * observer and the session id of the session for both the SM Node
 * and the FSP should be same.
 */
PVMFSessionId PVMFSMFSPBaseNode::Connect(const PVMFNodeSessionInfo &aSessionInfo)
{
    OSCL_UNUSED_ARG(aSessionInfo);
    OSCL_ASSERT(false);
    return 0;
}

OSCL_EXPORT_REF PVMFSessionId PVMFSMFSPBaseNode::Connect(const PVMFNodeSession &aUpstreamSession)
{
    PVMFNodeSession session;
    session.iId = aUpstreamSession.iId;
    session.iInfo = aUpstreamSession.iInfo;
    iSessions.push_back(session);
    return session.iId;
}

/**
 * retrieve node capabilities.
 */
PVMFStatus PVMFSMFSPBaseNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode:GetCapability"));
    aNodeCapability = iCapability;
    return PVMFSuccess;
}

/**
 * retrive a port iterator.
 */
PVMFPortIter* PVMFSMFSPBaseNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode:GetPorts"));
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    return NULL;
}

//Implementation of Synchronous Commands

/**
 * Provides interface with uuid PVMF_DATA_SOURCE_INIT_INTERFACE_UUID to the caller
*/
bool PVMFSMFSPBaseNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMRTSPUnicastNode::queryInterface - In"));

    if (uuid == PVMF_DATA_SOURCE_INIT_INTERFACE_UUID)
    {
        iface = OSCL_STATIC_CAST(PVMFDataSourceInitializationExtensionInterface*, this);
    }
    else
    {
        PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::queryInterface() please call async version for this UUID."));
        return false;
    }

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::queryInterface - Out"));
    return true;
}

//Asynchronous commands
/**
 * Queue an asynchronous QueryUUID command
 * QueryUUID for the streaming manager node is not complete until QueryUUIDs
 * are complete for all the children node (viz. session controller, jitter buffer
 * controller etc)
 */
PVMFCommandId PVMFSMFSPBaseNode::QueryUUID(PVMFSessionId aSession
        , const PvmfMimeString& aMimeType
        , Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids
        , bool aExactUuidsOnly
        , const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aUuids);
    OSCL_UNUSED_ARG(aExactUuidsOnly);
    OSCL_UNUSED_ARG(aContext);
    PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::QueryUUID - Error not expected"));
    OSCL_ASSERT(false);
    return 0;
}

/**
 * Queue an asynchronous node command QueryInterface
 */
PVMFCommandId PVMFSMFSPBaseNode::QueryInterface(PVMFSessionId aSession
        , const PVUuid& aUuid
        , PVInterface*& aInterfacePtr
        , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::QueryInterface - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::QueryInterface - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - RequestPort
 */
PVMFCommandId PVMFSMFSPBaseNode::RequestPort(PVMFSessionId aSession
        , int32 aPortTag
        , const PvmfMimeString* aPortConfig
        , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::RequestPort - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession,
            PVMF_SMFSP_NODE_REQUESTPORT,
            aPortTag,
            aPortConfig,
            aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::RequestPort - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - ReleasePort
 */
PVMFCommandId PVMFSMFSPBaseNode::ReleasePort(PVMFSessionId aSession
        , PVMFPortInterface& aPort
        , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::ReleasePort - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_RELEASEPORT, aPort, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::ReleasePort - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - Init
 */
PVMFCommandId PVMFSMFSPBaseNode::Init(PVMFSessionId aSession, const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Init - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_INIT, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Init - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - Prepare
 */
PVMFCommandId PVMFSMFSPBaseNode::Prepare(PVMFSessionId aSession
        , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Prepare - In"));
    /* Queue an internal command for Graph construct */
    PVMFSMFSPBaseNodeCommand cmdGC;
    cmdGC.PVMFSMFSPBaseNodeCommandBase::Construct(aSession,
            PVMF_SMFSP_NODE_CONSTRUCT_SESSION,
            NULL);

    QueueCommandL(cmdGC);

    PVMFSMFSPBaseNodeCommand cmdPrep;
    cmdPrep.PVMFSMFSPBaseNodeCommandBase::Construct(aSession,
            PVMF_SMFSP_NODE_PREPARE,
            aContext);

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Prepare - Out"));
    return QueueCommandL(cmdPrep);
}

/**
 * Queue an asynchronous node command - Start
 */
PVMFCommandId PVMFSMFSPBaseNode::Start(PVMFSessionId aSession
                                       , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Start - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_START, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Start - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - Stop
 */
PVMFCommandId PVMFSMFSPBaseNode::Stop(PVMFSessionId aSession
                                      , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Stop - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_STOP, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Stop - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - Flush
 */
PVMFCommandId PVMFSMFSPBaseNode::Flush(PVMFSessionId aSession
                                       , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Flush - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_FLUSH, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Flush - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - Pause
 */
PVMFCommandId PVMFSMFSPBaseNode::Pause(PVMFSessionId aSession
                                       , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Pause - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_PAUSE, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Pause - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - Reset
 */
PVMFCommandId PVMFSMFSPBaseNode::Reset(PVMFSessionId aSession
                                       , const OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Reset - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_RESET, aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::Reset - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - CancelAllCommands
 */
PVMFCommandId PVMFSMFSPBaseNode::CancelAllCommands(PVMFSessionId aSession
        , const OsclAny* aContextData)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode:CancelAllCommands"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_CANCELALLCOMMANDS, aContextData);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CancelAllCommands - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - CancelCommand
 */
PVMFCommandId PVMFSMFSPBaseNode::CancelCommand(PVMFSessionId aSession
        , PVMFCommandId aCmdId
        , const OsclAny* aContextData)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode:CancelCommand CommandId [%d]", aCmdId));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSession, PVMF_SMFSP_NODE_CANCELCOMMAND, aCmdId, aContextData);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CancelCommand - Out"));
    return QueueCommandL(cmd);
}

/**
 *Implementation of HandlePortActivity
 */
void PVMFSMFSPBaseNode::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    OSCL_UNUSED_ARG(aActivity);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode:HandlePortActivity - Not Implemented"));
}

///////////////////////////////////////////////////////////////////////////////
// Implementation of virtuals from OsclActiveObject
///////////////////////////////////////////////////////////////////////////////
void PVMFSMFSPBaseNode::Run()
{
    if (EPVMFNodeError == iInterfaceState)
    {
        if (!iErrHandlingCommandQ.empty())
        {
            if (ProcessCommand(iErrHandlingCommandQ.front()))
            {
                /*
                 * re-schedule if more commands to do
                 * and node isn't reset.
                 */
                if (!iErrHandlingCommandQ.empty() && iInterfaceState != EPVMFNodeCreated)
                {
                    if (IsAdded())
                    {
                        RunIfNotReady();
                    }
                }
                return;
            }
        }
        return;
    }
    /* Process commands */
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            /*
             * re-schedule if more commands to do
             * and node isn't reset.
             */
            if (!iInputCommands.empty() && iInterfaceState != EPVMFNodeCreated)
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
     * If we get here we did not process any commands.
     * Check for completion of a flush command...
     */
    if (FlushPending())
    {
        /*
         * Flush is complete.  Go to initialized state.
         */
        SetState(EPVMFNodeInitialized);
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
}

void PVMFSMFSPBaseNode::DoCancel()
{
    /* the base class cancel operation is sufficient */
    OsclActiveObject::DoCancel();
}

///////////////////////////////////////////////////////////////////////////////
//Implemenataion fo pure virtual asyn calls from PvmiCapabilityAndConfig
///////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFSMFSPBaseNode::setParametersAsync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp*& aRet_kvp,
        OsclAny* context)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::setParametersAsync - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(0,
                                            PVMF_SMFSP_NODE_CAPCONFIG_SETPARAMS,
                                            aSession, aParameters, num_elements, aRet_kvp, context);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::setParametersAsync - Out"));
    return QueueCommandL(cmd);
}

///////////////////////////////////////////////////////////////////////////////
//Implementation of PvmfDataSourcePlaybackControlInterface's async pure virtual func
///////////////////////////////////////////////////////////////////////////////
/**
 * Queue an asynchronous node command - SetDataSourcePosition
 */
PVMFCommandId PVMFSMFSPBaseNode::SetDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aActualNPT,
        PVMFTimestamp& aActualMediaDataTS,
        bool aSeekToSyncPoint,
        uint32 aStreamID,
        OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SetDataSourcePosition - In"));

    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_SET_DATASOURCE_POSITION,
                                            aTargetNPT,
                                            &aActualNPT,
                                            &aActualMediaDataTS,
                                            aSeekToSyncPoint,
                                            aStreamID,
                                            aContext);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SetDataSourcePosition - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - SetDataSourcePosition (if supported)
 */
PVMFCommandId PVMFSMFSPBaseNode::SetDataSourcePosition(PVMFSessionId aSessionId,
        PVMFDataSourcePositionParams& aPVMFDataSourcePositionParams,
        OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SetDataSourcePosition - Playlist In"));
    PVMFSMFSPBaseNodeCommand cmd;
    if (iPlayListRepositioningSupported)
    {
        cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                                PVMF_SMFSP_NODE_SET_DATASOURCE_POSITION,
                                                &aPVMFDataSourcePositionParams,
                                                aContext);
        iPlayListRepositioning = true;
    }
    else
    {
        PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::SetDataSourcePosition - PlayList - Not Supported for non RTSP"));
        OSCL_LEAVE(PVMFErrNotSupported);
    }
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SetDataSourcePosition - Playlist Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - QueryDataSourcePosition
 */
PVMFCommandId PVMFSMFSPBaseNode::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aActualNPT,
        bool aSeekToSyncPoint,
        OsclAny* aContext
                                                        )
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::QueryDataSourcePosition - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_QUERY_DATASOURCE_POSITION,
                                            aTargetNPT,
                                            &aActualNPT,
                                            aSeekToSyncPoint,
                                            aContext);

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::QueryDataSourcePosition - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - QueryDataSourcePosition
 */
PVMFCommandId PVMFSMFSPBaseNode::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aSyncBeforeTargetNPT,
        PVMFTimestamp& aSyncAfterTargetNPT,
        OsclAny* aContext,
        bool aSeekToSyncPoint
                                                        )
{
    // This is only to comply the interface file change due to Mp4 parser node.
    // Actual testing/supportfs will be done here if required.
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::QueryDataSourcePosition - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_QUERY_DATASOURCE_POSITION,
                                            aTargetNPT,
                                            &aSyncBeforeTargetNPT,
                                            &aSyncAfterTargetNPT,
                                            aContext,
                                            aSeekToSyncPoint);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::QueryDataSourcePosition - Out"));
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - SetDataSourceRate (if supported)
 */
PVMFCommandId PVMFSMFSPBaseNode::SetDataSourceRate(PVMFSessionId aSessionId,
        int32 aRate,
        PVMFTimebase* aTimebase,
        OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SetDataSourceRate - In"));
    OSCL_UNUSED_ARG(aSessionId);
    OSCL_UNUSED_ARG(aRate);
    OSCL_UNUSED_ARG(aTimebase);
    OSCL_UNUSED_ARG(aContext);
    OSCL_LEAVE(OsclErrNotSupported);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SetDataSourceRate - Out"));
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//Implementation of virtuals declared in PVMFMetadataExtensionInterface
///////////////////////////////////////////////////////////////////////////////
uint32 PVMFSMFSPBaseNode::GetNumMetadataKeysBase(char* aQueryKeyString)
{
    uint32 num_entries = 0;

    if (aQueryKeyString == NULL)
    {
        // No query key so just return all the available keys
        num_entries = iAvailableMetadataKeys.size();
    }
    else
    {
        // Determine the number of metadata keys based on the query key string provided
        uint32 i;
        for (i = 0; i < iAvailableMetadataKeys.size(); i++)
        {
            // Check if the key matches the query key
            if (pv_mime_strcmp(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) >= 0)
            {
                num_entries++;
            }
        }
    }

    if ((iCPMMetaDataExtensionInterface != NULL) && (iSessionSourceInfo->iDRMProtected  == true))
    {
        num_entries +=
            iCPMMetaDataExtensionInterface->GetNumMetadataKeys(aQueryKeyString);
    }

    return num_entries;
}

uint32 PVMFSMFSPBaseNode::GetNumMetadataValuesBase(PVMFMetadataList& aKeyList)
{

    uint32 numkeys = aKeyList.size();

    if ((numkeys == 0) || !(iMetaDataInfo->iMetadataAvailable))
    {
        return 0;
    }

    // Get Num Tracks
    uint32 numtracks = iMetaDataInfo->iNumTracks;

    uint32 numvalentries = 0;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_ALBUM_KEY) == 0 &&
                iMetaDataInfo->iAlbumPresent)
        {
            // Album
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_AUTHOR_KEY) == 0 &&
                 iMetaDataInfo->iAuthorPresent)
        {
            // Author
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_ARTIST_KEY) == 0 &&
                 iMetaDataInfo->iPerformerPresent)
        {
            // Artist/performer
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TITLE_KEY) == 0 &&
                 iMetaDataInfo->iTitlePresent)
        {
            // Title
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_DESCRIPTION_KEY) == 0 &&
                 iMetaDataInfo->iDescriptionPresent)
        {
            // Description
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_RATING_KEY) == 0 &&
                 iMetaDataInfo->iRatingPresent)
        {
            // Rating
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_COPYRIGHT_KEY) == 0 &&
                 iMetaDataInfo->iCopyRightPresent)
        {
            // Copyright
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_GENRE_KEY) == 0 &&
                 iMetaDataInfo->iGenrePresent)
        {
            // Genre
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_LYRICS_KEY) == 0 &&
                 iMetaDataInfo->iLyricsPresent)
        {
            // Lyrics
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_CLASSIFICATION_KEY) == 0 &&
                 iMetaDataInfo->iClassificationPresent)
        {
            // Classification
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_KEYWORDS_KEY) == 0 &&
                 iMetaDataInfo->iKeyWordsPresent)
        {
            // Keywords
            // Increment the counter for the number of values found so far
            numvalentries += iMetaDataInfo->iNumKeyWords;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_LOCATION_KEY) == 0 &&
                 iMetaDataInfo->iLocationPresent)
        {
            // Location
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_DURATION_KEY) == 0)
        {
            // Session Duration
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_NUMTRACKS_KEY) == 0 &&
                 numtracks > 0)
        {
            // Number of tracks
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_RANDOM_ACCESS_DENIED_KEY) == 0)
        {
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_YEAR_KEY) == 0)
        {
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_NUM_GRAPHICS_KEY) == 0 &&
                 iMetaDataInfo->iWMPicturePresent)
        {
            // Num Picture
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_GRAPHICS_KEY) == 0 &&
                 iMetaDataInfo->iWMPicturePresent)
        {
            // Picture
            // Increment the counter for the number of values found so far
            uint32 startindex = 0;
            const uint32 numPictures = iMetaDataInfo->iWMPictureIndexVec.size();
            uint32 endindex = numPictures - 1;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }

            /* Validate the indices */
            if (startindex > endindex || startindex >= numPictures || endindex >= numPictures)
            {
                continue;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_TYPE_KEY) != NULL)
        {
            // Track type
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = numtracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                continue;
            }

            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackMetaDataInfo.iMimeType.get_cstr())
                    {
                        numvalentries ++;
                    }
                }
            }

        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_DURATION_KEY) != NULL)
        {
            // Track duration
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = numtracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                continue;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_SELECTED_KEY) != NULL)
        {
            // Track selected
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = numtracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                continue;
            }
            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_WIDTH_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_HEIGHT_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_SAMPLERATE_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_NUMCHANNELS_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_TRACKID_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_CLIP_TYPE_KEY) == 0)
        {
            // Increment the counter for the number of values found so far
            ++numvalentries;
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_FRAME_RATE_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            numvalentries += (endindex + 1 - startindex);
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_NAME_KEY) != NULL))
        {
            /*
             * Codec Name
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }

            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackMetaDataInfo.iCodecName.get_size() > 0)
                    {
                        ++numvalentries;
                    }
                }
            }
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DESCRIPTION_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackInfo.iCodecDescription.get_size() > 0)
                    {
                        ++numvalentries;
                    }
                }
            }
        }
        else if ((oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DATA_KEY) != NULL))
        {
            /*
             * Codec Description
             * Determine the index requested.
             */
            uint32 startindex = 0;
            uint32 endindex   = 0;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(),
                                             PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if ((startindex > endindex) ||
                    (startindex >= (uint32)numtracks) ||
                    (endindex >= (uint32)numtracks))
            {
                continue;
            }
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackInfo.iCodecSpecificInfo.getMemFragPtr())
                    {
                        ++numvalentries;
                    }
                }
            }
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVMF_DRM_INFO_IS_PROTECTED_QUERY) == 0)
                 && ((iUseCPMPluginRegistry == false) || (iSessionSourceInfo->iDRMProtected == false)))
        {
            /*
             * is-protected
             * Increment the counter for the number of values found so far
             */
            ++numvalentries;
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_BITRATE_KEY) != NULL)
        {
            // Track bitrate
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = numtracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                continue;
            }
            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_PAUSE_DENIED_KEY) != NULL)
        {
            ++numvalentries;
        }
        else if (oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_MAX_BITRATE_KEY) != NULL)
        {
            // Track bitrate
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = numtracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr(aKeyList[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= (uint32)numtracks || endindex >= (uint32)numtracks)
            {
                continue;
            }

            // Increment the counter for the number of values found so far
            numvalentries += (endindex + 1 - startindex);
        }
        else
        {
            /* Check the extended meta data list */
            for (uint32 i = 0; i < iMetaDataInfo->iExtendedMetaDataNameVec.size(); i++)
            {
                OSCL_HeapString<OsclMemAllocator> extMetaDataName =
                    iMetaDataInfo->iExtendedMetaDataNameVec[i];
                if (oscl_strcmp(aKeyList[lcv].get_cstr(), extMetaDataName.get_cstr()) == 0)
                {
                    /*
                     * Increment the counter for the number of values found so far
                     */
                    ++numvalentries;
                }
            }
        }
    }
    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iSessionSourceInfo->iDRMProtected  == true))
    {
        numvalentries +=
            iCPMMetaDataExtensionInterface->GetNumMetadataValues(aKeyList);
    }

    return numvalentries; // Number of elements
}

PVMFCommandId PVMFSMFSPBaseNode::GetNodeMetadataKeys(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        uint32 aStartingKeyIndex,
        int32 aMaxKeyEntries,
        char* aQueryKeyString,
        const OsclAny* aContextData)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetNodeMetadataKeys - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_GETNODEMETADATAKEYS,
                                            aKeyList,
                                            aStartingKeyIndex,
                                            aMaxKeyEntries,
                                            aQueryKeyString,
                                            aContextData);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetNodeMetadataKeys - Out"));
    return QueueCommandL(cmd);
}

PVMFCommandId PVMFSMFSPBaseNode::GetNodeMetadataValues(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 aStartingValueIndex,
        int32 aMaxValueEntries,
        const OsclAny* aContextData)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetNodeMetadataValues - In"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_GETNODEMETADATAVALUES,
                                            aKeyList,
                                            aValueList,
                                            aStartingValueIndex,
                                            aMaxValueEntries,
                                            aContextData);
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetNodeMetadataValues - Out"));
    return QueueCommandL(cmd);
}

PVMFStatus PVMFSMFSPBaseNode::ReleaseNodeMetadataKeysBase(PVMFMetadataList& aKeyList,
        uint32 aStartingKeyIndex,
        uint32 aEndKeyIndex)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ReleaseNodeMetadataKeys() called"));
    OSCL_UNUSED_ARG(aKeyList);
    OSCL_UNUSED_ARG(aStartingKeyIndex);
    OSCL_UNUSED_ARG(aEndKeyIndex);
    //nothing needed-- there's no dynamic allocation in this node's key list
    return PVMFSuccess;
}

PVMFStatus PVMFSMFSPBaseNode::ReleaseNodeMetadataValuesBase(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 aStartingValueIndex,
        uint32 aEndValueIndex)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ReleaseNodeMetadataValues() called"));

    if (aStartingValueIndex > aEndValueIndex || aValueList.size() == 0)
    {
        PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::ReleaseNodeMetadataValues() Invalid start/end index"));
        return PVMFErrArgument;
    }

    //Only CPM related metadata is retrived. Then this one should be 0.
    if (iPVMFStreamingManagerNodeMetadataValueCount == 0) return PVMFSuccess;

    //To remove madatada related with un-drm value
    aEndValueIndex = iPVMFStreamingManagerNodeMetadataValueCount - 1;
    if (aEndValueIndex >= aValueList.size())
    {
        aEndValueIndex = aValueList.size() - 1;
    }

    for (uint32 i = aStartingValueIndex; i <= aEndValueIndex; i++)
    {
        if (aValueList[i].key != NULL)
        {
            switch (GetValTypeFromKeyString(aValueList[i].key))
            {
                case PVMI_KVPVALTYPE_WCHARPTR:
                    if (aValueList[i].value.pWChar_value != NULL)
                    {
                        OSCL_ARRAY_DELETE(aValueList[i].value.pWChar_value);
                        aValueList[i].value.pWChar_value = NULL;
                    }
                    break;

                case PVMI_KVPVALTYPE_CHARPTR:
                    if (aValueList[i].value.pChar_value != NULL)
                    {
                        OSCL_ARRAY_DELETE(aValueList[i].value.pChar_value);
                        aValueList[i].value.pChar_value = NULL;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT8PTR:
                    if (aValueList[i].value.pUint8_value != NULL)
                    {
                        OSCL_ARRAY_DELETE(aValueList[i].value.pUint8_value);
                        aValueList[i].value.pUint8_value = NULL;
                    }
                    break;

                case PVMI_KVPVALTYPE_UINT32:
                case PVMI_KVPVALTYPE_FLOAT:
                case PVMI_KVPVALTYPE_BOOL:
                    // No need to free memory for this valtype
                    break;

                case PVMI_KVPVALTYPE_KSV:
                {
                    //PVMFSTREAMINGMGRNODE_GRAPHICS_KEY value need to be freed in derived class (cos alocation also took there)
                }
                break;

                default:
                    // Should not get a value that wasn't created from this node
                    OSCL_ASSERT(false);
                    break;
            }

            OSCL_ARRAY_DELETE(aValueList[i].key);
            aValueList[i].key = NULL;
        }
    }

    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//Implementation of virtuals declared in PVMFNodeErrorEventObserver
///////////////////////////////////////////////////////////////////////////////
void PVMFSMFSPBaseNode::HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::HandleNodeErrorEvent - In iInterfaceState[%d]", iInterfaceState));
    //If node is already in error state, then dont propagate the err event up and ignore it
    if (EPVMFNodeError == iInterfaceState)
    {
        return;
    }
    //check if the err event is fatal err event (requires err handling to be done)
    //if yes, then move the node in err state and propagate the err event up ONLY after completion of err handling
    if (IsFatalErrorEvent(aEvent.GetEventType()))
    {
        PVMF_SM_FSP_BASE_LOGDEBUG((0, "PVMFSMFSPBaseNode::HandleNodeErrorEvent - Fatal Error"));
        //Do we have a pending command in current command Q/Cancel Q

        if (EPVMFNodeError != iInterfaceState)
        {
            SetState(EPVMFNodeError);
            iChildNodeErrHandler->InitiateErrorHandling(aEvent);
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
//Implementation of some asyn functions of PVMFNodeInterface serviced in the
//base class
///////////////////////////////////////////////////////////////////////////////
void PVMFSMFSPBaseNode::DoCancelAllCommands(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoCancelAllCommands In"));

    //first cancel the current command if any
    if (iCurrentCommand.size() > 0)
    {
        PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::DoCancelAllCommands iCurrentCommand.size()>0"));
        {
            //We wipe all node cmd states, since we are no longer going to care about cmd completes
            //coming from nodes for anything other than cancelall
            ResetNodeContainerCmdState();
            for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
            {
                PVMFSMFSPCommandContext* internalCmd = RequestNewInternalCmd();
                if (internalCmd != NULL)
                {
                    internalCmd->cmd =
                        iFSPChildNodeContainerVec[i].commandStartOffset +
                        PVMF_SM_FSP_NODE_INTERNAL_CANCEL_ALL_OFFSET;
                    internalCmd->parentCmd = PVMF_SMFSP_NODE_CANCELALLCOMMANDS;
                    OsclAny *cmdContextData = OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                    PVMFNodeInterface* iNode = iFSPChildNodeContainerVec[i].iNode;
                    iNode->CancelAllCommands(iFSPChildNodeContainerVec[i].iSessionId, cmdContextData);
                    iFSPChildNodeContainerVec[i].iNodeCmdState = PVMFSMFSP_NODE_CMD_CANCEL_PENDING;
                }
                else
                {
                    PVMF_SM_FSP_BASE_LOGERR((0, "PVMFStreamingManagerNode:DoCancelAllCommands:RequestNewInternalCmd - Failed"));
                    CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                    return;
                }
            }
        }
        MoveCmdToCancelQueue(aCmd);
    }
    else
    {
        //Assumptions:
        //Command in front of input Q is cancel command.
        //None of other commands in the input queue is hipri cmd, and hence are store in the order of their cmd id's
        //All commands in the input Q with command id < command id of the cancel command need to be completed
        //with cancelled status.
        const int32 cancelCmdId = iInputCommands.front().iId;
        const int32 inputCmndsSz = iInputCommands.size();
        for (int ii = inputCmndsSz - 1; ii > 0 ; ii--) //we dont want to process element at index 0 (being cancel command)
        {
            if (iInputCommands[ii].iId < cancelCmdId)
            {
                if (IsInternalCmd(iInputCommands[ii].iCmd))
                {
                    InternalCommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
                }
                else
                {
                    CommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
                }
            }
        }
        CommandComplete(iInputCommands, aCmd, PVMFSuccess); //complete the cancel all command
    }

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoCancelAllCommands Out"));
}

void PVMFSMFSPBaseNode::DoCancelAllPendingCommands(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoCancelAllPendingCommands In"));
    //First cancel the current command if any...
    if (iCurrentCommand.size() > 0)
    {
        PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::DoCancelAllPendingCommands iCurrentCommand.size()>0"));
        {
            for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
            {
                if (iFSPChildNodeContainerVec[i].iNodeCmdState == PVMFSMFSP_NODE_CMD_PENDING)
                {
                    PVMFSMFSPCommandContext* internalCmd = RequestNewInternalCmd();
                    if (internalCmd != NULL)
                    {
                        internalCmd->cmd =
                            iFSPChildNodeContainerVec[i].commandStartOffset +
                            PVMF_SM_FSP_NODE_INTERNAL_CANCEL_ALL_OFFSET;
                        internalCmd->parentCmd = PVMF_SMFSP_NODE_CANCELALLCOMMANDS;
                        OsclAny *cmdContextData = OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                        PVMFNodeInterface* iNode = iFSPChildNodeContainerVec[i].iNode;
                        iNode->CancelAllCommands(iFSPChildNodeContainerVec[i].iSessionId, cmdContextData);
                        iFSPChildNodeContainerVec[i].iNodeCmdState = PVMFSMFSP_NODE_CMD_CANCEL_PENDING;
                    }
                    else
                    {
                        PVMF_SM_FSP_BASE_LOGERR((0, "PVMFStreamingManagerNode:DoCancelAllPendingCommands:RequestNewInternalCmd - Failed"));
                        CommandComplete(iErrHandlingCommandQ, aCmd, PVMFErrNoMemory);
                        return;
                    }
                }
            }
        }
        MoveErrHandlingCmdToCurErrHandlingQ(aCmd);
    }
    else
    {
        iChildNodeErrHandler->ErrHandlingCommandComplete(iErrHandlingCommandQ, aCmd, PVMFSuccess);
    }
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoCancelAllPendingCommands Out"));
}

void PVMFSMFSPBaseNode::DoResetDueToErr(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoResetDueToErr - In"));
    /* this node allows a reset from any state, as long as the AO is in scheduler queue */
    OSCL_ASSERT(IsAdded());
    /*
     * Reset for streaming manager cannot be completed unless
     * Reset for all the children nodes are complete
     */
    if ((iCPM) && (iSessionSourceInfo->iDRMProtected == true) && iDRMResetPending == false)
    {
        iDRMResetPending = true;
        SendUsageComplete();
    }


    PVMFSMFSPChildNodeContainerVector::iterator it;
    for (it = iFSPChildNodeContainerVec.begin(); it != iFSPChildNodeContainerVec.end(); it++)
    {
        PVMFSMFSPCommandContext* internalCmd = RequestNewInternalCmd();
        if (internalCmd != NULL)
        {
            internalCmd->cmd =
                it->commandStartOffset +
                PVMF_SM_FSP_NODE_INTERNAL_RESET_CMD_OFFSET;
            internalCmd->parentCmd = aCmd.iCmd;
            OsclAny *cmdContextData = OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
            PVMFNodeInterface* iNode = it->iNode;
            iNode->Reset(it->iSessionId, cmdContextData);
            it->iNodeCmdState = PVMFSMFSP_NODE_CMD_PENDING;
        }
        else
        {
            PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode:DoReset:RequestNewInternalCmd - Failed"));
            CommandComplete(iErrHandlingCommandQ, aCmd, PVMFErrNoMemory);
            return;
        }
    }
    MoveErrHandlingCmdToCurErrHandlingQ(aCmd);

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoResetDueToErr - Out"));
}

void PVMFSMFSPBaseNode::CompleteChildNodesCmdCancellation()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteCancelAll - In"));
    if (CheckChildrenNodesCancelAll())
    {
        if (PVMF_SMFSP_NODE_CANCELALLCOMMANDS == iCancelCommand.front().iCmd)
        {
            /*
             * CancelAllCommands is issued by upper layer
             */
            if (!iCurrentCommand.empty())
            {
                if (IsInternalCmd(iCurrentCommand.front().iCmd) == false)
                {
                    CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                }
                else
                {
                    InternalCommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                }
            }

            //Assumptions:
            //Command in front of cancel Q is cancelall command.
            //None of commands in the input queue is hipri cmd, and hence are store in the order of their cmd id's
            //All commands in the input Q with command id < command id of the cancel command need to be completed
            //with cancelled status.
            const int32 cancelCmdId = iCancelCommand.front().iId;
            const int32 inputCmndsSz = iInputCommands.size();
            for (int ii = inputCmndsSz - 1; ii >= 0 ; ii--)
            {
                if (iInputCommands[ii].iId < cancelCmdId)
                {
                    if (IsInternalCmd(iInputCommands[ii].iCmd))
                    {
                        InternalCommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
                    }
                    else
                    {
                        CommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
                    }
                }
            }

            /* finally send command complete for the cancel all command */
            PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteCancelAll - CancelAllCommands complete"));
            CommandComplete(iCancelCommand,
                            iCancelCommand.front(),
                            PVMFSuccess);
        }
        else if (PVMF_SMFSP_NODE_CANCELCOMMAND == iCancelCommand.front().iCmd)
        {
            if (!iCurrentCommand.empty())
            {
                if (IsInternalCmd(iCurrentCommand.front().iCmd) == false)
                {
                    CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                }
                else
                {
                    InternalCommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                }
            }
            /* end command complete for the cancel all command */
            PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteCancelAll - CancelCmd complete"));
            CommandComplete(iCancelCommand,
                            iCancelCommand.front(),
                            PVMFSuccess);
        }
    }

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFStreamingManagerNode::CompleteCancelAll - Out"));
    return;
}

bool PVMFSMFSPBaseNode::CheckChildrenNodesCancelAll()
{
    for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
    {
        if (iFSPChildNodeContainerVec[i].iNodeCmdState != PVMFSMFSP_NODE_CMD_IDLE)
        {
            return false;
        }
    }
    return true;
}

void PVMFSMFSPBaseNode::DoCancelCommand(PVMFSMFSPBaseNodeCommand& aCmd)
{
    //todo:need to implement
    OSCL_UNUSED_ARG(aCmd);
    OSCL_ASSERT(false);
}

/**
 * Called by the command handler AO to do the node Reset.
 */
void PVMFSMFSPBaseNode::DoReset(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoReset - In"));
    /* this node allows a reset from any state, as long as the AO is in scheduler queue */
    OSCL_ASSERT(IsAdded());
    /*
     * Reset for streaming manager cannot be completed unless
     * Reset for all the children nodes are complete
     */
    if ((iCPM) && (iSessionSourceInfo->iDRMProtected == true) && iDRMResetPending == false)
    {
        iDRMResetPending = true;
        SendUsageComplete();
    }

    bool childNodeResetPending = false;
    PVMFSMFSPChildNodeContainerVector::iterator it;
    for (it = iFSPChildNodeContainerVec.begin(); it != iFSPChildNodeContainerVec.end(); it++)
    {
        if (EPVMFNodeCreated != it->iNode->GetState())
        {
            PVMFSMFSPCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    it->commandStartOffset +
                    PVMF_SM_FSP_NODE_INTERNAL_RESET_CMD_OFFSET;
                internalCmd->parentCmd = aCmd.iCmd;
                OsclAny *cmdContextData = OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                PVMFNodeInterface* iNode = it->iNode;
                iNode->Reset(it->iSessionId, cmdContextData);
                childNodeResetPending = true;
                it->iNodeCmdState = PVMFSMFSP_NODE_CMD_PENDING;
            }
            else
            {
                PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode:DoReset:RequestNewInternalCmd - Failed"));
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
        }
    }

    if (iDRMResetPending || childNodeResetPending)
        MoveCmdToCurrentQueue(aCmd);
    else
    {
        PVMFStatus status = ThreadLogoff();
        CommandComplete(iInputCommands, aCmd, status);
    }

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoReset - Out"));
}



void PVMFSMFSPBaseNode::CompleteReset()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteReset - In"));
    if (CheckChildrenNodesReset() && iDRMResetPending == false)
    {
        ResetNodeContainerCmdState();
        if (!iCurrentCommand.empty() && iCancelCommand.empty())
        {
            /* Indicates that the init for Children Nodes was successfull */
            /* At protected clip, Reset CPM also was successfull */
            PVMFSMFSPBaseNodeCommand& aCmd = iCurrentCommand.front();
            if (aCmd.iCmd == PVMF_SMFSP_NODE_RESET)
            {
                /* logoff & go back to Created state */
                SetState(EPVMFNodeIdle);
                /* Reset Params */
                ResetNodeParams();
                CommandComplete(iCurrentCommand, aCmd, PVMFSuccess);
            }
        }
    }
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteReset - Out"));
    return;
}

void PVMFSMFSPBaseNode::CompleteResetDueToErr()
{
    ResetNodeContainerCmdState();
    //check the state of the child nodes
    bool childNodesResetSucccessful = true;

    for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
    {
        TPVMFNodeInterfaceState childNodeState = iFSPChildNodeContainerVec[i].iNode->GetState();
        if ((childNodeState != EPVMFNodeIdle) && (childNodeState != EPVMFNodeCreated))
        {
            childNodesResetSucccessful = false;
            break;
        }
    }

    if (childNodesResetSucccessful)
    {
        SetState(EPVMFNodeIdle);
        /* Reset Params */
        ResetNodeParams();
    }
    else
    {
        OSCL_ASSERT(false);
    }
}

bool PVMFSMFSPBaseNode::CheckChildrenNodesReset()
{
    for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
    {
        if (iFSPChildNodeContainerVec[i].iNodeCmdState != PVMFSMFSP_NODE_CMD_IDLE)
        {
            return false;
        }
    }
    return true;
}

void PVMFSMFSPBaseNode::DoFlush(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoFlush - In"));
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            /*
             * Flush for streaming manager cannot be completed unless
             * Flush for all the children nodes are complete
             */
            PVMFSMFSPChildNodeContainerVector::iterator it;
            for (it = iFSPChildNodeContainerVec.begin(); it != iFSPChildNodeContainerVec.end(); it++)
            {
                PVMFSMFSPCommandContext* internalCmd = RequestNewInternalCmd();
                if (internalCmd != NULL)
                {
                    internalCmd->cmd =
                        it->commandStartOffset +
                        PVMF_SM_FSP_NODE_INTERNAL_FLUSH_CMD_OFFSET;
                    internalCmd->parentCmd = aCmd.iCmd;

                    OsclAny *cmdContextData = OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                    PVMFNodeInterface* iNode = it->iNode;

                    iNode->Flush(it->iSessionId, cmdContextData);
                    it->iNodeCmdState = PVMFSMFSP_NODE_CMD_PENDING;
                }
                else
                {
                    PVMF_SM_FSP_BASE_LOGERR((0, "StreamingManagerNode:DoFlush:RequestNewInternalCmd - Failed"));
                    CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                    return;
                }
            }
            MoveCmdToCurrentQueue(aCmd);
            /*
             * Notify all ports to suspend their input - TBD
             */
            /*
             * If the node is not running we need to wakeup the
             * AO to further complete the flush, which means all
             * port activity needs to be completed.
             */
            if (IsAdded())
            {
                RunIfNotReady();
            }
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoFlush - Out"));
}

void PVMFSMFSPBaseNode::CompleteFlush()
{
    /*
    * If the node is not running we need to wakeup the
    * AO to further complete the flush, which means all
    * port activity needs to be completed.
    */
    if (iInterfaceState != EPVMFNodeStarted)
    {
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
    return;
}

bool PVMFSMFSPBaseNode::CheckChildrenNodesFlush()
{
    for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
    {
        if (iFSPChildNodeContainerVec[i].iNodeCmdState != PVMFSMFSP_NODE_CMD_IDLE)
        {
            return false;
        }
    }
    return true;
}
/**
///////////////////////////////////////////////////////////////////////////////
//Implementation of virtuals declared in PVMFSMFSPBaseNode
///////////////////////////////////////////////////////////////////////////////
*/

void PVMFSMFSPBaseNode::ReportInfoEvent(PVMFEventType aEventType,
                                        OsclAny* aEventData,
                                        PVUuid* aEventUUID,
                                        int32* aEventCode)

{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::NodeInfoEvent Type %d Data %d"
                                    , aEventType, aEventData));

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


void PVMFSMFSPBaseNode::CommandComplete(PVMFFSPNodeCmdQ& aCmdQ,
                                        PVMFSMFSPBaseNodeCommand& aCmd,
                                        PVMFStatus aStatus,
                                        OsclAny* aEventData,
                                        PVUuid* aEventUUID,
                                        int32* aEventCode,
                                        PVInterface* aExtMsg,
                                        uint32 aEventDataLen)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "StreamingManagerNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aExtMsg)
    {
        extif = aExtMsg;
    }
    else if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    if (aEventDataLen != 0)
    {
        resp.SetEventDataLen(aEventDataLen);
    }

    if (ErrorHandlingRequired(aStatus))
    {
        HandleError(resp);
        if (errormsg)
        {
            errormsg->removeRef();
        }
        return;
    }

    PVMFSessionId session = aCmd.iSession;

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }

    /* Reschedule AO if input command queue is not empty */
    if (!iInputCommands.empty() && IsAdded())
    {
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
}

void PVMFSMFSPBaseNode::InternalCommandComplete(PVMFFSPNodeCmdQ& aCmdQ,
        PVMFSMFSPBaseNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode,
        PVInterface* aExtMsg)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode:CommandComplete Id %d Cmd %d Status %d Context %x"
                                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aExtMsg)
    {
        extif = aExtMsg;
    }
    else if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);

    if (ErrorHandlingRequired(aStatus))
    {
        HandleError(resp);
        if (errormsg)
        {
            errormsg->removeRef();
        }
        return;
    }


    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Reschedule AO if input command queue is not empty */
    if (!iInputCommands.empty() && IsAdded())
    {
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }

}




/**
//////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////
*/

bool PVMFSMFSPBaseNode::IsInternalCmd(PVMFCommandId aId)
{
    if ((PVMF_SMFSP_NODE_CONSTRUCT_SESSION == aId) || (PVMF_SMFSP_NODE_CANCEL_DUE_TO_ERROR == aId) ||
            IsFSPInternalCmd(aId))
    {
        return true;
    }
    return false;
}

bool PVMFSMFSPBaseNode::IsFatalErrorEvent(const PVMFEventType& event)
{
    bool retval = false;
    switch (event)
    {
        case PVMFErrCorrupt:
        case PVMFErrOverflow:
        case PVMFErrResource:
        case PVMFErrProcessing:
        case PVMFErrUnderflow:
        case PVMFErrNoResources:
        case PVMFErrResourceConfiguration:
        case PVMFErrTimeout:
        case PVMFErrNoMemory:
        case PVMFFailure:
            retval = true;
            break;
        default:
            retval = false;
    }
    return retval;
}

bool PVMFSMFSPBaseNode::ErrorHandlingRequired(PVMFStatus aStatus)
{
    if ((EPVMFNodeIdle != iInterfaceState) && (PVMFSuccess != aStatus) && (PVMFErrCancelled != aStatus) && (PVMFErrLicenseRequired != aStatus) && (PVMFErrNotSupported != aStatus) && (PVMFErrArgument != aStatus))
    {
        return true;
    }
    return false;
}

PVMFStatus PVMFSMFSPBaseNode::GetIndexParamValues(char* aString, uint32& aStartIndex, uint32& aEndIndex)
{
    // This parses a string of the form "index=N1...N2" and extracts the integers N1 and N2.
    // If string is of the format "index=N1" then N2=N1

    if (aString == NULL)
    {
        return PVMFErrArgument;
    }

    // Go to end of "index="
    char* n1string = aString + 6;

    PV_atoi(n1string, 'd', oscl_strlen(n1string), aStartIndex);

    char* n2string = OSCL_CONST_CAST(char*, oscl_strstr(aString, _STRLIT_CHAR("...")));

    if (n2string == NULL)
    {
        aEndIndex = aStartIndex;
    }
    else
    {
        // Go to end of "index=N1..."
        n2string += 3;

        PV_atoi(n2string, 'd', oscl_strlen(n2string), aEndIndex);
    }

    return PVMFSuccess;
}

/**
//A routine to tell if a flush operation is in progress.
*/
bool PVMFSMFSPBaseNode::FlushPending()
{
    if ((iCurrentCommand.size() > 0) &&
            (iCurrentCommand.front().iCmd == PVMF_SMFSP_NODE_FLUSH) &&
            (CheckChildrenNodesFlush() == false))
    {
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PVMFSMFSPCommandContext* PVMFSMFSPBaseNode::RequestNewInternalCmd()
{
    int32 i = 0;
    /* Search for the next free node command in the pool */
    while (i < PVMF_SMFSP_INTERNAL_CMDQ_SIZE)
    {
        if (iInternalCmdPool[i].oFree)
        {
            iInternalCmdPool[i].oFree = false;
            return &(iInternalCmdPool[i]);
        }
        ++i;
    }
    /* Free one not found so return NULL */
    return NULL;
}

void PVMFSMFSPBaseNode::ResetNodeContainerCmdState()
{
    for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
    {
        iFSPChildNodeContainerVec[i].iNodeCmdState = PVMFSMFSP_NODE_CMD_IDLE;
    }
}

PVMFSMFSPChildNodeContainer* PVMFSMFSPBaseNode::getChildNodeContainer(int32 tag)
{
    for (uint32 i = 0; i < iFSPChildNodeContainerVec.size(); i++)
    {
        if (iFSPChildNodeContainerVec[i].iNodeTag == tag)
        {
            return (&(iFSPChildNodeContainerVec[i]));
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//Command Q processing functions
///////////////////////////////////////////////////////////////////////////////
//For processing command Queues
void PVMFSMFSPBaseNode::MoveCmdToCurrentQueue(PVMFSMFSPBaseNodeCommand& aCmd)
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

void PVMFSMFSPBaseNode::MoveCmdToCancelQueue(PVMFSMFSPBaseNodeCommand& aCmd)
{
    /*
     * note: the StoreL cannot fail since the queue is never more than 1 deep
     * and we reserved space.
     */
    iCancelCommand.StoreL(aCmd);
    iInputCommands.Erase(&aCmd);
    return;
}

void PVMFSMFSPBaseNode::MoveErrHandlingCmdToCurErrHandlingQ(PVMFSMFSPBaseNodeCommand& aCmd)
{
    iCurrErrHandlingCommand.StoreL(aCmd);
    iErrHandlingCommandQ.Erase(&aCmd);
    return;
}

/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to
 * memory allocation failure.
 */
PVMFCommandId PVMFSMFSPBaseNode::QueueCommandL(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMFCommandId id = -1;
    id = iInputCommands.AddL(aCmd);
    PVMF_SM_FSP_BASE_LOGCMDSEQ((0, "Added command [%d] to Q, PVMFCommandId returned [%d]", aCmd.iCmd, id));
    if (IsAdded())
    {
        //wakeup the AO
        RunIfNotReady();
    }
    else
    {
        PVMF_SM_FSP_BASE_LOGERR((0, "AO not in queueu while attempting to add command [%d] to Q, PVMFCommandId returned [%d]", aCmd.iCmd, id));
    }
    return id;
}

/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to
 * memory allocation failure.
 */
PVMFCommandId PVMFSMFSPBaseNode::QueueErrHandlingCommandL(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMFCommandId id = -1;
    id = iErrHandlingCommandQ.AddL(aCmd);
    PVMF_SM_FSP_BASE_LOGCMDSEQ((0, "Added command [%d] to Q, PVMFCommandId returned [%d]", aCmd.iCmd, id));
    if (IsAdded())
    {
        //wakeup the AO
        RunIfNotReady();
    }
    else
    {
        PVMF_SM_FSP_BASE_LOGERR((0, "AO not in queueu while attempting to add command [%d] to Q, PVMFCommandId returned [%d]", aCmd.iCmd, id));
    }
    return id;
}

///////////////////////////////////////////////////////////////////////////////
//Command Completion/Error event notification/Info event notification functions
///////////////////////////////////////////////////////////////////////////////
void PVMFSMFSPBaseNodeCommand::Copy(const PVMFGenericNodeCommand<OsclMemAllocator>& aCmd)
{
    PVMFGenericNodeCommand<OsclMemAllocator>::Copy(aCmd);
    switch (aCmd.iCmd)
    {
        case PVMF_SMFSP_NODE_GETNODEMETADATAKEYS:
            if (aCmd.iParam4)
            {
                /* copy the allocated string */
                OSCL_HeapString<OsclMemAllocator>* aStr =
                    (OSCL_HeapString<OsclMemAllocator>*)aCmd.iParam4;
                Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                iParam4 = str.ALLOC_AND_CONSTRUCT(*aStr);
            }
            break;
        default:
            break;
    }
}

/* need to overlaod the base Destroy routine to cleanup metadata key */
void PVMFSMFSPBaseNodeCommand::Destroy()
{
    PVMFGenericNodeCommand<OsclMemAllocator>::Destroy();
    switch (iCmd)
    {
        case PVMF_SMFSP_NODE_GETNODEMETADATAKEYS:
            if (iParam4)
            {
                /* cleanup the allocated string */
                Oscl_TAlloc<OSCL_HeapString<OsclMemAllocator>, OsclMemAllocator> str;
                str.destruct_and_dealloc(iParam4);
            }
            break;
        default:
            break;
    }
}

/* Called during a Reset */
void PVMFSMFSPBaseNode::ResetNodeParams(bool aReleaseMem)
{
    //This function will reset the member variables to their initial values.
    //except command queues
    //This function intentionally does not reset the command queues.
    //Reason:This func may be called in response to handle some asyn command(say Reset).
    //So the command will be in the some of the Q. Reporting command completion for
    //non existing command in Q will result in segmentation fault.

    iCapability.iCanSupportMultipleInputPorts = false;
    iCapability.iCanSupportMultipleOutputPorts = true;
    iCapability.iHasMaxNumberOfPorts = false;
    iCapability.iMaxNumberOfPorts = 0;

    iRepositioning = false;
    iPlaylistPlayInProgress = false;
    iRepositionRequestedStartNPTInMS = 0;
    iActualRepositionStartNPTInMS = 0;
    iActualRepositionStartNPTInMSPtr = NULL;
    iActualMediaDataTS = 0;
    iActualMediaDataTSPtr = NULL;
    iJumpToIFrame = false;

    iSessionStartTime = 0;
    iSessionStopTime = 0;
    iSessionStopTimeAvailable = true;
    iSessionSeekAvailable = false;

    iPVMFDataSourcePositionParamsPtr = NULL;
    iStreamID = 0;
    iPlayListRepositioning = false;

    iNoOfValuesPushedInValueVect = 0;

    iNumRequestPortsPending = 0;
    iTotalNumRequestPortsComplete = 0;
    iGraphConstructComplete = false;
    iGraphConnectComplete = false;

    iAvailableMetadataKeys.clear();
    iCPMMetadataKeys.clear();

    if (iMetaDataInfo)
        iMetaDataInfo->Reset();

    //CPM related
    ResetCPMParams(aReleaseMem);
    for (int32 i = 0; i < PVMF_STREAMING_MANAGER_INTERNAL_CMDQ_SIZE; i++)
    {
        iInternalCmdPool[i].cmd = PVMF_STREAMING_MANAGER_INTERNAL_CMDQ_SIZE;
        iInternalCmdPool[i].oFree = true;
    }

    PVMFSMFSPChildNodeContainerVector::iterator it;
    for (it = iFSPChildNodeContainerVec.begin();
            it != iFSPChildNodeContainerVec.end();
            it++)
    {
        it->iInputPorts.clear();
        it->iOutputPorts.clear();
        it->iFeedBackPorts.clear();
    }

    if (aReleaseMem)
    {
        OSCL_DELETE(iJBFactory);
    }
    iJBFactory = NULL;
}

void PVMFSMFSPBaseNode::ResetCPMParams(bool aReleaseMem)
{
    if (aReleaseMem)
    {
        if (iCPMContentAccessFactory != NULL)
        {
            if (iDecryptionInterface != NULL)
            {
                iDecryptionInterface->Reset();
                /* Remove the decrpytion interface */
                PVUuid uuid = PVMFCPMPluginDecryptionInterfaceUuid;
                iCPMContentAccessFactory->DestroyPVMFCPMPluginAccessInterface(uuid, iDecryptionInterface);
                iDecryptionInterface = NULL;
            }
            iCPMContentAccessFactory->removeRef();
            iCPMContentAccessFactory = NULL;
        }

        if (iRequestedUsage.key)
        {
            OSCL_ARRAY_DELETE(iRequestedUsage.key);
            iRequestedUsage.key = NULL;
        }

        if (iApprovedUsage.key)
        {
            OSCL_ARRAY_DELETE(iApprovedUsage.key);
            iApprovedUsage.key = NULL;
        }

        if (iAuthorizationDataKvp.key)
        {
            OSCL_ARRAY_DELETE(iAuthorizationDataKvp.key);
            iAuthorizationDataKvp.key = NULL;
        }
    }
    iCPMMetadataKeys.clear();
    iPreviewMode = false;
    iUseCPMPluginRegistry =	false;
    iDRMResetPending = false;
    iCPMInitPending = false;
    maxPacketSize = 0;
    iPVMFStreamingManagerNodeMetadataValueCount = 0;

    iCPMSourceData.iRefCounter = 0;
    iCPMSourceData.iFileHandle	= NULL;
    iCPMSourceData.iStreamStatsLoggingURL = _STRLIT("");
    iCPMSourceData.iPreviewMode = false;
    iCPMSourceData.iIntent = BITMASK_PVMF_SOURCE_INTENT_PLAY;
    iCPMSourceData.iProxyName = _STRLIT("");
    iCPMSourceData.iProxyPort = 0;
    PVMFSourceContextData tmpObj;
    iSourceContextData = tmpObj;
    iSourceContextDataValid = false;
    iCPMSessionID = 0xFFFFFFFF;
    iCPMContentType = PVMF_CPM_CONTENT_FORMAT_UNKNOWN;
    iCPMContentAccessFactory = NULL;
    iDecryptionInterface = NULL;
    iCPMLicenseInterface = NULL;
    iCPMLicenseInterfacePVI = NULL;
    iCPMCapConfigInterface = NULL;
    iCPMCapConfigInterfacePVI = NULL;
    iCPMMetaDataExtensionInterface = NULL;
    iCPMKvpStore.destroy();
    iRequestedUsage.key = NULL;
    iApprovedUsage.key = NULL;
    iAuthorizationDataKvp.key = NULL;
    iUsageID = 0;
    iCPMRequestUsageCommandStatus = 0;

    iCPMInitCmdId = 0;
    iCPMOpenSessionCmdId = 0;
    iCPMRegisterContentCmdId = 0;
    iCPMRequestUsageId = 0;
    iCPMUsageCompleteCmdId = 0;
    iCPMCloseSessionCmdId = 0;
    iCPMResetCmdId = 0;
    iCPMGetMetaDataKeysCmdId = 0;
    iCPMGetMetaDataValuesCmdId = 0;
    iCPMGetLicenseInterfaceCmdId = 0;
    iCPMGetLicenseCmdId = 0;
    iCPMGetCapConfigCmdId = 0;
    iCPMCancelGetLicenseCmdId = 0;
}

void PVMFSMFSPBaseNode::PopulateAvailableMetadataKeys()
{
    iAvailableMetadataKeys.clear();

    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode,
             iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_DURATION_KEY);
             if (iMetaDataInfo->iAlbumPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_ALBUM_KEY);
    }
    if (iMetaDataInfo->iAuthorPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_AUTHOR_KEY);
    }
    if (iMetaDataInfo->iPerformerPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_ARTIST_KEY);
    }
    if (iMetaDataInfo->iTitlePresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_TITLE_KEY);
    }
    if (iMetaDataInfo->iDescriptionPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_DESCRIPTION_KEY);
    }
    if (iMetaDataInfo->iRatingPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_RATING_KEY);
    }
    if (iMetaDataInfo->iCopyRightPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_COPYRIGHT_KEY);
    }
    if (iMetaDataInfo->iGenrePresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_GENRE_KEY);
    }
    if (iMetaDataInfo->iLyricsPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_LYRICS_KEY);
    }
    if (iMetaDataInfo->iClassificationPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_CLASSIFICATION_KEY);
    }
    if (iMetaDataInfo->iKeyWordsPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_KEYWORDS_KEY);
    }
    if (iMetaDataInfo->iLocationPresent)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_LOCATION_KEY);
    }
    if (iMetaDataInfo->iNumTracks > 0)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_NUMTRACKS_KEY);

        // Create the parameter string for the index range
        char indexparam[18];
        oscl_snprintf(indexparam, 18, ";index=0...%d", (iMetaDataInfo->iNumTracks - 1));
        indexparam[17] = NULL_TERM_CHAR;

        bool valueAvailableForKey = false;
        {
            for (uint32 i = 0; i < (iMetaDataInfo->iNumTracks - 1); i++)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackMetaDataInfo.iMimeType.get_cstr())
                    {
                        valueAvailableForKey = true;
                        break;
                    }
                }
            }
        }
        if (valueAvailableForKey)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_TYPE_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }

        iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_DURATION_KEY);
        iAvailableMetadataKeys[0] += indexparam;

        iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_SELECTED_KEY);
        iAvailableMetadataKeys[0] += indexparam;

        valueAvailableForKey = false;
        {
            for (uint32 i = 0; i < (iMetaDataInfo->iNumTracks - 1); i++)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackMetaDataInfo.iCodecName.get_size() > 0)
                    {
                        valueAvailableForKey = true;
                        break;
                    }
                }
            }
        }
        if (valueAvailableForKey)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_NAME_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }

        valueAvailableForKey = false;
        {
            for (uint32 i = 0; i < (iMetaDataInfo->iNumTracks - 1); i++)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackMetaDataInfo.iCodecDescription.get_size() > 0)
                    {
                        valueAvailableForKey = true;
                        break;
                    }
                }
            }
        }
        if (valueAvailableForKey)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DESCRIPTION_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }

        valueAvailableForKey = false;
        {
            for (uint32 i = 0; i < (iMetaDataInfo->iNumTracks - 1); i++)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    if (trackMetaDataInfo.iCodecSpecificInfo.getMemFragPtr())
                    {
                        valueAvailableForKey = true;
                        break;
                    }
                }
            }
        }
        if (valueAvailableForKey)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DATA_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }

        iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_TRACKID_KEY);
        iAvailableMetadataKeys[0] += indexparam;
    }

    if (iMetaDataInfo->iYear)
{
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_YEAR_KEY);
    }
    if (iMetaDataInfo->iWMPicturePresent)
{
    iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_NUM_GRAPHICS_KEY);
    }
    if (iMetaDataInfo->iWMPicturePresent)
{
    // Create the parameter string for the index range
    char indexparam[18];
        oscl_snprintf(indexparam, 18, ";index=0...%d", (iMetaDataInfo->iWMPictureIndexVec.size() - 1));
        indexparam[17] = NULL_TERM_CHAR;
        iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_GRAPHICS_KEY);
        iAvailableMetadataKeys[0] += indexparam;
    }
    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_RANDOM_ACCESS_DENIED_KEY);

    iAvailableMetadataKeys.push_back(PVMFSTREAMINGMGRNODE_CLIP_TYPE_KEY);

    if ((iUseCPMPluginRegistry == false) || (iSessionSourceInfo->iDRMProtected == false))
{
    iAvailableMetadataKeys.push_back(PVMF_DRM_INFO_IS_PROTECTED_QUERY);
    }
    uint32 i = 0;
    for (i = 0; i < iMetaDataInfo->iTrackMetaDataInfoVec.size(); i++)
{
    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

        char indexparam[18];
        oscl_snprintf(indexparam, 18, ";index=%d", (i));
        indexparam[17] = NULL_TERM_CHAR;

        if (trackMetaDataInfo.iTrackWidth > 0)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_WIDTH_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }
        if (trackMetaDataInfo.iTrackHeight > 0)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_HEIGHT_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }
        if (trackMetaDataInfo.iVideoFrameRate > 0)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_FRAME_RATE_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }
        if (trackMetaDataInfo.iAudioSampleRate > 0)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_SAMPLERATE_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }
        if (trackMetaDataInfo.iAudioNumChannels > 0)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_NUMCHANNELS_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }
        if (trackMetaDataInfo.iAudioBitsPerSample > 0)
        {
            iAvailableMetadataKeys.push_front(PVMFSTREAMINGMGRNODE_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY);
            iAvailableMetadataKeys[0] += indexparam;
        }
    }
            );
    if (leavecode != OsclErrNone)
    {
        OSCL_LEAVE(leavecode);
    }
}

PVMFStatus PVMFSMFSPBaseNode::DoGetMetadataKeysBase(PVMFSMFSPBaseNodeCommand& aCmd)
{
    if (!iMetaDataInfo->iMetadataAvailable)
    {
        return PVMFErrInvalidState;
    }

    iCPMMetadataKeys.clear();
    /* Get Metadata keys from CPM for protected content only */
    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iSessionSourceInfo->iDRMProtected == true))
    {
        GetCPMMetaDataKeys();
        return PVMFPending;
    }

    return (CompleteGetMetadataKeys(aCmd));
}

PVMFStatus PVMFSMFSPBaseNode::CompleteGetMetadataKeys(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key = NULL;

    aCmd.PVMFSMFSPBaseNodeCommand::Parse(keylistptr, starting_index, max_entries, query_key);

    // Check parameters
    if (keylistptr == NULL)
    {
        // The list pointer is invalid
        return PVMFErrArgument;
    }

    if ((starting_index > (iAvailableMetadataKeys.size() - 1)) || max_entries == 0)
    {
        // Invalid starting index and/or max entries
        return PVMFErrArgument;
    }

    // Copy the requested keys
    uint32 num_entries = 0;
    int32 num_added = 0;
    uint32 lcv = 0;
    for (lcv = 0; lcv < iAvailableMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            // No query key so this key is counted
            ++num_entries;
            if (num_entries > starting_index)
            {
                // Past the starting index so copy the key
                PVMFStatus status = PushKeyToMetadataList(keylistptr, iAvailableMetadataKeys[lcv]);
                if (PVMFSuccess == status)
                    num_added++;
                else
                    return status;
            }
        }
        else
        {
            // Check if the key matche the query key
            if (pv_mime_strcmp(iAvailableMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                // This key is counted
                ++num_entries;
                if (num_entries > starting_index)
                {
                    PVMFStatus status = PushKeyToMetadataList(keylistptr, iAvailableMetadataKeys[lcv]);
                    if (PVMFSuccess == status)
                        num_added++;
                    else
                        return status;
                }
            }
        }
    }
    for (lcv = 0; lcv < iCPMMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            /* No query key so this key is counted */
            ++num_entries;
            if (num_entries > (uint32)starting_index)
            {
                PVMFStatus status = PushKeyToMetadataList(keylistptr, iCPMMetadataKeys[lcv]);
                if (PVMFSuccess == status)
                    num_added++;
                else
                    return status;
            }
        }
        else
        {
            /* Check if the key matches the query key */
            if (pv_mime_strcmp(iCPMMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                /* This key is counted */
                ++num_entries;
                if (num_entries > (uint32)starting_index)
                {
                    PVMFStatus status = PushKeyToMetadataList(keylistptr, iCPMMetadataKeys[lcv]);
                    if (PVMFSuccess == status)
                        num_added++;
                    else
                        return status;
                }
            }
        }
        // Check if max number of entries have been copied
        if (max_entries > 0 && num_added >= max_entries)
        {
            break;
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVMFSMFSPBaseNode::PushKeyToMetadataList(PVMFMetadataList* aMetaDataListPtr, const OSCL_HeapString<OsclMemAllocator> & aKey)const
{
    PVMFStatus status = PVMFSuccess;
    if (aMetaDataListPtr)
    {
        int32 leavecode = 0;
        OSCL_TRY(leavecode, aMetaDataListPtr->push_back(aKey));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSMFSPBaseNode::PushKeyToMetadataList - Memory allocation failure when copying metadata key"));
                             status = PVMFErrNoMemory);
    }
    else
        status = PVMFErrArgument;
    return status;

}

PVMFStatus PVMFSMFSPBaseNode::DoGetMetadataValuesBase(PVMFSMFSPBaseNodeCommand& aCmd)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFStreamingManagerNode::DoGetMetadataValues() In"));

    if (!iMetaDataInfo->iMetadataAvailable)
    {
        return PVMFErrInvalidState;
    }

    PVMFMetadataList* keylistptr = NULL;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    aCmd.PVMFSMFSPBaseNodeCommand::Parse(keylistptr, valuelistptr, starting_index, max_entries);

    // Check the parameters
    if (keylistptr == NULL || valuelistptr == NULL)
    {
        return PVMFErrArgument;
    }

    uint32 numkeys = keylistptr->size();

    if (numkeys <= 0 || max_entries == 0)
    {
        // Don't do anything
        return PVMFErrArgument;
    }

    uint32 numvalentries = iNoOfValuesIteratedForValueVect;
    int32 numentriesadded = iNoOfValuesPushedInValueVect;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        int32 leavecode = 0;
        PvmiKvp KeyVal;
        KeyVal.key = NULL;
        KeyVal.value.pWChar_value = NULL;
        KeyVal.value.pChar_value = NULL;

        if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_ALBUM_KEY) == 0 &&
                iMetaDataInfo->iAlbumPresent)
        {
            // Album
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsAuthorUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_ALBUM_KEY,
                             iMetaDataInfo->iAlbum.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_ALBUM_KEY,
                             iMetaDataInfo->iAlbumUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_AUTHOR_KEY) == 0 &&
                 iMetaDataInfo->iAuthorPresent)
        {
            // Author
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsAuthorUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_AUTHOR_KEY,
                             iMetaDataInfo->iAuthor.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_AUTHOR_KEY,
                             iMetaDataInfo->iAuthorUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_ARTIST_KEY) == 0 &&
                 iMetaDataInfo->iPerformerPresent)
        {
            // Artist/performer
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsPerformerUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_ARTIST_KEY,
                             iMetaDataInfo->iPerformer.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_ARTIST_KEY,
                             iMetaDataInfo->iPerformerUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TITLE_KEY) == 0 &&
                 iMetaDataInfo->iTitlePresent)
        {
            // Title
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsTitleUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_TITLE_KEY,
                             iMetaDataInfo->iTitle.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_TITLE_KEY,
                             iMetaDataInfo->iTitleUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_DESCRIPTION_KEY) == 0 &&
                 iMetaDataInfo->iDescriptionPresent)
        {
            // Description
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsDescriptionUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_DESCRIPTION_KEY,
                             iMetaDataInfo->iDescription.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_DESCRIPTION_KEY,
                             iMetaDataInfo->iDescriptionUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_RATING_KEY) == 0 &&
                 iMetaDataInfo->iRatingPresent)
        {
            // Rating
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsRatingUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_RATING_KEY,
                             iMetaDataInfo->iRating.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_RATING_KEY,
                             iMetaDataInfo->iRatingUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_COPYRIGHT_KEY) == 0 &&
                 iMetaDataInfo->iCopyRightPresent)
        {
            // Copyright
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsCopyRightUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_COPYRIGHT_KEY,
                             iMetaDataInfo->iCopyright.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_COPYRIGHT_KEY,
                             iMetaDataInfo->iCopyrightUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_GENRE_KEY) == 0 &&
                 iMetaDataInfo->iGenrePresent)
        {
            // Genre
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsGenreUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_GENRE_KEY,
                             iMetaDataInfo->iGenre.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_GENRE_KEY,
                             iMetaDataInfo->iGenreUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_LYRICS_KEY) == 0 &&
                 iMetaDataInfo->iLyricsPresent)
        {
            // Lyrics
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsLyricsUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_LYRICS_KEY,
                             iMetaDataInfo->iLyrics.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_LYRICS_KEY,
                             iMetaDataInfo->iLyricsUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_CLASSIFICATION_KEY) == 0 &&
                 iMetaDataInfo->iClassificationPresent)
        {
            // Classification
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval;
                if (iMetaDataInfo->iIsClassificationUnicode == false)
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_CLASSIFICATION_KEY,
                             iMetaDataInfo->iClassification.get_cstr());
                }
                else
                {
                    retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(KeyVal,
                             PVMFSTREAMINGMGRNODE_CLASSIFICATION_KEY,
                             iMetaDataInfo->iClassificationUnicode);
                }
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_KEYWORDS_KEY) == 0 &&
                 iMetaDataInfo->iKeyWordsPresent)
        {
            for (uint32 i = 0; i < iMetaDataInfo->iNumKeyWords; i++)
            {
                // Keywords
                // Increment the counter for the number of values found so far
                PvmiKvp keywordKVP;
                keywordKVP.key = NULL;

                if (iMetaDataInfo->iIsKeyWordsUnicode)
                {
                    if (iMetaDataInfo->iKeyWordUnicode[i].get_cstr())
                    {
                        ++numvalentries;
                    }
                }
                else
                {
                    if (iMetaDataInfo->iKeyWords[i].get_cstr())
                    {
                        ++numvalentries;
                    }
                }

                // Create a value entry if past the starting index
                if (numvalentries > starting_index)
                {
                    PVMFStatus retval;
                    KeyVal.value.pChar_value = NULL;
                    KeyVal.value.pWChar_value = NULL;
                    if (iMetaDataInfo->iIsKeyWordsUnicode == false)
                    {
                        retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(keywordKVP,
                                 PVMFSTREAMINGMGRNODE_KEYWORDS_KEY,
                                 iMetaDataInfo->iKeyWords[i].get_cstr());
                    }
                    else
                    {
                        retval = PVMFCreateKVPUtils::CreateKVPForWStringValue(keywordKVP,
                                 PVMFSTREAMINGMGRNODE_KEYWORDS_KEY,
                                 iMetaDataInfo->iKeyWordUnicode[i]);
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    if (keywordKVP.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, keywordKVP))
                        {
                            if (keywordKVP.value.pChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                                keywordKVP.value.pChar_value = NULL;
                            }
                            if (keywordKVP.value.pWChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(KeyVal.value.pWChar_value);
                                keywordKVP.value.pChar_value = NULL;
                            }
                            OSCL_ARRAY_DELETE(keywordKVP.key);
                            keywordKVP.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }

                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_LOCATION_KEY) == 0 &&
                 iMetaDataInfo->iLocationPresent)
        {
            // Location - Valid only for RTSP Streaming
            // Increment the counter for the number of values found so far

            ++numvalentries;
            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                //memory for this struct is owned in SDP parser lib
                PVMFStatus retval;
                retval = PVMFCreateKVPUtils::CreateKVPForKSVValue(KeyVal,
                         PVMFSTREAMINGMGRNODE_LOCATION_KEY,
                         (OsclAny*)(&(iMetaDataInfo->iLocationStruct)));
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_DURATION_KEY) == 0)
        {
            // Session Duration
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                if (iMetaDataInfo->iSessionDurationAvailable == true)
                {
                    uint32 duration = Oscl_Int64_Utils::get_uint64_lower32(iMetaDataInfo->iSessionDuration);
                    char timescalestr[20];
                    oscl_snprintf(timescalestr, 20, ";%s%d", PVMFSTREAMINGMGRNODE_TIMESCALE, iMetaDataInfo->iSessionDurationTimeScale);
                    timescalestr[19] = NULL_TERM_CHAR;
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                        PVMFSTREAMINGMGRNODE_DURATION_KEY,
                                        duration,
                                        timescalestr);
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
                else
                {
                    OSCL_StackString<32> unknownDuration(_STRLIT_CHAR("unknown"));
                    PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                                        PVMFSTREAMINGMGRNODE_DURATION_KEY,
                                        unknownDuration.get_cstr());
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_NUMTRACKS_KEY) == 0 &&
                 iMetaDataInfo->iNumTracks > 0)
        {
            // Number of tracks
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                    PVMFSTREAMINGMGRNODE_NUMTRACKS_KEY,
                                    iMetaDataInfo->iNumTracks);
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_RANDOM_ACCESS_DENIED_KEY) == 0)
        {
            // random access
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForBoolValue(KeyVal,
                                    PVMFSTREAMINGMGRNODE_RANDOM_ACCESS_DENIED_KEY,
                                    iMetaDataInfo->iRandomAccessDenied);
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_YEAR_KEY) == 0 &&
                 iMetaDataInfo->iYear)
        {
            // Year
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                                    PVMFSTREAMINGMGRNODE_YEAR_KEY,
                                    iMetaDataInfo->iYear);
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_NUM_GRAPHICS_KEY) == 0 &&
                 iMetaDataInfo->iWMPicturePresent)
        {
            /*
             * Num Picture
             * Increment the counter for the number of values found so far
             */
            ++numvalentries;

            /* Create a value entry if past the starting index */
            if (numvalentries > (uint32)starting_index)
            {
                PVMFStatus retval =
                    PVMFCreateKVPUtils::CreateKVPForUInt32Value(KeyVal,
                            PVMFSTREAMINGMGRNODE_NUM_GRAPHICS_KEY,
                            iMetaDataInfo->iNumWMPicture,
                            NULL);
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        //value of Graphics key to be provided by FSP
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_TYPE_KEY) != NULL)
        {
            // Track type

            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex   = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    trackkvp.value.pChar_value = NULL;

                    // Increment the counter for the number of values found so far
                    const char* mimeType = trackMetaDataInfo.iMimeType.get_cstr();
                    if (mimeType)
                    {
                        ++numvalentries;
                    }
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;
                        retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_TYPE_KEY,
                                 mimeType,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        leavecode = 0;
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            if (trackkvp.value.pChar_value != NULL)
                            {
                                OSCL_ARRAY_DELETE(trackkvp.value.pChar_value);
                                trackkvp.value.pChar_value = NULL;
                            }

                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_DURATION_KEY) != NULL)
        {
            // Track duration

            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        if (trackMetaDataInfo.iTrackDurationAvailable == true)
                        {
                            char indextimescaleparam[36];
                            oscl_snprintf(indextimescaleparam, 36, ";%s%d;%s%d", PVMFSTREAMINGMGRNODE_INDEX, i, PVMFSTREAMINGMGRNODE_TIMESCALE, trackMetaDataInfo.iTrackDurationTimeScale);
                            indextimescaleparam[35] = NULL_TERM_CHAR;

                            uint32 trackduration =
                                Oscl_Int64_Utils::get_uint64_lower32(trackMetaDataInfo.iTrackDuration);
                            retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                     PVMFSTREAMINGMGRNODE_TRACKINFO_DURATION_KEY,
                                     trackduration,
                                     indextimescaleparam);
                        }
                        else
                        {
                            char indextimescaleparam[36];
                            oscl_snprintf(indextimescaleparam, 36, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                            indextimescaleparam[35] = NULL_TERM_CHAR;

                            OSCL_StackString<32> unknownDuration(_STRLIT_CHAR("unknown"));
                            PVMFStatus retval = PVMFCreateKVPUtils::CreateKVPForCharStringValue(trackkvp,
                                                PVMFSTREAMINGMGRNODE_TRACKINFO_DURATION_KEY,
                                                unknownDuration.get_cstr(),
                                                indextimescaleparam);
                            if (retval != PVMFSuccess && retval != PVMFErrArgument)
                            {
                                break;
                            }
                        }

                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_SELECTED_KEY) != NULL)
        {
            // Track bitrate

            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);

                        indexparam[15] = NULL_TERM_CHAR;

                        bool selected = trackMetaDataInfo.iTrackSelected;
                        retval = PVMFCreateKVPUtils::CreateKVPForBoolValue(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_SELECTED_KEY,
                                 selected,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_WIDTH_KEY) != NULL)
        {
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_WIDTH_KEY,
                                 trackMetaDataInfo.iTrackWidth,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_HEIGHT_KEY) != NULL)
        {
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_HEIGHT_KEY,
                                 trackMetaDataInfo.iTrackHeight,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_SAMPLERATE_KEY) != NULL)
        {
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_SAMPLERATE_KEY,
                                 trackMetaDataInfo.iAudioSampleRate,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_NUMCHANNELS_KEY) != NULL)
        {
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_NUMCHANNELS_KEY,
                                 trackMetaDataInfo.iAudioNumChannels,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY) != NULL)
        {
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_AUDIO_BITS_PER_SAMPLE_KEY,
                                 trackMetaDataInfo.iAudioBitsPerSample,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_TRACKID_KEY) != NULL)
        {
            /* Codec Description */
            /* Determine the index requested. Default to all tracks */
            uint32 startindex = 0;
            uint32 endindex = (uint32)iMetaDataInfo->iNumTracks - 1;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }
            /* Return a KVP for each index */
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    /* Increment the counter for the number of values found so far */
                    ++numvalentries;
                    /* Add the value entry if past the starting index */
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > (uint32)starting_index)
                    {
                        char indexparam[29];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;
                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_TRACKID_KEY,
                                 trackInfo.iTrackID,
                                 indexparam);
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_CLIP_TYPE_KEY) == 0)
        {
            /*
             * Clip Type
             * Increment the counter for the number of values found so far
             */
            ++numvalentries;

            /* Create a value entry if past the starting index */
            if (numvalentries > (uint32)starting_index)
            {
                uint32 len = 0;
                char* clipType = NULL;
                len = oscl_strlen("streaming");
                clipType = OSCL_ARRAY_NEW(char, len + 1);
                oscl_memset(clipType, 0, len + 1);
                oscl_strncpy(clipType, ("streaming"), len);
                PVMFStatus retval =
                    PVMFCreateKVPUtils::CreateKVPForCharStringValue(KeyVal,
                            PVMFSTREAMINGMGRNODE_CLIP_TYPE_KEY,
                            clipType);

                OSCL_ARRAY_DELETE(clipType);
                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }

            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_FRAME_RATE_KEY) != NULL)
        {
            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_FRAME_RATE_KEY,
                                 trackMetaDataInfo.iVideoFrameRate,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_NAME_KEY) != NULL)
        {
            /* Codec Name */
            /* Determine the index requested. Default to all tracks */
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }
            /* Return a KVP for each index */
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {

                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    /* Increment the counter for the number of values found so far */
                    if (trackMetaDataInfo.iCodecName.get_size() > 0)
                    {
                        ++numvalentries;
                    }
                    /* Add the value entry if past the starting index */
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > (uint32)starting_index)
                    {
                        char indexparam[29];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;
                        char* maxsizestr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_MAXSIZE));
                        char* truncatestr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRUNCATE_FLAG));
                        uint32 maxSize = 0xFFFFFFFF;
                        if (maxsizestr != NULL)
                        {
                            if (GetMaxSizeValue(maxsizestr, maxSize) != PVMFSuccess)
                            {
                                break;
                            }
                        }

                        uint32 truncateFlag = true;

                        if (truncatestr != NULL)
                        {
                            if (GetTruncateFlagValue(truncatestr, truncateFlag) != PVMFSuccess)
                            {
                                break;
                            }
                        }

                        uint32 codecNameLen = trackMetaDataInfo.iCodecName.get_size();
                        char maxsizemessage[13];
                        maxsizemessage[0] = '\0';
                        if (maxSize < codecNameLen)

                        {
                            if (!truncateFlag)
                            {
                                oscl_snprintf(maxsizemessage, 13, ";%s%d", PVMFSTREAMINGMGRNODE_REQSIZE, codecNameLen);
                                oscl_strncat(indexparam, maxsizemessage, oscl_strlen(maxsizemessage));
                            }
                        }

                        retval =
                            PVMFCreateKVPUtils::CreateKVPForWStringValue(trackkvp,
                                    PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_NAME_KEY,
                                    (trackMetaDataInfo.iCodecName),
                                    indexparam, maxSize, truncateFlag);
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }

                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DESCRIPTION_KEY) != NULL)
        {
            /* Codec Description */
            /* Determine the index requested. Default to all tracks */
            uint32 startindex = 0;
            uint32 endindex = (uint32)iMetaDataInfo->iNumTracks - 1;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }
            /* Return a KVP for each index */
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    /* Increment the counter for the number of values found so far */
                    if (trackInfo.iCodecDescription.get_size() > 0)
                    {
                        ++numvalentries;
                    }
                    /* Add the value entry if past the starting index */
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > (uint32)starting_index)
                    {
                        char indexparam[29];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;
                        char* maxsizestr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_MAXSIZE));
                        char* truncatestr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRUNCATE_FLAG));
                        uint32 maxSize = 0xFFFFFFFF;
                        if (maxsizestr != NULL)
                        {
                            if (GetMaxSizeValue(maxsizestr, maxSize) != PVMFSuccess)
                            {
                                break;
                            }
                        }

                        uint32 truncateFlag = true;

                        if (truncatestr != NULL)
                        {
                            if (GetTruncateFlagValue(truncatestr, truncateFlag) != PVMFSuccess)
                            {
                                break;
                            }
                        }

                        uint32 codecDescLen = trackInfo.iCodecDescription.get_size();
                        char maxsizemessage[13];
                        maxsizemessage[0] = '\0';
                        if (maxSize < codecDescLen)
                        {
                            if (!truncateFlag)
                            {
                                oscl_snprintf(maxsizemessage, 13, ";%s%d", PVMFSTREAMINGMGRNODE_REQSIZE, codecDescLen);
                                oscl_strncat(indexparam, maxsizemessage, oscl_strlen(maxsizemessage));
                            }
                        }

                        retval =
                            PVMFCreateKVPUtils::CreateKVPForWStringValue(trackkvp,
                                    PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DESCRIPTION_KEY,
                                    (trackInfo.iCodecDescription),
                                    indexparam, maxSize, truncateFlag);
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DATA_KEY) != NULL)
        {
            /* Codec Description */
            /* Determine the index requested. Default to all tracks */
            uint32 startindex = 0;
            uint32 endindex = (uint32)iMetaDataInfo->iNumTracks - 1;
            /* Check if the index parameter is present */
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                /* Retrieve the index values */
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            /* Validate the indices */
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }
            /* Return a KVP for each index */
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];
                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    /* Increment the counter for the number of values found so far */
                    if ((uint8*)(trackInfo.iCodecSpecificInfo.getMemFragPtr()))
                    {
                        ++numvalentries;
                    }
                    /* Add the value entry if past the starting index */
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > (uint32)starting_index)
                    {
                        char indexparam[29];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;
                        retval =
                            PVMFCreateKVPUtils::CreateKVPForByteArrayValue(trackkvp,
                                    PVMFSTREAMINGMGRNODE_TRACKINFO_CODEC_DATA_KEY,
                                    (uint8*)(trackInfo.iCodecSpecificInfo.getMemFragPtr()),
                                    (uint32)trackInfo.iCodecSpecificInfo.getMemFragSize()
                                    , indexparam);
                    }
                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }
                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVMF_DRM_INFO_IS_PROTECTED_QUERY) == 0)
                 && ((iUseCPMPluginRegistry == false) || (iSessionSourceInfo->iDRMProtected == false)))
        {
            /*
             * is-protected
             * Increment the counter for the number of values found so far
             */
            ++numvalentries;

            /* Create a value entry if past the starting index */
            if (numvalentries > (uint32)starting_index)
            {
                PVMFStatus retval =
                    PVMFCreateKVPUtils::CreateKVPForBoolValue(KeyVal,
                            PVMF_DRM_INFO_IS_PROTECTED_QUERY,
                            iSessionSourceInfo->iDRMProtected,
                            NULL);

                if (retval != PVMFSuccess && retval != PVMFErrArgument)
                {
                    break;
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_BITRATE_KEY) != NULL)
        {
            // Track bitrate

            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;

                    // Increment the counter for the number of values found so far
                    ++numvalentries;
                    // Add the value entry if past the starting index
                    PVMFStatus retval = PVMFErrArgument;
                    if (numvalentries > starting_index)
                    {
                        char indexparam[16];
                        oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                        indexparam[15] = NULL_TERM_CHAR;

                        uint32 trackbitrate = trackMetaDataInfo.iTrackBitRate;
                        retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                 PVMFSTREAMINGMGRNODE_TRACKINFO_BITRATE_KEY,
                                 trackbitrate,
                                 indexparam);
                    }

                    if (retval != PVMFSuccess && retval != PVMFErrArgument)
                    {
                        break;
                    }

                    if (trackkvp.key != NULL)
                    {
                        if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                        {
                            OSCL_ARRAY_DELETE(trackkvp.key);
                            trackkvp.key = NULL;
                        }
                        else
                        {
                            // Increment the value list entry counter
                            ++numentriesadded;
                        }

                        // Check if the max number of value entries were added
                        if (max_entries > 0 && numentriesadded >= max_entries)
                        {
                            iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                            return PVMFSuccess;
                        }
                    }
                }
            }
        }
        else if (oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_TRACKINFO_MAX_BITRATE_KEY) != NULL)
        {
            // Track bitrate

            // Determine the index requested. Default to all tracks
            uint32 startindex = 0;
            uint32 endindex = iMetaDataInfo->iNumTracks - 1;
            // Check if the index parameter is present
            char* indexstr = OSCL_CONST_CAST(char*, oscl_strstr((*keylistptr)[lcv].get_cstr(), PVMFSTREAMINGMGRNODE_INDEX));
            if (indexstr != NULL)
            {
                // Retrieve the index values
                GetIndexParamValues(indexstr, startindex, endindex);
            }
            // Validate the indices
            if (startindex > endindex || startindex >= iMetaDataInfo->iNumTracks || endindex >= iMetaDataInfo->iNumTracks)
            {
                continue;
            }

            // Return a KVP for each index
            for (uint32 i = startindex; i <= endindex; ++i)
            {
                if (i < iMetaDataInfo->iTrackMetaDataInfoVec.size())
                {
                    PVMFSMTrackMetaDataInfo trackMetaDataInfo = iMetaDataInfo->iTrackMetaDataInfoVec[i];

                    PvmiKvp trackkvp;
                    trackkvp.key = NULL;
                    uint32 trackbitrate = trackMetaDataInfo.iTrackMaxBitRate;
                    if (trackbitrate > 0)
                    {
                        // Increment the counter for the number of values found so far
                        ++numvalentries;
                        // Add the value entry if past the starting index
                        PVMFStatus retval = PVMFErrArgument;
                        if (numvalentries > starting_index)
                        {
                            char indexparam[16];
                            oscl_snprintf(indexparam, 16, ";%s%d", PVMFSTREAMINGMGRNODE_INDEX, i);
                            indexparam[15] = NULL_TERM_CHAR;
                            retval = PVMFCreateKVPUtils::CreateKVPForUInt32Value(trackkvp,
                                     PVMFSTREAMINGMGRNODE_TRACKINFO_MAX_BITRATE_KEY,
                                     trackbitrate,
                                     indexparam);
                        }

                        if (retval != PVMFSuccess && retval != PVMFErrArgument)
                        {
                            break;
                        }

                        if (trackkvp.key != NULL)
                        {
                            if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, trackkvp))
                            {
                                OSCL_ARRAY_DELETE(trackkvp.key);
                                trackkvp.key = NULL;
                            }
                            else
                            {
                                // Increment the value list entry counter
                                ++numentriesadded;
                            }

                            // Check if the max number of value entries were added
                            if (max_entries > 0 && numentriesadded >= max_entries)
                            {
                                iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();
                                return PVMFSuccess;
                            }
                        }
                    }
                }
            }
        }
//Check with FSP
        // Add the KVP to the list if the key string was created
        if (KeyVal.key != NULL)
        {
            if (PVMFSuccess != PushKVPToMetadataValueList(valuelistptr, KeyVal))
            {
                switch (GetValTypeFromKeyString(KeyVal.key))
                {
                    case PVMI_KVPVALTYPE_CHARPTR:
                        if (KeyVal.value.pChar_value != NULL)
                        {
                            OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                            KeyVal.value.pChar_value = NULL;
                        }
                        break;

                    default:
                        // Add more case statements if other value types are returned
                        break;
                }

                OSCL_ARRAY_DELETE(KeyVal.key);
                KeyVal.key = NULL;
            }
            else
            {
                // Increment the counter for number of value entries added to the list
                ++numentriesadded;
            }

            // Check if the max number of value entries were added
            if (max_entries > 0 && numentriesadded >= max_entries)
            {
                // Maximum number of values added so break out of the loop
                break;
            }
        }
    }

    iNoOfValuesPushedInValueVect = numentriesadded;
    iNoOfValuesIteratedForValueVect = numvalentries;
    iPVMFStreamingManagerNodeMetadataValueCount = (*valuelistptr).size();


    if ((iCPMMetaDataExtensionInterface != NULL) &&
            (iSessionSourceInfo->iDRMProtected  == true))
    {
        if (OSCL_STATIC_CAST(int32, iNoOfValuesPushedInValueVect) < max_entries)
        {
            uint32 cpmStartingIndex = 0;
            uint32 cpmMaxEntries = max_entries - iNoOfValuesPushedInValueVect;

            if (iNoOfValuesIteratedForValueVect >= starting_index)
            {
                cpmStartingIndex = 0;
            }
            else
            {
                cpmStartingIndex = starting_index - iNoOfValuesIteratedForValueVect;
            }
            iCPMGetMetaDataValuesCmdId =
                iCPMMetaDataExtensionInterface->GetNodeMetadataValues(iCPMSessionID,
                        (*keylistptr),
                        (*valuelistptr),
                        cpmStartingIndex,
                        cpmMaxEntries
                                                                     );
            return PVMFPending;
        }
    }



    return PVMFSuccess;
}

PVMFStatus PVMFSMFSPBaseNode::PushKVPToMetadataValueList(Oscl_Vector<PvmiKvp, OsclMemAllocator>* aValueList, const PvmiKvp& aKVP)const
{
    PVMFStatus status = PVMFSuccess;
    if (aValueList)
    {
        int32 leavecode = 0;
        OSCL_TRY(leavecode, aValueList->push_back(aKVP));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSMFSPBaseNode::PushKeyToMetadataList - Memory allocation failure when copying metadata key"));
                             status = PVMFErrNoMemory);
    }
    else
        status = PVMFErrArgument;
    return status;
}

PVMFStatus PVMFSMFSPBaseNode::GetMaxSizeValue(char* aString, uint32& aMaxSize)
{
    aMaxSize = 0xFFFFFFFF;
    /*
     * This parses a string of the form "maxsize=N1" and extracts the integer N1.
     */
    if (aString == NULL)
    {
        return PVMFErrArgument;
    }

    /* Go to end of "maxsize=" */
    char* n1string = aString + 8;
    char* truncatestr = OSCL_CONST_CAST(char*, oscl_strstr(n1string, PVMFSTREAMINGMGRNODE_TRUNCATE_FLAG));

    uint32 maxsizelen = 0;

    if (truncatestr != NULL)
    {
        maxsizelen = oscl_strlen(n1string) - (oscl_strlen(truncatestr) + 1);
        n1string[maxsizelen] = '\0';
    }

    if (PV_atoi(n1string, 'd', oscl_strlen(n1string), aMaxSize))
    {
        return PVMFSuccess;
    }
    return PVMFFailure;
}

PVMFStatus PVMFSMFSPBaseNode::GetTruncateFlagValue(char* aString, uint32& aTruncateFlag)
{
    aTruncateFlag = 0;
    /*
     * This parses a string of the form "truncate=N1" and extracts the integer N1.
     */
    if (aString == NULL)
    {
        return PVMFErrArgument;
    }

    /* Go to end of "truncate=" */
    char* n1string = aString + 9;

    if (!oscl_strcmp(n1string, "true"))
    {
        aTruncateFlag = true;
    }
    else if (!oscl_strcmp(n1string, "false"))
    {
        aTruncateFlag = false;
    }
    else
    {
        return PVMFFailure;
    }
    return PVMFSuccess;

}

void PVMFSMFSPBaseNode::HandleError(const PVMFCmdResp& aResponse)
{
    PVMFSMFSPCommandContext *cmdContextData =
        OSCL_REINTERPRET_CAST(PVMFSMFSPCommandContext*, aResponse.GetContext());
    if (EPVMFNodeError != iInterfaceState)
    {
        if (cmdContextData)	//It may be possible that cmdContextData == NULL for internal commands
        {
            OSCL_ASSERT(cmdContextData->parentCmd != PVMF_SMFSP_NODE_RESET);
        }
        SetState(EPVMFNodeError);
        iChildNodeErrHandler->InitiateErrorHandling(aResponse);
    }
    else
    {
        iChildNodeErrHandler->CompleteErrorHandling(aResponse);
    }
}

void PVMFSMFSPBaseNode::ErrHandlingComplete(const PVMFSMFSPBaseNodeCommand* aErroneousCmd)
{
    OSCL_ASSERT(iChildNodeErrHandler);
    if (!iChildNodeErrHandler)
    {
        PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::ErrHandlingComplete Error - iChildNodeErrHandler not available"));
        return;
    }
    if (aErroneousCmd)
    {
        const PVMFCmdResp* const cmdResponse = iChildNodeErrHandler->GetErroneousCmdResponse();
        OSCL_ASSERT(cmdResponse);
        if (!cmdResponse)
        {
            PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::ErrHandlingComplete Error - Command Response not available"));
            return;
        }


        if (iCancelCommand.size() > 0)
        {
            //We got err during cancel command
            if (PVMF_SMFSP_NODE_CANCELALLCOMMANDS == iCancelCommand.front().iCmd)
            {
                if (!iCurrentCommand.empty())
                {
                    if (IsInternalCmd(iCurrentCommand.front().iCmd) == false)
                    {
                        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                    }
                    else
                    {
                        InternalCommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                    }
                }

                //Assumptions:
                //Command in front of cancel Q is cancel command.
                //None of commands in the input queue is hipri cmd, and hence are store in the order of their cmd id's
                //All commands in the input Q with command id < command id of the cancel command need to be completed
                //with cancelled status.
                const int32 cancelCmdId = iCancelCommand.front().iId;
                const int32 inputCmndsSz = iInputCommands.size();
                for (int ii = inputCmndsSz - 1; ii >= 0 ; ii--)
                {
                    if (iInputCommands[ii].iId < cancelCmdId)
                    {
                        if (IsInternalCmd(iInputCommands[ii].iCmd))
                        {
                            InternalCommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
                        }
                        else
                        {
                            CommandComplete(iInputCommands, iInputCommands[ii], PVMFErrCancelled);
                        }
                    }
                }
                /* finally send command complete for the cancel all command */
                PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::CompleteCancelAll - CancelAllCommands complete error"));
                bool eventDataLenAvailable = false;
                uint32 eventDataLength = 0;
                cmdResponse->GetEventDataLen(eventDataLenAvailable, eventDataLength);
                CommandComplete(iCancelCommand, iCancelCommand.front(), cmdResponse->GetCmdStatus(), cmdResponse->GetEventData(), NULL, NULL, cmdResponse->GetEventExtensionInterface(), eventDataLenAvailable ? eventDataLength : 0);
            }
            else if (PVMF_SMFSP_NODE_CANCELCOMMAND == iCancelCommand.front().iCmd)
            {
                if (!iCurrentCommand.empty())
                {
                    if (IsInternalCmd(iCurrentCommand.front().iCmd) == false)
                    {
                        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                    }
                    else
                    {
                        InternalCommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
                    }
                }
                /* end command complete for the cancel all command */
                PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::CompleteCancelAll - CancelCmd complete error"));
                bool eventDataLenAvailable = false;
                uint32 eventDataLength = 0;
                cmdResponse->GetEventDataLen(eventDataLenAvailable, eventDataLength);
                CommandComplete(iCancelCommand, iCancelCommand.front(), PVMFFailure, cmdResponse->GetEventData(), NULL, NULL, cmdResponse->GetEventExtensionInterface(), eventDataLenAvailable ? eventDataLength : 0);
            }
            else
            {
                //Only commands in cancel Q can be cancel command/cancelall command,or some command entertained by CPM plugin.
                //Err handling logic is done only for the child nodes of the  SM node, CPM plugin is not handled by current err handling logic
                OSCL_ASSERT(false);
            }
        }
        else
        {
            //We got err during command other than cancel command
            //Command may be present in input command Q/current command Q
            if (iCurrentCommand.size() > 0)
            {
                if (IsInternalCmd(iCurrentCommand.front().iCmd))
                {
                    //aErroneousCmd
                    //We do not use the event data of cmd response and asyn event , becuas ewe cache the response/event and hence data may be dangling when we attempt to use it after caching.
                    InternalCommandComplete(iCurrentCommand, iCurrentCommand.front(), cmdResponse->GetCmdStatus(), NULL, NULL, NULL, cmdResponse->GetEventExtensionInterface());
                }
                else
                {
                    bool eventDataLenAvailable = false;
                    uint32 eventDataLength = 0;
                    cmdResponse->GetEventDataLen(eventDataLenAvailable, eventDataLength);
                    CommandComplete(iCurrentCommand, iCurrentCommand.front(), cmdResponse->GetCmdStatus(), cmdResponse->GetEventData(), NULL, NULL, cmdResponse->GetEventExtensionInterface(), eventDataLenAvailable ? eventDataLength : 0);
                }
            }
            else
            {
                if (IsInternalCmd(iInputCommands.front().iCmd))
                {
                    //aErroneousCmd
                    //We do not use the event data of cmd response and asyn event , becuas ewe cache the response/event and hence data may be dangling when we attempt to use it after caching.
                    InternalCommandComplete(iInputCommands, iInputCommands.front(), cmdResponse->GetCmdStatus(), NULL, NULL, NULL, cmdResponse->GetEventExtensionInterface());
                }
                else
                {
                    bool eventDataLenAvailable = false;
                    uint32 eventDataLength = 0;
                    cmdResponse->GetEventDataLen(eventDataLenAvailable, eventDataLength);
                    CommandComplete(iInputCommands, iInputCommands.front(), cmdResponse->GetCmdStatus(), cmdResponse->GetEventData(), NULL, NULL, cmdResponse->GetEventExtensionInterface(), eventDataLenAvailable ? eventDataLength : 0);
                }
            }
        }
    }
    else
    {
        OSCL_ASSERT(iChildNodeErrHandler->GetAsyncErrEvent());
        if (!iChildNodeErrHandler->GetAsyncErrEvent())
            return;
        else
        {
            PVMFAsyncEvent event = OSCL_CONST_CAST(PVMFAsyncEvent, *(iChildNodeErrHandler->GetAsyncErrEvent()));
            PVMFNodeInterface::ReportErrorEvent(event);
        }
    }
    iChildNodeErrHandler->Reset();
    //remove commands from the input command Q
    const int32 inputCmndsSz = iInputCommands.size();
    if (inputCmndsSz > 0 && (!iInputCommands[0].hipri()))	//For command at index 0 is hipri, let the command complete asynchronously in next AO cycles.
    {
        for (int ii = 0; ii < inputCmndsSz ; ii++)
        {
            if (iInputCommands[ii].iCmd !=  PVMF_SMFSP_NODE_RESET)
            {
                if (IsInternalCmd(iInputCommands[ii].iCmd))
                {
                    InternalCommandComplete(iInputCommands, iInputCommands[ii], PVMFFailure);
                }
                else
                {
                    CommandComplete(iInputCommands, iInputCommands[ii], PVMFFailure);
                }
            }
            else
                break;
        }
    }
    if (IsAdded())
    {
        RunIfNotReady();
    }

}

bool PVMFSMFSPBaseNode::SupressInfoEvent()
{
    if ((iInterfaceState == EPVMFNodeError) ||
            (!iCancelCommand.empty() || (iCurrentCommand.front().iCmd == PVMF_SMFSP_NODE_RESET)))
    {
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//PVMFSMFSPChildNodeErrorHandler
///////////////////////////////////////////////////////////////////////////////
PVMFSMFSPChildNodeErrorHandler* PVMFSMFSPChildNodeErrorHandler::CreateErrHandler(PVMFSMFSPBaseNode* aFSPBaseNode)
{
    PVMFSMFSPChildNodeErrorHandler* errHandler = NULL;
    int32 err;
    OSCL_TRY(err,
             errHandler = OSCL_NEW(PVMFSMFSPChildNodeErrorHandler, (aFSPBaseNode));
             errHandler->Construct();
            );
    return errHandler;
}

void PVMFSMFSPChildNodeErrorHandler::DeleteErrHandler(PVMFSMFSPChildNodeErrorHandler*& aErrHandler)
{
    if (aErrHandler)
        OSCL_DELETE(aErrHandler);
    aErrHandler = NULL;
}

void PVMFSMFSPChildNodeErrorHandler::InitiateErrorHandling(const PVMFCmdResp& aCmdResponse)
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::InitiateErrorHandling In CmdId [%d]", aCmdResponse.GetCmdId()));
    iCmdResponse = NULL;
    SaveErrorInfo(aCmdResponse);
    PerformErrorHandling();
}

void PVMFSMFSPChildNodeErrorHandler::InitiateErrorHandling(const PVMFAsyncEvent& aAsyncEvent)
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::InitiateErrorHandling In Event [%d]", aAsyncEvent.GetEventType()));
    iAsyncEvent = NULL;
    SaveErrorInfo(aAsyncEvent);
    PerformErrorHandling();
}

void PVMFSMFSPChildNodeErrorHandler::CompleteErrorHandling(const PVMFCmdResp& aResponse)	//called by NodeCommandCompleted
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::CompleteErrorHandling In CmdId [%d] ErrHandlerState [%d]", aResponse.GetCmdId(), iState));
    OSCL_UNUSED_ARG(aResponse);

    switch (iState)
    {
        case SMFSP_ERRHANDLER_WAITING_FOR_CANCEL_COMPLETION:
        {
            ContinueChildNodesCmdCancellation();
        }
        case SMFSP_ERRHANDLER_WAITING_FOR_CANCEL_DUE_TO_ERR_COMPLETION:
        {
            CompleteChildNodesCmdCancellationDueToErr();
        }
        break;
        case SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION:
        {
            OSCL_ASSERT(aResponse.GetCmdStatus() != PVMFFailure);
            CompleteChildNodesResetDueToError();
        }
        break;
        default:
            OSCL_ASSERT(false);
    }
}

void PVMFSMFSPChildNodeErrorHandler::Reset()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::Reset"));
    bool eventDataLenAvailable = false;
    uint32 eventDataLen = 0;
    if (SMFSP_ERR_SOURCE_EVENT == iErrSource)
    {
        iAsyncEvent->GetEventDataLen(eventDataLenAvailable, eventDataLen);
        if (iAsyncEvent)
        {
            if (eventDataLenAvailable)
            {
                OSCL_ARRAY_DELETE(OSCL_STATIC_CAST(octet*, iAsyncEvent->GetEventData()));
            }
            if (iAsyncEvent->GetEventExtensionInterface())
                iAsyncEvent->GetEventExtensionInterface()->removeRef();
            OSCL_DELETE(iAsyncEvent);
            iAsyncEvent = NULL;
        }
    }
    if (SMFSP_ERR_SOURCE_NODE_CMD_COMPLETION == iErrSource)
    {
        iCmdResponse->GetEventDataLen(eventDataLenAvailable, eventDataLen);
        if (iCmdResponse)
        {
            if (eventDataLenAvailable)
            {
                OSCL_ARRAY_DELETE(OSCL_STATIC_CAST(octet*, iCmdResponse->GetEventData()));
            }
            if (iCmdResponse->GetEventExtensionInterface())
                iCmdResponse->GetEventExtensionInterface()->removeRef();
            OSCL_DELETE(iCmdResponse);
            iCmdResponse = NULL;
        }
    }
    iErrCmd = NULL;
    if (iErroneousCmdResponse)
        OSCL_DELETE(iErroneousCmdResponse);
    iErrSource = SMFSP_ERR_SOURCE_INDETERMINATE;
    iState = SMFSP_ERRHANDLER_IDLE;
}

const PVMFCmdResp* PVMFSMFSPChildNodeErrorHandler::GetErroneousCmdResponse()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::GetErroneousCmdResponse [%x]", iErrCmd));
    if (iErrCmd)
        return iErroneousCmdResponse;
    else
        return NULL;
}

const PVMFAsyncEvent* PVMFSMFSPChildNodeErrorHandler::GetAsyncErrEvent()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::GetAsyncErrEvent [%x]", iAsyncEvent));
    if (SMFSP_ERR_SOURCE_EVENT == iErrSource)
        return iAsyncEvent;
    else
        return NULL;
}


void PVMFSMFSPChildNodeErrorHandler::SaveErrorInfo(const PVMFCmdResp& aCmdResponse)
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::SaveErrorInfo - ErrSource is Command Completion"));
    iErrSource = SMFSP_ERR_SOURCE_NODE_CMD_COMPLETION;
    if (!iCmdResponse)	//make deep copy of aCmdResponse, may be we can use the response info to persist event data too
    {
        bool eventDataLenAvailable = false;
        uint32 eventDataLen = 0;
        octet* eventData = OSCL_STATIC_CAST(octet*, aCmdResponse.GetEventData());
        aCmdResponse.GetEventDataLen(eventDataLenAvailable, eventDataLen);
        if (eventDataLenAvailable)
        {
            eventData = OSCL_ARRAY_NEW(octet, eventDataLen);
            oscl_memcpy(eventData, aCmdResponse.GetEventData(), eventDataLen);
            iCmdResponse = OSCL_NEW(PVMFCmdResp , (aCmdResponse.GetCmdId(), aCmdResponse.GetContext(), aCmdResponse.GetCmdStatus(), aCmdResponse.GetEventExtensionInterface(), eventData));
            iCmdResponse->SetEventDataLen(eventDataLen);
        }
        else
            iCmdResponse = OSCL_NEW(PVMFCmdResp , (aCmdResponse.GetCmdId(), aCmdResponse.GetContext(), aCmdResponse.GetCmdStatus(), aCmdResponse.GetEventExtensionInterface(), eventData));
        if (iCmdResponse->GetEventExtensionInterface())
            iCmdResponse->GetEventExtensionInterface()->addRef();
    }
}

void PVMFSMFSPChildNodeErrorHandler::SaveErrorInfo(const PVMFAsyncEvent& aAsyncEvent)
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::SaveErrorInfo - ErrSource is Event"));
    iErrSource = SMFSP_ERR_SOURCE_EVENT;
    if (!iAsyncEvent) //make deep copy of aAsyncEvent, may be we can use the event info to persist event data too
    {
        bool eventDataLenAvailable = false;
        uint32 eventDataLen = 0;
        octet* eventData = OSCL_STATIC_CAST(octet*, aAsyncEvent.GetEventData());
        aAsyncEvent.GetEventDataLen(eventDataLenAvailable, eventDataLen);
        if (eventDataLenAvailable)
        {
            eventData = OSCL_ARRAY_NEW(octet, eventDataLen);
            oscl_memcpy(eventData, aAsyncEvent.GetEventData(), eventDataLen);
            iAsyncEvent = OSCL_NEW(PVMFAsyncEvent, (OSCL_CONST_CAST(PVMFAsyncEvent&, aAsyncEvent).IsA(), aAsyncEvent.GetEventType(), OSCL_CONST_CAST(OsclAny*, (aAsyncEvent.GetContext())), aAsyncEvent.GetEventExtensionInterface(), eventData, aAsyncEvent.GetLocalBuffer(), aAsyncEvent.GetLocalBufferSize()));
            iAsyncEvent->SetEventDataLen(eventDataLen);
        }
        else
            iAsyncEvent = OSCL_NEW(PVMFAsyncEvent, (OSCL_CONST_CAST(PVMFAsyncEvent&, aAsyncEvent).IsA(), aAsyncEvent.GetEventType(), OSCL_CONST_CAST(OsclAny*, (aAsyncEvent.GetContext())), aAsyncEvent.GetEventExtensionInterface(), eventData, aAsyncEvent.GetLocalBuffer(), aAsyncEvent.GetLocalBufferSize()));
        if (iAsyncEvent->GetEventExtensionInterface())
            iAsyncEvent->GetEventExtensionInterface()->addRef();
    }
}

void PVMFSMFSPChildNodeErrorHandler::PerformErrorHandling()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::PerformErrorHandling In"));
    // Three Cases:
    //Case 1: There is some comand in the command Q that was being processed and error occurs
    //queue cancelduetoerr and cancelduetoreset in the error Q.
    //Let the canceduetoerr and resetduetoerr complete
    //Complete the pending command with error info

    //Case 2:There is some comand in the command Q that was being cancelled and error occurred while cancellation
    //Let the cancel complete but do not report comepltion to observer of SM node.
    //Queue the Resetduetoerr.
    //Complete resetduetoerr and notify current command's completion to the obs of the SM node
    //If the cancel command is CancelAll, notify command completion of all teh command in the input command Q, with command id < cmdid of cancelall
    //Notify command completion of cance/cancelall to the obs of the SM node


    //Case 3: There is no command in the current command Q and error was notified via error event
    //queue resetduetoerr in the error Q.
    //Let the resetduetoerr complete
    //Report err event to observer

    if (iSMFSPNode->iCancelCommand.size() > 0)
    {
        iErrCmd = &(iSMFSPNode->iCancelCommand.front());
    }
    else if (iSMFSPNode->iCurrentCommand.size() > 0)
    {
        iErrCmd = &(iSMFSPNode->iCurrentCommand.front());
    }

    if ((iErrSource == SMFSP_ERR_SOURCE_NODE_CMD_COMPLETION) && (NULL == iErrCmd))
    {
        iErrCmd = &(iSMFSPNode->iInputCommands.front());
    }

    if (iErrCmd)
    {
        bool eventDataLenAvailable = false;
        uint32 eventDataLength = 0;
        if (SMFSP_ERR_SOURCE_EVENT == iErrSource)
        {
            iErroneousCmdResponse = OSCL_NEW(PVMFCmdResp, (iErrCmd->iId, iAsyncEvent->GetContext(), PVMFFailure, iAsyncEvent->GetEventExtensionInterface(), iAsyncEvent->GetEventData()));
            iAsyncEvent->GetEventDataLen(eventDataLenAvailable, eventDataLength);
            if (eventDataLenAvailable)
                iErroneousCmdResponse->SetEventDataLen(eventDataLength);
        }
        else
        {
            iErroneousCmdResponse = OSCL_NEW(PVMFCmdResp , (iCmdResponse->GetCmdId(), iCmdResponse->GetContext(), iCmdResponse->GetCmdStatus(), iCmdResponse->GetEventExtensionInterface(), iCmdResponse->GetEventData()));
            iCmdResponse->GetEventDataLen(eventDataLenAvailable, eventDataLength);
            if (eventDataLenAvailable)
                iErroneousCmdResponse->SetEventDataLen(eventDataLength);
        }

        //Do we have any pending command with any of the child node?
        bool hasPendingChildNodeCmd = false;
        for (uint32 i = 0; i < iSMFSPNode->iFSPChildNodeContainerVec.size(); i++)
        {
            if (iSMFSPNode->iFSPChildNodeContainerVec[i].iNodeCmdState == PVMFSMFSP_NODE_CMD_PENDING)
            {
                hasPendingChildNodeCmd = true;
                break;
            }
        }
        if ((PVMF_SMFSP_NODE_CANCELALLCOMMANDS == iErrCmd->iCmd) || (PVMF_SMFSP_NODE_CANCELCOMMAND == iErrCmd->iCmd))
        {
            if (hasPendingChildNodeCmd)
                iState = SMFSP_ERRHANDLER_WAITING_FOR_CANCEL_COMPLETION;
            else
            {
                PVMFSessionId s = 0;
                PVMFSMFSPBaseNodeCommand cmdResetDueToErr;
                cmdResetDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                        PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR,
                        NULL);
                iSMFSPNode->QueueErrHandlingCommandL(cmdResetDueToErr);
                iState = SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION;
            }

        }
        else
        {
            if (hasPendingChildNodeCmd)
            {
                PVMFSessionId s = 0;
                PVMFSMFSPBaseNodeCommand cmdCancelPendingCmdDueToErr;
                cmdCancelPendingCmdDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                        PVMF_SMFSP_NODE_CANCEL_DUE_TO_ERROR,
                        NULL);
                iSMFSPNode->QueueErrHandlingCommandL(cmdCancelPendingCmdDueToErr);
                iState = SMFSP_ERRHANDLER_WAITING_FOR_CANCEL_DUE_TO_ERR_COMPLETION;
            }
            else
            {
                PVMFSessionId s = 0;
                PVMFSMFSPBaseNodeCommand cmdResetDueToErr;
                cmdResetDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                        PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR,
                        NULL);
                iSMFSPNode->QueueErrHandlingCommandL(cmdResetDueToErr);
                iState = SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION;
            }
        }
    }
    else
    {
        PVMFSessionId s = 0;
        PVMFSMFSPBaseNodeCommand cmdResetDueToErr;
        cmdResetDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR,
                NULL);
        iSMFSPNode->QueueErrHandlingCommandL(cmdResetDueToErr);
        iState = SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION;
    }
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::PerformErrorHandling Out iErrCmd - [%x] iState [%d]", iErrCmd, iState));
}

void PVMFSMFSPChildNodeErrorHandler::ContinueChildNodesCmdCancellation()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::ContinueChildNodesCmdCancellation In"));
    if (iSMFSPNode->CheckChildrenNodesCancelAll())
    {
        PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::ContinueChildNodesCmdCancellation - Q Reset Due To Err"));
        PVMFSessionId s = 0;
        PVMFSMFSPBaseNodeCommand cmdResetDueToErr;
        cmdResetDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR,
                NULL);
        iSMFSPNode->QueueErrHandlingCommandL(cmdResetDueToErr);
        iState = SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION;
    }
}

void PVMFSMFSPChildNodeErrorHandler::CompleteChildNodesCmdCancellationDueToErr()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::CompleteChildNodesCmdCancellationDueToErr In"));
    if (iSMFSPNode->CheckChildrenNodesCancelAll())
    {
        //There are two possible cases:
        //- Before command for cancellation due to error could even be queued up, all the pending commands got completed.
        //- Cancel due to error got completed
        if ((iSMFSPNode->iErrHandlingCommandQ.size() > 0) && (PVMF_SMFSP_NODE_CANCEL_DUE_TO_ERROR == iSMFSPNode->iErrHandlingCommandQ.front().iCmd))
        {
            iSMFSPNode->iErrHandlingCommandQ.Erase(&iSMFSPNode->iErrHandlingCommandQ.front());
            PVMFSessionId s = 0;
            PVMFSMFSPBaseNodeCommand cmdResetDueToErr;
            cmdResetDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                    PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR,
                    NULL);
            iSMFSPNode->QueueErrHandlingCommandL(cmdResetDueToErr);
            iState = SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION;
        }
        else
        {
            PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::CompleteChildNodesCmdCancellationDueToErr- Cancellation complete"));
            OSCL_ASSERT(iSMFSPNode->iCurrErrHandlingCommand.size() > 0 && (PVMF_SMFSP_NODE_CANCEL_DUE_TO_ERROR == iSMFSPNode->iCurrErrHandlingCommand.front().iCmd));
            ErrHandlingCommandComplete(iSMFSPNode->iCurrErrHandlingCommand, iSMFSPNode->iCurrErrHandlingCommand.front(), PVMFSuccess);
        }
    }
    return;
}

void PVMFSMFSPChildNodeErrorHandler::CompleteChildNodesResetDueToError()
{
    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::CompleteChildNodesResetDueToErrorIn"));
    if (iSMFSPNode->CheckChildrenNodesReset() && iSMFSPNode->iDRMResetPending == false)
    {
        PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::CompleteChildNodesResetDueToErrorIn - Restting Child Nodes complete %d", iSMFSPNode->iDRMResetPending));
        OSCL_ASSERT(iSMFSPNode->iCurrErrHandlingCommand.size() > 0 && (iSMFSPNode->iCurrErrHandlingCommand.front().iCmd == PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR));
        ErrHandlingCommandComplete(iSMFSPNode->iCurrErrHandlingCommand, iSMFSPNode->iCurrErrHandlingCommand.front(), PVMFSuccess);
    }
}


void PVMFSMFSPChildNodeErrorHandler::ErrHandlingCommandComplete(PVMFFSPNodeCmdQ& aCmdQ,
        PVMFSMFSPBaseNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aData,
        PVUuid* aEventUUID,
        int32* aEventCode,
        PVInterface* aExtMsg)
{
    OSCL_UNUSED_ARG(aStatus);
    OSCL_UNUSED_ARG(aData);
    OSCL_UNUSED_ARG(aEventUUID);
    OSCL_UNUSED_ARG(aEventCode);
    OSCL_UNUSED_ARG(aExtMsg);

    PVMF_SM_ERRHANDLER_LOGSTACKTRACE((0, "PVMFSMFSPChildNodeErrorHandler::ErrHandlingCommandComplete In iCmd[%d]", aCmd.iCmd));
    switch (aCmd.iCmd)
    {
        case PVMF_SMFSP_NODE_CANCEL_DUE_TO_ERROR:
        {
            PVMF_SM_ERRHANDLER_LOGDEBUG((0, "PVMFSMFSPChildNodeErrorHandler::ErrHandlingCommandComplete Queing Reset Due To Err"));
            PVMFSessionId s = 0;
            PVMFSMFSPBaseNodeCommand cmdResetDueToErr;
            cmdResetDueToErr.PVMFSMFSPBaseNodeCommandBase::Construct(s,
                    PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR,
                    NULL);
            iSMFSPNode->QueueErrHandlingCommandL(cmdResetDueToErr);
            iState = SMFSP_ERRHANDLER_WAITING_FOR_RESET_DUE_TO_ERR_COMPLETION;
        }
        break;
        case PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR:
        {
            PVMF_SM_ERRHANDLER_LOGDEBUG((0, "PVMFSMFSPChildNodeErrorHandler::ErrHandlingCommandComplete - Reset Due To Err completed"));

            //Do command completion or notify err event
            iState = SMFSP_ERRHANDLER_IDLE;	//error handling complete
            iSMFSPNode->CompleteResetDueToErr();
            iSMFSPNode->ErrHandlingComplete(iErrCmd);
        }
        break;
    }

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Reschedule AO if input command queue is not empty */
    if (!iSMFSPNode->iErrHandlingCommandQ.empty() && iSMFSPNode->IsAdded())
    {
        iSMFSPNode->RunIfNotReady();
    }
}

///////////////////////////////////////////////////////////////////////////////
//PVMFSMNodeKVPStore
///////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFSMNodeKVPStore::addKVPString(const char* aKeyTypeString, OSCL_wString& aValString)
{
    PvmiKvp aKeyVal;
    aKeyVal.key = NULL;
    PVMFStatus status = PVMFCreateKVPUtils::CreateKVPForWStringValue(aKeyVal, aKeyTypeString, aValString);
    if (status != PVMFSuccess) return status;
    KVPValueTypeForMemoryRelease valType = KVPValueTypeForMemoryRelease_WString;
    return pushKVPToVector(aKeyVal, valType);
}

// add kvp string with normal string value
PVMFStatus PVMFSMNodeKVPStore::addKVPString(const char* aKeyTypeString, const char* aValString)
{
    PvmiKvp aKeyVal;
    aKeyVal.key = NULL;
    PVMFStatus status = PVMFCreateKVPUtils::CreateKVPForCharStringValue(aKeyVal, aKeyTypeString, aValString);
    if (status != PVMFSuccess) return status;
    KVPValueTypeForMemoryRelease valType = KVPValueTypeForMemoryRelease_String;
    return pushKVPToVector(aKeyVal, valType);
}

void PVMFSMNodeKVPStore::releaseMemory()
{
    OSCL_ASSERT(iKvpVector.size() == iKVPValueTypeForMemoryRelease.size());

    for (uint32 i = 0; i < iKvpVector.size(); i++)
    {
        if (iKvpVector[i].key) OSCL_ARRAY_DELETE(iKvpVector[i].key);

        // release memory for appropriate types of KVP value
        if ((KVPValueTypeForMemoryRelease)iKVPValueTypeForMemoryRelease[i] == KVPValueTypeForMemoryRelease_WString &&
                iKvpVector[i].value.pWChar_value) OSCL_ARRAY_DELETE(iKvpVector[i].value.pWChar_value);
        if ((KVPValueTypeForMemoryRelease)iKVPValueTypeForMemoryRelease[i] == KVPValueTypeForMemoryRelease_String &&
                iKvpVector[i].value.pChar_value) OSCL_ARRAY_DELETE(iKvpVector[i].value.pChar_value);
    }
}

PVMFStatus PVMFSMNodeKVPStore::addKVPuint32Value(const char* aKeyTypeString, uint32 aValue)
{
    PvmiKvp aKeyVal;
    aKeyVal.key = NULL;
    PVMFStatus status = PVMFCreateKVPUtils::CreateKVPForUInt32Value(aKeyVal, aKeyTypeString, aValue);
    if (status != PVMFSuccess) return status;
    KVPValueTypeForMemoryRelease valType = KVPValueTypeForMemoryRelease_NoInterest;
    return pushKVPToVector(aKeyVal, valType);
}

PVMFStatus PVMFSMNodeKVPStore::pushKVPToVector(const PvmiKvp& aKeyVal, const KVPValueTypeForMemoryRelease& aValueTypeForMemoryRelease)
{
    int32 err = 0;
    OSCL_TRY(err,
             iKvpVector.push_back(aKeyVal);
             iKVPValueTypeForMemoryRelease.push_back(OSCL_STATIC_CAST(uint32, aValueTypeForMemoryRelease));
            );
    return (err == 0 ? PVMFSuccess : PVMFErrNoMemory);
}

