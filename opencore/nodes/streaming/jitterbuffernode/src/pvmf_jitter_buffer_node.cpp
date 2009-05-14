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
#ifndef PVMF_JB_JITTERBUFFERMISC_H_INCLUDED
#include "pvmf_jitter_buffer_node.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_TYPES_H_INCLUDED
#include "pvmf_jitter_buffer_common_types.h"
#endif


#ifndef OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef __MEDIA_CLOCK_CONVERTER_H
#include "media_clock_converter.h"
#endif

#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_jitter_buffer_node.h"
#endif

#ifndef PVMF_JITTER_BUFFER_FACTORY_H
#include "pvmf_jitter_buffer_factory.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif

#ifndef PVMF_MEDIA_MSG_FORMAT_IDS_H_INCLUDED
#include "pvmf_media_msg_format_ids.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#ifndef OSCL_RAND_H_INCLUDED
#include "oscl_rand.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

//Construction and Destruction
OSCL_EXPORT_REF PVMFJitterBufferNode::PVMFJitterBufferNode(int32 aPriority,
        JitterBufferFactory* aJBFactory): OsclActiveObject(aPriority, "JitterBufferNode")
{
    //Initialize capability
    iCapability.iCanSupportMultipleInputPorts = true;
    iCapability.iCanSupportMultipleOutputPorts = true;
    iCapability.iHasMaxNumberOfPorts = false;
    iCapability.iMaxNumberOfPorts = 0;//no maximum
    iCapability.iInputFormatCapability.push_back(PVMF_MIME_RTP);
    iCapability.iInputFormatCapability.push_back(PVMF_MIME_ASFFF);
    iCapability.iInputFormatCapability.push_back(PVMF_MIME_RMFF);
    iCapability.iOutputFormatCapability.push_back(PVMF_MIME_RTP);
    iCapability.iOutputFormatCapability.push_back(PVMF_MIME_ASFFF);
    //Jitter buffer factory
    ipJitterBufferFactory	=	aJBFactory;

    //Initialize loggers
    ipLogger = NULL;
    ipDataPathLogger = NULL;
    ipDataPathLoggerIn = NULL;
    ipDataPathLoggerOut = NULL;
    ipDataPathLoggerFlowCtrl = NULL;
    ipClockLogger = NULL;
    ipClockLoggerSessionDuration = NULL;
    ipClockLoggerRebuff = NULL;
    ipDiagnosticsLogger = NULL;
    ipJBEventsClockLogger = NULL;

    //Diagniostics related
    iDiagnosticsLogged = false;
    iNumRunL = 0;

    Construct();
    ResetNodeParams(false);
}

void PVMFJitterBufferNode::Construct()
{
    //creation and initialization of objects that need to be created on heap in the ctor is done here
    iInputCommands.Construct(PVMF_JITTER_BUFFER_NODE_COMMAND_ID_START,
                             PVMF_JITTER_BUFFER_VECTOR_RESERVE);
    iCurrentCommand.Construct(0, 1);

    iPortVector.Construct(PVMF_JITTER_BUFFER_NODE_PORT_VECTOR_RESERVE);
}

void PVMFJitterBufferNode::ResetNodeParams(bool aReleaseMemory)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ResetNodeParams In aReleaseMemory[%d]", aReleaseMemory));
    //Session specific initializations and resetting

    oStartPending = false;
    oStopOutputPorts = true;
    iPauseTime = 0;
    ipClientPlayBackClock = NULL;
    iMediaReceiveingChannelPrepared = false;

    iBroadCastSession = false;

    iDelayEstablished = false;
    iJitterBufferState = PVMF_JITTER_BUFFER_READY;
    iJitterDelayPercent = 0;

    //Extension interface initializations
    if (ipExtensionInterface && aReleaseMemory)
    {
        ipExtensionInterface->removeRef();
    }
    ipExtensionInterface = NULL;

    //Variables to persist info passed on by the extension interface
    iRebufferingThreshold = DEFAULT_JITTER_BUFFER_UNDERFLOW_THRESHOLD_IN_MS;
    iJitterBufferDurationInMilliSeconds = DEFAULT_JITTER_BUFFER_DURATION_IN_MS;
    iMaxInactivityDurationForMediaInMs = DEFAULT_MAX_INACTIVITY_DURATION_IN_MS;
    iEstimatedServerKeepAheadInMilliSeconds = DEFAULT_ESTIMATED_SERVER_KEEPAHEAD_FOR_OOO_SYNC_IN_MS;

    iJitterBufferSz = 0;
    iMaxNumBufferResizes = DEFAULT_MAX_NUM_SOCKETMEMPOOL_RESIZES;
    iBufferResizeSize = DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT;
    iBufferingStatusIntervalInMs =
        (PVMF_JITTER_BUFFER_BUFFERING_STATUS_EVENT_CYCLES * 1000) / PVMF_JITTER_BUFFER_BUFFERING_STATUS_EVENT_FREQUENCY;

    iDisableFireWallPackets = false;
    //iPlayingAfterSeek = false;

    //Event Notifier initialization/reseting
    iIncomingMediaInactivityDurationCallBkId = 0;
    iIncomingMediaInactivityDurationCallBkPending = false;
    iNotifyBufferingStatusCallBkId = 0;
    iNotifyBufferingStatusCallBkPending = false;


    if (aReleaseMemory)
    {
        if (ipJitterBufferMisc)
            OSCL_DELETE(ipJitterBufferMisc);
    }

    ipJitterBufferMisc = NULL;
    ipEventNotifier = NULL;


    /* Clear queued messages in ports */
    uint32 i;
    for (i = 0; i < iPortVector.size(); i++)
    {
        PVMFJitterBufferPortParams* pPortParams = NULL;
        bool bRet = getPortContainer(iPortVector[i], pPortParams);
        if (bRet)
        {
            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                pPortParams->ipJitterBuffer->ResetJitterBuffer();
            }

            pPortParams->ResetParams();
        }
        iPortVector[i]->ClearMsgQueues();
    }

    //Cleaning up of conatiner objects
    /* delete corresponding port params */

    if (aReleaseMemory)
    {
        //port vect and port params Q
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
        for (it = iPortParamsQueue.begin();
                it != iPortParamsQueue.end();
                it++)
        {
            PVMFJitterBufferPortParams* pPortParams = *it;

            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                if (ipJitterBufferFactory)
                    ipJitterBufferFactory->Destroy(pPortParams->ipJitterBuffer);
            }

            OSCL_DELETE(&pPortParams->irPort);
            OSCL_DELETE(pPortParams);
        }
        iPortParamsQueue.clear();
        iPortVector.clear();
        iPortVector.Reconstruct();
    }

    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ResetNodeParams Out -"));
    return;
}

PVMFJitterBufferNode::~PVMFJitterBufferNode()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::~PVMFJitterBufferNode In"));
    LogSessionDiagnostics();
    ResetNodeParams();

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

    Cancel();
    /* thread logoff */
    if (IsAdded())
        RemoveFromScheduler();
    CleanUp();
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::~PVMFJitterBufferNode Out"));
}

void PVMFJitterBufferNode::CleanUp()	//Reverse of Construct
{
    //noop
}

///////////////////////////////////////////////////////////////////////////////
//Implementation of overrides from PVInterface
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Checks if the instance of PVMFJitterBufferExtensionInterfaceImpl is existing
//If existing: Query from this interface if UUID mentioned is supported
//If not existing: Instantiate PVMFJitterBufferExtensionInterfaceImpl
//and query requested interface from the PVMFJitterBufferExtensionInterfaceImpl
//Return Values:true/false
//Leave Codes: OsclErrNoMemory
//Leave Condition: If instance of PVMFJitterBufferExtensionInterfaceImpl cannot
//be instantiated.
///////////////////////////////////////////////////////////////////////////////
bool PVMFJitterBufferNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::queryInterface In"));
    iface = NULL;
    if (uuid == PVUuid(PVMF_JITTERBUFFERNODE_EXTENSIONINTERFACE_UUID))
    {
        if (!ipExtensionInterface)
        {
            OsclMemAllocator alloc;
            int32 err;
            OsclAny*ptr = NULL;
            OSCL_TRY(err,
                     ptr = alloc.ALLOCATE(sizeof(PVMFJitterBufferExtensionInterfaceImpl));
                    );
            if (err != OsclErrNone || !ptr)
            {
                PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::queryInterface: Error - Out of memory"));
                OSCL_LEAVE(OsclErrNoMemory);
            }
            ipExtensionInterface =
                OSCL_PLACEMENT_NEW(ptr, PVMFJitterBufferExtensionInterfaceImpl(this));
        }
        return (ipExtensionInterface->queryInterface(uuid, iface));
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
//Implementation  of overrides from PVMFNodeInterface
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//Does thread-specific node creation and go to "Idle" state.
//Creates logger objects
//Adds the AO to the scheduler
//Return values: PVMFSuccess/PVMFErrInvalidState
//PVMFSuccess: If API call is successful and was made in EPVMFNodeCreated state
//PVMFErrInvalidState: If API is called in the invalid state
//Leave Codes: NA
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferNode::ThreadLogon()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ThreadLogon In"));
    PVMFStatus status;

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
        {
            if (!IsAdded())
                AddToScheduler();

            ipLogger = PVLogger::GetLoggerObject("jitterbuffernode");
            ipDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffernode");
            ipDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffernode.in");
            ipDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffernode.out");
            ipDataPathLoggerFlowCtrl = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffernode.flowctrl");

            ipClockLogger = PVLogger::GetLoggerObject("clock.jitterbuffernode");
            ipClockLoggerSessionDuration = PVLogger::GetLoggerObject("clock.streaming_manager.sessionduration");
            ipClockLoggerRebuff = PVLogger::GetLoggerObject("clock.jitterbuffernode.rebuffer");

            ipDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.streamingmanager");
            ipJBEventsClockLogger = PVLogger::GetLoggerObject("jitterbuffernode.eventsclock");

            iDiagnosticsLogged = false;
            SetState(EPVMFNodeIdle);
            status = PVMFSuccess;
        }
        break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ThreadLogon Out retval - %d", status));
    return status;
}

///////////////////////////////////////////////////////////////////////////////
//Does thread-specific node cleanup and go to "Created" state.
//Releases logger objects
//Removes the AO to the scheduler
//Return values: PVMFSuccess/PVMFErrInvalidState
//PVMFSuccess: If API call is successful and was made in EPVMFNodeIdle state
//PVMFErrInvalidState: If API is called in the invalid state
//Leave Codes: NA
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferNode::ThreadLogoff()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ThreadLogoff In"));
    PVMFStatus status = PVMFFailure;

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
        {
            ResetNodeParams();
            ipLogger = NULL;
            ipDataPathLogger = NULL;
            ipDataPathLoggerIn = NULL;
            ipDataPathLoggerOut = NULL;
            ipClockLogger = NULL;
            ipClockLoggerSessionDuration = NULL;
            ipDiagnosticsLogger = NULL;
            ipDataPathLoggerFlowCtrl = NULL;
            if (IsAdded())
            {
                RemoveFromScheduler();
            }
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

///////////////////////////////////////////////////////////////////////////////
//Retrieves node capabilities.
//Decides supported input/output formats and provides node capabilities
//Return values: PVMFSuccess/PVMFErrInvalidState
//PVMFSuccess: If API call is successful
//If Input/Output format could not be determined
//Leave Codes: NA
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::GetCapability In"));
    aNodeCapability = iCapability;
    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//Retrives a port iterator.
//Can Leave:No
//Return values: PVMFSuccess/PVMFErrInvalidState
//PVMFSuccess - If API call is successful
////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PVMFJitterBufferNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::GetPorts"));
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}

///////////////////////////////////////////////////////////////////////////////
//Retrives a port iterator.
//Can Leave:No
//Return values: PVMFSuccess/PVMFErrInvalidState
//PVMFSuccess - If API call is successful
//PVMFErrInvalidState - If API is called in the invalid state
////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::QueryUUID(PVMFSessionId s,
        const PvmfMimeString& aMimeType,
        Oscl_Vector< PVUuid, OsclMemAllocator >& aUuids,
        bool aExactUuidsOnly ,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::QueryUUID"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_QUERYUUID,
            aMimeType,
            aUuids,
            aExactUuidsOnly,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::QueryInterface(PVMFSessionId s,
        const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:QueryInterface"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_QUERYINTERFACE,
            aUuid,
            aInterfacePtr,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::RequestPort(PVMFSessionId s,
        int32 aPortTag,
        const PvmfMimeString* aPortConfig,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:RequestPort"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_REQUESTPORT,
            aPortTag,
            aPortConfig,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::ReleasePort(PVMFSessionId s,
        PVMFPortInterface& aPort,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:ReleasePort"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_RELEASEPORT,
            aPort,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Init(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Init"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_INIT,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Prepare(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Prepare"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_PREPARE,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Start(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Start"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_START,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Stop(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Stop"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_STOP,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Flush(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Flush"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_FLUSH,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Pause(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Pause"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_PAUSE,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::Reset(PVMFSessionId s,
        const OsclAny* aContext)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:Reset"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_RESET,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::CancelAllCommands(PVMFSessionId s,
        const OsclAny* aContextData)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:CancelAllCommands"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_CANCELALLCOMMANDS,
            aContextData);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFJitterBufferNode::CancelCommand(PVMFSessionId s,
        PVMFCommandId aCmdId,
        const OsclAny* aContextData)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:CancelCommand"));
    PVMFJitterBufferNodeCommand cmd;
    cmd.PVMFJitterBufferNodeCommandBase::Construct(s,
            PVMF_JITTER_BUFFER_NODE_CANCELCOMMAND,
            aCmdId,
            aContextData);
    return QueueCommandL(cmd);
}

void PVMFJitterBufferNode::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::PortActivity: port=0x%x, type=%d",
                         aActivity.iPort, aActivity.iType));

    PVMFJitterBufferPortParams* portParamsPtr = NULL;
    PVMFJitterBufferPort* jbPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aActivity.iPort);
    portParamsPtr = jbPort->iPortParams;

    if (aActivity.iType != PVMF_PORT_ACTIVITY_DELETED)
    {
        if (portParamsPtr == NULL)
        {
            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
            PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferNode::HandlePortActivity - getPortContainer Failed", this));
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
            /*
             * Report port deleted info event to the node.
             */
            ReportInfoEvent(PVMFInfoPortDeleted, (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
        {
            if (ipJitterBufferMisc)
            {
                LogSessionDiagnostics();
                ipJitterBufferMisc->StreamingSessionStopped();
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
        {
            if (portParamsPtr->iProcessOutgoingMessages)
            {
                /*
                 * An outgoing message was queued on this port.
                 * All ports have outgoing messages
                 * in this node
                 */
                QueuePortActivity(portParamsPtr, aActivity);
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
        {
            /*
             * An outgoing message was queued on this port.
             * Only input and feedback ports have incoming messages
             * in this node
             */
            int32 portTag = portParamsPtr->iTag;
            switch (portTag)
            {
                case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
                case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
                    if (portParamsPtr->iProcessIncomingMessages)
                    {
                        QueuePortActivity(portParamsPtr, aActivity);
                    }
                    break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
        {
            int32 portTag = portParamsPtr->iTag;
            switch (portTag)
            {
                case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
                    /*
                     * We typically use incoming port's outgoing q
                     * only in case of 3GPP streaming, wherein we
                     * send firewall packets. If it is busy, it does
                     * not stop us from registering incoming data pkts.
                     * so do nothing.
                     */
                    break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port cannot accept any more
                     * msgs on its outgoing queue. This would usually imply that
                     * the corresponding input port must stop processing messages,
                     * however in case of jitter buffer the input and output ports
                     * are separated by a huge jitter buffer. Therefore continue
                     * to process the input.
                     */
                }
                break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
                    portParamsPtr->iProcessIncomingMessages = false;
                    break;

                default:
                    OSCL_ASSERT(false);
                    break;
            }
        }
        break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
        {
            int32 portTag = portParamsPtr->iTag;
            /*
             * Outgoing queue was previously busy, but is now ready.
             * We may need to schedule new processing events depending
             * on the port type.
             */
            switch (portTag)
            {
                case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
                    /*
                     * We never did anything in PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY
                     * so do nothing
                     */
                    break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port can accept more
                     * msgs on its outgoing queue. This implies that the corresponding
                     * input port can start processing messages again.
                     */
                    PVMFJitterBufferPort* jbPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aActivity.iPort);
                    PVMFJitterBufferPortParams* inPortParams = jbPort->iCounterpartPortParams;
                    if (inPortParams != NULL)
                    {
                        inPortParams->iProcessIncomingMessages = true;
                    }
                    else
                    {
                        OSCL_ASSERT(false);
                    }
                }
                break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
                    portParamsPtr->iProcessIncomingMessages = true;
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
            int32 portTag = portParamsPtr->iTag;
            switch (portTag)
            {
                case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
                    break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT:
                {
                    /*
                     * This implies that this output port cannot send any more
                     * msgs from its outgoing queue. It should stop processing
                     * messages till the connect port is ready.
                     */
                    portParamsPtr->iProcessOutgoingMessages = false;
                }
                break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
                    portParamsPtr->iProcessOutgoingMessages = false;
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
            int32 portTag = portParamsPtr->iTag;
            switch (portTag)
            {
                case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
                    break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT:
                    /*
                     * This implies that this output port can now send
                     * msgs from its outgoing queue. It can start processing
                     * messages now.
                     */
                    portParamsPtr->iProcessOutgoingMessages = true;
                    break;

                case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
                    portParamsPtr->iProcessOutgoingMessages = true;
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

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////
void PVMFJitterBufferNode::QueuePortActivity(PVMFJitterBufferPortParams* aPortParams,
        const PVMFPortActivity &aActivity)
{
    OSCL_UNUSED_ARG(aPortParams);
    OSCL_UNUSED_ARG(aActivity);

    if (IsAdded())
    {
        /*
         * wake up the AO to process the port activity event.
         */
        RunIfNotReady();
    }
}

///////////////////////////////////////////////////////////////////////////////
//Extension interfaces function implementation
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval)
{
    OSCL_UNUSED_ARG(aRTCPInterval);
}

bool PVMFJitterBufferNode::SetPortParams(PVMFPortInterface* aPort,
        uint32 aTimeScale,
        uint32 aBitRate,
        OsclRefCounterMemFrag& aConfig,
        bool aRateAdaptation,
        uint32 aRateAdaptationFeedBackFrequency)
{
    return SetPortParams(aPort, aTimeScale, aBitRate, aConfig, aRateAdaptation,
                         aRateAdaptationFeedBackFrequency, false);
}

bool PVMFJitterBufferNode::SetPlayRange(int32 aStartTimeInMS,
                                        int32 aStopTimeInMS,
                                        bool aPlayAfterASeek,
                                        bool aStopTimeAvailable)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetPlayRange In StartTime[%d], StopTime[%d] StopTimeValid[%d] PlayingAfterSeek[%d]", aStartTimeInMS, aStopTimeInMS, aStopTimeAvailable, aPlayAfterASeek));
    ipJitterBufferMisc->SetPlayRange(aStartTimeInMS, aStopTimeInMS, aPlayAfterASeek, aStopTimeAvailable);
    return true;
}

void PVMFJitterBufferNode::SetPlayBackThresholdInMilliSeconds(uint32 aThreshold)
{
    OSCL_UNUSED_ARG(aThreshold);
}

void PVMFJitterBufferNode::SetJitterBufferRebufferingThresholdInMilliSeconds(uint32 aThreshold)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetJitterBufferRebufferingThresholdInMilliSeconds Threshhold[%d]", aThreshold));
    if (aThreshold < iJitterBufferDurationInMilliSeconds)
    {
        iRebufferingThreshold = aThreshold;
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
        for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
        {
            PVMFJitterBufferPortParams* pPortParams = *it;
            if ((pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT))
            {
                PVMFJitterBuffer* jitterBuffer = pPortParams->ipJitterBuffer;
                if (jitterBuffer)
                {
                    jitterBuffer->SetRebufferingThresholdInMilliSeconds(aThreshold);
                }
            }
        }
    }
}

void PVMFJitterBufferNode::GetJitterBufferRebufferingThresholdInMilliSeconds(uint32& aThreshold)
{
    aThreshold = iRebufferingThreshold;
}

void PVMFJitterBufferNode::SetJitterBufferDurationInMilliSeconds(uint32 aDuration)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetJitterBufferDurationInMilliSeconds Duration [%d]", aDuration));
    uint32 duration = iJitterBufferDurationInMilliSeconds;
    if (aDuration > iRebufferingThreshold)
    {
        duration = aDuration;
        iJitterBufferDurationInMilliSeconds = duration;
    }

    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
    {
        PVMFJitterBufferPortParams* pPortParams  = *it;
        if ((pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT))
        {
            PVMFJitterBuffer* jitterBuffer = pPortParams->ipJitterBuffer;
            if (jitterBuffer)
            {
                jitterBuffer->SetDurationInMilliSeconds(duration);
            }
        }
    }
}

void PVMFJitterBufferNode::GetJitterBufferDurationInMilliSeconds(uint32& duration)
{
    duration = iJitterBufferDurationInMilliSeconds;
}

void PVMFJitterBufferNode::SetEarlyDecodingTimeInMilliSeconds(uint32 duration)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetEarlyDecodingTimeInMilliSeconds - Early Decoding Time [%d]", duration));
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = iPortParamsQueue.begin(); iter != iPortParamsQueue.end(); iter++)
    {
        PVMFJitterBufferPortParams* pPortParams = *iter;
        if (pPortParams && (pPortParams->ipJitterBuffer) && (PVMF_JITTER_BUFFER_PORT_TYPE_INPUT == pPortParams->iTag))
        {
            pPortParams->ipJitterBuffer->SetEarlyDecodingTimeInMilliSeconds(duration);
        }
    }
}

void PVMFJitterBufferNode::SetBurstThreshold(float burstThreshold)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetBurstThreshold burstThreshold[%f]", burstThreshold));
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = iPortParamsQueue.begin(); iter != iPortParamsQueue.end(); iter++)
    {
        PVMFJitterBufferPortParams* pPortParams = *iter;
        if (pPortParams && (pPortParams->ipJitterBuffer) && (PVMF_JITTER_BUFFER_PORT_TYPE_INPUT == pPortParams->iTag))
        {
            pPortParams->ipJitterBuffer->SetBurstThreshold(burstThreshold);
        }
    }
}

void PVMFJitterBufferNode::SetMaxInactivityDurationForMediaInMs(uint32 aDuration)
{
    iMaxInactivityDurationForMediaInMs = aDuration;
}

void PVMFJitterBufferNode::GetMaxInactivityDurationForMediaInMs(uint32& aDuration)
{
    aDuration = iMaxInactivityDurationForMediaInMs;
}

void PVMFJitterBufferNode::SetClientPlayBackClock(PVMFMediaClock* aClientClock)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetClientPlayBackClock %x", aClientClock));
    //remove ourself as observer of old clock, if any.
    //Todo: Repetition should make sence only after call to Reset function.
    ipClientPlayBackClock = aClientClock;
}

bool PVMFJitterBufferNode::PrepareForRepositioning(bool oUseExpectedClientClockVal ,
        uint32 aExpectedClientClockVal)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::PrepareForRepositioning oUseExpectedClientClockVal[%d], aExpectedClientClockVal[%d]", oUseExpectedClientClockVal, aExpectedClientClockVal));
    iJitterBufferState = PVMF_JITTER_BUFFER_IN_TRANSITION;
    ipJitterBufferMisc->PrepareForRepositioning(oUseExpectedClientClockVal, aExpectedClientClockVal);
    return true;
}

bool PVMFJitterBufferNode::SetPortSSRC(PVMFPortInterface* aPort, uint32 aSSRC)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetPortSSRC aPort[%x], aSSRC[%d]", aPort, aSSRC));
    return ipJitterBufferMisc->SetPortSSRC(aPort, aSSRC);
}

bool PVMFJitterBufferNode::SetPortRTPParams(PVMFPortInterface* aPort,
        bool   aSeqNumBasePresent,
        uint32 aSeqNumBase,
        bool   aRTPTimeBasePresent,
        uint32 aRTPTimeBase,
        bool   aNPTTimeBasePresent,
        uint32 aNPTInMS,
        bool oPlayAfterASeek)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetPortRTPParams In Port - 0x%x", aPort));
    uint32 i;
    //The above method is called only during 3GPP repositioning, however, since the aPort param in the signature
    // takes care only of the input port, the output port msg queues aren't cleared.
    // As a result ClearMsgQueues need to be called explicity on all the ports.
    //The oPlayAfterASeek check is necessary since the clearing of msg queues has to be carried out only during repositioning,
    // not otherwise
    if (oPlayAfterASeek)
    {
        for (i = 0; i < iPortParamsQueue.size(); i++)
        {
            PVMFJitterBufferPortParams* pPortParams = iPortParamsQueue[i];
            pPortParams->irPort.ClearMsgQueues();
        }
    }
    for (i = 0; i < iPortParamsQueue.size(); i++)
    {
        PVMFJitterBufferPortParams* pPortParams = iPortParamsQueue[i];

        if (&pPortParams->irPort == aPort)
        {
            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                if (pPortParams->ipJitterBuffer != NULL)
                {
                    PVMFRTPInfoParams rtpInfoParams;
                    rtpInfoParams.seqNumBaseSet = aSeqNumBasePresent;
                    rtpInfoParams.seqNum = aSeqNumBase;
                    rtpInfoParams.rtpTimeBaseSet = aRTPTimeBasePresent;
                    rtpInfoParams.rtpTime = aRTPTimeBase;
                    rtpInfoParams.nptTimeBaseSet = aNPTTimeBasePresent;
                    rtpInfoParams.nptTimeInMS = aNPTInMS;
                    rtpInfoParams.rtpTimeScale = pPortParams->iTimeScale;
                    pPortParams->ipJitterBuffer->setRTPInfoParams(rtpInfoParams, oPlayAfterASeek);
                    /* In case this is after a reposition purge the jitter buffer */
                    if (oPlayAfterASeek)
                    {
                        uint32 timebase32 = 0;
                        uint32 clientClock32 = 0;
                        bool overflowFlag = false;
                        if (ipClientPlayBackClock != NULL)
                            ipClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)

                        PVMF_JBNODE_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::setPortRTPParams - Purging Upto SeqNum =%d", aSeqNumBase));
                        PVMF_JBNODE_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::setPortRTPParams - Before Purge - ClientClock=%d",
                                                     clientClock32));
#endif
                        pPortParams->ipJitterBuffer->PurgeElementsWithSeqNumsLessThan(aSeqNumBase,
                                clientClock32);
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                        PVMF_JBNODE_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::setPortRTPParams - After Purge - ClientClock=%d",
                                                     clientClock32));
#endif
                        /*
                         * Since we flushed the jitter buffer, set it to ready state,
                         * reset the delay flag
                         */
                        iDelayEstablished = false;
                        iJitterBufferState = PVMF_JITTER_BUFFER_READY;
                        //iPlayingAfterSeek = true;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool PVMFJitterBufferNode::SetPortRTCPParams(PVMFPortInterface* aPort,
        int aNumSenders,
        uint32 aRR,
        uint32 aRS)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetPortRTCPParams aPort - [0x%x]", aPort));
    return ipJitterBufferMisc->SetPortRTCPParams(aPort, aNumSenders, aRR, aRS);
}

PVMFTimestamp PVMFJitterBufferNode::GetActualMediaDataTSAfterSeek()
{
    return ipJitterBufferMisc->GetActualMediaDataTSAfterSeek();
}

PVMFTimestamp PVMFJitterBufferNode::GetMaxMediaDataTS()
{
    return ipJitterBufferMisc->GetMaxMediaDataTS();
}

PVMFStatus PVMFJitterBufferNode::SetServerInfo(PVMFJitterBufferFireWallPacketInfo& aServerInfo)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetServerInfo In"));
    if (iDisableFireWallPackets == false)
    {
        ipJitterBufferMisc->SetServerInfo(aServerInfo);
    }
    else
    {
        PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::setServerInfo: FW Pkts Disabled"));
        if (iCurrentCommand.size() > 0)
        {
            if (iCurrentCommand.front().iCmd == PVMF_JITTER_BUFFER_NODE_PREPARE)
            {
                /* No firewall packet exchange - Complete Prepare */
                CompletePrepare();
            }
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVMFJitterBufferNode::NotifyOutOfBandEOS()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS In"));
    // Ignore Out Of Band EOS for any Non Live stream
    if (ipJitterBufferMisc && (!ipJitterBufferMisc->PlayStopTimeAvailable()))
    {
        if (iJitterBufferState != PVMF_JITTER_BUFFER_IN_TRANSITION)
        {
            ipJitterBufferMisc->SetSessionDurationExpired();
            CancelEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
            PVMF_JBNODE_LOGDATATRAFFIC((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS - Out Of Band EOS Recvd"));
            PVMF_JBNODE_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS - Out Of Band EOS Recvd"));
        }
        else
        {
            PVMF_JBNODE_LOGDATATRAFFIC((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS - Ignoring Out Of Band EOS in Transition State"));
            PVMF_JBNODE_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS - Ignoring Out Of Band EOS in Transition State"));
        }
    }
    else
    {
        PVMF_JBNODE_LOGDATATRAFFIC((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS - Ignoring Out Of Band EOS for Non Live Stream"));
        PVMF_JBNODE_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::NotifyOutOfBandEOS - Ignoring Out Of Band EOS for Non Live Stream"));
    }
    return PVMFSuccess;
}

PVMFStatus PVMFJitterBufferNode::SendBOSMessage(uint32 aStreamID)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SendBOSMessage In"));
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = iPortParamsQueue.begin();
            it != iPortParamsQueue.end();
            it++)
    {
        PVMFJitterBufferPortParams* pPortParams = *it;
        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (pPortParams->ipJitterBuffer)
            {
                pPortParams->ipJitterBuffer->QueueBOSCommand(aStreamID);
            }
        }
    }
    PVMF_JBNODE_LOGDATATRAFFIC((0, "PVMFJitterBufferNode::SendBOSMessage - BOS Recvd"));
    return PVMFSuccess;
}

void PVMFJitterBufferNode::SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator*
        aDataBufferAllocator, const PVMFPortInterface* aPort)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetJitterBufferChunkAllocator -aPort 0x%x", aPort));
    PVMFJitterBufferPort* port = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aPort);
    if (port->iPortParams->ipJitterBuffer)
    {
        port->iPortParams->ipJitterBuffer->SetJitterBufferChunkAllocator(aDataBufferAllocator);
    }
}

void PVMFJitterBufferNode::SetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort,
        uint32 aSize,
        uint32 aResizeSize,
        uint32 aMaxNumResizes,
        uint32 aExpectedNumberOfBlocksPerBuffer)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetJitterBufferMemPoolInfo Port 0x%x", aPort));
    PVMFJitterBufferPort* port = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aPort);
    if (port->iPortParams->ipJitterBuffer)
    {
        port->iPortParams->ipJitterBuffer->SetJitterBufferMemPoolInfo(aSize, aResizeSize, aMaxNumResizes, aExpectedNumberOfBlocksPerBuffer);
    }
}

void PVMFJitterBufferNode::GetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort,
        uint32& aSize,
        uint32& aResizeSize,
        uint32& aMaxNumResizes,
        uint32& aExpectedNumberOfBlocksPerBuffer) const
{
    PVMFJitterBufferPort* port = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aPort);
    if (port->iPortParams->ipJitterBuffer)
    {
        port->iPortParams->ipJitterBuffer->GetJitterBufferMemPoolInfo(aSize, aResizeSize, aMaxNumResizes, aExpectedNumberOfBlocksPerBuffer);
    }
}

void PVMFJitterBufferNode::SetSharedBufferResizeParams(uint32 maxNumResizes,
        uint32 resizeSize)
{
    // make sure we're in a state that makes sense
    OSCL_ASSERT((iInterfaceState == EPVMFNodeCreated) ||
                (iInterfaceState == EPVMFNodeIdle) ||
                (iInterfaceState == EPVMFNodeInitialized));

    iMaxNumBufferResizes = maxNumResizes;
    iBufferResizeSize = resizeSize;
}

void PVMFJitterBufferNode::GetSharedBufferResizeParams(uint32& maxNumResizes,
        uint32& resizeSize)
{
    maxNumResizes = iMaxNumBufferResizes;
    resizeSize = iBufferResizeSize;
}

bool PVMFJitterBufferNode::ClearJitterBuffer(PVMFPortInterface* aPort,
        uint32 aSeqNum)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ClearJitterBuffer Port 0x%x aSeqNum[%d]", aPort, aSeqNum));
    /* Typically called only for HTTP streaming sessions */
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
    {
        PVMFJitterBufferPortParams* pPortParams = *it;
        pPortParams->irPort.ClearMsgQueues();
    }

    for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
    {
        PVMFJitterBufferPortParams* pPortParams = *it;
        if (&pPortParams->irPort == aPort && (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT) && (pPortParams->ipJitterBuffer != NULL))
        {
            uint32 timebase32 = 0;
            uint32 clientClock32 = 0;
            bool overflowFlag = false;
            if (ipClientPlayBackClock != NULL)
                ipClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
            pPortParams->ipJitterBuffer->PurgeElementsWithSeqNumsLessThan(aSeqNum,
                    clientClock32);
            ipJitterBufferMisc->ResetSession();
            iJitterBufferState = PVMF_JITTER_BUFFER_READY;
            return true;
        }
    }
    return false;
}

void PVMFJitterBufferNode::FlushJitterBuffer()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::FlushJitterBuffer In"));
    for (uint32 i = 0; i < iPortParamsQueue.size(); i++)
    {
        PVMFJitterBufferPortParams* pPortParams = iPortParamsQueue[i];
        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (pPortParams->ipJitterBuffer != NULL)
            {
                pPortParams->ipJitterBuffer->FlushJitterBuffer();
            }
        }
    }
}

PVMFStatus PVMFJitterBufferNode::SetInputMediaHeaderPreParsed(PVMFPortInterface* aPort,
        bool aHeaderPreParsed)
{
    PVMFStatus status = PVMFFailure;
    PVMFJitterBufferPort *port = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aPort);
    if (port)
    {
        PVMFJitterBufferPortParams* portParams = port->GetPortParams();
        if (portParams && portParams->ipJitterBuffer)
        {
            status = portParams->ipJitterBuffer->SetInputPacketHeaderPreparsed(aHeaderPreParsed);
        }
    }
    return status;
}

PVMFStatus PVMFJitterBufferNode::HasSessionDurationExpired(bool& aExpired)
{
    aExpired = ipJitterBufferMisc->IsSessionExpired();
    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::HasSessionDurationExpired %d", aExpired));
    return PVMFSuccess;
}

bool PVMFJitterBufferNode::PurgeElementsWithNPTLessThan(NptTimeFormat &aNPTTime)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::PurgeElementsWithNPTLessThan"));
    bool retval = false;

    if (ipJitterBufferMisc)
    {
        retval = ipJitterBufferMisc->PurgeElementsWithNPTLessThan(aNPTTime);
    }

    iJitterBufferState = PVMF_JITTER_BUFFER_READY;

    return retval;
}

void PVMFJitterBufferNode::SetBroadCastSession()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetBroadCastSession"));
    iBroadCastSession = true;
    if (ipJitterBufferMisc)
    {
        ipJitterBufferMisc->SetBroadcastSession();
    }
}

void PVMFJitterBufferNode::DisableFireWallPackets()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DisableFireWallPackets"));
    if (ipJitterBufferMisc)
        ipJitterBufferMisc->MediaReceivingChannelPreparationRequired(false);
}

void PVMFJitterBufferNode::UpdateJitterBufferState()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::UpdateJitterBufferState"));
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = iPortParamsQueue.begin(); iter != iPortParamsQueue.end(); iter ++)
    {
        PVMFJitterBufferPortParams* ptr = *iter;
        if (PVMF_JITTER_BUFFER_PORT_TYPE_INPUT == ptr->iTag)
        {
            ptr->ipJitterBuffer->SetJitterBufferState(PVMF_JITTER_BUFFER_READY);
        }
    }
    iDelayEstablished = true;
}

void PVMFJitterBufferNode::StartOutputPorts()
{
    oStopOutputPorts = false;
}

void PVMFJitterBufferNode::StopOutputPorts()
{
    oStopOutputPorts = true;
}

///////////////////////////////////////////////////////////////////////////////
//Used for the implementation of extension interface functions
///////////////////////////////////////////////////////////////////////////////
bool
PVMFJitterBufferNode::SetPortParams(PVMFPortInterface* aPort,
                                    uint32 aTimeScale,
                                    uint32 aBitRate,
                                    OsclRefCounterMemFrag& aConfig,
                                    bool aRateAdaptation,
                                    uint32 aRateAdaptationFeedBackFrequency,
                                    uint aMaxNumBuffResizes, uint aBuffResizeSize)
{
    return SetPortParams(aPort, aTimeScale, aBitRate, aConfig, aRateAdaptation,
                         aRateAdaptationFeedBackFrequency, true,
                         aMaxNumBuffResizes, aBuffResizeSize);
}
bool
PVMFJitterBufferNode::SetPortParams(PVMFPortInterface* aPort,
                                    uint32 aTimeScale,
                                    uint32 aBitRate,
                                    OsclRefCounterMemFrag& aConfig,
                                    bool aRateAdaptation,
                                    uint32 aRateAdaptationFeedBackFrequency,
                                    bool aUserSpecifiedBuffParams,
                                    uint aMaxNumBuffResizes, uint aBuffResizeSize)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::SetPortParams"));
    OSCL_UNUSED_ARG(aUserSpecifiedBuffParams);
    uint32 ii;
    for (ii = 0; ii < iPortParamsQueue.size(); ii++)
    {
        PVMFJitterBufferPortParams* pPortParams = iPortParamsQueue[ii];

        if (&pPortParams->irPort == aPort)
        {
            pPortParams->iTimeScale = aTimeScale;
            pPortParams->iMediaClockConverter.set_timescale(aTimeScale);
            pPortParams->iBitrate = aBitRate;
            if (pPortParams->ipJitterBuffer)
            {
                pPortParams->ipJitterBuffer->SetTrackConfig(aConfig);
                pPortParams->ipJitterBuffer->SetTimeScale(aTimeScale);
                pPortParams->ipJitterBuffer->SetMediaClockConverter(&pPortParams->iMediaClockConverter);
            }


            /* Compute buffer size based on bitrate and jitter duration*/
            uint32 sizeInBytes = 0;
            if (((int32)iJitterBufferDurationInMilliSeconds > 0) &&
                    ((int32)aBitRate > 0))
            {
                uint32 byteRate = aBitRate / 8;
                uint32 overhead = (byteRate * PVMF_JITTER_BUFFER_NODE_MEM_POOL_OVERHEAD) / 100;
                uint32 durationInSec = iJitterBufferDurationInMilliSeconds / 1000;
                sizeInBytes = ((byteRate + overhead) * durationInSec);
                if (sizeInBytes < MIN_RTP_SOCKET_MEM_POOL_SIZE_IN_BYTES)
                {
                    sizeInBytes = MIN_RTP_SOCKET_MEM_POOL_SIZE_IN_BYTES;
                }
                sizeInBytes += (2 * MAX_SOCKET_BUFFER_SIZE);
            }

            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                PVMFJitterBuffer* jitterBuffer = NULL;
                jitterBuffer = pPortParams->ipJitterBuffer;

                if (jitterBuffer)
                {
                    pPortParams->ipJitterBuffer->SetJitterBufferMemPoolInfo(sizeInBytes, aBuffResizeSize, aMaxNumBuffResizes, 3000);
                }

                if (ipJitterBufferMisc)
                    ipJitterBufferMisc->SetRateAdaptationInfo(&pPortParams->irPort, aRateAdaptation, aRateAdaptationFeedBackFrequency, sizeInBytes);
            }

            iPortParamsQueue[ii] = pPortParams;
            return true;
        }
    }
    return false;
}

// This routine is called by various command APIs to queue an
// asynchronous command for processing by the command handler AO.
// This function may leave if the command can't be queued due to
// memory allocation failure.
PVMFCommandId PVMFJitterBufferNode::QueueCommandL(PVMFJitterBufferNodeCommand& aCmd)
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

///////////////////////////////////////////////////////////////////////////////
//OsclActiveObject Implementation
///////////////////////////////////////////////////////////////////////////////
/**
  * This AO handles both API commands and port activity.
  * The AO will either process one command or service one connected
  * port per call.  It will re-schedule itself and run continuously
  * until it runs out of things to do.
  */
void PVMFJitterBufferNode::Run()
{
    iNumRunL++;
    /*
     * Process commands.
     */
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
     * Process port activity
     */
    if (((iInterfaceState == EPVMFNodeInitialized) ||
            (iInterfaceState == EPVMFNodePrepared) ||
            (iInterfaceState == EPVMFNodeStarted)  ||
            (iInterfaceState == EPVMFNodePaused)) ||
            FlushPending())
    {
        uint32 i;
        for (i = 0; i < iPortVector.size(); i++)
        {
            PVMFJitterBufferPortParams* portContainerPtr =
                iPortVector[i]->iPortParams;

            if (portContainerPtr == NULL)
            {
                if (!getPortContainer(iPortVector[i], portContainerPtr))
                {
                    PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferNode::Run: Error - getPortContainer failed", this));
                    return;
                }
                iPortVector[i]->iPortParams = portContainerPtr;
            }

            ProcessPortActivity(portContainerPtr);
        }

        if (CheckForPortRescheduling())
        {
            if (IsAdded())
            {
                /*
                 * Re-schedule since there is atleast one port that needs processing
                 */
                RunIfNotReady();
            }
            return;
        }
    }

    /*
     * If we get here we did not process any ports or commands.
     * Check for completion of a flush command...
     */
    if (FlushPending() && (!CheckForPortActivityQueues()))
    {
        uint32 i;
        /*
         * Debug check-- all the port queues should be empty at
         * this point.
         */
        for (i = 0;i < iPortVector.size();i++)
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
        for (i = 0;i < iPortVector.size();i++)
        {
            iPortVector[i]->ResumeInput();
        }
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }
    return;
}

void PVMFJitterBufferNode::DoCancel()
{
    /*
     * the base class cancel operation is sufficient.
     */
    OsclActiveObject::DoCancel();
}

bool PVMFJitterBufferNode::ProcessPortActivity(PVMFJitterBufferPortParams* aPortParams)
{
    if (!aPortParams)
    {
        return false;
    }

    PVMFStatus status = PVMFSuccess;
    switch (aPortParams->iTag)
    {
        case PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT:
        {
            if ((aPortParams->iProcessOutgoingMessages) &&
                    (aPortParams->irPort.OutgoingMsgQueueSize() > 0))
            {
                status = ProcessOutgoingMsg(aPortParams);
            }
            /*
             * Send data out of jitter buffer as long as there's:
             *  - more data to send
             *  - outgoing queue isn't in a Busy state.
             *  - ports are not paused
             */
            PVMFJitterBufferPort* outPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, &aPortParams->irPort);
            PVMFJitterBufferPortParams* inPortParamsPtr = outPort->iCounterpartPortParams;
            if (aPortParams->iProcessOutgoingMessages)
            {
                if ((oStopOutputPorts == false) && (inPortParamsPtr->iCanReceivePktFromJB))
                {
                    SendData(OSCL_STATIC_CAST(PVMFPortInterface*, &inPortParamsPtr->irPort));
                }
            }
        }
        break;

        case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
        {
            PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ProcessPortActivity: input port- aPortParams->iProcessIncomingMessages %d aPortParams->iPort->IncomingMsgQueueSize()  %d" ,
                                 aPortParams->iProcessIncomingMessages, aPortParams->irPort.IncomingMsgQueueSize()));
            if ((aPortParams->iProcessIncomingMessages) &&
                    (aPortParams->irPort.IncomingMsgQueueSize() > 0))
            {
                status = ProcessIncomingMsg(aPortParams);
            }
            if ((aPortParams->iProcessOutgoingMessages) &&
                    (aPortParams->irPort.OutgoingMsgQueueSize() > 0))
            {
                status = ProcessOutgoingMsg(aPortParams);
            }
        }
        break;

        case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
        {
            PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ProcessPortActivity: - aPortParams->iProcessIncomingMessages %d aPortParams->iPort->IncomingMsgQueueSize()  %d" ,
                                 aPortParams->iProcessIncomingMessages, aPortParams->irPort.IncomingMsgQueueSize()));

            if ((aPortParams->iProcessIncomingMessages) &&
                    (aPortParams->irPort.IncomingMsgQueueSize() > 0))
            {
                status = ProcessIncomingMsg(aPortParams);
            }
            if ((aPortParams->iProcessOutgoingMessages) &&
                    (aPortParams->irPort.OutgoingMsgQueueSize() > 0))
            {
                status = ProcessOutgoingMsg(aPortParams);
            }
        }
        break;

        default:
            break;
    }

    /*
     * Report any unexpected failure in port processing...
     * (the InvalidState error happens when port input is suspended,
     * so don't report it.)
     */
    if (status != PVMFErrBusy
            && status != PVMFSuccess
            && status != PVMFErrInvalidState)
    {
        PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessPortActivity: Error - ProcessPortActivity failed. port=0x%x",
                              &aPortParams->irPort));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(&aPortParams->irPort));
    }

    /*
     * return true if we processed an activity...
     */
    return (status != PVMFErrBusy);
}

///////////////////////////////////////////////////////////////////////////////
//Processing of incoming msg
///////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferNode::ProcessIncomingMsg(PVMFJitterBufferPortParams* aPortParams)
{
    PVUuid eventuuid = PVMFJitterBufferNodeEventTypeUUID;
    PVMFPortInterface* aPort = &aPortParams->irPort;

    PVMF_JBNODE_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: %s Tag %d", aPortParams->iMimeType.get_cstr(), aPortParams->iTag));

    aPortParams->iNumMediaMsgsRecvd++;

    if (aPortParams->iMonitorForRemoteActivity == true)
    {
        CancelEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
        RequestEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
    }

    switch (aPortParams->iTag)
    {
        case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
        {
            /* Parse packet header - mainly to retrieve time stamp */
            PVMFJitterBuffer* jitterBuffer = aPortParams->ipJitterBuffer;
            if (jitterBuffer == NULL)
            {
                PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: findJitterBuffer failed"));
                int32 errcode = PVMFJitterBufferNodeUnableToRegisterIncomingPacket;
                ReportErrorEvent(PVMFErrArgument, (OsclAny*)(aPort), &eventuuid, &errcode);
                return PVMFErrArgument;
            }

            /*
             * Incoming message recvd on the input port.
             * Dequeue the message
             */
            PVMFSharedMediaMsgPtr msg;
            PVMFStatus status = aPort->DequeueIncomingMsg(msg);
            if (status != PVMFSuccess)
            {
                ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aPort));
                PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Error - INPUT PORT - DequeueIncomingMsg failed"));
                return status;
            }

            PVMFJitterBufferRegisterMediaMsgStatus regStatus = jitterBuffer->RegisterMediaMsg(msg);
            switch (regStatus)
            {

                case PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS:
                {
                    PVMF_JBNODE_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Packet registered successfully Mime %s", aPortParams->iMimeType.get_cstr()));
                }
                break;
                case PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_JB_FULL:
                {
                    aPortParams->iProcessIncomingMessages = false;
                    jitterBuffer->NotifyFreeSpaceAvailable();
                    int32 infocode = PVMFJitterBufferNodeJitterBufferFull;
                    ReportInfoEvent(PVMFInfoOverflow, (OsclAny*)(aPort), &eventuuid, &infocode);
                    PVMF_JBNODE_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Jitter Buffer full"));
                    PVMF_JBNODE_LOGDATATRAFFIC_FLOWCTRL_E((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Jitter Buffer full"));
                    return PVMFErrBusy;
                }
                break;
                case PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_INSUFFICIENT_MEMORY_FOR_PACKETIZATION:
                {
                    aPortParams->iProcessIncomingMessages = false;
                    jitterBuffer->NotifyFreeSpaceAvailable();
                    return PVMFErrBusy;
                }
                break;
                case PVMF_JB_REGISTER_MEDIA_MSG_ERR_CORRUPT_PACKET:
                {
                    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: unable to register packet"));
                    int32 errcode = PVMFJitterBufferNodeUnableToRegisterIncomingPacket;
                    ReportErrorEvent(PVMFErrArgument, (OsclAny*)(aPort), &eventuuid, &errcode);
                    return PVMFErrArgument;
                }
                case PVMF_JB_REGISTER_MEDIA_MSG_ERR_EOS_SIGNALLED:
                {
                    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: data received after signalling EOS"));
                }
                break;
                default:
                {
                    PVMF_JBNODE_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Packet could not be registered Register packet returned status %d", regStatus));
                    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Packet could not be registered Register packet returned status %d", regStatus));
                }
            }
            SendData(aPort);
        }
        break;

        case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
        {
            /*
            * Incoming RTCP reports - recvd on the input port.
            * Dequeue the message - Need to fully implement
            * RTCP
            */
            PVMFSharedMediaMsgPtr msg;
            PVMFStatus status = aPort->DequeueIncomingMsg(msg);
            if (status != PVMFSuccess)
            {
                ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aPort));
                PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferNode::ProcessIncomingMsg: Error - FB PORT - DequeueIncomingMsg failed", this));
                return status;
            }
            status = ipJitterBufferMisc->ProcessFeedbackMessage(*aPortParams, msg);
            PVMF_JBNODE_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: Feedback Packet registered with status code status %d", status));
        }
        break;

        default:
        {
            ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aPort));
            PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg - Invalid Port Tag"));
            return PVMFFailure;
        }
    }
    return (PVMFSuccess);
}

///////////////////////////////////////////////////////////////////////////////
//Processing of outgoing msg
///////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFJitterBufferNode::ProcessOutgoingMsg(PVMFJitterBufferPortParams* aPortParams)
{
    PVMFPortInterface* aPort = &aPortParams->irPort;
    /*
     * Called by the AO to process one message off the outgoing
     * message queue for the given port.  This routine will
     * try to send the data to the connected port.
     */
    PVMF_JBNODE_LOGINFO((0, "0x%x PVMFJitterBufferNode::ProcessOutgoingMsg: aPort=0x%x", this, aPort));

    /*
     * If connected port is busy, the outgoing message cannot be process. It will be
     * queued to be processed again after receiving PORT_ACTIVITY_CONNECTED_PORT_READY
     * from this port.
     */
    if (aPort->IsConnectedPortBusy())
    {
        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "0x%x PVMFJitterBufferNode::ProcessOutgoingMsg: Connected port is busy", this));
        aPortParams->iProcessOutgoingMessages = false;
        return PVMFErrBusy;
    }

    PVMFStatus status = PVMFSuccess;

    status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        PVMF_JBNODE_LOGDATATRAFFIC((0, "PVMFJitterBufferNode::ProcessOutgoingMsg: Connected port goes into busy state"));
        aPortParams->iProcessOutgoingMessages = false;
    }
    else
    {
        aPortParams->iNumMediaMsgsSent++;
    }
    return status;
}

PVMFStatus
PVMFJitterBufferNode::SendData(PVMFPortInterface* aPort)
{
    PVMFJitterBufferPort* jbPort =
        OSCL_STATIC_CAST(PVMFJitterBufferPort*, aPort);

    PVMFPortInterface* outputPort = jbPort->iPortCounterpart;
    PVMFJitterBufferPortParams* portParamsPtr = jbPort->iPortParams;
    PVMFJitterBufferPortParams* outPortParamsPtr = jbPort->iCounterpartPortParams;
    PVMFJitterBuffer* jitterBuffer = portParamsPtr->ipJitterBuffer;

    PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData In %s", outPortParamsPtr->iMimeType.get_cstr())) ;

    if (!(portParamsPtr->iCanReceivePktFromJB))
    {
        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData - Cant retrieve pkt from JB"));
        return PVMFFailure;
    }

    if (outPortParamsPtr->irPort.IsOutgoingQueueBusy())
    {
        outPortParamsPtr->iProcessOutgoingMessages = false;
        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData - Port found to be busy %s", outPortParamsPtr->iMimeType.get_cstr())) ;
        return PVMFErrBusy;
    }

    if (oStopOutputPorts)
    {
        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData from Mime[%s] Output ports Stopped--", jbPort->iPortParams->iMimeType.get_cstr()));
        return PVMFSuccess;
    }

    PVMFSharedMediaMsgPtr mediaOutMsg;
    bool cmdPacket = false;
    PVMFStatus status = jbPort->iPortParams->ipJitterBuffer->RetrievePacket(mediaOutMsg, cmdPacket);
    while (PVMFSuccess == status)
    {
        if (!cmdPacket)
        {
            //media msg
            outPortParamsPtr->iLastMsgTimeStamp = mediaOutMsg->getTimestamp();
        }

        status = outputPort->QueueOutgoingMsg(mediaOutMsg);

        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData from Mime[%s] MediaMsg SeqNum[%d], Ts[%d]", jbPort->iPortParams->iMimeType.get_cstr(), mediaOutMsg->getSeqNum(), mediaOutMsg->getTimestamp()));

        if (outPortParamsPtr->irPort.IsOutgoingQueueBusy())
        {
            outPortParamsPtr->iProcessOutgoingMessages = false;
            PVMFJitterBufferStats stats = jbPort->iPortParams->ipJitterBuffer->getJitterBufferStats();
            PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "Send Data Mime %s stats.currentOccupancy[%d], stats.maxSeqNumRegistered[%d], stats.lastRetrievedSeqNum[%d] stats.maxTimeStampRetrievedWithoutRTPOffset[%d] SendOut Pkt[%d]", jbPort->iPortParams->iMimeType.get_cstr(), stats.currentOccupancy, stats.maxSeqNumRegistered, stats.lastRetrievedSeqNum, stats.maxTimeStampRetrievedWithoutRTPOffset, outPortParamsPtr->iNumMediaMsgsSent));
            return PVMFErrBusy;
        }
        status = jbPort->iPortParams->ipJitterBuffer->RetrievePacket(mediaOutMsg, cmdPacket);
    }

    if (PVMFErrNotReady == status)
    {
        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData from Mime[%s] JB not ready", jbPort->iPortParams->iMimeType.get_cstr()));


        uint32 currentTime32 = 0;
        uint32 currentTimeBase32 = 0;
        bool overflowFlag = false;
        ipJitterBufferMisc->GetEstimatedServerClock().GetCurrentTime32(currentTime32,
                overflowFlag,
                PVMF_MEDIA_CLOCK_MSEC,
                currentTimeBase32);


        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData - Estimated serv clock %d", currentTime32));
        currentTime32 = 0;
        currentTimeBase32 = 0;
        ipClientPlayBackClock->GetCurrentTime32(currentTime32,
                                                overflowFlag,
                                                PVMF_MEDIA_CLOCK_MSEC,
                                                currentTimeBase32);
        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData - Client serv clock %d", currentTime32));


        PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::SendData - Occupancy is %d delayestablish %d", jbPort->iPortParams->ipJitterBuffer->getJitterBufferStats().currentOccupancy, iDelayEstablished));

        portParamsPtr->iCanReceivePktFromJB = false;
        jitterBuffer->NotifyCanRetrievePacket();
        return PVMFErrNotReady;
    }

    return status;
}

bool PVMFJitterBufferNode::CheckForPortRescheduling()
{
    //This method is only called from JB Node AO's Run.
    //Purpose of this method is to determine whether the node
    //needs scheduling based on any outstanding port activities
    //Here is the scheduling criteria for different port types:
    //a) PVMF_JITTER_BUFFER_PORT_TYPE_INPUT - If there are incoming
    //msgs waiting in incoming msg queue then node needs scheduling,
    //as long oProcessIncomingMessages is true. This boolean stays true
    //as long we can register packets in JB. If JB is full this boolean
    //is made false (when CheckForSpaceInJitterBuffer() returns false)
    //and is once again made true in JitterBufferFreeSpaceAvailable() callback.
    //We also use the input port briefly as a bidirectional port in case of
    //RTSP streaming to do firewall packet exchange. So if there are outgoing
    //msgs and oProcessOutgoingMessages is true then node needs scheduling.
    //b) PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT - As long as:
    //	- there are msgs in outgoing queue
    //	- oProcessOutgoingMessages is true
    //	- and as long as there is data in JB and we are not in buffering
    //then node needs scheduling.
    //c) PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK - As long as:
    //	- there are msgs in incoming queue and oProcessIncomingMessages is true
    //	- there are msgs in outgoing queue and oProcessOutgoingMessages is true
    uint32 i;
    for (i = 0; i < iPortVector.size(); i++)
    {
        PVMFJitterBufferPortParams* portContainerPtr = iPortVector[i]->iPortParams;
        if (portContainerPtr == NULL)
        {
            PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::CheckForPortRescheduling: Error - GetPortContainer failed"));
            return false;
        }
        if (portContainerPtr->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (portContainerPtr->irPort.IncomingMsgQueueSize() > 0)
            {
                if (portContainerPtr->iProcessIncomingMessages)
                {
                    /*
                     * Found a port that has outstanding activity and
                     * is not busy.
                     */
                    return true;
                }
            }
            if (portContainerPtr->irPort.OutgoingMsgQueueSize() > 0)
            {
                if (portContainerPtr->iProcessOutgoingMessages)
                {
                    /*
                     * Found a port that has outstanding activity and
                     * is not busy.
                     */
                    return true;
                }
            }
        }
        else if (portContainerPtr->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT)
        {
            PVMFJitterBufferPort* jbPort =
                OSCL_STATIC_CAST(PVMFJitterBufferPort*, &portContainerPtr->irPort);
            PVMFJitterBufferPortParams* inPortParamsPtr = jbPort->iCounterpartPortParams;
            if ((portContainerPtr->irPort.OutgoingMsgQueueSize() > 0) ||
                    (inPortParamsPtr->iCanReceivePktFromJB))
            {
                if ((portContainerPtr->iProcessOutgoingMessages) && (oStopOutputPorts == false))
                {
                    /*
                     * Found a port that has outstanding activity and
                     * is not busy.
                     */
                    return true;
                }
            }
        }
        else if (portContainerPtr->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK)
        {
            if (portContainerPtr->irPort.IncomingMsgQueueSize() > 0)
            {
                if (portContainerPtr->iProcessIncomingMessages)
                {
                    /*
                     * Found a port that has outstanding activity and
                     * is not busy.
                     */
                    return true;
                }
            }
            if (portContainerPtr->irPort.OutgoingMsgQueueSize() > 0)
            {
                if (portContainerPtr->iProcessOutgoingMessages)
                {
                    /*
                     * Found a port that has outstanding activity and
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

bool PVMFJitterBufferNode::CheckForPortActivityQueues()
{
    uint32 i;
    for (i = 0; i < iPortVector.size(); i++)
    {
        PVMFJitterBufferPortParams* portContainerPtr = NULL;

        if (!getPortContainer(iPortVector[i], portContainerPtr))
        {
            PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferNode::CheckForPortActivityQueues: Error - GetPortContainer failed", this));
            return false;
        }

        if ((portContainerPtr->irPort.IncomingMsgQueueSize() > 0) ||
                (portContainerPtr->irPort.OutgoingMsgQueueSize() > 0))
        {
            /*
             * Found a port that still has an outstanding activity.
             */
            return true;
        }
    }

    return false;
}

/**
 * A routine to tell if a flush operation is in progress.
 */
bool PVMFJitterBufferNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_JITTER_BUFFER_NODE_FLUSH);
}

// Called by the command handler AO to process a command from
// the input queue.
// Return true if a command was processed, false if the command
// processor is busy and can't process another command now.

bool PVMFJitterBufferNode::ProcessCommand(PVMFJitterBufferNodeCommand& aCmd)
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
        case PVMF_JITTER_BUFFER_NODE_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_INIT:
            DoInit(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_START:
            DoStart(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_STOP:
            DoStop(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_PAUSE:
            DoPause(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_RESET:
            DoReset(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_JITTER_BUFFER_NODE_CANCELCOMMAND:
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

void
PVMFJitterBufferNode::MoveCmdToCurrentQueue(PVMFJitterBufferNodeCommand& aCmd)
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

void PVMFJitterBufferNode::CommandComplete(PVMFJitterBufferNodeCmdQ& aCmdQ,
        PVMFJitterBufferNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                         , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        PVMF_JITTER_BUFFER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), errormsg);
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    /* Erase the command from the queue */
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

void PVMFJitterBufferNode::CommandComplete(PVMFJitterBufferNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                         , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        PVMF_JITTER_BUFFER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), errormsg);
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

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
 * The various command handlers call this when a INTERNAL command is complete.
 * Does not report completion as it is an internal command
 */
void PVMFJitterBufferNode::InternalCommandComplete(PVMFJitterBufferNodeCmdQ& aCmdQ,
        PVMFJitterBufferNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData)
{
    OSCL_UNUSED_ARG(aEventData);

    PVMF_JBNODE_LOGINFO((0, "JitterBufferNode:InternalCommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                         , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

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

///////////////////////////////////////////////////////////////////////////////
//Called by the command handler AO to do the Query UUID
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::DoQueryUuid(PVMFJitterBufferNodeCommand& aCmd)
{
    // This node supports Query UUID from any state
    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFJitterBufferNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    // Try to match the input mimetype against any of
    // the custom interfaces for this node

    // Match against custom interface1...
    // also match against base mimetypes for custom interface1,
    // unless exactmatch is set.

    if (*mimetype == PVMF_JITTERBUFFER_CUSTOMINTERFACE_MIMETYPE
            || (!exactmatch && *mimetype == PVMF_JITTERBUFFER_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_JITTERBUFFER_BASEMIMETYPE))
    {
        PVUuid uuid(PVMF_JITTERBUFFERNODE_EXTENSIONINTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

///////////////////////////////////////////////////////////////////////////////
//Called by the command handler AO to do the Query Interface.
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::DoQueryInterface(PVMFJitterBufferNodeCommand& aCmd)
{
    //This node supports Query Interface from any state
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFJitterBufferNodeCommandBase::Parse(uuid, ptr);

    if (*uuid == PVUuid(PVMF_JITTERBUFFERNODE_EXTENSIONINTERFACE_UUID))
    {
        if (!ipExtensionInterface)
        {
            OsclMemAllocator alloc;
            int32 err;
            OsclAny*ptr = NULL;
            OSCL_TRY(err,
                     ptr = alloc.ALLOCATE(sizeof(PVMFJitterBufferExtensionInterfaceImpl));
                    );
            if (err != OsclErrNone || !ptr)
            {
                PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoQueryInterface: Error - Out of memory"));
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            ipExtensionInterface =
                OSCL_PLACEMENT_NEW(ptr, PVMFJitterBufferExtensionInterfaceImpl(this));
        }
        if (ipExtensionInterface->queryInterface(*uuid, *ptr))
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
        // not supported
        *ptr = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Called by the command handler AO to do the port request
// Decides the type of requested port
// Reserve space in port vector for the new port
// Instantiate the port and push it in the port vector
// Populate portparms for tag (port type), jitterbuffer, port, iId,
//
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::DoRequestPort(PVMFJitterBufferNodeCommand& aCmd)
{
    // This node supports port request from any state
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DoRequestPort"));

    // retrieve port tag
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PVMFJitterBufferNodeCommandBase::Parse(tag, mimetype);

    PVMFJitterBufferNodePortTag jitterbufferPortTag = PVMF_JITTER_BUFFER_PORT_TYPE_INPUT;

    if (tag % 3)
    {
        if (tag % 3 == 1)
        {
            jitterbufferPortTag = PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT;
        }
        else if (tag % 3 == 2)
        {
            jitterbufferPortTag = PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK;
        }
    }
    else
    {
        jitterbufferPortTag = PVMF_JITTER_BUFFER_PORT_TYPE_INPUT;
    }

    // Input ports have tags: 0, 3, 6, ...
    // Output ports have tags: 1, 4, 7, ...
    // Feedback ports have tags: 2, 5, 8, ...

    //set port name for datapath logging.
    OSCL_StackString<20> portname;
    switch (jitterbufferPortTag)
    {
        case PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT:
            portname = "JitterBufOut";
            break;
        case PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK:
            //don't log this port for now...
            //portname="JitterBufFeedback";
            break;
        case PVMF_JITTER_BUFFER_PORT_TYPE_INPUT:
            //don't log this port for now...
            //portname="JitterBufIn";
            break;
        default:
            // avoid compiler warning
            break;
    }

    // Allocate a new port
    OsclAny *ptr = AllocatePort();
    if (!ptr)
    {
        PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoRequestPort: Error - iPortVector Out of memory"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    OsclExclusivePtr<PVMFJitterBufferPort> portAutoPtr;
    PVMFJitterBufferPort* port = NULL;

    OsclExclusivePtr<PVMFJitterBuffer> jitterBufferAutoPtr;

    // create base port with default settings
    port = OSCL_PLACEMENT_NEW(ptr, PVMFJitterBufferPort(tag, *this, portname.get_str()));
    portAutoPtr.set(port);

    /* Add the port to the port vector. */
    if (!PushPortToVect(port))
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    PVMFJitterBufferPortParams* pPortParams = OSCL_NEW(PVMFJitterBufferPortParams, (*port));
    pPortParams->iTag = jitterbufferPortTag;
    PVMFJitterBuffer* jbPtr = NULL;
    pPortParams->ipJitterBuffer = NULL;
    pPortParams->iId = tag;
    if (mimetype != NULL)
    {
        pPortParams->iMimeType = mimetype->get_str();
    }

    // create jitter buffer if input port
    if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
    {
        PVMFJitterBufferConstructParams jbConstructParams(ipJitterBufferMisc->GetEstimatedServerClock(), *ipClientPlayBackClock, pPortParams->iMimeType, *ipJitterBufferMisc->GetEventNotifier(), iDelayEstablished, iJitterDelayPercent, iJitterBufferState, this, port);
        jbPtr = ipJitterBufferFactory->Create(jbConstructParams);
        if (jbPtr)
            jbPtr->SetDurationInMilliSeconds(iJitterBufferDurationInMilliSeconds);
        jitterBufferAutoPtr.set(jbPtr);
        pPortParams->ipJitterBuffer = jbPtr;
        if (iBroadCastSession == true)
        {
            jbPtr->SetBroadCastSession();
        }
    }

    if (!PushPortParamsToQ(pPortParams))
    {
        PVMF_JBNODE_LOGERROR((0, "0x%x PVMFJitterBufferNode::DoRequestPort: Error - iPortParamsQueue.push_back() failed", this));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }


    // Update the iPortParams for all existing ports since adding a new Port Parameters element might
    // have caused reallocation of the vector elements.
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = iPortParamsQueue.begin();
            it != iPortParamsQueue.end();
            it++)
    {
        PVMFJitterBufferPortParams* portParametersPtr = *it;
        PVMFJitterBufferPort* portPtr = OSCL_REINTERPRET_CAST(PVMFJitterBufferPort*, &portParametersPtr->irPort);
        portPtr->iPortParams = portParametersPtr;

        // Update also the port counterpart and port counterpart parameters
        PVMFPortInterface* cpPort = getPortCounterpart(portPtr);
        if (cpPort != NULL)
        {
            portPtr->iPortCounterpart = (PVMFJitterBufferPort*)cpPort;
            PVMFJitterBufferPortParams* cpPortContainerPtr = NULL;
            if (getPortContainer(portPtr->iPortCounterpart, cpPortContainerPtr))
            {
                portPtr->iCounterpartPortParams = cpPortContainerPtr;
            }
            else
            {
                PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoRequestPort: getPortContainer for port counterpart failed"));
                CommandComplete(iInputCommands, aCmd, PVMFFailure);
                return;
            }
        }

    }

    portAutoPtr.release();
    jitterBufferAutoPtr.release();


    /* Return the port pointer to the caller. */
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port);
}

OsclAny* PVMFJitterBufferNode::AllocatePort()
{
    OsclAny *ptr = NULL;
    int32 err;
    OSCL_TRY(err, ptr = iPortVector.Allocate(););
    if (err != OsclErrNone)
    {
        ptr = NULL;
    }
    return ptr;
}

bool PVMFJitterBufferNode::PushPortToVect(PVMFJitterBufferPort*& aPort)
{
    int32 err;
    OSCL_TRY(err, iPortVector.AddL(aPort););
    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}

bool PVMFJitterBufferNode::PushPortParamsToQ(PVMFJitterBufferPortParams*& aPortParams)
{
    int32 err;
    OSCL_TRY(err, iPortParamsQueue.push_back(aPortParams););
    if (err != OsclErrNone)
    {
        return false;
    }
    return true;
}

/**
 * Called by the command handler AO to do the port release
 */
void PVMFJitterBufferNode::DoReleasePort(PVMFJitterBufferNodeCommand& aCmd)
{
    /*This node supports release port from any state*/

    /* Find the port in the port vector */
    ResetNodeParams();

    PVMFPortInterface* p = NULL;
    aCmd.PVMFJitterBufferNodeCommandBase::Parse(p);

    PVMFJitterBufferPort* port = (PVMFJitterBufferPort*)p;

    PVMFJitterBufferPort** portPtr = iPortVector.FindByValue(port);
    if (portPtr)
    {
        /* delete corresponding port params */
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;

        for (it = iPortParamsQueue.begin();
                it != iPortParamsQueue.end();
                it++)
        {
            PVMFJitterBufferPortParams* pPortParams = *it;
            if (&pPortParams->irPort == iPortVector.front())
            {
                if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                {
                    ipJitterBufferFactory->Destroy(pPortParams->ipJitterBuffer);
                }
                iPortParamsQueue.erase(it);
                break;
            }
        }
        /* delete the port */
        iPortVector.Erase(portPtr);

        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        /* port not found */
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Called by the command handler AO to initialize the node
// Decides the type of requested port
// Reserve space in port vector for the new port
// Instantiate the port and push it in the port vector
// Populate portparms for tag (port type), jitterbuffer, port, iId,
//
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::DoInit(PVMFJitterBufferNodeCommand& aCmd)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DoInit"));
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (ipJitterBufferMisc)
            {
                ipJitterBufferMisc->Reset();
                OSCL_DELETE(ipJitterBufferMisc);
                ipJitterBufferMisc = NULL;
            }
            ipJitterBufferMisc = PVMFJitterBufferMisc::New(this, *ipClientPlayBackClock, iPortParamsQueue);
            if (ipJitterBufferMisc)
            {
                ipEventNotifier = ipJitterBufferMisc->GetEventNotifier();
                if (iBroadCastSession == true)
                    ipJitterBufferMisc->SetBroadcastSession();
            }

            SetState(EPVMFNodeInitialized);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
 * Called by the command handler AO to do the node Prepare
 */
void PVMFJitterBufferNode::DoPrepare(PVMFJitterBufferNodeCommand& aCmd)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DoPrepare"));
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        {
            uint32 i;
            for (i = 0; i < iPortVector.size(); i++)
            {
                PVMFJitterBufferPortParams* portContainerPtr1 = NULL;
                if (getPortContainer(iPortVector[i], portContainerPtr1))
                {
                    iPortVector[i]->iPortParams = portContainerPtr1;
                }
                else
                {
                    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoPrepare: getPortContainer - Self"));
                    CommandComplete(iInputCommands, aCmd, PVMFFailure);
                    break;
                }
                PVMFPortInterface* cpPort = getPortCounterpart(iPortVector[i]);
                if (cpPort != NULL)
                {
                    iPortVector[i]->iPortCounterpart = (PVMFJitterBufferPort*)cpPort;
                    PVMFJitterBufferPortParams* portContainerPtr2 = NULL;
                    if (getPortContainer(iPortVector[i]->iPortCounterpart, portContainerPtr2))
                    {
                        iPortVector[i]->iCounterpartPortParams = portContainerPtr2;
                    }
                    else
                    {
                        PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoPrepare: getPortContainer - Counterpart"));
                        CommandComplete(iInputCommands, aCmd, PVMFFailure);
                        break;
                    }
                }
            }

            ipJitterBufferMisc->Prepare();
            PVMFStatus status = ipJitterBufferMisc->PrepareMediaReceivingChannel();
            if (PVMFPending == status)
            {
                MoveCmdToCurrentQueue(aCmd);
            }
            else
            {
                if (PVMFSuccess == status)
                {
                    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DoPrepare: FW Pkts Disabled"));
                    /* Complete prepare */
                    SetState(EPVMFNodePrepared);
                    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
                }
                else
                {
                    CommandComplete(iInputCommands, aCmd, status);
                }
            }
        }
        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

void PVMFJitterBufferNode::CompletePrepare()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::CompletePrepare"));
    SetState(EPVMFNodePrepared);
    PVMFJitterBufferNodeCommand cmd = iCurrentCommand.front();
    CommandComplete(cmd, PVMFSuccess);
    iCurrentCommand.Erase(&iCurrentCommand.front());
    return;
}

void PVMFJitterBufferNode::CancelPrepare()
{
    ipJitterBufferMisc->CancelMediaReceivingChannelPreparation();
    PVMFJitterBufferNodeCommand cmd = iCurrentCommand.front();
    CommandComplete(cmd, PVMFErrCancelled);
    iCurrentCommand.Erase(&iCurrentCommand.front());
    return;
}

/**
 * Called by the command handler AO to do the node Start
 */
void PVMFJitterBufferNode::DoStart(PVMFJitterBufferNodeCommand& aCmd)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DoStart"));
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
        {
            ipJitterBufferMisc->StreamingSessionStarted();
            /* Diagnostic logging */
            iDiagnosticsLogged = false;
            iMediaReceiveingChannelPrepared = true;

            if (iInterfaceState == EPVMFNodePaused)
            {
                uint32 currticks = OsclTickCount::TickCount();
                uint32 startTime = OsclTickCount::TicksToMsec(currticks);
                uint32 diff = (startTime - iPauseTime);
                if (diff > PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_DEFAULT_PAUSE_DURATION_IN_MS)
                {
                    if (PVMFPending == ipJitterBufferMisc->PrepareMediaReceivingChannel())
                    {
                        iMediaReceiveingChannelPrepared = false;
                    }
                }
            }

            if (!ipJitterBufferMisc->IsSessionExpired())
                RequestEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);

            /* If auto paused, implies jitter buffer is not empty */
            if ((iDelayEstablished == false) ||
                    (iJitterBufferState == PVMF_JITTER_BUFFER_IN_TRANSITION))
            {

                Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
                for (iter = iPortParamsQueue.begin(); iter != iPortParamsQueue.end(); iter++)
                {
                    PVMFJitterBufferPortParams* pPortParams = *iter;
                    if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                    {
                        pPortParams->ipJitterBuffer->NotifyCanRetrievePacket();
                    }
                }
                /*
                 * Move start to current msg queue where it would stay
                 * jitter buffer is full.
                 */
                oStartPending = true;
                MoveCmdToCurrentQueue(aCmd);
                ReportInfoEvent(PVMFInfoBufferingStart);
                RequestEventCallBack(JB_NOTIFY_REPORT_BUFFERING_STATUS);
            }
            else
            {
                if (false == iMediaReceiveingChannelPrepared)
                {
                    oStartPending = true;
                    MoveCmdToCurrentQueue(aCmd);
                }
                else
                {
                    /* Just resuming from a paused state with enough data in jitter buffer */
                    oStartPending = false;
                    SetState(EPVMFNodeStarted);
                    /* Enable Output Ports */
                    StartOutputPorts();
                    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
                }
            }
        }
        break;

        default:
            status = PVMFErrInvalidState;
            CommandComplete(iInputCommands, aCmd, status);
            break;
    }
    return;
}

void PVMFJitterBufferNode::CompleteStart()
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::CompleteStart"));
    PVMF_JBNODE_LOGDATATRAFFIC((0, "PVMFJitterBufferNode::CompleteStart"));

    if (!iMediaReceiveingChannelPrepared)
        return;

    PVMFJitterBufferNodeCommand aCmd = iCurrentCommand.front();
    if (iJitterBufferState == PVMF_JITTER_BUFFER_READY)
    {
        switch (iInterfaceState)
        {
            case EPVMFNodePrepared:
            case EPVMFNodePaused:
            case EPVMFNodeStarted:
            {
                /* transition to Started */
                oStartPending = false;
                SetState(EPVMFNodeStarted);
                /* Enable Output Ports */
                StartOutputPorts();
                /* Enable remote activity monitoring */
                Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
                for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
                {
                    PVMFJitterBufferPortParams* pPortParams = *it;
                    if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                    {
                        pPortParams->iMonitorForRemoteActivity = true;
                    }
                }
                CommandComplete(aCmd, PVMFSuccess);
                /* Erase the command from the current queue */
                iCurrentCommand.Erase(&iCurrentCommand.front());
            }
            break;

            default:
            {
                SetState(EPVMFNodeError);
                CommandComplete(aCmd, PVMFErrInvalidState);
                /* Erase the command from the current queue */
                iCurrentCommand.Erase(&iCurrentCommand.front());
            }
            break;
        }
    }
    else
    {
        SetState(EPVMFNodeError);
        CommandComplete(aCmd, PVMFErrInvalidState);
        /* Erase the command from the current queue */
        iCurrentCommand.Erase(&iCurrentCommand.front());
    }
}

void PVMFJitterBufferNode::CancelStart()
{
    if (ipJitterBufferMisc)
        ipJitterBufferMisc->Reset();

    PVMFJitterBufferNodeCommand aCmd = iCurrentCommand.front();
    oStartPending = false;
    CommandComplete(aCmd, PVMFErrCancelled);
    /* Erase the command from the current queue */
    iCurrentCommand.Erase(&iCurrentCommand.front());
    return;
}

void PVMFJitterBufferNode::DoStop(PVMFJitterBufferNodeCommand& aCmd)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::DoStop"));
    LogSessionDiagnostics();
    PVMFStatus aStatus = PVMFSuccess;

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            if (ipJitterBufferMisc)
                ipJitterBufferMisc->StreamingSessionStopped();

            /* Clear queued messages in ports */
            for (uint32 i = 0; i < iPortVector.size(); i++)
            {
                PVMFJitterBufferPortParams* pPortParams = NULL;
                bool bRet = getPortContainer(iPortVector[i], pPortParams);
                if (bRet)
                {
                    if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                    {
                        pPortParams->ipJitterBuffer->ResetJitterBuffer();
                    }
                    pPortParams->ResetParams();
                }
                iPortVector[i]->ClearMsgQueues();
            }

            if (aStatus == PVMFSuccess)
            {
                /* Reset State Variables */
                iDelayEstablished = false;
                if (ipJitterBufferMisc)
                    ipJitterBufferMisc->SetSessionDurationExpired();
                oStopOutputPorts = true;
                oStartPending = false;
                iJitterBufferState = PVMF_JITTER_BUFFER_READY;
                iJitterDelayPercent = 0;

                /* transition to Prepared state */
                SetState(EPVMFNodePrepared);
            }
            CommandComplete(iInputCommands, aCmd, aStatus);
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
void PVMFJitterBufferNode::DoFlush(PVMFJitterBufferNodeCommand& aCmd)
{
    OSCL_UNUSED_ARG(aCmd);
}

/**
 * Called by the command handler AO to do the node Pause
 */
void PVMFJitterBufferNode::DoPause(PVMFJitterBufferNodeCommand& aCmd)
{
    iPauseTime = 0;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            uint32 currticks = OsclTickCount::TickCount();
            iPauseTime = OsclTickCount::TicksToMsec(currticks);
            ipJitterBufferMisc->StreamingSessionPaused();
            SetState(EPVMFNodePaused);
            StopOutputPorts();
            CancelEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
            CancelEventCallBack(JB_NOTIFY_REPORT_BUFFERING_STATUS);
            PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoPause Success"));
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;

        default:
            PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::DoPause PVMFErrInvalidState iInterfaceState %d", iInterfaceState));
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

/**
 * Called by the command handler AO to do the node Reset.
 */
void PVMFJitterBufferNode::DoReset(PVMFJitterBufferNodeCommand& aCmd)
{
    PVMF_JBNODE_LOGERROR((0, "JitterBufferNode:DoReset %d", iInterfaceState));
    LogSessionDiagnostics();
    ResetNodeParams();
    SetState(EPVMFNodeIdle);
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
 * Called by the command handler AO to do the Cancel single command
 */
void PVMFJitterBufferNode::DoCancelCommand(PVMFJitterBufferNodeCommand& aCmd)
{
    /* extract the command ID from the parameters.*/
    PVMFCommandId id;
    aCmd.PVMFJitterBufferNodeCommandBase::Parse(id);

    /* first check "current" command if any */
    {
        PVMFJitterBufferNodeCommand* cmd = iCurrentCommand.FindById(id);
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
        PVMFJitterBufferNodeCommand* cmd = iInputCommands.FindById(id, 1);
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
    CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
}

/**
 * Called by the command handler AO to do the Cancel All
 */
void PVMFJitterBufferNode::DoCancelAllCommands(PVMFJitterBufferNodeCommand& aCmd)
{
    /* first cancel the current command if any */
    if (!iCurrentCommand.empty())
    {
        if (iCurrentCommand.front().iCmd == PVMF_JITTER_BUFFER_NODE_PREPARE)
        {
            CancelPrepare();
        }
        else if (iCurrentCommand.front().iCmd == PVMF_JITTER_BUFFER_NODE_START)
        {
            CancelStart();
        }
        else
        {
            OSCL_ASSERT(false);
        }
    }
    /* next cancel all queued commands */
    {
        /* start at element 1 since this cancel command is element 0. */
        while (iInputCommands.size() > 1)
            CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
    }

    uint32 i;
    for (i = 0; i < iPortVector.size(); i++)
    {
        PVMFJitterBufferPortParams* pPortParams = NULL;
        bool bRet = getPortContainer(iPortVector[i], pPortParams);
        if (bRet)
        {
            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                pPortParams->ipJitterBuffer->ResetJitterBuffer();
            }
            pPortParams->ResetParams();
        }
        iPortVector[i]->ClearMsgQueues();
    }


    /* finally, report cancel complete.*/
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

PVMFPortInterface*
PVMFJitterBufferNode::getPortCounterpart(PVMFPortInterface* aPort)
{
    uint32 ii;
    /*
     * Get port params
     */
    for (ii = 0; ii < iPortParamsQueue.size(); ii++)
    {
        if (&iPortParamsQueue[ii]->irPort == aPort)
        {
            break;
        }
    }
    if (ii >= iPortParamsQueue.size())
    {
        return NULL;
    }

    PVMFJitterBufferNodePortTag tag = iPortParamsQueue[ii]->iTag;
    int32 id = iPortParamsQueue[ii]->iId;
    uint32 jj;

    /* Even numbered ports are input ports */
    if (tag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
    {
        for (jj = 0; jj < iPortParamsQueue.size(); jj++)
        {
            if ((id + 1) == iPortParamsQueue[jj]->iId)
            {
                return (&iPortParamsQueue[jj]->irPort);
            }
        }
    }
    /* odd numbered ports are output ports */
    else if (tag == PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT)
    {
        for (jj = 0; jj < iPortParamsQueue.size(); jj++)
        {
            if ((id - 1) == iPortParamsQueue[jj]->iId)
            {
                return (&iPortParamsQueue[jj]->irPort);


            }
        }
    }
    return NULL;
}




///////////////////////////////////////////////////////////////////////////////
//Jitter Buffer Extension Interface Implementation
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//OsclActiveObject
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//PVMFJitterBufferObserver Implementation
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::JitterBufferFreeSpaceAvailable(OsclAny* aContext)
{
    PVMFPortInterface* port = OSCL_STATIC_CAST(PVMFPortInterface*, aContext);
    PVMFJitterBufferPort* jbPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, port);
    PVMFJitterBufferPortParams* portParams = jbPort->iPortParams;
    if (portParams)
        portParams->iProcessIncomingMessages = true;

    if (IsAdded())
    {
        RunIfNotReady();
    }
}

void PVMFJitterBufferNode::ProcessJBInfoEvent(PVMFAsyncEvent& aEvent)
{
    PVMF_JBNODE_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::ProcessJBInfoEvent: Event Type [%d]", aEvent.GetEventType()));
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::ProcessJBInfoEvent Event: Type [%d]", aEvent.GetEventType()));
    switch (aEvent.GetEventType())
    {
        case PVMFInfoUnderflow:
        {
            RequestEventCallBack(JB_NOTIFY_REPORT_BUFFERING_STATUS);
            if (oStartPending == false)
            {
                UpdateRebufferingStats(PVMFInfoUnderflow);
                ipJitterBufferMisc->StreamingSessionBufferingStart();
                ReportInfoEvent(PVMFInfoUnderflow);
                ReportInfoEvent(PVMFInfoBufferingStart);
                ReportInfoEvent(PVMFInfoBufferingStatus);
            }
        }
        break;
        case PVMFInfoDataReady:
        {
            UpdateRebufferingStats(PVMFInfoDataReady);
            ReportInfoEvent(PVMFInfoBufferingStatus);
            ReportInfoEvent(PVMFInfoDataReady);
            ReportInfoEvent(PVMFInfoBufferingComplete);
            CancelEventCallBack(JB_NOTIFY_REPORT_BUFFERING_STATUS);

            Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
            for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
            {
                PVMFJitterBufferPortParams* pPortParams = *it;
                if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                {
                    pPortParams->iCanReceivePktFromJB = true;
                    pPortParams->ipJitterBuffer->CancelNotifyCanRetrievePacket();
                    PVMFJitterBufferStats stats = pPortParams->ipJitterBuffer->getJitterBufferStats();
                    PVMF_JBNODE_LOGDATATRAFFIC((0, "Mime %s stats.currentOccupancy[%d], stats.maxSeqNumRegistered[%d], stats.lastRetrievedSeqNum[%d] stats.maxTimeStampRetrievedWithoutRTPOffset[%d]", pPortParams->iMimeType.get_cstr(), stats.currentOccupancy, stats.maxSeqNumRegistered, stats.lastRetrievedSeqNum, stats.maxTimeStampRetrievedWithoutRTPOffset));
                }
            }

            if (oStartPending)
            {
                CompleteStart();
            }
            else
            {
                ipJitterBufferMisc->StreamingSessionBufferingEnd();

            }
        }
        break;
        case PVMFInfoOverflow:
        {
            ReportInfoEvent(PVMFInfoOverflow);
        }
        break;
        case PVMFJitterBufferNodeJitterBufferLowWaterMarkReached:
        {
            Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
            for (iter = iPortParamsQueue.begin(); iter != iPortParamsQueue.end(); iter++)
            {
                PVMFJitterBufferPortParams* pPortParams = *iter;
                if (pPortParams->iMonitorForRemoteActivity == false)
                {
                    pPortParams->iMonitorForRemoteActivity = true;
                    RequestEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
                }

            }
            ReportInfoEvent(PVMFJitterBufferNodeJitterBufferLowWaterMarkReached);
        }
        break;
        case PVMFJitterBufferNodeJitterBufferHighWaterMarkReached:
        {
            Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
            for (iter = iPortParamsQueue.begin(); iter != iPortParamsQueue.end(); iter++)
            {
                PVMFJitterBufferPortParams* pPortParams = *iter;
                if (pPortParams->iMonitorForRemoteActivity == true)
                {
                    pPortParams->iMonitorForRemoteActivity = false;
                    CancelEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
                }
            }
            ReportInfoEvent(PVMFJitterBufferNodeJitterBufferHighWaterMarkReached);
        }
        break;
        case PVMFJitterBufferNodeStreamThinningRecommended:
        {
            PVMFNodeInterface::ReportInfoEvent(aEvent);
        }
        break;
        default:
        {
            //noop
        }
    }

}

void PVMFJitterBufferNode::PacketReadyToBeRetrieved(OsclAny* aContext)
{
    if (aContext)
    {
        PVMFJitterBufferPort* port = OSCL_REINTERPRET_CAST(PVMFJitterBufferPort*, aContext);
        PVMFJitterBufferPortParams* portparams = port->GetPortParams();
        if (portparams)
        {
            PVMF_JBNODE_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::PacketReadyToBeRetrieved for mime type %s", portparams->iMimeType.get_cstr()));
            portparams->iCanReceivePktFromJB = true;
        }
    }
}

void PVMFJitterBufferNode::EndOfStreamSignalled(OsclAny* aContext)
{
    if (aContext)
    {
        PVLogger* ipDataPathLoggerRTCP = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp");
        PVMF_JBNODE_LOG_RTCP_DATAPATH((0, "PVMFJitterBufferNode::EndOfStreamSignalled"));
        PVMFJitterBufferPort* port = OSCL_REINTERPRET_CAST(PVMFJitterBufferPort*, aContext);
        PVMFJitterBufferPortParams* portparams = port->GetPortParams();
        if (portparams)
        {
            RunIfNotReady();
        }
    }
}

void PVMFJitterBufferNode::UpdateRebufferingStats(PVMFEventType aEventType)
{
    if (aEventType == PVMFInfoUnderflow)
    {
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
        for (it = iPortParamsQueue.begin();
                it != iPortParamsQueue.end();
                it++)
        {
            PVMFJitterBufferPortParams* pJitterBufferPortParams = *it;
            if (pJitterBufferPortParams->iMonitorForRemoteActivity == false)
            {
                pJitterBufferPortParams->iMonitorForRemoteActivity = true;
                CancelEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
                RequestEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
            }
        }

        PVMF_JBNODE_LOGDATATRAFFIC_FLOWCTRL_E((0, "PVMFJitterBufferNode::UpdateRebufferingStats: Sending Auto Resume"));
    }

}
///////////////////////////////////////////////////////////////////////////////
//PVMFJitterBufferMiscObserver
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::MessageReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage)
{
    PVMF_JBNODE_LOGINFO((0, "0x%x PVMFJitterBufferNode::MessageReadyToSend: aPort=0x%x", this, aPort));

    PVMFJitterBufferPort* jitterbufferPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, aPort);

    //Kind of messages received here
    //RTCP reports
    //Firewall packets
    //We do not expect port to go in busy state at firewall port and if somehow port used for sending rtcp messages
    //is in busy state, or gets into busy state, we just ignore it and discard the message
    PVMFStatus status = PVMFSuccess;
    aPort->QueueOutgoingMsg(aMessage);
    status = aPort->Send();
    if (status == PVMFSuccess)
    {
        jitterbufferPort->iPortParams->iNumMediaMsgsSent++;
    }
    return;
}

void PVMFJitterBufferNode::MediaReceivingChannelPrepared(bool aStatus)
{
    //ignore the status param.
    OSCL_UNUSED_ARG(aStatus);
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode::MediaRecvChannelPrerared In"));
    iMediaReceiveingChannelPrepared = true;
    if (iCurrentCommand.size())
    {
        PVMFJitterBufferNodeCommand& cmd = iCurrentCommand.front();
        if (PVMF_JITTER_BUFFER_NODE_PREPARE == cmd.iCmd)
        {
            CompletePrepare();
        }
        if (PVMF_JITTER_BUFFER_NODE_START == cmd.iCmd)
        {
            CompleteStart();
        }
    }
}

void PVMFJitterBufferNode::ProcessRTCPControllerEvent(PVMFAsyncEvent& aEvent)
{
    PVMFNodeInterface::ReportInfoEvent(aEvent);
}

void PVMFJitterBufferNode::SessionSessionExpired()
{

    CancelEventCallBack(JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED);
}

///////////////////////////////////////////////////////////////////////////////
//PVMFJBEventNotifierObserver implementation
///////////////////////////////////////////////////////////////////////////////
void PVMFJitterBufferNode::ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aClockNotificationInterfaceType);
    OSCL_UNUSED_ARG(aContext);
    PVMF_JBNODE_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::ProcessCallBack In CallBackId [%d] ", aCallBkId));

    if (PVMFSuccess == aStatus)
    {
        if (aCallBkId == iIncomingMediaInactivityDurationCallBkId)
        {
            iIncomingMediaInactivityDurationCallBkPending = false;
            HandleEvent_IncomingMediaInactivityDurationExpired();
        }
        else if (aCallBkId == iNotifyBufferingStatusCallBkId)
        {
            iNotifyBufferingStatusCallBkPending = false;
            HandleEvent_NotifyReportBufferingStatus();
        }
    }
    else
    {

        //Log it
    }
    PVMF_JBNODE_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::ProcessCallBack Out"));
}

void PVMFJitterBufferNode::HandleEvent_IncomingMediaInactivityDurationExpired()
{
    PVMF_JBNODE_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::HandleEvent_IncomingMediaInactivityDurationExpired In"));

    PVUuid eventuuid = PVMFJitterBufferNodeEventTypeUUID;
    int32 errcode = PVMFJitterBufferNodeRemoteInactivityTimerExpired;

    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::HandleEvent_IncomingMediaInactivityDurationExpired- iCurrentCommand.size()[%d]", iCurrentCommand.size()));
    if (iCurrentCommand.size() > 0)
    {
        PVMFJitterBufferNodeCommand cmd = iCurrentCommand.front();
        CommandComplete(cmd, PVMFFailure, NULL, &eventuuid, &errcode);
        iCurrentCommand.Erase(&iCurrentCommand.front());
    }
    else
    {
        ReportInfoEvent(PVMFErrTimeout, NULL, &eventuuid, &errcode);
        ipJitterBufferMisc->SetSessionDurationExpired();
        if (IsAdded())
        {
            RunIfNotReady();
        }
    }

    PVMF_JBNODE_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::HandleEvent_IncomingMediaInactivityDurationExpired Out"));
}

void PVMFJitterBufferNode::HandleEvent_NotifyReportBufferingStatus()
{
    PVMF_JBNODE_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::HandleEvent_NotifyReportBufferingStatus In"));
    if (iDelayEstablished == false)
    {
        /*
         * Check to see if the session duration has expired
         */
        if (ipJitterBufferMisc->IsSessionExpired())
        {
            PVMF_JBNODE_LOGCLOCK((0, "PVMFJitterBufferNode::TimeoutOccurred - Session Duration Expired"));
            /* Force out of rebuffering */
            Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
            for (it = iPortParamsQueue.begin();
                    it != iPortParamsQueue.end();
                    it++)
            {
                PVMFJitterBufferPortParams* pPortParams = *it;
                if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                {
                    SendData(&(pPortParams->irPort));
                }
            }
            if (IsAdded())
            {
                RunIfNotReady();
            }
        }
        else
        {
            ReportInfoEvent(PVMFInfoBufferingStatus);
            RequestEventCallBack(JB_NOTIFY_REPORT_BUFFERING_STATUS);
        }
    }
    PVMF_JBNODE_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::HandleEvent_NotifyReportBufferingStatus Out"));
}

///////////////////////////////////////////////////////////////////////////////
//Utility functions
///////////////////////////////////////////////////////////////////////////////

bool
PVMFJitterBufferNode::getPortContainer(PVMFPortInterface* aPort,
                                       PVMFJitterBufferPortParams*& aPortParamsPtr)
{
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;

    for (it = iPortParamsQueue.begin();
            it != iPortParamsQueue.end();
            it++)
    {
        PVMFJitterBufferPortParams* pPortParams = *it;
        if (&pPortParams->irPort == aPort)
        {
            aPortParamsPtr = *it;
            return true;
        }
    }
    return false;
}

bool PVMFJitterBufferNode::RequestEventCallBack(JB_NOTIFY_CALLBACK aEventType, uint32 aDelay, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aDelay);
    OSCL_UNUSED_ARG(aContext);
    bool retval = false;
    switch (aEventType)
    {
        case JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED:
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
            retval = ipEventNotifier->RequestCallBack(eventRequestInfo, iMaxInactivityDurationForMediaInMs, iIncomingMediaInactivityDurationCallBkId);
            if (retval)
            {
                iIncomingMediaInactivityDurationCallBkPending = true;
            }
        }
        break;
        case JB_NOTIFY_REPORT_BUFFERING_STATUS:
        {
            if (iNotifyBufferingStatusCallBkPending)
            {
                CancelEventCallBack(JB_NOTIFY_REPORT_BUFFERING_STATUS);
            }
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
            retval = ipEventNotifier->RequestCallBack(eventRequestInfo, iBufferingStatusIntervalInMs, iNotifyBufferingStatusCallBkId);
            if (retval)
            {
                iNotifyBufferingStatusCallBkPending = true;
            }
        }
        break;

        default:
        {
            //Log it
        }
    }
    return retval;
}

void PVMFJitterBufferNode::CancelEventCallBack(JB_NOTIFY_CALLBACK aEventType, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    switch (aEventType)
    {
        case JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED:
        {
            if (iIncomingMediaInactivityDurationCallBkPending)
            {
                PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
                ipEventNotifier->CancelCallBack(eventRequestInfo, iIncomingMediaInactivityDurationCallBkId);
                iIncomingMediaInactivityDurationCallBkPending = false;
            }
        }
        break;
        case JB_NOTIFY_REPORT_BUFFERING_STATUS:
        {
            if (iNotifyBufferingStatusCallBkPending)
            {
                PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
                ipEventNotifier->CancelCallBack(eventRequestInfo, iNotifyBufferingStatusCallBkId);
                iNotifyBufferingStatusCallBkPending = false;
            }
        }
        break;

        default:
        {
            //Log it
        }
    }
    return;
}

void PVMFJitterBufferNode::SetState(TPVMFNodeInterfaceState s)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}

void PVMFJitterBufferNode::ReportErrorEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode:NodeErrorEvent Type %d Data %d"
                          , aEventType, aEventData));

    if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg;
        PVMF_JITTER_BUFFER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), eventmsg);
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

void PVMFJitterBufferNode::ReportInfoEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_JBNODE_LOGINFO((0, "PVMFJitterBufferNode:NodeInfoEvent Type %d Data %d"
                         , aEventType, aEventData));

    if (aEventType == PVMFInfoBufferingStatus)
    {
        uint8 localbuffer[8];
        oscl_memset(localbuffer, 0, 8);
        localbuffer[0] = 1;
        oscl_memcpy(&localbuffer[4], &iJitterDelayPercent, sizeof(uint32));
        PVMFAsyncEvent asyncevent(PVMFInfoEvent,
                                  aEventType,
                                  NULL,
                                  NULL,
                                  aEventData,
                                  localbuffer,
                                  8);
        PVMFNodeInterface::ReportInfoEvent(asyncevent);
    }
    else if (aEventUUID && aEventCode)
    {
        PVMFBasicErrorInfoMessage* eventmsg;
        PVMF_JITTER_BUFFER_NEW(NULL, PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL), eventmsg);
        PVMFErrorInfoMessageInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFErrorInfoMessageInterface*, eventmsg);
        PVMFAsyncEvent asyncevent(PVMFInfoEvent,
                                  aEventType,
                                  NULL,
                                  OSCL_STATIC_CAST(PVInterface*, interimPtr),
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

PVMFJitterBuffer*
PVMFJitterBufferNode::findJitterBuffer(PVMFPortInterface* aPort)
{
    uint32 ii;
    for (ii = 0; ii < iPortParamsQueue.size(); ii++)
    {

        if (&iPortParamsQueue[ii]->irPort == aPort)
        {
            return (iPortParamsQueue[ii]->ipJitterBuffer);

        }
    }
    return NULL;
}


void PVMFJitterBufferNode::LogSessionDiagnostics()
{
    if (iDiagnosticsLogged == false)
    {
        ipDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.streamingmanager");

        LogPortDiagnostics();

        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
        for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
        {
            PVMFJitterBufferPortParams* pPortParams = *it;
            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                PVMFJitterBuffer* jitterBuffer = findJitterBuffer(&pPortParams->irPort);
                if (jitterBuffer != NULL)
                {
                    PVMFJitterBufferStats jbStats = jitterBuffer->getJitterBufferStats();
                    uint32 in_wrap_count = 0;
                    uint32 max_ts_reg = jbStats.maxTimeStampRegistered;
                    pPortParams->iMediaClockConverter.set_clock(max_ts_reg, in_wrap_count);

                    in_wrap_count = 0;
                    uint32 max_ts_ret = jbStats.maxTimeStampRetrieved;
                    pPortParams->iMediaClockConverter.set_clock(max_ts_ret, in_wrap_count);

                    uint32 currentTime32 = 0;
                    uint32 currentTimeBase32 = 0;
                    bool overflowFlag = false;
                    ipJitterBufferMisc->GetEstimatedServerClock().GetCurrentTime32(currentTime32,
                            overflowFlag,
                            PVMF_MEDIA_CLOCK_MSEC,
                            currentTimeBase32);
                    uint32 bitrate32 = 0;
                    uint32 totalNumBytesRecvd = jbStats.totalNumBytesRecvd;
                    if (currentTime32 != 0)
                    {
                        bitrate32 = (totalNumBytesRecvd / currentTime32);
                    }

                    bitrate32 *= 8;

                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "JitterBuffer - TrackMime Type = %s", pPortParams->iMimeType.get_cstr()));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "Total Num Packets Recvd = %d", jbStats.totalNumPacketsReceived));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "Total Num Packets Registered Into JitterBuffer = %d", jbStats.totalNumPacketsRegistered));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "Total Num Packets Retrieved From JitterBuffer = %d", jbStats.totalNumPacketsRetrieved));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "MaxSeqNum Recvd = %d", jbStats.maxSeqNumReceived));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "MaxSeqNum Registered = %d", jbStats.maxSeqNumRegistered));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "MaxSeqNum Retrieved = %d", jbStats.lastRetrievedSeqNum));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "MaxTimeStamp Registered In MS = %d", pPortParams->iMediaClockConverter.get_converted_ts(1000)));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "MaxTimeStamp Retrieved In MS = %d", pPortParams->iMediaClockConverter.get_converted_ts(1000)));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "Total Number of Packets Lost = %d", jbStats.totalPacketsLost));
                    PVMF_JBNODE_LOGDIAGNOSTICS((0, "Estimated Bitrate = %d", bitrate32));
                }
            }
        }
        iDiagnosticsLogged = true;
    }
}

void PVMFJitterBufferNode::LogPortDiagnostics()
{
    PVLogger* ipDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.streamingmanager");

    PVMF_JBNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
    PVMF_JBNODE_LOGDIAGNOSTICS((0, "PVMFJitterBufferNode - iNumRunL = %d", iNumRunL));

    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = iPortParamsQueue.begin(); it != iPortParamsQueue.end(); it++)
    {
        PVMFJitterBufferPortParams* pPortParams = *it;
        PvmfPortBaseImpl* ptr =
            OSCL_STATIC_CAST(PvmfPortBaseImpl*, &pPortParams->irPort);
        PvmfPortBaseImplStats stats;
        ptr->GetStats(stats);

        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "PVMF_JITTER_BUFFER_PORT_TYPE_INPUT"));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingMsgRecv = %d", stats.iIncomingMsgRecv));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingMsgConsumed = %d", stats.iIncomingMsgConsumed));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingQueueBusy = %d", stats.iIncomingQueueBusy));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgQueued = %d", stats.iOutgoingMsgQueued));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgSent = %d", stats.iOutgoingMsgSent));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingQueueBusy = %d", stats.iOutgoingQueueBusy));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgDiscarded = %d", stats.iOutgoingMsgDiscarded));
        }
        else if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK)
        {
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK"));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingMsgRecv = %d", stats.iIncomingMsgRecv));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingMsgConsumed = %d", stats.iIncomingMsgConsumed));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingQueueBusy = %d", stats.iIncomingQueueBusy));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgQueued = %d", stats.iOutgoingMsgQueued));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgSent = %d", stats.iOutgoingMsgSent));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingQueueBusy = %d", stats.iOutgoingQueueBusy));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgDiscarded = %d", stats.iOutgoingMsgDiscarded));
        }
        else if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT)
        {
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "PVMF_JITTER_BUFFER_PORT_TYPE_OUTPUT"));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingMsgRecv = %d", stats.iIncomingMsgRecv));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingMsgConsumed = %d", stats.iIncomingMsgConsumed));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iIncomingQueueBusy = %d", stats.iIncomingQueueBusy));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgQueued = %d", stats.iOutgoingMsgQueued));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgSent = %d", stats.iOutgoingMsgSent));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingQueueBusy = %d", stats.iOutgoingQueueBusy));
            PVMF_JBNODE_LOGDIAGNOSTICS((0, "iOutgoingMsgDiscarded = %d", stats.iOutgoingMsgDiscarded));
        }
    }
}

bool PVMFJitterBufferNode::PrepareForPlaylistSwitch()
{
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
    uint32 clientClock32 = 0;
    bool overflowFlag = false;
    ipClientPlayBackClock->GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    uint32 serverClock32 = ipJitterBufferMisc->GetEstimatedServerClockValue();
    PVMF_JBNODE_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::PrepareForPlaylistSwitch - Before - EstServClock=%d",
                                 serverClock32));
    PVMF_JBNODE_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::PrepareForPlaylistSwitch - Before - ClientClock=%d",
                                 clientClock32));
#endif
    iJitterBufferState = PVMF_JITTER_BUFFER_IN_TRANSITION;
    ipClientPlayBackClock->Pause();

    return true;
}

void PVMFJitterBufferNode::ClockStateUpdated()
{
    PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ClockStateUpdated - iClientPlayBackClock[%d]", ipClientPlayBackClock->GetState()));
    if (!iDelayEstablished)
    {
        // Don't let anyone start the clock while
        // we're rebuffering
        if (ipClientPlayBackClock != NULL)
        {
            if (ipClientPlayBackClock->GetState() == PVMFMediaClock::RUNNING)
            {
                PVMF_JBNODE_LOGERROR((0, "PVMFJitterBufferNode::ClockStateUpdated - Clock was started during rebuffering.  Pausing..."));
                ipClientPlayBackClock->Pause();
            }
        }
    }
}

void PVMFJitterBufferNode::NotificationsInterfaceDestroyed()
{
    //noop
}
