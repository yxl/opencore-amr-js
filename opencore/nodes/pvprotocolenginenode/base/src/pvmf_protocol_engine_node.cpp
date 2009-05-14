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
#include "pvmf_protocol_engine_node.h"
#include "pvmf_protocol_engine_command_format_ids.h"
#include "pvmf_protocolengine_node_tunables.h"
#include "pvlogger.h"
#include "oscl_utf8conv.h"


/**
//////////////////////////////////////////////////
// Node Constructor & Destructor
//////////////////////////////////////////////////
*/

PVMFProtocolEngineNode::PVMFProtocolEngineNode(int32 aPriority) :
        OsclTimerObject(aPriority, "PVMFProtocolEngineNode"),
        iStatusCode(0),
        iProcessingState(ProcessingState_Idle),
        iInterfacingObjectContainer(NULL),
        iProtocol(NULL),
        iProtocolContainer(NULL),
        iProtocolContainerFactory(NULL),
        iNodeOutput(NULL),
        iCurrEventHandler(NULL),
        iCfgFileContainer(NULL),
        iDownloadSource(NULL),
        iDownloadControl(NULL),
        iDownloadProgess(NULL),
        iSDPInfo(NULL),
        iUserAgentField(NULL),
        iEventReport(NULL),
        iPortConfigFSInfoAlloc(NULL),
        iPortConfigMemPool(PVHTTPDOWNLOADOUTPUT_CONTENTDATA_POOLNUM, OSCL_REFCOUNTER_MEMFRAG_DEFAULT_SIZE),
        iNodeTimer(NULL),
        iPortInForData(NULL),
        iPortInForLogging(NULL),
        iPortOut(NULL),
        iCurrentCmdId(0),
        iCmdRespPort(NULL),
        iLogger(NULL),
        iDataPathLogger(NULL),
        iClockLogger(NULL),
        iExtensionRefCount(0),
        iCurrentDataStreamCmdId(0)
{
    int32 err = 0;
    OSCL_TRY(err,
             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iInputCommands.Construct(PVMF_PROTOCOLENGINE_NODE_COMMAND_ID_START, PVMF_PROTOCOLENGINE_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Create the port vector.
             iPortVector.Construct(PVMF_PROTOCOLENGINE_NODE_PORT_VECTOR_RESERVE);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = false;
             iCapability.iCanSupportMultipleOutputPorts = false;
             iCapability.iHasMaxNumberOfPorts = false;
             iCapability.iMaxNumberOfPorts = 0;//no maximum
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_INET_TCP);

             // create the internal data queue
             iDataInQueue.reserve(PVPROTOCOLENGINE_RESERVED_NUMBER_OF_FRAMES);
             iInternalEventQueue.reserve(PVPROTOCOLENGINE_RESERVED_NUMBER_OF_FRAMES);
            );

    if (err)
    {
        //if a leave happened, cleanup and re-throw the error
        iInputCommands.clear();
        iCurrentCommand.clear();
        iPortVector.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        iDataInQueue.clear();
        iInternalEventQueue.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclTimerObject);
        OSCL_LEAVE(err);
    }

    for (uint32 i = 0; i < EVENT_HANDLER_TOTAL; i++) iEventHandlers[i] = NULL;
}

PVMFProtocolEngineNode::~PVMFProtocolEngineNode()
{
    //thread logoff
    if (IsAdded()) RemoveFromScheduler();


    //Cleanup commands
    //The command queues are self-deleting, but we want to
    //notify the observer of unprocessed commands.
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
    }

    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }

    Clear(true);
}

/**
//////////////////////////////////////////////////
// Public Node API implementation
//////////////////////////////////////////////////
*/


PVMFStatus PVMFProtocolEngineNode::ThreadLogon()
{
    LOGINFO((0, "PVMFProtocolEngineNode::ThreadLogon()"));
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded()) AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFProtocolEngineNode");
            iDataPathLogger = PVLogger::GetLoggerObject(NODEDATAPATHLOGGER_TAG);
            iClockLogger = PVLogger::GetLoggerObject("clock");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}


PVMFStatus PVMFProtocolEngineNode::ThreadLogoff()
{
    LOGINFO((0, "PVMFProtocolEngineNode::ThreadLogoff()"));
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded()) RemoveFromScheduler();

            iLogger = NULL;
            iDataPathLogger = NULL;
            iClockLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}


PVMFStatus PVMFProtocolEngineNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    OSCL_UNUSED_ARG(aNodeCapability);
    LOGINFO((0, "PVMFProtocolEngineNode::GetCapability()"));
    //aNodeCapability=iCapability;
    return PVMFSuccess;
}


PVMFPortIter* PVMFProtocolEngineNode::GetPorts(const PVMFPortFilter* aFilter)
{
    LOGINFO((0, "PVMFProtocolEngineNode::GetPorts()"));
    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}


PVMFCommandId PVMFProtocolEngineNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, PVMFProtocolEngineNodeAllocator>& aUuids,
        bool aExactUuidsOnly, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::QueryUUID()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::QueryInterface()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aPortConfig);
    LOGINFO((0, "PVMFProtocolEngineNode::RequestPort()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::ReleasePort()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Init()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Prepare()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Start()"));

    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Stop()"));

    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Flush()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Pause()"));

    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Reset()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::CancelAllCommands()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::CancelCommand()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFProtocolEngineNode::Seek(PVMFSessionId aSessionId, uint64 aNPTInMS, uint32& aFirstSeqNumAfterSeek, OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::Seek()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommand::Construct(aSessionId, PVPROTOCOLENGINE_NODE_CMD_SEEK, aNPTInMS, aFirstSeqNumAfterSeek, aContext);
    return QueueCommandL(cmd);
}

PVMFCommandId PVMFProtocolEngineNode::BitstreamSwitch(PVMFSessionId aSessionId, uint64 aNPTInMS, uint32& aFirstSeqNumAfterSwitch, OsclAny* aContext)
{
    LOGINFO((0, "PVMFProtocolEngineNode::BitstreamSwitch()"));
    PVMFProtocolEngineNodeCommand cmd;
    cmd.PVMFProtocolEngineNodeCommand::Construct(aSessionId, PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH, aNPTInMS, aFirstSeqNumAfterSwitch, aContext);
    return QueueCommandL(cmd);
}

PvmiDataStreamCommandId PVMFProtocolEngineNode::DataStreamRequest(PvmiDataStreamSession aSessionID, PvmiDataStreamRequest aRequestID,
        OsclAny* aRequestData, OsclAny* aContextData)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::DataStreamRequest()"));

    // This is for asynchronous requests, such as repositioning
    // Caller's SourceRequestCompleted will be called
    if (aRequestID != PVDS_REQUEST_REPOSITION)
    {
        // currently we only define reposition request
        OSCL_LEAVE(OsclErrArgument);
        return 0;
    }

    PVMFProtocolEngineNodeCommand cmd; // internal command, use 0 as session id.
    cmd.PVMFProtocolEngineNodeCommand::Construct((PVMFSessionId)0, PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION,
            aSessionID, aRequestID, aRequestData, iCurrentDataStreamCmdId, aContextData);
    QueueCommandL(cmd);
    return iCurrentDataStreamCmdId++;
}

PvmiDataStreamStatus PVMFProtocolEngineNode::DataStreamRequestSync(PvmiDataStreamSession aSessionID, PvmiDataStreamRequest aRequestID, OsclAny* aRequestData)
{
    OSCL_UNUSED_ARG(aSessionID);
    // This is for synchronous requests, such as returning memory fragments
    PvmiDataStreamStatus status = PVDS_FAILURE;
    switch (aRequestID)
    {
        case PVDS_REQUEST_MEM_FRAG_RELEASED:
            if (aRequestData != NULL)
            {
                LOGINFODATAPATH((0, "PVMFProtocolEngineNode::DataStreamRequestSync, got PVDS_REQUEST_MEM_FRAG_RELEASED request"));
                OsclRefCounterMemFrag* frag = ((OsclRefCounterMemFrag*)aRequestData);
                if (iNodeOutput->releaseMemFrag(frag)) status = PVDS_SUCCESS;
            }
            break;

        default:
            break;
    }

    return status;
}

bool PVMFProtocolEngineNode::IsRepositioningRequestPending()
{
    PVMFProtocolEngineNodeCommand *pInputCmd   = FindCmd(iInputCommands, PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION);
    PVMFProtocolEngineNodeCommand *pPendingCmd = FindCmd(iCurrentCommand, PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION);

    return (pInputCmd || pPendingCmd);
}

void PVMFProtocolEngineNode::addRef()
{
    ++iExtensionRefCount;
}

void PVMFProtocolEngineNode::removeRef()
{
    --iExtensionRefCount;
}

bool PVMFProtocolEngineNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMF_DATA_SOURCE_INIT_INTERFACE_UUID)
    {
        PVMFDataSourceInitializationExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFDataSourceInitializationExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
        return true;
    }
    else if (uuid == PVMIDatastreamuserInterfaceUuid)
    {
        PVMIDatastreamuserInterface* myInterface = OSCL_STATIC_CAST(PVMIDatastreamuserInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
        return true;
    }
    else if (uuid == KPVMFProtocolEngineNodeExtensionUuid)
    {
        PVMFProtocolEngineNodeExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFProtocolEngineNodeExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
        return true;
    }
    else if (uuid == PVMF_DOWNLOAD_PROGRESS_INTERFACE_UUID)
    {
        PVMFDownloadProgressInterface* myInterface = OSCL_STATIC_CAST(PVMFDownloadProgressInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
        return true;
    }
    else if (uuid == KPVMFProtocolEngineNodeMSHTTPStreamingExtensionUuid)
    {
        PVMFProtocolEngineNodeMSHTTPStreamingExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFProtocolEngineNodeMSHTTPStreamingExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
        return true;
    }
    else if (uuid == PVMF_TRACK_SELECTION_INTERFACE_UUID)
    {
        if (!iInterfacingObjectContainer || iInterfacingObjectContainer->getDownloadFormat() != PVMF_MIME_DATA_SOURCE_PVX_FILE) return false;
        PVMFTrackSelectionExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFTrackSelectionExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
        return true;
    }

    return false;
}

/**
//This routine is called by various command APIs to queue an
//asynchronous command for processing by the command handler AO.
//This function may leave if the command can't be queued due to
//memory allocation failure.
*/
PVMFCommandId PVMFProtocolEngineNode::QueueCommandL(PVMFProtocolEngineNodeCommand& aCmd)
{
    PVMFCommandId id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();
    return id;
}

PVMFProtocolEngineNodeCommand* PVMFProtocolEngineNode::FindCmd(PVMFProtocolEngineNodeCmdQ &aCmdQueue, int32 aCmdId)
{
    for (uint32 i = 0; i < aCmdQueue.size(); i++)
    {
        if (aCmdQueue[i].iCmd == aCmdId) return &aCmdQueue[i];
    }
    return NULL;
}

/**
/////////////////////////////////////////////////////
// Asynchronous Command processing routines.
// These routines are all called under the AO.
/////////////////////////////////////////////////////
*/

/**
//Called by the command handler AO to process a command from
//the input queue.
//Return true if a command was processed, false if the command
//processor is busy and can't process another command now.
*/
bool PVMFProtocolEngineNode::ProcessCommand(PVMFProtocolEngineNodeCommand& aCmd)
{
    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCommand.empty() && !aCmd.hipri())
    {
        return false; // keep waiting
    }

    PVMFStatus cmdStatus;
    switch (aCmd.iCmd)
    {
        case PVMF_GENERIC_NODE_REQUESTPORT:
            cmdStatus = DoRequestPort(aCmd);
            break;

        case PVMF_GENERIC_NODE_RELEASEPORT:
            cmdStatus = DoReleasePort(aCmd);
            break;

        case PVMF_GENERIC_NODE_QUERYUUID:
            cmdStatus = DoQueryUuid(aCmd);
            break;

        case PVMF_GENERIC_NODE_QUERYINTERFACE:
            cmdStatus = DoQueryInterface(aCmd);
            break;

        case PVMF_GENERIC_NODE_INIT:
            cmdStatus = DoInit(aCmd);
            break;

        case PVMF_GENERIC_NODE_PREPARE:
            cmdStatus = DoPrepare(aCmd);
            break;

        case PVMF_GENERIC_NODE_START:
            cmdStatus = DoStart(aCmd);
            break;

        case PVMF_GENERIC_NODE_STOP:
            cmdStatus = DoStop(aCmd);
            break;

        case PVMF_GENERIC_NODE_FLUSH:
            cmdStatus = DoFlush(aCmd);
            break;

        case PVMF_GENERIC_NODE_PAUSE:
            cmdStatus = DoPause(aCmd);
            break;

        case PVMF_GENERIC_NODE_RESET:
            cmdStatus = DoReset(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            cmdStatus = DoCancelAllCommands(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
            cmdStatus = DoCancelCommand(aCmd);
            break;

        case PVPROTOCOLENGINE_NODE_CMD_SEEK:
            cmdStatus = DoSeek(aCmd);
            break;

        case PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH:
            cmdStatus = DoBitsteamSwitch(aCmd);
            break;

        case PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION:
            cmdStatus = DoReposition(aCmd);
            break;

        default://unknown command type. Assert and treat as not supported error
            OSCL_ASSERT(false);
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
            cmdStatus = PVMFErrNotSupported;
            break;
    }

    //If completion is pending, move the command from the input queue to the current command.
    //This is necessary since the input queue could get rearranged by new commands coming in.
    if (cmdStatus == PVMFPending)
    {
        iCurrentCommand.StoreL(aCmd);
        iInputCommands.Erase(&aCmd);
    }

    return true;
}

/**
//The various command handlers call this when a command is complete.
*/
int32 PVMFProtocolEngineNode::HandleCommandComplete(PVMFProtocolEngineNodeCmdQ& aCmdQ,
        PVMFProtocolEngineNodeCommand& aCmd,
        int32 aStatus)
{
    if (aStatus == PVMFPending) return PVMFPending;
    if (aStatus > 0 || IsPVMFErrCode(aStatus))
    {
        CommandComplete(aCmdQ, aCmd, aStatus); // no extension error code in case of error
        return aStatus;
    }

    // should be PE node extension error code
    PVUuid uuid = PVProtocolEngineNodeErrorEventTypesUUID;
    // Check for error code for not enough disk space
    int32 pvmfReturnCode = PVMFFailure;
    if (aStatus == PROCESS_DATA_STREAM_OPEN_FAILURE) pvmfReturnCode = PVMFErrResource;
    int32 errorCode = (int32)PVProtocolEngineNodeErrorProcessingFailure - (PROCESS_ERROR_FIRST - aStatus);
    CommandComplete(aCmdQ, aCmd, pvmfReturnCode, NULL, &uuid, &errorCode);
    return pvmfReturnCode;
}


void PVMFProtocolEngineNode::CommandComplete(PVMFProtocolEngineNodeCmdQ& aCmdQ,
        PVMFProtocolEngineNodeCommand& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode,
        int32 aEventDataLen)

{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFProtocolEngineNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    if (aEventDataLen != 0)
    {
        resp.SetEventDataLen(aEventDataLen);
    }

    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);

    if (errormsg) errormsg->removeRef();
}


/**
//Called by the command handler AO to do the node Reset.
*/
PVMFStatus PVMFProtocolEngineNode::DoReset(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoReset()"));

    // Allow a reset on ANY state.
    ResetClear(true); // true means deleting the relevant objects

    // Logoff and go back to Created state.
    SetState(EPVMFNodeIdle);
    PVMFStatus	status = ThreadLogoff();
    CommandComplete(iInputCommands, aCmd, status);
    return status;
}


/**
//Called by the command handler AO to do the port request
*/
PVMFStatus PVMFProtocolEngineNode::DoRequestPort(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoRequestPort()"));

    //This node supports port request from any state

    //retrieve port tag.
    int32 tag;
    OSCL_String* mimetype;
    aCmd.PVMFProtocolEngineNodeCommandBase::Parse(tag, mimetype);

    //(mimetype is not used on this node)

    //validate the tag...
    switch (tag)
    {
        case PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT:
        case PVMF_PROTOCOLENGINENODE_PORT_TYPE_OUTPUT:
        case PVMF_PROTOCOLENGINENODE_PORT_TYPE_FEEDBACK:
            break;

        default:
        {
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFProtocolEngineNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return PVMFFailure;
        }
        // break;	This statement was removed to avoid compiler warning for Unreachable Code
    }

    //Allocate a new port
    OsclAny *ptr = NULL;
    int32 err;
    OSCL_TRY(err, ptr = iPortVector.Allocate(););
    if (err != OsclErrNone || !ptr)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFProtocolEngineNode::DoRequestPort: Error - iPortVector Out of memory"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return PVMFErrNoMemory;
    }

    //create base port with default settings...
    PVMFProtocolEnginePort*port = NULL;
    switch (tag)
    {
        case PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT:
        case PVMF_PROTOCOLENGINENODE_PORT_TYPE_FEEDBACK:
            //create base port with default settings...
            port = new(ptr) PVMFProtocolEnginePort(tag, this,
                                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                                   DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT,
                                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                                   DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT,
                                                   "ProtocolEngineIn(Protocol)");

            if (tag == PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT)		iPortInForData = port;
            if (tag == PVMF_PROTOCOLENGINENODE_PORT_TYPE_FEEDBACK)	iPortInForLogging = port;
            break;

        case PVMF_PROTOCOLENGINENODE_PORT_TYPE_OUTPUT:
            port = new(ptr) PVMFProtocolEnginePort(tag, this,
                                                   0, 0, 0,	// input queue isn't needed.
                                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                                   DEFAULT_DATA_QUEUE_CAPACITY,
                                                   DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT,
                                                   "ProtocolEngineOut(Protocol)");

            iPortOut = port;
            break;
    }

    iPortActivityQueue.reserve(PVMF_PROTOCOLENGINE_NODE_COMMAND_VECTOR_RESERVE);

    //Add the port to the port vector.
    OSCL_TRY(err, iPortVector.AddL(port););
    if (err != OsclErrNone)
    {
        iPortInForData = iPortInForLogging = iPortOut = NULL;
        OSCL_DELETE(port);
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return PVMFErrNoMemory;
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)port, 0, 0, sizeof(PVMFProtocolEnginePort));
    return PVMFSuccess;
}

/**
//Called by the command handler AO to do the port release
*/
PVMFStatus PVMFProtocolEngineNode::DoReleasePort(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoReleasePort()"));

    //This node supports release port from any state
    PVMFStatus status = PVMFSuccess;

    //Find the port in the port vector
    PVMFPortInterface* p = NULL;
    aCmd.PVMFProtocolEngineNodeCommandBase::Parse(p);

    PVMFProtocolEnginePort* port = (PVMFProtocolEnginePort*)p;


    if (port == NULL) status = PVMFErrArgument;

    PVMFProtocolEnginePort** portPtr = iPortVector.FindByValue(port);
    if (portPtr)
    {
        if (*portPtr == iPortInForData)	iPortInForData = NULL;
        if (*portPtr == iPortInForLogging) iPortInForLogging = NULL;
        if (*portPtr == iPortOut)			iPortOut = NULL;

        Clear(true);

        //delete the port.
        iPortVector.Erase(portPtr);
        status = PVMFSuccess;
    }
    else
    {
        //port not found.
        status = PVMFErrArgument;
    }

    CommandComplete(iInputCommands, aCmd, status);
    return status;
}


PVMFStatus PVMFProtocolEngineNode::DoQueryUuid(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoQueryUuid()"));

    //This node supports Query UUID from any state
    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFProtocolEngineNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //TODO: Try to match the input mimetype against any of
    //the custom interfaces for this node
    PVUuid uuid1(PVMF_DATA_SOURCE_INIT_INTERFACE_UUID);
    uuidvec->push_back(uuid1);
    PVUuid uuid2(PVMIDatastreamuserInterfaceUuid);
    uuidvec->push_back(uuid2);
    PVUuid uuid3(KPVMFProtocolEngineNodeExtensionUuid);
    uuidvec->push_back(uuid3);
    PVUuid uuid4(PVMF_DOWNLOAD_PROGRESS_INTERFACE_UUID);
    uuidvec->push_back(uuid4);
    PVUuid uuid5(KPVMFProtocolEngineNodeMSHTTPStreamingExtensionUuid);
    uuidvec->push_back(uuid5);
    PVUuid uuid6(PVMF_TRACK_SELECTION_INTERFACE_UUID);
    uuidvec->push_back(uuid6);

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    return PVMFSuccess;
}


PVMFStatus PVMFProtocolEngineNode::DoQueryInterface(PVMFProtocolEngineNodeCommand&  aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoQueryInterface()"));

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFProtocolEngineNodeCommandBase::Parse(uuid, ptr);
    PVMFStatus status = PVMFSuccess;
    if (!queryInterface(*uuid, *ptr))
    {
        //Not supported
        *ptr = NULL;
        status = PVMFFailure;
    }

    CommandComplete(iInputCommands, aCmd, status);
    return status;
}


/**
//Called by the command handler AO to do the node Init
*/
PVMFStatus PVMFProtocolEngineNode::DoInit(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoInit()"));

    PVMFStatus status = PVMFSuccess;
    // exceptional cases
    if (!iProtocolContainer) status = PVMFFailure;
    if (iInterfaceState != EPVMFNodeIdle) status = PVMFErrInvalidState;
    PassInObjects();

    // normal case
    if (iInterfaceState == EPVMFNodeIdle && iProtocolContainer)
    {
        // do init, if init is async call, then return PVMFPending
        if ((status = iProtocolContainer->doInit()) == PVMFSuccess)
        {
            SetState(EPVMFNodeInitialized);
        }
    }
    return HandleCommandComplete(iInputCommands, aCmd, status);
}

void PVMFProtocolEngineNode::PassInObjects()
{
    iProtocolContainer->setSupportObject((OsclAny*)iPortInForData, NodeObjectType_InputPortForData);
    iProtocolContainer->setSupportObject((OsclAny*)iPortInForLogging, NodeObjectType_InputPortForLogging);
    iProtocolContainer->setSupportObject((OsclAny*)iPortOut, NodeObjectType_OutPort);
    iProtocolContainer->setSupportObject((OsclAny*)(&iInternalEventQueue), NodeObjectType_InternalEventQueue);
}


/**
//Called by the command handler AO to do the node Prepare
*/
PVMFStatus PVMFProtocolEngineNode::DoPrepare(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoPrepare()"));
    if (!iProtocolContainer)	return PVMFFailure;

    PVMFStatus status = PVMFSuccess; //PVMFPending;
    PassInObjects();
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
        {
            status = iProtocolContainer->doPrepare();
            if (status == PVMFSuccess) SetState(EPVMFNodePrepared);
            break;
        }
        default:
            status = PVMFErrInvalidState;
            break;
    }

    return HandleCommandComplete(iInputCommands, aCmd, status);
}


/**
//Called by the command handler AO to do the node Start
*/
PVMFStatus PVMFProtocolEngineNode::DoStart(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoStart()"));
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::DoStart() : Start command gets called and executed, iInterfaceState=%d, currSocketConnection=%d",
                     (int32)iInterfaceState, (uint32)iInterfacingObjectContainer->isSocketConnectionUp()));

    // check if download/streaming is done, if it is done, no need to start/resume download/streaming
    if (CheckAvailabilityOfDoStart(aCmd)) return PVMFSuccess;

    PVMFStatus status = PVMFSuccess;
    iProcessingState = ProcessingState_NormalDataflow;
    iInterfacingObjectContainer->setInputDataUnwanted(false);
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        {
            if (iProtocolContainer->doPreStart() == PROCESS_SUCCESS)
            {
                // do socket reconnect for init->start, prepare->start and stop->start
                // for pause->start, no need. For seek, no start command is issued. doseek() will
                // do socket reconnect
                iProtocolContainer->startDataFlowByCommand(iProtocolContainer->needSocketReconnect()); // make start command (prepare->start) asynchonous

                // Transition to BeingStarted
                // intentionally set node state as this node internal state to ignore any left-over messages from POST port from previous stop or EOS handling
                SetState((TPVMFNodeInterfaceState)PVMFProtocolEngineNodeState_BeingStarted);

                return PVMFPending;
            }
            status = PVMFSuccess;
            break;
        }

        case EPVMFNodePaused:
        {
            // Transition to BeingStarted
            // intentionally set node state as this node internal state to ignore any left-over messages from POST port because currently node is paused state.
            // need to differentiate this case from the paused case.
            SetState((TPVMFNodeInterfaceState)PVMFProtocolEngineNodeState_BeingStarted);

            iProtocol->resume();
            iProtocolContainer->startDataFlowByCommand();
            return PVMFPending;

            // break;	This statement was removed to avoid compiler warning for Unreachable Code
        }

        /*
         * If the node is already started just return success - multiple starts can happen with
         * flow control (auto-pause / auto-resume) scenarios
         */
        case EPVMFNodeStarted:
            status = PVMFSuccess;
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
    return status;
}

bool PVMFProtocolEngineNode::CheckAvailabilityOfDoStart(PVMFProtocolEngineNodeCommand& aCmd)
{
    // check if download/streaming is done, if it is done, no need to start/resume download/streaming
    if (iInterfacingObjectContainer->isDownloadStreamingDone())
    {
        SetState(EPVMFNodeStarted);
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        return true;
    }
    return false;
}

/**
//Called by the command handler AO to do the node Pause
*/
PVMFStatus PVMFProtocolEngineNode::DoPause(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoPause()"));
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::DoPause() : Pause command gets called and executed, iInterfaceState=%d", (int32)iInterfaceState));

    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        {
            TPVMFNodeInterfaceState prevState = iInterfaceState;
            SetState(EPVMFNodePaused);
            if (!iProtocolContainer->doPause())
            {
                SetState(prevState);
                status = PVMFFailure;
            }
        }
        break;

        case EPVMFNodePaused:
            status = PVMFSuccess;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
    return status;
}

/**
//Called by the command handler AO to do the node Seek
*/
PVMFStatus PVMFProtocolEngineNode::DoSeek(PVMFProtocolEngineNodeCommand& aCmd)
{
    return iProtocolContainer->doSeek(aCmd);
}


/**
//Called by the command handler AO to do the node BitstreamSwitch
*/
PVMFStatus PVMFProtocolEngineNode::DoBitsteamSwitch(PVMFProtocolEngineNodeCommand& aCmd)
{
    return iProtocolContainer->doBitstreamSwitch(aCmd);
}

PVMFStatus PVMFProtocolEngineNode::DoReposition(PVMFProtocolEngineNodeCommand& aCmd)
{
    return iProtocolContainer->doSeek(aCmd);
}

/**
//Called by the command handler AO to do the node Stop
*/
PVMFStatus PVMFProtocolEngineNode::DoStop(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoStop()"));
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::Stop() : Stop command gets called, iInterfaceState=%d", (int32)iInterfaceState));

    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:

            status = iProtocolContainer->doStop();
            if (status == PVMFPending) return PVMFPending;
            if (status == PVMFSuccess)
            {

                StopClear();
                // Transition to Prepared state
                SetState(EPVMFNodePrepared);
            }
            break;

        case EPVMFNodeError:
            SetState(EPVMFNodePrepared);
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
    return status;
}

/**
//Called by the command handler AO to do the node Flush
*/
PVMFStatus PVMFProtocolEngineNode::DoFlush(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoFlush()"));

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
        {
            //the flush is asynchronous. Move the command from
            //the input command queue to the current command, where
            //it will remain until the flush completes.
            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return PVMFErrNoMemory;
            }
            iInputCommands.Erase(&aCmd);

            //Notify all ports to suspend their input
            for (uint32 i = 0;i < iPortVector.size();i++)
            {
                iPortVector[i]->SuspendInput();
            }
        }

        break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            return PVMFErrInvalidState;
            // break;	This statement was removed to avoid compiler warning for Unreachable Code
    }
    return PVMFSuccess;
}

/**
//A routine to tell if a flush operation is in progress.
*/
bool PVMFProtocolEngineNode::FlushPending()
{
    return (iCurrentCommand.size() > 0 &&
            iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}


/**
//Called by the command handler AO to do the Cancel All
*/
PVMFStatus PVMFProtocolEngineNode::DoCancelAllCommands(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoCancelAllCommands()"));

    //first cancel the current command if any
    while (!iCurrentCommand.empty())
    {
        CancelClear();
        CommandComplete(iCurrentCommand, iCurrentCommand[0], PVMFErrCancelled);
    }

    //next cancel all queued commands
    //start at element 1 since this cancel command is element 0.
    while (iInputCommands.size() > 1)
    {
        CancelClear();
        CommandComplete(iInputCommands, iInputCommands[1], PVMFErrCancelled);
    }

    //finally, report cancel complete.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    if (iInterfacingObjectContainer) iInterfacingObjectContainer->setCancelCmdHappened();
	if (iInterfacingObjectContainer) iInterfacingObjectContainer->setInputDataUnwanted();
    return PVMFSuccess;
}

/**
//Called by the command handler AO to do the Cancel single command
*/
PVMFStatus PVMFProtocolEngineNode::DoCancelCommand(PVMFProtocolEngineNodeCommand& aCmd)
{
    LOGINFO((0, "PVMFProtocolEngineNode::DoCancelCommand()"));

    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFProtocolEngineNodeCommandBase::Parse(id);
    if (iInterfacingObjectContainer) iInterfacingObjectContainer->setInputDataUnwanted();

    // first check "current" command if any, and check the input queue starting
    // at element 1 since this cancel command is element 0.
    PVMFProtocolEngineNodeCommand* cmd = iCurrentCommand.FindById(id);
    if (cmd)
    {
        CancelClear();
        //cancel the queued command
        CommandComplete(iCurrentCommand, *cmd, PVMFErrCancelled);
        //report cancel success
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        return PVMFSuccess;
    }

    //next check input queue.
    //start at element 1 since this cancel command is element 0.
    cmd = iInputCommands.FindById(id, 1);
    if (cmd)
    {
        CancelClear();
        //cancel the queued command
        CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
        //report cancel success
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        return PVMFSuccess;
    }

    //at this point, nothing needs to be cancelled and thus report cancel complete
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    // set cancel cmd happened flag for stop command processing
    if (iInterfacingObjectContainer) iInterfacingObjectContainer->setCancelCmdHappened();
	if (iInterfacingObjectContainer) iInterfacingObjectContainer->setInputDataUnwanted();
    return PVMFSuccess;
}


/////////////////////////////////////////////////////
// Event reporting routines.
/////////////////////////////////////////////////////
void PVMFProtocolEngineNode::SetState(TPVMFNodeInterfaceState s)
{
    LOGINFO((0, "PVMFProtocolEngineNode::SetState() %d", s));
    PVMFNodeInterface::SetState(s);
}

void PVMFProtocolEngineNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData, const int32 aEventCode, OsclAny* aEventLocalBuffer, const uint32 aEventLocalBufferSize)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFProtocolEngineNode:NodeInfoEvent Type %d Data %d",
                    aEventType, aEventData));

    if (aEventCode == 0)
    {
        //report basic event, no extended event
        PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
    }
    else
    {
        //report extended event.
        PVUuid uuid = PVMFPROTOCOLENGINENODEInfoEventTypesUUID;
        PVMFBasicErrorInfoMessage *msg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aEventCode, uuid, NULL));

        PVMFAsyncEvent event(PVMFInfoEvent,
                             aEventType,
                             NULL, //context
                             msg,
                             aEventData,
                             (uint8*)aEventLocalBuffer,
                             aEventLocalBufferSize);

        //report to the session observers.
        PVMFNodeInterface::ReportInfoEvent(event);

        //remove the ref to the extended response
        if (msg) msg->removeRef();
    }
}

void PVMFProtocolEngineNode::ReportErrorEvent(PVMFEventType aEventType,
        OsclAny* aEventData,
        const int32 aEventCode,
        int32 aEventDataLen)
{
    LOGINFO((0, "PVMFProtocolEngineNode::ReportErrorEvent() Type %d Data %d"
             , aEventType, aEventData));

    PVMFBasicErrorInfoMessage*msg  = NULL;

    if (aEventCode != 0)
    {
        // extended error event with aEventCode
        PVUuid uuid = PVProtocolEngineNodeErrorEventTypesUUID;
        msg = OSCL_NEW(PVMFBasicErrorInfoMessage, (aEventCode, uuid, NULL));
    }

    PVMFAsyncEvent event(PVMFErrorEvent,
                         aEventType,
                         NULL, // context
                         msg,
                         aEventData,
                         NULL,
                         0);

    if (aEventDataLen != 0)
        event.SetEventDataLen(aEventDataLen);

    PVMFNodeInterface::ReportErrorEvent(event);
    if (msg) msg->removeRef();

}

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////

void PVMFProtocolEngineNode::QueuePortActivity(const PVMFPortActivity &aActivity)
{
    //queue a new port activity event
    int32 err;
    OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFProtocolEngineNode::QueuePortActivity() Error - iPortActivityQueue.push_back() failed"));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort), sizeof(PVMFProtocolEnginePort));
    }
    else
    {
        //wake up the AO to process the port activity event.
        RunIfNotReady();
    }
}


void PVMFProtocolEngineNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::HandlePortActivity: port=0x%x, type=%d, IncomingQueueSize=%d, OutgoingQueueSize=%d, PAQSize=%d",
                     aActivity.iPort, aActivity.iType, aActivity.iPort->IncomingMsgQueueSize(), aActivity.iPort->OutgoingMsgQueueSize(), iPortActivityQueue.size()));

    //A port is reporting some activity or state change.  This code
    //figures out whether we need to  queue a processing event
    //for the AO, and/or report a node event to the observer.

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CREATED:
            //Report port created info event
            ReportInfoEvent(PVMFInfoPortCreated, (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_DELETED:
            //Report port deleted info event
            ReportInfoEvent(PVMFInfoPortDeleted, (OsclAny*)aActivity.iPort);
            //Purge any port activity events already queued
            //for this port.
            {
                for (uint32 i = 0;i < iPortActivityQueue.size();)
                {
                    if (iPortActivityQueue[i].iPort == aActivity.iPort)
                    {
                        iPortActivityQueue.erase(&iPortActivityQueue[i]);
                    }
                    else
                    {
                        i++;
                    }
                }
            }
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            if ((PVMFProtocolEnginePort*)aActivity.iPort == iPortOut)
            {
                LOGINFODATAPATH((0, "PVMFProtocolEngineNode::HandlePortActivity: port=0x%x, type=%d(PVMF_PORT_ACTIVITY_CONNECT), PE node output port connected to downstream node port!",
                                 aActivity.iPort, aActivity.iType));
                iInterfacingObjectContainer->setOutputPortConnect();
            }

            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            //clear the node input queue when either port is disconnected.
            while (!iDataInQueue.empty())
            {
                PVMFSharedMediaMsgPtr msg = iDataInQueue.front();
                iDataInQueue.erase(iDataInQueue.begin());
                ((PVMFProtocolEnginePort*)aActivity.iPort)->LogMediaMsgInfo(msg, "In Msg Cleared", iDataInQueue.size());
            }

            if ((PVMFProtocolEnginePort*)aActivity.iPort == iPortOut)
            {
                LOGINFODATAPATH((0, "PVMFProtocolEngineNode::HandlePortActivity: port=0x%x, type=%d(PVMF_PORT_ACTIVITY_DISCONNECT), PE node output port disconnected from downstream node port!",
                                 aActivity.iPort, aActivity.iType));
                iInterfacingObjectContainer->setOutputPortConnect(false);
            }
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //An outgoing message was queued on this port.
            //We only need to queue a port activity event on the
            //first message.  Additional events will be queued during
            //the port processing as needed.
            if ((aActivity.iPort->OutgoingMsgQueueSize() == 1) ||
                    (iPortActivityQueue.size() == 0))
            {
                QueuePortActivity(aActivity);
            }
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            //An incoming message was queued on this port.
            //We only need to queue a port activity event on the
            //first message.  Additional events will be queued during
            //the port processing as needed.
            if ((aActivity.iPort->IncomingMsgQueueSize() == 1) ||
                    (iPortActivityQueue.size() == 0))
            {
                QueuePortActivity(aActivity);
            }
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
            //Outgoing queue is now busy.
            //No action is needed here-- the node checks for
            //outgoing queue busy as needed during data processing.
            iProcessingState = ProcessingState_Idle; // disable data flow
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            //Outgoing queue was previously busy, but is now ready.
            //it's time to start processing incoming data again.
            LOGINFODATAPATH((0, "PVMFProtocolEngineNode::HandlePortActivity() : PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY, OutgoingQueueSize=%d",
                             aActivity.iPort->OutgoingMsgQueueSize()));
            ProcessOutgoingQueueReady();
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            // The connected port has become busy (its incoming queue is
            // busy).
            // No action is needed here-- the port processing code
            // checks for connected port busy during data processing.
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            // The connected port has transitioned from Busy to Ready.
            // It's time to start processing outgoing messages again.
            LOGINFODATAPATH((0, "PVMFProtocolEngineNode::HandlePortActivity() : PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY, OutgoingQueueSize=%d",
                             aActivity.iPort->OutgoingMsgQueueSize()));

            if (aActivity.iPort->OutgoingMsgQueueSize() > 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "0x%x PVMFProtocolEngineNode::PortActivity: Connected port is now ready", this));
                PVMFPortActivity activity(aActivity.iPort, PVMF_PORT_ACTIVITY_OUTGOING_MSG);
                QueuePortActivity(activity);
            }
            break;

        default:
            break;
    }
}


/////////////////////////////////////////////////////
// Called by the AO to process a port activity message
bool PVMFProtocolEngineNode::ProcessPortActivity()
{
    //Pop the queue...
    PVMFPortActivity activity(iPortActivityQueue.front());
    iPortActivityQueue.erase(&iPortActivityQueue.front());

    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::ProcessPortActivity() port=0x%x, type=%d, IncomingQueueSize=%d, OutgoingQueueSize=%d, iInterfaceState=%d, PAQSize=%d",
                     activity.iPort, activity.iType, activity.iPort->IncomingMsgQueueSize(), activity.iPort->OutgoingMsgQueueSize(), iInterfaceState, iPortActivityQueue.size()));

    PVMFStatus status = PVMFSuccess;
    switch (activity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            status = ProcessOutgoingMsg(activity.iPort);
            //Re-queue the port activity event as long as there's
            //more data to process and it isn't in a Busy state.
            QueueActivityOutgoingMessage(status, activity);
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            status = ProcessIncomingMsg(activity.iPort);
            //Re-queue the port activity event as long as there's
            //more data to process and it isn't in a Busy state.
            QueueActivityIncomingMessage(status, activity);
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            LOGINFODATAPATH((0, "PVMFProtocolEngineNode::ProcessPortActivity() : PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY, OutgoingQueueSize=%d",
                             activity.iPort->OutgoingMsgQueueSize()));
            ProcessOutgoingQueueReady();
            break;

        default:
            break;
    }

    //Report any unexpected failure in port processing...
    //(the InvalidState error happens when port input is suspended,
    //so don't report it.)
    if (status != PVMFErrBusy &&
            status != PVMFSuccess &&
            status != PVMFErrInvalidState)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFProtocolEngineNode::ProcessPortActivity() Error - ProcessPortActivity failed. port=0x%x, type=%d",
                         activity.iPort, activity.iType));
        ReportErrorEvent(PVMFErrPortProcessing);
    }

    //return true if we processed an activity...
    return (status != PVMFErrBusy);
}

// wrap OSCL_TRY block
void PVMFProtocolEngineNode::QueueActivityOutgoingMessage(const PVMFStatus aStatus, const PVMFPortActivity &aActivity)
{
    if (aStatus != PVMFErrBusy && aActivity.iPort->OutgoingMsgQueueSize() > 0)
    {
        int32 err;
        OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
        if (err) ReportErrorEvent(PVMFErrPortProcessing);
    }
}

// wrap OSCL_TRY block
void PVMFProtocolEngineNode::QueueActivityIncomingMessage(const PVMFStatus aStatus, const PVMFPortActivity &aActivity)
{
    //Re-queue the port activity event as long as there's more data to process and it isn't in a Busy state.
    if (aStatus != PVMFErrBusy && aActivity.iPort->IncomingMsgQueueSize() > 0)
    {
        //OSCL_TRY(err,iPortActivityQueue.push_back(activity););
        // only insert outgoing queue ready event in case of no such event in the port activity queue
        if (!SearchPortActivityInQueue(PVMF_PORT_ACTIVITY_INCOMING_MSG))
        {
            int32 err;
            OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
            if (err) ReportErrorEvent(PVMFErrPortProcessing);
        }
    }
}

/////////////////////////////////////////////////////
PVMFStatus PVMFProtocolEngineNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFProtocolEngineNode::ProcessIncomingMsg() aPort=0x%x, IncomingMsgQueueSize=%d, iProcessingState=%d", 
					aPort, aPort->IncomingMsgQueueSize(), iProcessingState));
    
    // Called by the AO to process one buffer off the port's
    // incoming data queue.  This routine will dequeue and dispatch the data.
    // for pause, don't dequeue the message
    if (aPort->IncomingMsgQueueSize() == 0) return PVMFSuccess;

    if (aPort->GetPortTag() != PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT &&
            aPort->GetPortTag() != PVMF_PROTOCOLENGINENODE_PORT_TYPE_FEEDBACK)
    {
        return PVMFFailure;
    }

    if (iDataInQueue.size() < DEFAULT_DATA_QUEUE_CAPACITY*DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT / 100)
    {
        // DEFAULT_DATA_QUEUE_CAPACITY*DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT/100 = 7
        // only dequeue the message when data queue buffers less number of media message than the port queue

        // Get one incoming message
        PVMFSharedMediaMsgPtr msg;
        PVMFStatus status = aPort->DequeueIncomingMsg(msg);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFProtocolEngineNode::ProcessIncomingMsg() Error - DequeueIncomingMsg failed"));
            return status;
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFProtocolEngineNode::ProcessIncomingMsg() FmtId=%d", msg->getFormatID()));

        bool isEOS = (msg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID);
        if (aPort->GetPortTag() == PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT)
        {
            iInterfacingObjectContainer->updateSocketConnectFlags(isEOS); // will update flags
        }

        if (IgnoreCurrentInputData(aPort, isEOS, msg))
        {
            LOGINFODATAPATH((0, "PVMFProtocolEngineNode::ProcessIncomingMsg() INCOMING MESSAGE IGNORED!! isEOS=%d, port=0x%x, iInterfaceState=%d",
                             (uint32)isEOS, aPort, iInterfaceState));
            return PVMFSuccess;
        }

        uint32 frameIndex = ((PVMFProtocolEnginePort*)aPort)->iNumFramesConsumed++;
        msg->setSeqNum(frameIndex);
        iDataInQueue.push_back(msg);

        UpdateTimersInProcessIncomingMsg(isEOS, aPort);
        LogIncomingMessage(msg, isEOS, aPort);

    } // end of if(iDataInQueue.size() < 7)
    else
    {
        if (iInterfaceState != EPVMFNodePaused && aPort->GetPortTag() == PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT)
        {
            iNodeTimer->start(SERVER_INACTIVITY_TIMER_ID); // reset inactivity timer to prevent unnessary timeout
        }
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::ProcessIncomingMsg() iDataInQueue becomes FULL! iProcessingState = %d (1 for normal data flow), iInterfaceState = %d",
                         (uint32)iProcessingState, (uint32)iInterfaceState));
        if (iProcessingState != ProcessingState_NormalDataflow) return PVMFErrBusy;
    }

    if (iProcessingState == ProcessingState_NormalDataflow)
    {
        if (!IsDataFlowEventAlreadyInQueue())
        {
            PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_IncomingMessageReady);
            iInternalEventQueue.push_back(aEvent);
        }
    }

    RunIfNotReady();
    return PVMFSuccess;
}

void PVMFProtocolEngineNode::UpdateTimersInProcessIncomingMsg(const bool aEOSMsg, PVMFPortInterface* aPort)
{
    // for response timers
    if (!aEOSMsg)
    {
        // only non-EOS media data can cancel the response timer, EOS message shouldn't(that will cause no timeout happens)
        iNodeTimer->cancel(SERVER_RESPONSE_TIMER_ID);
        iNodeTimer->cancel(SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING);
    }

    // for inactivity timer
    if (!aEOSMsg && iInterfaceState != EPVMFNodePaused)
    {
        if (aPort->GetPortTag() == PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT) iNodeTimer->start(SERVER_INACTIVITY_TIMER_ID); // set up inactivity timer
    }
    else   // aEOSMsg=true or iInterfaceState = EPVMFNodePaused
    {
        iNodeTimer->cancel(SERVER_INACTIVITY_TIMER_ID);
    }
}

bool PVMFProtocolEngineNode::IgnoreCurrentInputData(PVMFPortInterface* aPort, const bool isEOS, PVMFSharedMediaMsgPtr &aMsg)
{
    OSCL_UNUSED_ARG(aMsg);
    if (iInterfacingObjectContainer->ignoreCurrentInputData())
    {
        if (aPort->GetPortTag() == PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT)
        {
            CheckEndOfProcessingInIgoreData(isEOS);
        }
        return true;
    }

    // for input port (HTTP GET)
    if (aPort->GetPortTag() == PVMF_PROTOCOLENGINENODE_PORT_TYPE_INPUT)
    {
        // processing done => ignore the rest data
        if (CheckEndOfProcessingInIgoreData(isEOS)) return true;

        // paused state, ignore EOS for socket disconnect
        if (/*isEOS &&*/ iInterfaceState == EPVMFNodePaused) return true;
    }

    // for feedback port (HTTP POST)
    if (aPort->GetPortTag() == PVMF_PROTOCOLENGINENODE_PORT_TYPE_FEEDBACK)
    {
        if (isEOS) return true;
        if (!isEOS &&
                (iInterfaceState == EPVMFNodeStarted ||
                 iInterfaceState == (TPVMFNodeInterfaceState)PVMFProtocolEngineNodeState_BeingStarted) &&
                !iInterfacingObjectContainer->isDownloadStreamingDone())
        {
            return true;
        }
    }

    return false;
}

bool PVMFProtocolEngineNode::CheckEndOfProcessingInIgoreData(const bool isEOS, const bool isDataPort)
{
    if (!iInterfacingObjectContainer->isDownloadStreamingDone()) return false;

    EndOfDataProcessingInfo *aInfo = iInterfacingObjectContainer->getEOPInfo();
    if (isEOS)
    {
        if (iProtocolContainer->needCheckEOSAfterDisconnectSocket())
        {
            aInfo->clear();
            aInfo->iSendServerDisconnectEvent = true;
        }
    }
    else
    {
        if (iProtocolContainer->needCheckExtraDataComeIn())
        {
            aInfo->clear();
            aInfo->iExtraDataComeIn = true;
        }
    }

    if (aInfo->isValid())
    {
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_EndOfProcessing, (OsclAny*)aInfo);
        iInternalEventQueue.push_back(aEvent);
        SetProcessingState(ProcessingState_NormalDataflow);
        RunIfNotReady();
        return true;
    }

    // Current situation: iInterfacingObjectContainer->isDownloadStreamingDone() = true
    if (isDataPort) return true;
    return false;
}

void PVMFProtocolEngineNode::LogIncomingMessage(PVMFSharedMediaMsgPtr &aMsg, bool isEOS, PVMFPortInterface* aPort)
{
    if (isEOS)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::LogIncomingMessage() GOT EOS! port=0x%x, iInterfaceState=%d",
                         aPort, iInterfaceState));
        OSCL_UNUSED_ARG(aPort); // to avoid warning of unused variable, 'aPort'
    }
    else
    {
        // Compute data size for logging purposes
        PVMFSharedMediaDataPtr mediaData;
        convertToPVMFMediaData(mediaData, aMsg);
        uint32 dataSize = 0;
        uint32 numFrags = mediaData->getNumFragments();
        for (uint32 i = 0; i < numFrags; i++)
        {
            OsclRefCounterMemFrag memFragIn;
            mediaData->getMediaFragment(i, memFragIn);
            uint32 fragLen = memFragIn.getMemFrag().len;
            dataSize += fragLen;
        }

        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::LogIncomingMessage()  SEQNUM=%d,SIZE=%d,port=0x%x,portIncomingQueueSize(AfterDequeue)=%d,iInterfaceState=%d",
                         aMsg->getSeqNum(), dataSize, aPort, aPort->IncomingMsgQueueSize(), iInterfaceState));
    }
}


/////////////////////////////////////////////////////
PVMFStatus PVMFProtocolEngineNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one message off the outgoing
    //message queue for the given port.  This routine will
    //try to send the data to the connected port.
    if (aPort->OutgoingMsgQueueSize() == 0) return PVMFSuccess;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFProtocolEngineNode::ProcessOutgoingMsg() aPort=0x%x, OutgoingMsgQueueSize=%d",
                    aPort, aPort->OutgoingMsgQueueSize()));

    // check whether outgoing message is media command or media data
    PVMFSharedMediaMsgPtr msg;
    if (!((PVMFProtocolEnginePort*)aPort)->PeekOutgoingMsg(msg)) return PVMFFailure;

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFProtocolEngineNode::ProcessOutgoingMsg() Connected port goes into busy state"));
    }
    if (status != PVMFSuccess) return status;

    return PostProcessForMsgSentSuccess(aPort, msg);
}

PVMFStatus PVMFProtocolEngineNode::PostProcessForMsgSentSuccess(PVMFPortInterface* aPort, PVMFSharedMediaMsgPtr &aMsg)
{
    PVMFStatus status = PVMFSuccess;

    // keep track the sequence no of the media msg just sent out successfully
    uint32 aCurrPacketNumSet = iInterfacingObjectContainer->getLatestPacketNumSent();
    if (aMsg->getSeqNum() > aCurrPacketNumSet) iInterfacingObjectContainer->setLatestPacketNumSent(aMsg->getSeqNum());

    // set up server response timer and cancel inactivity timer
    bool isMediaData = (aMsg->getFormatID() <  PVMF_MEDIA_CMD_FORMAT_IDS_START) ||
                       (aMsg->getFormatID() == PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID);
    UpdateTimersInProcessOutgoingMsg(isMediaData, aPort);

    // send() success means outgoing queue should always be ready, then send out outgoing queue ready event
    if ((PVMFProtocolEnginePort*)aPort == iPortOut) SendOutgoingQueueReadyEvent(aPort);

    // handle the remaining post processing for the different media cmds.
    if (aMsg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::PostProcessForMsgSentSuccess() Send() SUCCESS: EOS SENT! SEQNUM= %d, MsgID=%d, port=0x%x", aMsg->getSeqNum(), aMsg->getFormatID(), aPort));

        // complete the pending command if there is, especially for the case of sending EOS due to error
        ProtocolStateErrorInfo aInfo(0, false);
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_ProtocolStateError, (OsclAny*)(&aInfo));
        DispatchInternalEvent(&aEvent);
    }
    else if (aMsg->getFormatID() == PVMF_MEDIA_CMD_RE_CONFIG_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::PostProcessForMsgSentSuccess() Send() SUCCESS: RE_CONFIG SENT! SEQNUM= %d, MsgID=%d, port=0x%x", aMsg->getSeqNum(), aMsg->getFormatID(), aPort));

        uint32 aFirstPacketNum = 0xFFFFFFFF;
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_FirstPacketAvailable, (OsclAny*)aFirstPacketNum);
        DispatchInternalEvent(&aEvent);
    }
    else if (aMsg->getFormatID() == PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::PostProcessForMsgSentSuccess() Send() SUCCESS: SOCKET CONNECT SENT! SEQNUM= %d, MsgID=%d, port=0x%x", aMsg->getSeqNum(), aMsg->getFormatID(), aPort));
        bool status = iProtocolContainer->completeRepositionRequest();
        if (status) LOGINFODATAPATH((0, "PVMFProtocolEngineNode::PostProcessForMsgSentSuccess() Send() SOCKET CONNECT cmd SUCCESS: complete data stream reposition request"));
    }
    else
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::PostProcessForMsgSentSuccess() Send() SUCCESS: SEQNUM= %d, MsgID=%d, port=0x%x", aMsg->getSeqNum(), aMsg->getFormatID(), aPort));
    }

    return status;
}


void PVMFProtocolEngineNode::UpdateTimersInProcessOutgoingMsg(const bool isMediaData, PVMFPortInterface* aPort)
{
    // set up server response timer
    if (((PVMFProtocolEnginePort*)aPort == iPortInForData ||
            (PVMFProtocolEnginePort*)aPort == iPortInForLogging) &&
            isMediaData)
    {
        iNodeTimer->cancel(SERVER_INACTIVITY_TIMER_ID);

        uint32 timerID = SERVER_RESPONSE_TIMER_ID;
        if (iInterfacingObjectContainer->isDownloadStreamingDone() && (PVMFProtocolEnginePort*)aPort == iPortInForLogging)
        {
            // logging POST for EOS and stop
            timerID = SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING;
        }
        iNodeTimer->start(timerID);
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::UpdateTimersInProcessOutgoingMsg() server response timer starts! timerID=%d, timeoutValue=%d", timerID, iNodeTimer->getTimeout(timerID)));
    }
}

void PVMFProtocolEngineNode::SendOutgoingQueueReadyEvent(PVMFPortInterface* aPort)
{
    if (aPort->OutgoingMsgQueueSize() == 0) return;

    // only insert outgoing queue ready event in case of no such event in the port activity queue
    if (!SearchPortActivityInQueue(PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY))
    {
        PVMFPortActivity activity(aPort, PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY);
        QueuePortActivity(activity);
    }
}

bool PVMFProtocolEngineNode::SearchPortActivityInQueue(const PVMFPortActivityType aType)
{
    bool bFound = false;
    for (uint32 i = 0; i < iPortActivityQueue.size(); i++)
    {
        if (iPortActivityQueue[i].iType == aType)
        {
            bFound = true;
            break;
        }
    }
    return bFound;
}

void PVMFProtocolEngineNode::ProcessOutgoingQueueReady()
{
    if (iPortInForData)
    {
        if (iPortInForData->IncomingMsgQueueSize() > 0)
        {
            ProcessIncomingMsg(iPortInForData);
        }
    }

    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_StartDataflowByPortOutgoingQueueReady);
    iInternalEventQueue.push_back(aEvent);
    iProcessingState = ProcessingState_NormalDataflow; // enable data flow
    if (IsAdded())
        RunIfNotReady();
}

/**
/////////////////////////////////////////////////////
// Active object implementation
/////////////////////////////////////////////////////
*/

/**
// The AO will either process one command or service one connected
// port per call.  It will re-schedule itself and run continuously
// until it runs out of things to do.
*/
void PVMFProtocolEngineNode::Run()
{
    //Process commands.
    if (!iInputCommands.empty() && ProcessCommand(iInputCommands.front()))
    {
        //note: need to check the state before re-scheduling
        //since the node could have been reset in the ProcessCommand call.
        if (iInterfaceState != EPVMFNodeCreated) RunIfNotReady();
        return;
    }

    // Process data, higher priority than port processing
    // Do this last after handling all node command and port activity but before checking if flush is complete
    if (HandleProcessingState())
    {
        RunIfNotReady();
        return;
    }

    // Process port activity
    if (HandleRunPortActivityProcessing())
    {
        //Re-schedule
        //if(iProcessingState == ProcessingState_NormalDataflow) RunIfNotReady();
        RunIfNotReady();
        return;
    }


    // If we get here we did not process any node commands, port activity, or decode.
    // Check for completion of a flush command...
    HandleRunFlush();

    // final check if there is the input command, but no pending command
    if (!iInputCommands.empty() && iCurrentCommand.empty()) RunIfNotReady();
}

bool PVMFProtocolEngineNode::HandleRunPortActivityProcessing()
{
    if (!iPortActivityQueue.empty() && !FlushPending()) ///*&& (iInterfaceState==EPVMFNodeStarted || FlushPending())*/)
    {
        // If the port activity cannot be processed because a port is
        // busy, discard the activity and continue to process the next
        // activity in queue until getting to one that can be processed.
        while (!iPortActivityQueue.empty())
        {
            if (ProcessPortActivity()) break; //processed a port
        }
        return true;
    }
    return false;
}

void PVMFProtocolEngineNode::HandleRunFlush()
{
    if (FlushPending() && iPortActivityQueue.empty())
    {
        uint32 i;
        //Debug check-- all the port queues should be empty at
        //this point.
        for (i = 0;i < iPortVector.size();i++)
        {
            if (iPortVector[i]->IncomingMsgQueueSize() > 0
                    || iPortVector[i]->OutgoingMsgQueueSize() > 0)
            {
                OSCL_ASSERT(false);
            }
        }

        //Flush is complete.  Go to prepared state.
        SetState(EPVMFNodePrepared);
        //resume port input so the ports can be re-started.
        for (i = 0;i < iPortVector.size();i++)
        {
            iPortVector[i]->ResumeInput();
        }
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        RunIfNotReady();
    }
}

void PVMFProtocolEngineNode::SetProcessingState(PVProtocolEngineNodePrcoessingState aState)
{
    iProcessingState = aState;
    RunIfNotReady();
}


// Using state to handle different scenarios
bool PVMFProtocolEngineNode::HandleProcessingState()
{
    if (iProcessingState == ProcessingState_NormalDataflow)
    {
        return DispatchInternalEvent();
    }
    return false;
}

// The following method is (internal) event dispatcher, which is an important component for event-driven model
// to handle the interactions inside the node, between protocol engine and the node. All events are defined as
// PVProtocolEngineNodeInternalEventType in pvmf_protocol_engine_node_internal.h.
// Typical events are,

// ** init/prepar/start/seek/bitstreamSwitch command trigger the data flow (class NormalDataFlowHandler),
// ** multple events based on the callbacks from protocol engine
//		-- http header available (class HttpHeaderAvailableHandler) to complete start command
//		-- first data packet available (class FirstPacketAvailableHandler) to complete seek/bitstreamSwitch command
//		-- normal data packet available (class NormalDataAvailableHandler) to pass down to node output object, and then output data packets
//		-- protocol state complete (class ProtocolStateCompleteHandler) to complete init/prepare command
//		-- protocol state error (class ProtocolStateErrorHandler) to handle all the errors from protocol engine

// ** timer out event (will go to ProtocolStateErrorHandler) to complete any pending commands or report error event
// ** end of data processing, especially for after download complete, parser node still send resume request, (class EndOfDataProcessingHandler)
// ** by-pass server response for HEAD request in progressive download (class ServerResponseErrorBypassingHandler)

// The design is OO-based, i.e. each event handler is encapsulated into an object following the simple generic interface (virtual bool handle()=0), a
// typical polymophimic treatment.The dispatching logic is table-based, i.e the mapping between most events and their corresponding handlers is
// achieved by table look-up.

// aLatestEvent!=NULL means the event source sends the event without pushing it into the event queue
// so no need to dequeue the event from the event queue, and just process it. This way aims to save
// queueing and dequeuing operations on the queue.
bool PVMFProtocolEngineNode::DispatchInternalEvent(PVProtocolEngineNodeInternalEvent *aLatestEvent)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::DispatchInternalEvent() IN, iInternalEventQueue.size=%d, aLatestEvent=%x",
                     iInternalEventQueue.size(), (uint32)aLatestEvent));

    if (iInternalEventQueue.empty() && !aLatestEvent) return false;

    PVProtocolEngineNodeInternalEvent aEvent = ((aLatestEvent != NULL) ? (*aLatestEvent) : (PVProtocolEngineNodeInternalEvent) iInternalEventQueue[0]);
    if (!aLatestEvent) iInternalEventQueue.erase(&(iInternalEventQueue.front()));

    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::DispatchInternalEvent() get the event from iInternalEventQueue, iInternalEventQueue.size=%d, aEvent.iEventId=%d, totalEventNum=%d",
                     iInternalEventQueue.size(), (uint32)aEvent.iEventId, (uint32)EVENT_HANDLER_TOTAL));

    if ((uint32)aEvent.iEventId >= (uint32)PVProtocolEngineNodeInternalEventType_IncomingMessageReady)
        iCurrEventHandler = iEventHandlers[EVENT_HANDLER_TOTAL-1];
    else
        iCurrEventHandler = iEventHandlers[(uint32)aEvent.iEventId];

    iCurrEventHandler->handle(aEvent);
    if (!iInternalEventQueue.empty()) return true; // true means this function will re-run again
    return false; // let specific event handler determine this function needs to be re-run again
}

inline bool PVMFProtocolEngineNode::IsDataFlowEventAlreadyInQueue()
{
    if (iInternalEventQueue.empty()) return false;

    for (uint32 i = 0; i < iInternalEventQueue.size(); i++)
    {
        if ((uint32)iInternalEventQueue[i].iEventId >= (uint32)PVProtocolEngineNodeInternalEventType_IncomingMessageReady) return true;
    }
    return false;
}

void PVMFProtocolEngineNode::ProtocolStateComplete(const ProtocolStateCompleteInfo &aInfo)
{
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_ProtocolStateComplete);
    iInterfacingObjectContainer->setProtocolStateCompleteInfo(aInfo);
    //aEvent.iEventInfo = (OsclAny*)iInterfacingObjectContainer->getProtocolStateCompleteInfo();
    DispatchInternalEvent(&aEvent);
}

void PVMFProtocolEngineNode::OutputDataAvailable(OUTPUT_DATA_QUEUE &aOutputQueue, ProtocolEngineOutputDataSideInfo& aSideInfo)
{
    OUTPUT_DATA_QUEUE *pOutput = &aOutputQueue;
    PVProtocolEngineNodeInternalEvent aEvent(aSideInfo, (OsclAny*)pOutput);
    DispatchInternalEvent(&aEvent);
}

void PVMFProtocolEngineNode::ProtocolStateError(int32 aErrorCode)
{
    if (iProtocol->isCurrentStateOptional())
    {
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_ServerResponseError_Bypassing);
        DispatchInternalEvent(&aEvent);
    }
    else
    {
        ProtocolStateErrorInfo aInfo(aErrorCode);
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_ProtocolStateError, (OsclAny*)(&aInfo));
        DispatchInternalEvent(&aEvent);
    }
}

bool PVMFProtocolEngineNode::GetBufferForRequest(PVMFSharedMediaDataPtr &aMediaData)
{
    return iNodeOutput->getBuffer(aMediaData);
}

void PVMFProtocolEngineNode::ProtocolRequestAvailable(uint32 aRequestType)
{
    uint32 aOutputType;
    if (aRequestType == ProtocolRequestType_Logging)
    {
        aOutputType = NodeOutputType_InputPortForLogging;
    }
    else
    {
        aOutputType = NodeOutputType_InputPortForData;
    }

    iNodeOutput->flushData(aOutputType);
}


void PVMFProtocolEngineNode::OutputBufferPoolFull()
{
    iProcessingState = ProcessingState_Idle; // hold off any data flow until the output buffer callback
}

void PVMFProtocolEngineNode::OutputBufferAvailable()
{
    if (iPortOut && !iPortOut->IsOutgoingQueueBusy())
    {
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_StartDataflowByBufferAvailability);
        iInternalEventQueue.push_back(aEvent);
        SetProcessingState(ProcessingState_NormalDataflow);
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::OutputBufferAvailable() - MemCallBackReturn"));
    }

    if (iPortInForData)
    {
        if (iPortInForData->IncomingMsgQueueSize() > 0)
        {
            PVMFPortActivity activity(iPortInForData, PVMF_PORT_ACTIVITY_INCOMING_MSG);
            QueuePortActivity(activity);
        }
    }
}

void PVMFProtocolEngineNode::ReadyToUpdateDownloadControl()
{
    iProtocolContainer->doInfoUpdate(PROCESS_SUCCESS);
}

bool PVMFProtocolEngineNode::QueueOutgoingMsgSentComplete(PVMFProtocolEnginePort *aPort, PVMFSharedMediaMsgPtr &aMsg, const PVMFStatus aStatus)
{
    if (aStatus == PVMFErrBusy) return false;
    if (aStatus == PVMFSuccess) return true;

    // for other status, PVMFSuccessOutgoingMsgSent, and other failures, and just complete the command process.
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::QueueOutgoingMsgSentComplete() - Msg queued and sent successfully!, Port=0x%x, status=%d", aPort, aStatus));
    if (aStatus == PVMFSuccessOutgoingMsgSent) iInterfacingObjectContainer->setLatestPacketNumSent(aMsg->getSeqNum());
    RerunForPostProcessAfterOutgoingMsgSent(aPort, aMsg);
    return (aStatus == PVMFSuccessOutgoingMsgSent);
}

void PVMFProtocolEngineNode::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timeoutInfo);
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::TimeoutOccurred()  timerID = %d, (0-server response, 1-inactivity, 2-KeepAlive, 3-Logging response, 4-Wall clock, 5-Buffer status timer), iInterfaceState=%d",
                     timerID, (int32)iInterfaceState));

    iProtocolContainer->handleTimeout(timerID);
}


// create iProtocolContainer, iProtocol and iNodeOutput and all other protocol related projects
bool PVMFProtocolEngineNode::CreateProtocolObjects(OsclAny* &aProtocolInfo)
{
    AutoCleanup cleanup(this); // cleanup's destructor will automatically call DeleteProtocolObjects() if failure happens

    // use PE node registry to create protocol container
    DeleteProtocolObjects();
    iProtocolContainer = iRegistry.CreateProtocolEngineContainer(aProtocolInfo, this);
    if (!iProtocolContainer) return false;
    iProtocolContainer->setObserver(this);

    // use protocol container to create all other protocol objects
    if (!iProtocolContainer->createProtocolObjects()) return false;

    // create the rest projects for node use itself
    if (!CreateRestObjects()) return false;

    cleanup.cancel();
    return true;
}


bool PVMFProtocolEngineNode::RecheckProtocolObjects(OsclAny* aSourceData, OsclAny* aPluginInfo)
{
    uint32 isProgressiveStreaming = (uint32)iProtocolContainer->isStreamingPlayback();
    if (isProgressiveStreaming == (uint32)aPluginInfo) return true;

    // in case of progressive streaming
    DeleteProtocolObjects();
    OsclAny* aNewPluginInfo = (OsclAny*)isProgressiveStreaming;
    if (!CreateProtocolObjects(aNewPluginInfo)) return false;

    // add source data again
    return iProtocolContainer->addSourceData(aSourceData);
}

bool PVMFProtocolEngineNode::CreateRestObjects()
{
    // create iPortConfigFSInfoAlloc for redirect port config
    iPortConfigFSInfoAlloc = OSCL_NEW(OsclRefCounterMemFragAlloc, (&iPortConfigMemPool));
    if (!iPortConfigFSInfoAlloc) return false;

    // create event handlers
    return CreateEventHandlers();
}


bool PVMFProtocolEngineNode::CreateEventHandlers()
{
    iEventHandlers[0] = OSCL_NEW(HttpHeaderAvailableHandler, (this));
    if (!iEventHandlers[0]) return false;
    iEventHandlers[1] = OSCL_NEW(FirstPacketAvailableHandler, (this));
    if (!iEventHandlers[1]) return false;
    iEventHandlers[2] = OSCL_NEW(NormalDataAvailableHandler, (this));
    if (!iEventHandlers[2]) return false;
    iEventHandlers[3] = OSCL_NEW(ProtocolStateCompleteHandler, (this));
    if (!iEventHandlers[3]) return false;
    iEventHandlers[4] = OSCL_NEW(EndOfDataProcessingHandler, (this));
    if (!iEventHandlers[4]) return false;
    iEventHandlers[5] = OSCL_NEW(ServerResponseErrorBypassingHandler, (this));
    if (!iEventHandlers[5]) return false;
    iEventHandlers[6] = OSCL_NEW(ProtocolStateErrorHandler, (this));
    if (!iEventHandlers[6]) return false;
    iEventHandlers[7] = OSCL_NEW(CheckResumeNotificationHandler, (this));
    if (!iEventHandlers[7]) return false;
    iEventHandlers[8] = OSCL_NEW(OutgoingMsgSentSuccessHandler, (this));
    if (!iEventHandlers[8]) return false;

    iEventHandlers[EVENT_HANDLER_TOTAL-1] = OSCL_NEW(NormalDataFlowHandler, (this));
    if (!iEventHandlers[EVENT_HANDLER_TOTAL-1]) return false;
    return true;
}

void PVMFProtocolEngineNode::DeleteProtocolObjects()
{
    if (iProtocolContainer)
    {
        iProtocolContainer->deleteProtocolObjects();
        //delete iProtocolContainer
        iRegistry.ReleaseProtocolEngineContainer(iProtocolContainer);
    }
    iProtocolContainer = NULL;
    DeleteRestObjects();
}


void PVMFProtocolEngineNode::DeleteRestObjects()
{
    if (iPortConfigFSInfoAlloc)		OSCL_DELETE(iPortConfigFSInfoAlloc);
    iPortConfigFSInfoAlloc = NULL;

    for (uint32 i = 0; i < EVENT_HANDLER_TOTAL; i++)
    {
        if (iEventHandlers[i]) OSCL_DELETE(iEventHandlers[i]);
        iEventHandlers[i] = NULL;
    }

    iInterfacingObjectContainer = NULL;
    iNodeTimer					= NULL;
    iProtocol					= NULL;
    iNodeOutput					= NULL;
    iDownloadControl			= NULL;
    iDownloadProgess			= NULL;
    iUserAgentField				= NULL;
    iEventReport				= NULL;
}

void PVMFProtocolEngineNode::ClearPorts(const bool aNeedDelete)
{
    // clear queued messages in ports
    uint32 i;
    for (i = 0; i < iPortVector.size(); i++) iPortVector[i]->ClearMsgQueues();

    // Discard any port activity events
    iPortActivityQueue.clear();

    if (aNeedDelete)
    {
        while (!iPortVector.empty())
        {
            PVMFProtocolEnginePort* port = iPortVector.front();
            iPortVector.Erase(&iPortVector.front());

            if (port == iPortInForData)	iPortInForData = NULL;
            if (port == iPortInForLogging) iPortInForLogging = NULL;
            if (port == iPortOut) iPortOut = NULL;
        }
        // Restore original port vector reserve.
        iPortVector.Reconstruct();
    }
}

void PVMFProtocolEngineNode::Clear(const bool aNeedDelete)
{
    if (iProtocolContainer) iProtocolContainer->doClear(aNeedDelete);
    if (aNeedDelete) DeleteProtocolObjects();

}

void PVMFProtocolEngineNode::ResetClear(const bool aNeedDelete)
{
    if (iProtocolContainer) iProtocolContainer->doClear(aNeedDelete);
    if (iProtocolContainer) iProtocolContainer->deleteProtocolObjects();
    DeleteRestObjects();
    // leave iProtocolContainer undeleted

}

void PVMFProtocolEngineNode::StopClear()
{
    if (iProtocolContainer) iProtocolContainer->doStopClear();
}

void PVMFProtocolEngineNode::CancelClear()
{
    if (iProtocolContainer) iProtocolContainer->doCancelClear();
}

////////// PVMFDataSourceInitializationExtensionInterface implementation ////////////////////////
PVMFStatus PVMFProtocolEngineNode::SetSourceInitializationData(OSCL_wString& aSourceURL,
        PVMFFormatType& aSourceFormat,
        OsclAny* aSourceData)
{
    // check supported protocol plugin
    if (!iRegistry.CheckPluginAvailability(aSourceFormat, aSourceData)) return PVMFErrNotSupported;

    // create protocol objects
    OsclAny *aPluginInfo = NULL;
    if (!CreateProtocolObjects(aPluginInfo)) return PVMFErrNoMemory;

    // check and add source data
    if (!iProtocolContainer->addSourceData(aSourceData)) return PVMFFailure;

    // need to recreate protocol objects for progressive streaming based on source data
    if (!RecheckProtocolObjects(aSourceData, aPluginInfo)) return PVMFErrNoMemory;

    // get references of all the necessary objects from iProtocolContainer
    GetObjects();

    // set download format
    if (iInterfacingObjectContainer) iInterfacingObjectContainer->setDownloadFormat(aSourceFormat);

    // set URI
    if (!iInterfacingObjectContainer->getURIObject().setURI(aSourceURL)) return PVMFFailure;


    // create and set iCfgFile
    if (!iProtocolContainer->createCfgFile(iInterfacingObjectContainer->getURIObject().getURI())) return PVMFFailure;

    return PVMFSuccess;
}


PVMFStatus PVMFProtocolEngineNode::SetClientPlayBackClock(PVMFMediaClock* aClientClock)
{
    if (iDownloadControl)
    {
        iDownloadControl->setSupportObject((OsclAny *)aClientClock, DownloadControlSupportObjectType_EnginePlaybackClock);
        return PVMFSuccess;
    }
    return PVMFFailure;
}


// From PVMIDatastreamuserInterface
void PVMFProtocolEngineNode::PassDatastreamFactory(PVMFDataStreamFactory& aFactory, int32 aFactoryTag, const PvmfMimeString* aFactoryConfig)
{
    OSCL_UNUSED_ARG(aFactoryTag);
    OSCL_UNUSED_ARG(aFactoryConfig);
    iInterfacingObjectContainer->setDataStreamFactory(&aFactory);
}


// From PVMFProtocolEngineNodeExtensionInterface
PVMFStatus PVMFProtocolEngineNode::GetHTTPHeader(uint8*& aHeader, uint32& aHeaderLen)
{
    iInterfacingObjectContainer->getHTTPHeader(aHeader, aHeaderLen);
    return PVMFSuccess;
}

bool PVMFProtocolEngineNode::GetSocketConfig(OSCL_String &aPortConfig)
{
    if (CheckUsingProxy(aPortConfig)) return true;
    return GetSocketConfigImp(iInterfacingObjectContainer->getURIObject(), aPortConfig);
}

bool PVMFProtocolEngineNode::GetSocketConfigForLogging(OSCL_String &aPortConfig)
{
    if (CheckUsingProxy(aPortConfig)) return true;
    return GetSocketConfigImp(iInterfacingObjectContainer->getLoggingURIObject(), aPortConfig);
}

bool PVMFProtocolEngineNode::GetSocketConfigImp(const INetURI &aURI, OSCL_String &aPortConfig)
{
    INetURI *pURI = &((INetURI &)aURI);
    if (pURI->empty()) return false;

    OSCL_HeapString<OsclMemAllocator> serverAddr;
    int32 serverPort = 0;
    if (!pURI->getHostAndPort(serverAddr, serverPort)) return false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, \
                    (0, "PVMFProtocolEngineNode::GetSocketConfigImp(), serverAddr=%s , serverPort=%d", \
                     serverAddr.get_cstr(), serverPort));

    return ComposeSocketConfig(serverAddr, serverPort, aPortConfig);
}

bool PVMFProtocolEngineNode::ComposeSocketConfig(OSCL_String &aServerAddr, const uint32 aPortNum, OSCL_String &aPortConfig)
{
    // compose port config string: "TCP/remote_address=pvs.pv.com;remote_port=554"
    uint32 tempBufSize = aServerAddr.get_size() + 64;
    OsclMemAllocator alloc;
    char *buffer = (char*)alloc.allocate(tempBufSize);
    if (!buffer) return false;
    oscl_snprintf(buffer, tempBufSize, "TCP/remote_address=");
    oscl_strcat(buffer, aServerAddr.get_cstr());
    OSCL_FastString port(_STRLIT_CHAR(";remote_port="));
    oscl_strcat(buffer, port.get_cstr());
    char portString[16];
    oscl_snprintf(portString, 16, "%d", aPortNum);
    oscl_strcat(buffer, (char*)portString);

    aPortConfig = OSCL_HeapString<OsclMemAllocator> (buffer, oscl_strlen(buffer));
    alloc.deallocate(buffer);
    return true;
}


bool PVMFProtocolEngineNode::CheckUsingProxy(OSCL_String &aPortConfig)
{
    uint32 aProxyPort = 0;
    OSCL_HeapString<OsclMemAllocator> aProxyName;
    if (!getProxy(aProxyName, aProxyPort)) return false;
    iInterfacingObjectContainer->getURIObject().setUsAbsoluteURI();
    iInterfacingObjectContainer->getLoggingURIObject().setUsAbsoluteURI();
    return ComposeSocketConfig(aProxyName, aProxyPort, aPortConfig);
}

bool PVMFProtocolEngineNode::getProxy(OSCL_String& aProxyName, uint32 &aProxyPort)
{
    if (!iProtocolContainer) return false;
    return iProtocolContainer->getProxy(aProxyName, aProxyPort);
}


bool PVMFProtocolEngineNode::SetUserAgent(OSCL_wString &aUserAgent, const bool isOverwritable)
{
    // assume setsourceinit API already gets called
    if (!iUserAgentField) return false;
    return iUserAgentField->setUserAgent(aUserAgent, isOverwritable);
}

void PVMFProtocolEngineNode::SetHttpVersion(const uint32 aHttpVersion)
{
    if (iProtocolContainer) iProtocolContainer->setHttpVersion(aHttpVersion);
}

void PVMFProtocolEngineNode::SetHttpExtensionHeaderField(OSCL_String &aFieldKey, OSCL_String &aFieldValue, const HttpMethod aMethod, const bool aPurgeOnRedirect)
{
    if (iProtocolContainer) iProtocolContainer->setHttpExtensionHeaderField(aFieldKey, aFieldValue, aMethod, aPurgeOnRedirect);
}

void PVMFProtocolEngineNode::SetLoggingURL(OSCL_wString& aSourceURL)
{
    iInterfacingObjectContainer->setLoggingURI(aSourceURL);
}

void PVMFProtocolEngineNode::SetNetworkTimeout(const uint32 aTimeout)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::SetNetworkTimeout(), responseTimeout=%d", aTimeout));
    uint32 timeout = aTimeout;
    if ((int32)timeout < 0) timeout = 0x7fffffff;
    if (iNodeTimer) iNodeTimer->set(SERVER_RESPONSE_TIMER_ID, timeout);
}

void PVMFProtocolEngineNode::SetNetworkLoggingTimeout(const uint32 aTimeout)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::SetNetworkLoggingTimeout(), loggingTimeout=%d", aTimeout));
    uint32 timeout = aTimeout;
    if ((int32)timeout < 0) timeout = 0x7fffffff;
    if (iNodeTimer) iNodeTimer->set(SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING, timeout);
}

bool PVMFProtocolEngineNode::IsWMServerVersion4()
{
    if (iProtocol)
    {
        return (iProtocol->getServerVersionNum() < DEFAULT_MS_HTTP_STREAMING_SERVER_VERSION);
    }
    return false;
}


void PVMFProtocolEngineNode::setFormatDownloadSupportInterface(PVMFFormatProgDownloadSupportInterface* download_support_interface)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::setFormatDownloadSupportInterface() IN, download_support_interface=0x%x", download_support_interface));

    if (iDownloadControl && iDownloadProgess)
    {
        iDownloadControl->setSupportObject((OsclAny *)download_support_interface, DownloadControlSupportObjectType_SupportInterface);
        iDownloadProgess->setSupportObject((OsclAny *)download_support_interface, DownloadControlSupportObjectType_SupportInterface);

        PVMFDownloadProgressInterface *aProgDownload = OSCL_STATIC_CAST(PVMFDownloadProgressInterface*, this);
        iDownloadControl->setSupportObject((OsclAny *)aProgDownload, DownloadControlSupportObjectType_ProgressInterface);
    }
}

void PVMFProtocolEngineNode::setClipDuration(const uint32 aClipDurationMsec)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::setClipDuration(), aClipDurationMsec = %dms", aClipDurationMsec));

    if (iDownloadControl) iDownloadControl->setClipDuration(aClipDurationMsec);
    if (iDownloadProgess) iDownloadProgess->setClipDuration(aClipDurationMsec);
}

OsclSharedPtr<PVMFMediaClock> PVMFProtocolEngineNode::getDownloadProgressClock()
{
    OsclSharedPtr<PVMFMediaClock> clock;
    iDownloadControl->getDownloadClock(clock);
    return clock;
}

void PVMFProtocolEngineNode::requestResumeNotification(const uint32 currentNPTReadPosition, bool& aDownloadComplete)
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::requestResumeNotification() IN, currentNPTReadPosition=%d", currentNPTReadPosition));

    bool needSendUnderflowEvent = false;
    iDownloadControl->requestResumeNotification(currentNPTReadPosition, aDownloadComplete, needSendUnderflowEvent);
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::requestResumeNotification(), after iDownloadControl->requestResumeNotification(), currentNPTReadPosition=%d, needSendUnderflowEvent=%d, aDownloadComplete=%d",
                     currentNPTReadPosition, (uint32)needSendUnderflowEvent, (uint32)aDownloadComplete));

    // report underflow event for download incomplete
    // but only send it once, for multiple back to back requestResumeNotification
    if (!aDownloadComplete && needSendUnderflowEvent) ReportInfoEvent(PVMFInfoUnderflow);

    if (aDownloadComplete)  // end of processing
    {
        EndOfDataProcessingInfo *aInfo = iInterfacingObjectContainer->getEOPInfo();
        aInfo->clear();
        aInfo->iSendResumeNotification = true;
        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_EndOfProcessing, (OsclAny*)aInfo);
        iInternalEventQueue.push_back(aEvent);
        SetProcessingState(ProcessingState_NormalDataflow);
        RunIfNotReady();
    }

    // check the need of sending resume notification manually
    iProtocolContainer->checkSendResumeNotification();
}

void PVMFProtocolEngineNode::cancelResumeNotification()
{
    LOGINFODATAPATH((0, "PVMFProtocolEngineNode::cancelResumeNotification"));

    if (iDownloadControl != NULL)
    {
        iDownloadControl->cancelResumeNotification();
    }
}

bool PVMFProtocolEngineNode::SendPortMediaCommand(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId, const bool isForLogging)
{
    // Create an output media command
    PVMFSharedMediaCmdPtr aCmdPtr = PVMFMediaCmd::createMediaCmd();

    // Set the input format ID
    aCmdPtr->setFormatID(aCmdId);

    // check format specific info
    if (!CheckFormatSpecificInfoForMediaCommand(aCmdPtr, aCmdId, isForLogging)) return false;

    // Convert to media message and send it out
    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, aCmdPtr);
    PVMFStatus status = aPort->QueueOutgoingMsg(mediaMsgOut);
    if (status < PVMFSuccess) return false;
    if (status == PVMFSuccessOutgoingMsgSent)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::SendPortMediaCommand() - Msg queued and sent successfully!, Port=0x%x", aPort));
        RerunForPostProcessAfterOutgoingMsgSent(aPort, mediaMsgOut);
    }

    LogPortMediaCmdQueued(aPort, aCmdId);
    return true;
}

void PVMFProtocolEngineNode::RerunForPostProcessAfterOutgoingMsgSent(PVMFProtocolEnginePort *aPort, PVMFSharedMediaMsgPtr &aMsg)
{
    // form PVProtocolEngineNodeInternalEventType_OutgoingMsgQueuedAndSentSuccessfully event
    OutgoingMsgSentSuccessInfo aInfo(aPort, aMsg);
    OutgoingMsgSentSuccessInfoVec *aVec = iInterfacingObjectContainer->getOutgoingMsgSentSuccessInfoVec();
    aVec->push_back(aInfo);
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_OutgoingMsgQueuedAndSentSuccessfully,
            (OsclAny*)aVec);
    iInternalEventQueue.push_back(aEvent);
    SetProcessingState(ProcessingState_NormalDataflow);
    RunIfNotReady();
}

void PVMFProtocolEngineNode::LogPortMediaCmdQueued(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId)
{
    // log message
    if (aCmdId == PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::LogPortMediaCmdQueued() - RECONNECT SENT, Port=0x%x", aPort));
    }
    else if (aCmdId == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::LogPortMediaCmdQueued() - EOS SENT, Port=0x%x", aPort));
    }
    else if (aCmdId == PVMF_MEDIA_CMD_RE_CONFIG_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::LogPortMediaCmdQueued() - RE_CONFIG SENT, Port=0x%x", aPort));
    }
    else if (aCmdId == PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID)
    {
        LOGINFODATAPATH((0, "PVMFProtocolEngineNode::LogPortMediaCmdQueued() - DISCONNECT SENT, Port=0x%x", aPort));
    }

    OSCL_UNUSED_ARG(aPort); // to avoid warning of unused variable 'aPort'

}


bool PVMFProtocolEngineNode::CheckFormatSpecificInfoForMediaCommand(PVMFSharedMediaCmdPtr &aCmdPtr, PVUid32 aCmdId, const bool isForLogging)
{
    if (aCmdId != PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID) return true;

    // for socket recconnect command, set port config as format specific info
    bool needAttachFormatSpecificInfo = false;
    OSCL_HeapString<OsclMemAllocator> aPortConfig;
    if (iInterfacingObjectContainer->getCurrNumRedirectTrials() > 0 || iInterfaceState == EPVMFNodePaused)
    {
        bool hasLoggingUrl = !iInterfacingObjectContainer->getLoggingURIObject().empty();
        if (isForLogging && hasLoggingUrl)
        {
            if (!GetSocketConfigForLogging(aPortConfig)) return false;
        }
        else
        {
            if (!GetSocketConfig(aPortConfig)) return false;
        }
        needAttachFormatSpecificInfo = true;
    }
    else if (iInterfacingObjectContainer->isDownloadStreamingDone())
    {
        bool hasLoggingUrl = !iInterfacingObjectContainer->getLoggingURIObject().empty();
        if (hasLoggingUrl)
        {
            if (!GetSocketConfigForLogging(aPortConfig)) return false;
        }
        else
        {
            if (!GetSocketConfig(aPortConfig)) return false;
        }
        needAttachFormatSpecificInfo = true;
    }

    if (needAttachFormatSpecificInfo)
    {
        OsclRefCounterMemFrag socketConfigMemfrag;
        int32 err = 0;
        OSCL_TRY(err, socketConfigMemfrag = iPortConfigFSInfoAlloc->allocate(aPortConfig.get_size() + 1));
        if (err) return false;
        oscl_memcpy((char*)(socketConfigMemfrag.getMemFragPtr()), aPortConfig.get_cstr(), aPortConfig.get_size());
        char *ptr = (char*)socketConfigMemfrag.getMemFragPtr() + aPortConfig.get_size();
        *ptr = 0; // make it string
        aCmdPtr->setFormatSpecificInfo(socketConfigMemfrag);
    }
    return true;
}

OSCL_EXPORT_REF PVMFStatus PVMFProtocolEngineNode::GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo)
{
    return iProtocolContainer->getMediaPresentationInfo(aInfo);
}

PVMFStatus PVMFProtocolEngineNode::SelectTracks(PVMFMediaPresentationInfo& aInfo)
{
    return iProtocolContainer->selectTracks(aInfo);
}

void PVMFProtocolEngineNode::ReportEvent(PVMFEventType aEventType, OsclAny* aEventData, const int32 aEventCode, OsclAny* aEventLocalBuffer, const uint32 aEventLocalBufferSize)
{
    ReportInfoEvent(aEventType, aEventData, aEventCode, aEventLocalBuffer, aEventLocalBufferSize);
}

void PVMFProtocolEngineNode::NotifyContentTooLarge()
{
    // before error out, settle down the interaction with parser node
    iDownloadControl->checkResumeNotification(false);

    ProtocolStateErrorInfo aInfo(PVMFErrContentTooLarge);
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_ProtocolStateError, (OsclAny*)(&aInfo));
    DispatchInternalEvent(&aEvent);
}

uint32 PVMFProtocolEngineNode::GetObserverState()
{
    return (uint32)iInterfaceState;
}

void PVMFProtocolEngineNode::SetObserverState(const uint32 aState)
{
    iInterfaceState = (TPVMFNodeInterfaceState)aState;
}

bool PVMFProtocolEngineNode::DispatchEvent(PVProtocolEngineNodeInternalEvent *aEvent)
{
    return DispatchInternalEvent(aEvent);
}

bool PVMFProtocolEngineNode::SendMediaCommand(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId, const bool isForLogging)
{
    return SendPortMediaCommand(aPort, aCmdId, isForLogging);
}

void PVMFProtocolEngineNode::ClearRest(const bool aNeedDelete)
{
    iDataInQueue.clear();
    ClearPorts(aNeedDelete); // clear any queued messages in ports
}

void PVMFProtocolEngineNode::RecheduleDataFlow()
{
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_StartDataflowByCommand);
    iInternalEventQueue.push_back(aEvent);
    iProcessingState = ProcessingState_NormalDataflow;
    iInterfacingObjectContainer->setInputDataUnwanted(false);
    RunIfNotReady();
}

void PVMFProtocolEngineNode::SendManualResumeNotificationEvent()
{
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_CheckResumeNotificationMaually);
    iInternalEventQueue.clear();
    iInternalEventQueue.push_back(aEvent);
    SetProcessingState(ProcessingState_NormalDataflow);
    RunIfNotReady();
}

bool PVMFProtocolEngineNode::IsRepositionCmdPending()
{
    PVMFProtocolEngineNodeCommand *pInputCmd   = FindCmd(iInputCommands, PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION);
    PVMFProtocolEngineNodeCommand *pPendingCmd = FindCmd(iCurrentCommand, PVPROTOCOLENGINE_NODE_CMD_DATASTREAM_REQUEST_REPOSITION);

    return (pInputCmd || pPendingCmd);
}

PVMFProtocolEngineNodeCommand* PVMFProtocolEngineNode::FindPendingCmd(int32 aCmdId)
{
    return FindCmd(iCurrentCommand, aCmdId);
}

void PVMFProtocolEngineNode::CompletePendingCmd(int32 status)
{
    CommandComplete(iCurrentCommand, iCurrentCommand.front(), status);
}

void PVMFProtocolEngineNode::CompleteInputCmd(PVMFProtocolEngineNodeCommand& aCmd, int32 status)
{
    CommandComplete(iInputCommands, aCmd, status);
}

void PVMFProtocolEngineNode::ErasePendingCmd(PVMFProtocolEngineNodeCommand *aCmd)
{
    iCurrentCommand.Erase(aCmd);
}

void PVMFProtocolEngineNode::GetObjects()
{
    iProtocol			= (HttpBasedProtocol*)iProtocolContainer->getObject(NodeObjectType_Protocol);
    iNodeOutput			= (PVMFProtocolEngineNodeOutput*)iProtocolContainer->getObject(NodeObjectType_Output);
    iDownloadControl	= (DownloadControlInterface*)iProtocolContainer->getObject(NodeObjectType_DownloadControl);
    iDownloadProgess	= (DownloadProgressInterface*)iProtocolContainer->getObject(NodeObjectType_DownloadProgress);
    iEventReport		= (EventReporter*)iProtocolContainer->getObject(NodeObjectType_EventReport);
    iCfgFileContainer	= (PVDlCfgFileContainer*)iProtocolContainer->getObject(NodeObjectType_DlCfgFileContainer);
    iDownloadSource		= (PVMFDownloadDataSourceContainer*)iProtocolContainer->getObject(NodeObjectType_DataSourceContainer);
    iNodeTimer			= (PVMFProtocolEngineNodeTimer*)iProtocolContainer->getObject(NodeObjectType_Timer);
    iInterfacingObjectContainer = (InterfacingObjectContainer*)iProtocolContainer->getObject(NodeObjectType_InterfacingObjectContainer);
    iUserAgentField		= (UserAgentField*)iProtocolContainer->getObject(NodeObjectType_UseAgentField);
}

////////////////////////////////////////////////////////////////////////////////////
//////	PVProtocolEngineNodeInternalEventHandler implementation
////////////////////////////////////////////////////////////////////////////////////

PVProtocolEngineNodeInternalEventHandler::PVProtocolEngineNodeInternalEventHandler(PVMFProtocolEngineNode *aNode) : iNode(aNode)
{
    iDataPathLogger = PVLogger::GetLoggerObject(NODEDATAPATHLOGGER_TAG);
}

bool PVProtocolEngineNodeInternalEventHandler::completePendingCommandWithError(PVProtocolEngineNodeInternalEvent &aEvent)
{
    int32 errorCode = (int32)aEvent.iEventInfo;
    if (iNode->iCurrentCommand.size() > 0)
    {
        if (IsPVMFErrCode(errorCode))  // basic error event
        {
            iNode->CommandComplete(iNode->iCurrentCommand, iNode->iCurrentCommand.front(), errorCode);
        }
        else   // extension error event
        {
            PVUuid uuid = PVProtocolEngineNodeErrorEventTypesUUID;
            int32 basePVMFErrorCode = getBasePVMFErrorReturnCode(errorCode);
            char *errEventData = NULL;
            uint32 errEventDataLen = 0;
            handleErrResponse(basePVMFErrorCode, errorCode, errEventData, errEventDataLen);
            iNode->CommandComplete(iNode->iCurrentCommand, iNode->iCurrentCommand.front(), basePVMFErrorCode, errEventData, &uuid, &errorCode, errEventDataLen);
            LOGINFODATAPATH((0, "PVProtocolEngineNodeInternalEventHandler::completePendingCommandWithError(), basePVMFErrorCode=%d, extensionErrorCode=%d",
                             basePVMFErrorCode, errorCode));
        }
    }
    else
    {
        // report error event
        if (IsPVMFErrCode(errorCode))
            iNode->ReportErrorEvent(errorCode);
        else
        {
            int32 basePVMFErrorCode = getBasePVMFErrorReturnCode(errorCode, false); // false for error event
            char *errEventData = NULL;
            uint32 eventDataLen = 0;
            handleErrResponse(basePVMFErrorCode, errorCode, errEventData, eventDataLen);
            iNode->ReportErrorEvent(basePVMFErrorCode, errEventData, errorCode, eventDataLen);
            LOGINFODATAPATH((0, "PVProtocolEngineNodeInternalEventHandler::completePendingCommandWithError(), basePVMFErrorCode=%d, extensionErrorCode=%d",
                             basePVMFErrorCode, errorCode));
        }
    }

    iNode->SetState(EPVMFNodeError);
    iNode->StopClear();
    //iNode->iProcessingState = ProcessingState_Idle;

    if (!iNode->iInputCommands.empty())
    {
        if (iNode->IsAdded()) iNode->RunIfNotReady();
    }

    return true;
}

int32 PVProtocolEngineNodeInternalEventHandler::getBasePVMFErrorReturnCode(const int32 errorCode, const bool isForCommandComplete)
{
    int32 pvmfReturnCode = PVMFFailure;
    if (!isForCommandComplete) pvmfReturnCode = PVMFErrProcessing; // if not for command complete, should for error event
    if (iNode->iProtocolContainer->isHTTP409ForLowDiskSpace(errorCode)) pvmfReturnCode = PVMFLowDiskSpace;

    switch (errorCode)
    {
        case PVProtocolEngineNodeErrorProcessingFailure_TimeoutServerNoResponce:
        case PVProtocolEngineNodeErrorProcessingFailure_TimeoutServerInactivity:
            pvmfReturnCode = PVMFErrTimeout;
            break;

        case PVProtocolEngineNodeErrorHTTPErrorCode401:
        case PVProtocolEngineNodeErrorHTTPErrorCode407:
        case PVProtocolEngineNodeErrorHTTPErrorCode401_InvalidRealm:
            pvmfReturnCode = PVMFErrHTTPAuthenticationRequired;
            break;

        case PVProtocolEngineNodeErrorHTTPErrorCode401_UnsupportedAuthenticationType:
            pvmfReturnCode = PVMFErrNotSupported;
            break;

        case PVProtocolEngineNodeErrorHTTPRedirect_TrialsExceedLimit:
            if (iNode->iInterfacingObjectContainer->getNumRedirectTrials() == 0)
            {
                pvmfReturnCode = PVMFErrRedirect;
            }
            break;
        default:
            break;
    }

    return pvmfReturnCode;
}

void PVProtocolEngineNodeInternalEventHandler::handleErrResponse(int32 &aBaseCode, int32 &errCode, char* &aEventData, uint32 &aEventDataLen)
{
    if (aBaseCode == PVMFErrRedirect)
    {
        handleRedirectErrResponse(aEventData, aEventDataLen);
    }
    else
    {
        handleAuthenErrResponse(errCode, aEventData, aEventDataLen);
        aBaseCode = getBasePVMFErrorReturnCode(errCode);
    }
}

void PVProtocolEngineNodeInternalEventHandler::handleAuthenErrResponse(int32 &aErrCode, char* &aEventData, uint32 &aEventDataLen)
{
    aEventData = NULL;
    aEventDataLen = 0;
    if (aErrCode == PVProtocolEngineNodeErrorHTTPErrorCode401)
    {
        if (false == iNode->iProtocol->isServerSendAuthenticationHeader())
        {
            aErrCode = PVProtocolEngineNodeErrorHTTPErrorCode401_NoAuthenticationHeader;
            return;
        }

        if (false == iNode->iProtocol->isServerSupportBasicAuthentication())
        {
            aErrCode = PVProtocolEngineNodeErrorHTTPErrorCode401_UnsupportedAuthenticationType;
            return;
        }

        if (iNode->iProtocol->getAuthenInfo(iAuthenInfoRealm))
        {
            aEventData = (char*)iAuthenInfoRealm.get_cstr();
            aEventDataLen = iAuthenInfoRealm.get_size() + 1; //Incremented by one to save c string terminating char ('\0')
        }
        else
        {
            aErrCode = PVProtocolEngineNodeErrorHTTPErrorCode401_InvalidRealm;
        }
    }
}

void PVProtocolEngineNodeInternalEventHandler::handleRedirectErrResponse(char* &aEventData, uint32 &aEventDataLen)
{
    aEventData = NULL;
    aEventDataLen = 0;
    // set the new url into info event
    OSCL_HeapString<OsclMemAllocator> newUrl;
    iNode->iProtocol->getRedirectURI(newUrl);

    // then set this value
    iNode->iInterfacingObjectContainer->setURI(newUrl, true);
    iNode->iProtocol->setURI(iNode->iInterfacingObjectContainer->getURIObject());

    aEventData = (char*)iNode->iInterfacingObjectContainer->getURIObject().getURI().get_cstr();
    aEventDataLen = iNode->iInterfacingObjectContainer->getURIObject().getURI().get_size() + 1;
}

inline bool PVProtocolEngineNodeInternalEventHandler::isCurrEventMatchCurrPendingCommand(uint32 aCurrEventId)
{
    if (iNode->iCurrentCommand.empty()) return false;
    PVMFProtocolEngineNodeCommand& aCmd = iNode->iCurrentCommand.front();

    // matching logic for event vs. pending command
    // init or prepare command
    if ((aCmd.iCmd == PVMF_GENERIC_NODE_INIT ||
            aCmd.iCmd == PVMF_GENERIC_NODE_PREPARE) &&
            (aCurrEventId == PVProtocolEngineNodeInternalEventType_ProtocolStateComplete ||
             aCurrEventId == PVProtocolEngineNodeInternalEventType_ServerResponseError_Bypassing)) return true;

    // start command
    if (aCmd.iCmd == PVMF_GENERIC_NODE_START &&
            aCurrEventId == PVProtocolEngineNodeInternalEventType_HttpHeaderAvailable) return true;

    //  stop command
    if (aCmd.iCmd == PVMF_GENERIC_NODE_STOP &&
            (aCurrEventId == PVProtocolEngineNodeInternalEventType_ProtocolStateComplete ||
             aCurrEventId == PVProtocolEngineNodeInternalEventType_EndOfProcessing)) return true;

    // seek or bitstream switch command
    if ((aCmd.iCmd == PVPROTOCOLENGINE_NODE_CMD_SEEK ||
            aCmd.iCmd == PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH) &&
            (aCurrEventId == PVProtocolEngineNodeInternalEventType_FirstPacketAvailable)) return true;

    return false; // no matching
}

// return value: true means completing pending command; false means no matching
bool PVProtocolEngineNodeInternalEventHandler::completePendingCommand(PVProtocolEngineNodeInternalEvent &aEvent)
{
    if (aEvent.iEventId == PVProtocolEngineNodeInternalEventType_ProtocolStateError)
    {
        return completePendingCommandWithError(aEvent);
    }
    if (!isCurrEventMatchCurrPendingCommand((uint32)aEvent.iEventId)) return false;

    PVMFProtocolEngineNodeCommand& aCmd = iNode->iCurrentCommand.front();
    iNode->SetState(SetStateByCommand[aCmd.iCmd-(int32)PVMF_GENERIC_NODE_INIT]);
    iNode->CommandComplete(iNode->iCurrentCommand, aCmd, PVMFSuccess);
    return true;
}

bool PVProtocolEngineNodeInternalEventHandler::isBeingStopped(const int32 aStatus)
{
    // stopped, stop command is completed and node state is changed to prepared state
    if (iNode->iInterfacingObjectContainer->isWholeSessionDone() &&
            iNode->iInterfaceState == EPVMFNodePrepared) return true;

    // being stopped, stop command is pending plus protcol state complete
    return (iNode->iInterfacingObjectContainer->isWholeSessionDone() &&
            isStopCmdPending() &&
            isProtocolStateComplete(aStatus));
}

inline bool PVProtocolEngineNodeInternalEventHandler::isStopCmdPending()
{
    for (uint32 i = 0; i < iNode->iCurrentCommand.size(); i++)
    {
        if (iNode->iCurrentCommand[i].iCmd == PVMF_GENERIC_NODE_STOP) return true;
    }
    return false;
}

inline bool PVProtocolEngineNodeInternalEventHandler::isProtocolStateComplete(const int32 aStatus)
{
    return (aStatus == PROCESS_SUCCESS_END_OF_MESSAGE ||
            aStatus == PROCESS_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA ||
            aStatus == PROCESS_SUCCESS_END_OF_MESSAGE_TRUNCATED ||
            aStatus == PROCESS_SUCCESS_END_OF_MESSAGE_BY_SERVER_DISCONNECT);
}

bool ProtocolStateErrorHandler::needCompletePendingCommandAtThisRound(PVProtocolEngineNodeInternalEvent &aEvent)
{
    // get error code if necessary
    ProtocolStateErrorInfo *aInfo = (ProtocolStateErrorInfo *)aEvent.iEventInfo;
    if (aInfo->iUseInputErrorCode) iErrorCode = aInfo->iErrorCode;

    // check if complete pending command at this round is needed
    if (!aInfo->iUseInputErrorCode) return true; // use previous error code, for sure complete pending command
    return !iNode->iProtocolContainer->needSendEOSDuetoError(iErrorCode);
}

bool ProtocolStateErrorHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    if (!needCompletePendingCommandAtThisRound(aEvent))
    {
        // send EOS message to downstream node, and doesn't complete pending command at all
        iNode->StopClear();
        iNode->SendPortMediaCommand(iNode->iPortOut, PVMF_MEDIA_CMD_EOS_FORMAT_ID); // enqueuing EOS should be successful
        return true;
    }

    int32 errorCode = iErrorCode;
    if (errorCode == 0) return true;
    if (NeedHandleContentRangeUnmatch(errorCode)) return true;

    if (errorCode < 0)
    {
        int32 nodeErrorEventTypeCode = (int32)PVProtocolEngineNodeErrorProcessingFailure - (PROCESS_ERROR_FIRST - errorCode);
        if (!IsPVMFErrCode(errorCode)) aEvent.iEventInfo = (OsclAny*)nodeErrorEventTypeCode; // convert to PVProtocolEngineNodeErrorEventType error code
        else aEvent.iEventInfo = (OsclAny*)errorCode;
        completePendingCommand(aEvent);
    }
    else
    {
        int32 errCode = checkRedirectHandling(errorCode);
        if (errCode == 0)
        {
            iErrorCode = 0;
            return true;
        }

        // command complete with error or error event
        aEvent.iEventInfo = (OsclAny*)errCode;
        completePendingCommand(aEvent);
    }

    return true;
}

// return value: 0 means caller needs to return immediately, not 0 means error
int32 ProtocolStateErrorHandler::checkRedirectHandling(const int32 aErrorCode)
{
    bool isInfoEvent = true;
    int32 errCode = parseServerResponseCode(aErrorCode, isInfoEvent);
    uint32 numRedirectTrials = iNode->iInterfacingObjectContainer->getNumRedirectTrials();
    uint32 numCurrRedirectTrials = iNode->iInterfacingObjectContainer->getCurrNumRedirectTrials();

    if (isInfoEvent && ++numCurrRedirectTrials <= numRedirectTrials)
    {
        iNode->iInterfacingObjectContainer->setCurrNumRedirectTrials(numCurrRedirectTrials);

        if (handleRedirect())
        {
            // set the new url into info event
            OSCL_HeapString<OsclMemAllocator> url(iNode->iInterfacingObjectContainer->getURIObject().getURI());
            iNode->ReportInfoEvent(PVMFInfoRemoteSourceNotification, (OsclAny*)(url.get_cstr()), errCode);
            return 0;
        }

        // treat it as error
        errCode = aErrorCode + PVProtocolEngineNodeErrorEventStart;
    }

    if (isInfoEvent && numCurrRedirectTrials > numRedirectTrials)
    {
        // redirect trials out of limit
        errCode = PVProtocolEngineNodeErrorHTTPRedirect_TrialsExceedLimit;
    }

    return errCode;
}


int32 ProtocolStateErrorHandler::parseServerResponseCode(const int32 aErrorCode, bool &isInfoEvent)
{
    isInfoEvent = true;
    int32 errCode = aErrorCode;
    // redirect code 3xx
    if (errCode >= (int32)PROTOCOLENGINE_REDIRECT_STATUS_CODE_START &&
            errCode <= (int32)PROTOCOLENGINE_REDIRECT_STATUS_CODE_END)
    {
        errCode += PVMFPROTOCOLENGINENODEInfo_Redirect;
    }
    else
    {
        errCode += PVProtocolEngineNodeErrorEventStart;
        isInfoEvent = false;
    }
    return errCode;
}

bool ProtocolStateErrorHandler::NeedHandleContentRangeUnmatch(const int32 aErrorCode)
{
    if (aErrorCode == PROCESS_CONTENT_RANGE_INFO_NOT_MATCH)
    {
        if (!handleContentRangeUnmatch()) return false;
        return true;
    }
    return false;
}

bool ProtocolStateErrorHandler::handleContentRangeUnmatch()
{
    return iNode->iProtocolContainer->handleContentRangeUnmatch();
}

bool ProtocolStateErrorHandler::handleRedirect()
{
    // Get redirect url
    OSCL_HeapString<OsclMemAllocator> newUrl;
    if (iNode->iProtocol->getRedirectURI(newUrl) && newUrl.get_size() > 0)
    {
        // then set info to protocol
        iNode->iInterfacingObjectContainer->setURI(newUrl, true);
        iNode->iProtocol->setURI(iNode->iInterfacingObjectContainer->getURIObject());
        iNode->iProtocolContainer->reconnectSocket();
        // Activate so processing will continue
        iNode->iEventReport->startRealDataflow();
        iNode->SetProcessingState(ProcessingState_NormalDataflow);
        return true;
    }
    return false;
}

bool HttpHeaderAvailableHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    // enable info update for download
    iNode->iProtocolContainer->enableInfoUpdate();

    // get Http header
    OUTPUT_DATA_QUEUE aHttpHeader = *((OUTPUT_DATA_QUEUE*)aEvent.iEventData);
    uint32 headerLength = iNode->iInterfacingObjectContainer->setHttpHeader(aHttpHeader);
    bool status = true;
    if (iNode->iProtocol && headerLength > 0)
    {
        uint32 length = iNode->iProtocol->getContentLength();
        iNode->iInterfacingObjectContainer->setFileSize(length);
        iNode->iNodeOutput->setContentLength(length);
        status = iNode->iProtocolContainer->downloadUpdateForHttpHeaderAvailable();
    }

    // check PVMFInfoContentLength, PVMFErrContentTooLarge and PVMFInfoContentTruncated before completing the command
    iNode->iEventReport->checkContentInfoEvent(PROCESS_SUCCESS);

    // complete start command if it is not completed
    if (completePendingCommand(aEvent))
    {
        iNode->iEventReport->startRealDataflow();
    }

    return status;
}

bool FirstPacketAvailableHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    uint32 aFirstPacketNumber = (uint32)aEvent.iEventInfo;

    // first packet number is only needed in seek or bitstream switching
    if (iNode->iInterfaceState == EPVMFNodeStarted && iNode->iCurrentCommand.size() > 0)
    {
        PVMFProtocolEngineNodeCommand& aCmd = iNode->iCurrentCommand.front();

        // for bitstream switching command, guarantee completing command AFTER sending RE_CONFIG port command
        if (aFirstPacketNumber == 0xFFFFFFFF && aCmd.iCmd == PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH)
        {
            return completePendingCommand(aEvent);
        }

        //extract the parameters.
        uint64 aNPTInMS;
        uint32 *aFirstSeqNumAfterSeekOrSwitch;
        aCmd.PVMFProtocolEngineNodeCommand::Parse(aNPTInMS, aFirstSeqNumAfterSeekOrSwitch);
        *aFirstSeqNumAfterSeekOrSwitch = aFirstPacketNumber;

        // send RE_CONFIG port command or complete seek command
        if (aCmd.iCmd == PVPROTOCOLENGINE_NODE_CMD_BITSTREAM_SWITCH)
        {
            // send RE_CONFIG port command
            iNode->SendPortMediaCommand(iNode->iPortOut, PVMF_MEDIA_CMD_RE_CONFIG_FORMAT_ID);
        }
        else   // for seek command, complete the command
        {
            return completePendingCommand(aEvent);
        }
    }
    return true;
}
bool NormalDataAvailableHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    OUTPUT_DATA_QUEUE aOutputData;
    if (aEvent.iEventData) aOutputData = *((OUTPUT_DATA_QUEUE*)aEvent.iEventData);
    if (iNode->iNodeOutput) iNode->iNodeOutput->passDownNewOutputData(aOutputData, aEvent.iEventInfo);

    if (!iNode->IsDataFlowEventAlreadyInQueue())
    {
        PVProtocolEngineNodeInternalEvent newEvent(PVProtocolEngineNodeInternalEventType_OutputDataReady);
        iNode->iInternalEventQueue.push_back(newEvent);
    }
    iNode->RunIfNotReady();
    return true;
}

bool ProtocolStateCompleteHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    bool aSessionDone = iNode->iInterfacingObjectContainer->isWholeSessionDone();
    bool aDownloadStreamingDone = iNode->iInterfacingObjectContainer->isDownloadStreamingDone();
    bool aEOSArrived = iNode->iInterfacingObjectContainer->isEOSAchieved();

    LOGINFODATAPATH((0, "ProtocolStateCompleteHandler::handle() : iNode->iInterfaceState = %d, sessionDone=%d,DownloadStreamingDone=%d,EOSArrived=%d",
                     iNode->iInterfaceState, (uint32)aSessionDone, (uint32)aDownloadStreamingDone, (uint32)aEOSArrived));

    OSCL_UNUSED_ARG(aSessionDone);
    OSCL_UNUSED_ARG(aDownloadStreamingDone);
    OSCL_UNUSED_ARG(aEOSArrived);
    return iNode->iProtocolContainer->handleProtocolStateComplete(aEvent, this);
}

bool NormalDataFlowHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    OSCL_UNUSED_ARG(aEvent);

    // First check and flush output data
    int32 statusFlushData = iNode->iNodeOutput->flushData();
    if (!flushDataPostProcessing(statusFlushData)) return false;

    // send reconnect cmd for logging request for stop or EOS packet received
    if (!handleEOSLogging()) return false;

    // Second, run state machine to continue data processing
    iNode->iProcessingState = ProcessingState_NormalDataflow;
    if (iNode->iDataInQueue.size() > 0) LOGINFODATAPATH((0, "NormalDataFlowHandler::handle() : iNode->iDataInQueue.size() = %d", iNode->iDataInQueue.size()));
    int32 status = iNode->iProtocol->runStateMachine(iNode->iDataInQueue);
    if (status < 0 || statusFlushData != PROCESS_SUCCESS || iNode->iInterfaceState == EPVMFNodeError) return false;

    // handle EOS
    if (handleEOS(status)) return true;

    // handle end of processing, e.g., stop
    if (handleEndOfProcessing(status)) return true;

    // check the next action, go to next protocol state
    return dataFlowContinue(status);
}

bool NormalDataFlowHandler::dataFlowContinue(const int32 aStatus)
{
    // info update for download and for streaming,
    if (!iNode->iProtocolContainer->doInfoUpdate(aStatus)) return false;

    // go to next protocol state for end of message in current protocol state
    // The reason for making this call explicit is, when end of message happens (protocol state complete),
    // the node still needs some information from current state to do some extra work, like the above
    // download control update and info events processing. If we make this call implicit or hidden and
    // right after ProtcolStateComplete(), then the above info processing would fail.
    if (isReadyGotoNextState(aStatus)) iNode->iProtocol->gotoNextState();


    if (iNode->iDataInQueue.empty() && iNode->iPortInForData->IncomingMsgQueueSize() > 0)
    {
        iNode->ProcessIncomingMsg(iNode->iPortInForData);
    }

    if (iNode->iDataInQueue.size() > 0)
    {
        if (!iNode->IsDataFlowEventAlreadyInQueue())
        {
            PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_HasExtraInputData);
            iNode->iInternalEventQueue.push_back(aEvent);
        }
        iNode->RunIfNotReady();
        return true;
    }

    if (aStatus == PROCESS_WAIT_FOR_INCOMING_DATA) return false;
    return true;
}

inline bool NormalDataFlowHandler::isReadyGotoNextState(const int32 aStatus)
{
    return (aStatus == PROCESS_SUCCESS_END_OF_MESSAGE &&
            !iNode->iInterfacingObjectContainer->isWholeSessionDone() &&
            iNode->iInterfaceState != EPVMFNodePaused);
}

bool NormalDataFlowHandler::flushDataPostProcessing(const int32 aStatusFlushData)
{
    // status is ok or need sending new request, then no post processing
    if (aStatusFlushData == PROCESS_SUCCESS || iNode->iProtocol->isSendingNewRequest()) return true;

    // post processing only occur for output port failure
    if (aStatusFlushData == PROCESS_OUTPUT_TO_OUTPUT_PORT_FAILURE)
    {

        if (iNode->iNodeOutput->isPortBusy()) iNode->iProcessingState = ProcessingState_Idle;

        // since queuing mesage for port outgoing queue fails, check the connected port state and try send
        if (!iNode->iPortOut->IsConnectedPortBusy()) iNode->ProcessOutgoingMsg(iNode->iPortOut);

        // if port failure is due to no memory for media data, reset inactivity timer to prevent unnessary timeout
        if (!iNode->iPortOut->IsOutgoingQueueBusy() && // this means port failure is due to no memory
                iNode->iInterfaceState != EPVMFNodePaused &&
                (iNode->iDataInQueue.size() > 0 || iNode->iPortInForData->IncomingMsgQueueSize() > 0))
        {
            LOGINFODATAPATH((0, "NormalDataFlowHandler::flushDataPostProcessing() : iNode->iDataInQueue.size()=%d, iNode->iPortInForData->IncomingMsgQueueSize()=%d",
                             iNode->iDataInQueue.size(), iNode->iPortInForData->IncomingMsgQueueSize()));
            iNode->iNodeTimer->start(SERVER_INACTIVITY_TIMER_ID); // reset inactivity timer to prevent unnessary timeout
        }
    }
    return false;
}

bool NormalDataFlowHandler::handleEOSLogging()
{
    if (iNode->iInterfacingObjectContainer->isDownloadStreamingDone() &&
            !iSendSocketReconnect &&
            iNode->iPortInForLogging)
    {
        // reconnect first
        if (!iNode->SendPortMediaCommand(iNode->iPortInForLogging, PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID)) return false;
        iNode->iProtocol->sendRequest();
        iSendSocketReconnect = true;

        // start stop/eos logging timer at this point to protect itself from the case where logging request
        // cannot be sent out due to connected port busy. For this case, logging timer timeout will help keep going
        iNode->iNodeTimer->start(SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING);
        LOGINFODATAPATH((0, "NormalDataFlowHandler::handleEOSLogging() server stop/eos logging response timer starts! timerID=3, timeoutValue=%d",
                         iNode->iNodeTimer->getTimeout(SERVER_RESPONSE_TIMER_ID_FOR_STOPEOS_LOGGING)));
    }
    else if (!iNode->iInterfacingObjectContainer->isDownloadStreamingDone())
    {
        // reset the flag
        iSendSocketReconnect = false;
    }
    return true;
}

bool NormalDataFlowHandler::handleEOS(const int32 aStatus)
{
    if (aStatus != PROCESS_SUCCESS_GOT_EOS) return false;

    if (//(iNode->iInterfaceState==EPVMFNodeStarted || iNode->iInterfaceState==EPVMFNodePrepared || iNode->iInterfaceState==EPVMFNodeInitialized) &&
        !iNode->iInterfacingObjectContainer->isDownloadStreamingDone())   // only issue socket reconnect during the streaming
    {
        LOGINFODATAPATH((0, "NormalDataFlowHandler::handleEOS() : status == PROCESS_SUCCESS_GOT_EOS"));
        iNode->iProtocolContainer->doEOS(false);

        PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_StartDataflowBySendRequestAction);
        iNode->iInternalEventQueue.push_back(aEvent);
        iNode->RunIfNotReady();
        return true;
    }

    return false;
}

bool NormalDataFlowHandler::handleEndOfProcessing(const int32 aStatus)
{
    if (!isBeingStopped(aStatus)) return false;

    // use end of processing event to streamline all end of processing cases for stop
    EndOfDataProcessingInfo *aEOPInfo = iNode->iInterfacingObjectContainer->getEOPInfo();
    aEOPInfo->clear();
    aEOPInfo->iStreamingDone = true;
    PVProtocolEngineNodeInternalEvent aEvent(PVProtocolEngineNodeInternalEventType_EndOfProcessing, (OsclAny*)aEOPInfo);
    iNode->DispatchInternalEvent(&aEvent);
    return true;
}

bool EndOfDataProcessingHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    EndOfDataProcessingInfo *aInfo = (EndOfDataProcessingInfo*)aEvent.iEventInfo;
    if (!aInfo) return true;

    if (aInfo->iSendResumeNotification)
    {
        iNode->iDownloadControl->checkResumeNotification();
        iNode->iNodeTimer->clear();
        LOGINFODATAPATH((0, "EndOfDataProcessingHandler::handle(), send resume notification to parser node, for DOWNLOAD COMPLETE"));
    }
    if (aInfo->iExtraDataComeIn)
    {
        iNode->iEventReport->checkReportEvent(PROCESS_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA);
        LOGINFODATAPATH((0, "EndOfDataProcessingHandler::handle(), check and send PVMFUnexpectedData info event after DOWNLOAD COMPLETE"));
    }
    if (aInfo->iSendServerDisconnectEvent)
    {
        iNode->iEventReport->checkReportEvent(PROCESS_SUCCESS_END_OF_MESSAGE_BY_SERVER_DISCONNECT);
        LOGINFODATAPATH((0, "EndOfDataProcessingHandler::handle(), check and send PVMFInfoSessionDisconnect info event after DOWNLOAD COMPLETE"));
    }
    if (aInfo->iStreamingDone || aInfo->iForceStop)
    {
        cleanupForStop(aEvent);
        LOGINFODATAPATH((0, "EndOfDataProcessingHandler::handle(), handle the remaining stuff for EOS or stop"));
    }

    aInfo->clear();
    iNode->iProcessingState = ProcessingState_Idle;
    return true;
}

void EndOfDataProcessingHandler::cleanupForStop(PVProtocolEngineNodeInternalEvent &aEvent)
{
    EndOfDataProcessingInfo *aInfo = (EndOfDataProcessingInfo*)aEvent.iEventInfo;
    if (isBeingStopped() || aInfo->iForceStop)
    {
        completePendingCommand(aEvent);
        iNode->iProtocol->stop(true);
        iNode->StopClear();
    }
}

bool ServerResponseErrorBypassingHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    completePendingCommand(aEvent);
    iNode->iProtocol->gotoNextState();
    //iNode->iProcessingState = ProcessingState_Idle;
    return true;
}

bool CheckResumeNotificationHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    OSCL_UNUSED_ARG(aEvent);

    // double check if the reposition request is pending or not
    if (iNode->IsRepositioningRequestPending()) return true;

    if (iNode->iDownloadControl->checkResumeNotification(false) == 1)   // false means download is not complete yet
    {
        LOGINFODATAPATH((0, "CheckResumeNotificationHandler::handle(), send resume notification to parser node, in case of MBDS getting full in progressive streaming"));
        // report data ready event
        iNode->iEventReport->sendDataReadyEvent();
    }
    return true;
}

bool OutgoingMsgSentSuccessHandler::handle(PVProtocolEngineNodeInternalEvent &aEvent)
{
    OutgoingMsgSentSuccessInfoVec *aVec = (OutgoingMsgSentSuccessInfoVec*)aEvent.iEventInfo;
    if (!aVec || aVec->empty()) return false;
    bool retVal = (iNode->PostProcessForMsgSentSuccess(aVec->front().iPort, aVec->front().iMsg) == PVMFSuccess);
    if (!aVec->empty()) aVec->erase(aVec->begin());
    return retVal;
}



