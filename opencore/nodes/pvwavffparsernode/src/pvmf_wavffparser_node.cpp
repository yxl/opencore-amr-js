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
#include "pvmf_wavffparser_node.h"
#include "pvmf_wavffparser_defs.h"
#include "wav_parsernode_tunables.h"
#include "pvmf_wavffparser_port.h"
#include "pvwavfileparser.h"
#include "media_clock_converter.h"
#include "pv_gau.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_fileformat_events.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pv_mime_string_utils.h"
#include "oscl_stdstring.h"
#include "oscl_snprintf.h"
#include "pvmi_kvp_util.h"


static const char PVWAVMETADATA_ALL_METADATA_KEY[] = "all";

#define PVMF_WAV_NUM_METADATA_VALUES 7
// Constant character strings for metadata keys
static const char PVWAVMETADATA_DURATION_KEY[] = "duration";
static const char PVWAVMETADATA_NUMTRACKS_KEY[] = "num-tracks";
static const char PVWAVMETADATA_TRACKINFO_BITRATE_KEY[] = "track-info/bit-rate";
static const char PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY[] = "track-info/sample-rate";
static const char PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY[] = "track-info/audio/channels";
static const char PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY[] = "track-info/audio/format";
static const char PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY[] = "track-info/audio/bits-per-sample";
static const char PVWAVMETADATA_SEMICOLON[] = ";";
static const char PVWAVMETADATA_TIMESCALE1000[] = "timescale=1000";
static const char PVWAVMETADATA_INDEX0[] = "index=0";


PVMFWAVFFParserNode::PVMFWAVFFParserNode(int32 aPriority) :
        OsclTimerObject(aPriority, "PVMFWAVFFParserNode"),
        iStreamID(0),
        iOutPort(NULL),
        iLogger(NULL),
        iCmdRespPort(NULL),
        iWAVParser(NULL),
        iExtensionRefCount(0)
{
    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of dynamic memory allocation.
             iInputCommands.Construct(PVMF_WAVFFPARSER_NODE_COMMAND_ID_START, PVMF_WAVFFPARSER_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = false;
             iCapability.iCanSupportMultipleOutputPorts = false;
             iCapability.iHasMaxNumberOfPorts = true;
             iCapability.iMaxNumberOfPorts = 2;//no maximum
             iCapability.iInputFormatCapability.push_back(PVMF_MIME_WAVFF);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_PCM8);
             iCapability.iOutputFormatCapability.push_back(PVMF_MIME_PCM16);
            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iInputCommands.clear();
        iCurrentCommand.clear();
        iCapability.iInputFormatCapability.clear();
        iCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterface);
        OSCL_CLEANUP_BASE_CLASS(OsclTimerObject);
        OSCL_LEAVE(err);
    }

    Construct();
}

PVMFWAVFFParserNode::~PVMFWAVFFParserNode()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::~PVMFWAVFFParserNode() called"));
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

    //cleanup port activity events
    iPortActivityQueue.clear();

    Cancel();

    //Cleanup allocated ports
    ReleaseAllPorts();
    CleanupFileSource();
    iFileServer.Close();
}


/**
 //Do thread-specific node creation and go to "Idle" state.
 */
PVMFStatus PVMFWAVFFParserNode::ThreadLogon()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:ThreadLogon"));
    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();

            iLogger = PVLogger::GetLoggerObject("PVMFWAVFFParserNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}


/**
 //Do thread-specific node cleanup and go to "Created" state.
 */
PVMFStatus PVMFWAVFFParserNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:ThreadLogoff"));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
                RemoveFromScheduler();

            iLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}


PVMFStatus PVMFWAVFFParserNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetCapability() called"));

    // TODO: Return the appropriate format capability
    aNodeCapability = iCapability;
    return PVMFSuccess;
}


PVMFPortIter* PVMFWAVFFParserNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetPorts() called"));

    OSCL_UNUSED_ARG(aFilter);
    // TODO: Return the currently available ports
    return NULL;
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:QueryUUID"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:QueryInterface"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:RequestPort"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}


/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:ReleasePort"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Init"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Prepare"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Start"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Stop"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Pause"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Flush"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:Reset"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:CancelAllCommands"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}

/**
 //Queue an asynchronous node command
 */
PVMFCommandId PVMFWAVFFParserNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:CancelCommand"));
    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}



/////////////////////
// Private Section //
/////////////////////
void PVMFWAVFFParserNode::Construct()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Construct()"));

    iFileServer.Connect();

    iAvailableMetadataKeys.reserve(PVMF_WAV_NUM_METADATA_VALUES);
    iAvailableMetadataKeys.clear();
}


void PVMFWAVFFParserNode::Run()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() In"));

    //Process commands.
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            //note: need to check the state before re-scheduling
            //since the node could have been reset in the ProcessCommand call.
            if (iInterfaceState != EPVMFNodeCreated)
                RunIfNotReady();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() Out 1"));

            return;
        }
    }

    // Process port activity
    if (!iPortActivityQueue.empty() &&
            (iInterfaceState == EPVMFNodeStarted || FlushPending()))
    {
        // If the port activity cannot be processed because a port is
        // busy, discard the activity and continue to process the next
        // activity in queue until getting to one that can be processed.
        while (!iPortActivityQueue.empty())
        {
            if (ProcessPortActivity())
                break; //processed a port
        }

        //Re-schedule
        RunIfNotReady();

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() Out 2"));
        return;
    }

    // Create new data and send to the output queue
    if (iInterfaceState == EPVMFNodeStarted && !FlushPending())
    {
        if (HandleTrackState()) // Handle track state returns true if there is more data to be sent.
        {
            // Run again if there is more data to send out
            RunIfNotReady();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() Out 33"));
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() Out 3"));

        return; // !FlushPending() = true ==> no flush
    }

    //this AO needs to monitor for node flush completion.
    if (FlushPending() &&
            iPortActivityQueue.empty())
    {
        if (iOutPort->OutgoingMsgQueueSize() > 0)
        {
            //need to wait on completion...
            RunIfNotReady();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() Out 4"));

            return;
        }

        //Flush is complete.  Go to prepared state.
        SetState(EPVMFNodePrepared);
        //resume port input so the ports can be re-started.
        iOutPort->ResumeInput();
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::Run() Out 5"));
}

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////
bool PVMFWAVFFParserNode::ProcessPortActivity()
{
    //called by the AO to process a port activity message
    //Pop the queue...
    PVMFPortActivity activity(iPortActivityQueue.front());
    iPortActivityQueue.erase(&iPortActivityQueue.front());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFWAVFFParserNode::ProcessPortActivity: port=0x%x, type=%d",
                     this, activity.iPort, activity.iType));

    int32 err = OsclErrNone;
    PVMFStatus status = PVMFSuccess;
    switch (activity.iType)
    {
        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            status = ProcessOutgoingMsg(activity.iPort);
            //Re-queue the port activity event as long as there's
            //more data to process and it isn't in a Busy state.
            if (status != PVMFErrBusy
                    && activity.iPort->OutgoingMsgQueueSize() > 0)
            {
                err = PushBackPortActivity(activity);
            }
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            err = PVMFFailure;
            break;

        default:
            break;
    }

    //report a failure in queueing new activity...
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFWAVFFParserNode::ProcessPortActivity: Error - queue port activity failed. port=0x%x, type=%d",
                         this, activity.iPort, activity.iType));
        ReportErrorEvent(PVMFErrPortProcessing);
    }


    //Report any unexpected failure in port processing...
    //(the InvalidState error happens when port input is suspended,
    //so don't report it.)
    if (status != PVMFErrBusy && status != PVMFSuccess && status != PVMFErrInvalidState)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFWAVFFParserNode::ProcessPortActivity: Error - ProcessPortActivity failed. port=0x%x, type=%d",
                         this, activity.iPort, activity.iType));
        ReportErrorEvent(PVMFErrPortProcessing);
    }

    //return true if we processed an activity...
    return (status != PVMFErrBusy);
}


/////////////////////////////////////////////////////
PVMFStatus PVMFWAVFFParserNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one message off the outgoing
    //message queue for the given port.  This routine will
    //try to send the data to the connected port.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFWAVFFParserNode::ProcessOutgoingMsg: aPort=0x%x", this, aPort));

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        //If Send() fails, the destination port will send the notification
        //PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY later, when it is ready,
        // in the HandlePortActivity(). Schedule AO then.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PVMFWAVFFParserNode::ProcessOutgoingMsg: Connected port goes into busy state", this));
    }

    return status;
}


bool PVMFWAVFFParserNode::HandleTrackState()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::HandleTrackState() In"));

    // Flag to be active again or not
    bool ret_status = false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::HandleTrackState track list size = %d", iSelectedTrackList.size()));
    for (uint32 i = 0; i < iSelectedTrackList.size(); ++i)
    {
        switch (iSelectedTrackList[i].iState)
        {
            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED:
                iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
                // Continue on to retrieve the first frame

            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA:
                // First do check for the availability of getting new data
                if (!CheckAvailabilityForSendingNewTrackData(iSelectedTrackList[i]))
                {
                    return false; // Not available to get data and send data
                }

                if (!RetrieveTrackData(iSelectedTrackList[i]))
                {
                    if (iSelectedTrackList[i].iState == PVWAVFFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK)
                    {
                        ReportInfoEvent(PVMFInfoEndOfData);
                    }
                    break;
                }

                iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA;
                // Continue on to send the frame

            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA:
                if (iSelectedTrackList[i].iSendBOS)
                {
                    if (!SendBeginOfMediaStreamCommand(iSelectedTrackList[i]))
                        return true;

                    if (!iSelectedTrackList[i].iMediaData)
                    {
                        iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
                        break;
                    }
                }

                if (SendTrackData(iSelectedTrackList[i]))
                {
                    iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
                    ret_status = true;
                }
                break;

            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK:
                if (iSelectedTrackList[i].iSendBOS)
                {
                    if (!SendBeginOfMediaStreamCommand(iSelectedTrackList[i]))
                        return true;
                }

                if (SendEndOfTrackCommand(iSelectedTrackList[i]))
                {
                    // EOS sent successfully.
                    iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_ENDOFTRACK;
                    ReportInfoEvent(PVMFInfoEndOfData);
                }
                else
                {
                    // EOS command sending failed -- wait on outgoing queue ready notice
                    // before trying again.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFWAVFFParserNode::HandleTrackState EOS media command sending failed"));
                    ret_status = true;
                }

            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_MEDIADATAPOOLEMPTY:
            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_INITIALIZED:
            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_ENDOFTRACK:
            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_DESTFULL:
            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_SOURCEEMPTY:
            case PVWAVFFNodeTrackPortInfo::TRACKSTATE_ERROR:

            default:
                // Do nothing for these states for now
                break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::HandleTrackState() Out"));
    return ret_status;
}


bool PVMFWAVFFParserNode::RetrieveTrackData(PVWAVFFNodeTrackPortInfo& aTrackPortInfo)
{
    // Create a data buffer from pool
    int32 errcode = OsclErrNone;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImplOut;
    OSCL_TRY(errcode, mediaDataImplOut = aTrackPortInfo.iMediaDataImplAlloc->allocate(trackdata_bufsize));

    if (errcode != OsclErrNone)
    {
        if (errcode == OsclErrNoResources)
        {
            aTrackPortInfo.iTrackDataMemoryPool->notifyfreechunkavailable(aTrackPortInfo);	// Enable flag to receive event when next deallocate() is called on pool
            return false;
        }
        else if (errcode == OsclErrNoMemory)
        {
            // Memory allocation for the pool failed
            aTrackPortInfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_ERROR;
            ReportErrorEvent(PVMFErrNoMemory, NULL);
            return false;
        }
        else if (errcode == OsclErrArgument)
        {
            // Invalid parameters passed to mempool
            aTrackPortInfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_ERROR;
            ReportErrorEvent(PVMFErrArgument, NULL);
            return false;
        }
        else
        {
            // General error
            aTrackPortInfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_ERROR;
            ReportErrorEvent(PVMFFailure, NULL);
            return false;
        }
    }


    // Now create a PVMF media data from pool
    errcode = OsclErrNoResources;
    PVMFSharedMediaDataPtr mediadataout;
    mediadataout = PVMFMediaData::createMediaData(mediaDataImplOut, aTrackPortInfo.iMediaDataMemPool);

    if (mediadataout.GetRep() != NULL)
    {
        errcode = OsclErrNone;
    }

    else
    {
        aTrackPortInfo.iMediaDataMemPool->notifyfreechunkavailable(aTrackPortInfo);		// Enable flag to receive event when next deallocate() is called on pool
        return false;
    }

    // Retrieve memory fragment to write to
    OsclRefCounterMemFrag refCtrMemFragOut;
    OsclMemoryFragment memFragOut;
    mediadataout->getMediaFragment(0, refCtrMemFragOut);
    memFragOut.ptr = refCtrMemFragOut.getMemFrag().ptr;

    uint32 samplesread = 0;
    PVWavParserReturnCode retcode = iWAVParser->GetPCMData((uint8*)memFragOut.ptr, refCtrMemFragOut.getCapacity(), trackdata_num_samples, samplesread);

    if ((retcode == PVWAVPARSER_OK) || (retcode == PVWAVPARSER_END_OF_FILE))
    {
        if (samplesread == 0)
        {
            // Only partial sample so return and set active to call HandleTrackState() again
            if (retcode != PVWAVPARSER_END_OF_FILE)
            {
                RunIfNotReady();
                return false;
            }
        }
        else
        {
            memFragOut.len = samplesread * wavinfo.BytesPerSample * wavinfo.NumChannels;
            mediadataout->setMediaFragFilledLen(0, memFragOut.len);

            // set the timestamp
            uint32 timestamp = aTrackPortInfo.iClockConverter->get_converted_ts(COMMON_PLAYBACK_CLOCK_TIMESCALE);

            // set the timestamp
            mediadataout->setTimestamp(timestamp);

            // set the sequence number
            mediadataout->setSeqNum(aTrackPortInfo.iSeqNum++);

            // set the stream id
            mediadataout->setStreamID(iStreamID);

            aTrackPortInfo.iMediaData = mediadataout;

            // increment the clock based on this sample duration
            timestamp = aTrackPortInfo.iClockConverter->get_current_timestamp();
            timestamp += samplesread;
            aTrackPortInfo.iClockConverter->update_clock(timestamp);
        }
        if (retcode == PVWAVPARSER_END_OF_FILE)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::RetrieveTrackData() WAV Parser END_OF_FILE"));

            if (SendEndOfTrackCommand(aTrackPortInfo))
            {
                // EOS message sent so change state
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFWAVFFParserNode::RetrieveTrackData() Sending EOS message succeeded"));
                aTrackPortInfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
                return false;
            }
            else
            {
                // EOS message could not be queued so keep in same state and try again later
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFWAVFFParserNode::RetrieveTrackData() Sending EOS message failed"));
                return true;
            }
        }

    }
    else
    {
        // Parser read error
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFWAVFFParserNode::RetrieveTrackData() Read error returned by parser"));
        aTrackPortInfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_ERROR;

        // Determine error code
        PVUuid eventuuid;
        int32 eventcode;
        if (!MapWAVErrorCodeToEventCode(retcode, eventuuid, eventcode))
        {
            eventuuid = PVMFFileFormatEventTypesUUID;
            eventcode = PVMFFFErrMisc;
        }

        // Report error event
        PVMFBasicErrorInfoMessage* eventmsg = NULL;
        errcode = 0;
        OSCL_TRY(errcode, eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (eventcode, eventuuid, NULL)));
        if (errcode == 0)
        {
            PVMFAsyncEvent asyncevent(PVMFErrorEvent, PVMFErrResource, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg), NULL, NULL, 0);
            PVMFNodeInterface::ReportErrorEvent(asyncevent);
            eventmsg->removeRef();
        }
    }

    return true;
}

bool PVMFWAVFFParserNode::SendTrackData(PVWAVFFNodeTrackPortInfo& aTrackPortInfo)
{
    // Send frame to downstream node via port
    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaMsg(mediaMsgOut, aTrackPortInfo.iMediaData);
    if (aTrackPortInfo.iPort->QueueOutgoingMsg(mediaMsgOut) != PVMFSuccess)
    {
        // output queue is busy, so wait for the output queue being ready
        return false;
    }

    // Don't need the ref to iMediaData so unbind it
    aTrackPortInfo.iMediaData.Unbind();
    return true;
}

bool PVMFWAVFFParserNode::CheckAvailabilityForSendingNewTrackData(PVWAVFFNodeTrackPortInfo& aTrackPortInfo)
{

    // check if the output queue of output port is not full
    if (aTrackPortInfo.iPort && aTrackPortInfo.iPort->IsOutgoingQueueBusy())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PVMFMP3DecNode::CheckAvailabilityForSendingNewTrackData: output queue of the output port is busy", this));
        return false;
    }

    return true;
}



/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////
void PVMFWAVFFParserNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFWAVFFParserNode::PortActivity: port=0x%x, type=%d",
                     this, aActivity.iPort, aActivity.iType));

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //An outgoing message was queued on this port.
            //We only need to queue a port activity event on the
            //first message.  Additional events will be queued during
            //the port processing as needed.
            if (aActivity.iPort->OutgoingMsgQueueSize() == 1)
                QueuePortActivity(aActivity);

            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            if (aActivity.iPort->IncomingMsgQueueSize() == 1)
                QueuePortActivity(aActivity);
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            //get the capability and config interface and set the parameter
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            //nothing needed.

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            //This message is send by destination port to notify that the earlier Send
            //call that failed due to its busy status can be resumed now.
            if (aActivity.iPort->OutgoingMsgQueueSize() > 0)
                QueuePortActivity(aActivity);
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            //this message is sent by the OutgoingQueue when it recovers from
            //the queue full status
            HandleOutgoingQueueReady(aActivity.iPort);

        default:
            break;
    }
}

void PVMFWAVFFParserNode::QueuePortActivity(const PVMFPortActivity &aActivity)
{
    //queue a new port activity event
    int32 err = 0;
    OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFWAVFFParserNode::PortActivity: Error - iPortActivityQueue.push_back() failed", this));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
    }
    else
    {
        //wake up the AO to process the port activity event.
        RunIfNotReady();
    }
}

bool PVMFWAVFFParserNode::ProcessCommand(PVMFWAVFFNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFWAVFFParserNode::ProcessCommand: cmd %d",
                     this, aCmd.iCmd));

    //normally this node will not start processing one command
    //until the prior one is finished.  However, a hi priority
    //command such as Cancel must be able to interrupt a command
    //in progress.
    if (!iCurrentCommand.empty() && !aCmd.hipri())
        return false;

    switch (aCmd.iCmd)
    {
        case PVMF_GENERIC_NODE_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_GENERIC_NODE_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_GENERIC_NODE_REQUESTPORT:
            DoRequestPort(aCmd);
            break;

        case PVMF_GENERIC_NODE_RELEASEPORT:
            DoReleasePort(aCmd);
            break;

        case PVMF_GENERIC_NODE_INIT:
            DoInit(aCmd);
            break;

        case PVMF_GENERIC_NODE_PREPARE:
            DoPrepare(aCmd);
            break;

        case PVMF_GENERIC_NODE_START:
            DoStart(aCmd);
            break;

        case PVMF_GENERIC_NODE_STOP:
            DoStop(aCmd);
            break;

        case PVMF_GENERIC_NODE_FLUSH:
            DoFlush(aCmd);
            break;

        case PVMF_GENERIC_NODE_PAUSE:
            DoPause(aCmd);
            break;

        case PVMF_GENERIC_NODE_RESET:
            DoReset(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELALLCOMMANDS:
            DoCancelAllCommands(aCmd);
            break;

        case PVMF_GENERIC_NODE_CANCELCOMMAND:
            DoCancelCommand(aCmd);
            break;

        case PVWAVFF_NODE_CMD_SETDATASOURCEPOSITION:
            DoSetDataSourcePosition(aCmd);
            break;

        case PVWAVFF_NODE_CMD_QUERYDATASOURCEPOSITION:
            DoQueryDataSourcePosition(aCmd);
            break;

        case PVWAVFF_NODE_CMD_SETDATASOURCERATE:
            DoSetDataSourceRate(aCmd);
            break;

        case PVWAVFF_NODE_CMD_GETNODEMETADATAKEY:
        {
            PVMFStatus retval = DoGetNodeMetadataKey(aCmd);
            CommandComplete(iInputCommands, aCmd, retval);
        }
        break;

        case PVWAVFF_NODE_CMD_GETNODEMETADATAVALUE:
        {
            PVMFStatus retval = DoGetNodeMetadataValue(aCmd);
            CommandComplete(iInputCommands, aCmd, retval);
        }
        break;

        default://unknown command type
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            break;
    }

    return true;
}

/////////////////////////////////////////////////////
// Event reporting routines.
/////////////////////////////////////////////////////
void PVMFWAVFFParserNode::SetState(TPVMFNodeInterfaceState s)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:SetState %d", s));
    PVMFNodeInterface::SetState(s);
}


void PVMFWAVFFParserNode::ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:ReportErrorEvent Type %d Data %d"
                    , aEventType, aEventData));

    PVMFNodeInterface::ReportErrorEvent(aEventType, aEventData);
}

void PVMFWAVFFParserNode::ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:ReportInfoEvent Type %d Data %d"
                    , aEventType, aEventData));

    PVMFNodeInterface::ReportInfoEvent(aEventType, aEventData);
}

void PVMFWAVFFParserNode::DoQueryUuid(PVMFWAVFFNodeCommand& aCmd)
{

    //This node supports Query UUID from any state
    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFWAVFFNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //right now, supported UUID is pushed directly. It needs to check for MIME type first
    if (*mimetype == PVMF_DATA_SOURCE_INIT_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMF_DATA_SOURCE_INIT_INTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_TRACK_SELECTION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMF_TRACK_SELECTION_INTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_DATA_SOURCE_PLAYBACK_CONTROL_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PvmfDataSourcePlaybackControlUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_META_DATA_EXTENSION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(KPVMFMetadataExtensionUuid);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFWAVFFParserNode::DoQueryInterface(PVMFWAVFFNodeCommand&  aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFWAVFFParserNode::DoQueryInterface"));

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFWAVFFNodeCommandBase::Parse(uuid, ptr);

    if (queryInterface(*uuid, *ptr))
    {
        (*ptr)->addRef();
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

void PVMFWAVFFParserNode::DoInit(PVMFWAVFFNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoInitNode() In"));

    PVMFStatus iRet = PVMFSuccess;
    // initialize the nextState to error
    TPVMFNodeInterfaceState nextState = EPVMFNodeError;
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:

            // first initialize the state to error
            //this node doesn't need to do anything to get ready
            //to start.

            break;

        default:
            iRet = PVMFErrInvalidState;
            CommandComplete(iInputCommands, aCmd, iRet);
            return;
    }

    if (iWAVParser != NULL)
    {
        iRet = PVMFFailure;
        SetState(EPVMFNodeError);
        CommandComplete(iInputCommands, aCmd, iRet);
        return;
    }

    int32 leavecode = 0;
    OSCL_TRY(leavecode, iWAVParser = OSCL_NEW(PV_Wav_Parser, ()));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         SetState(EPVMFNodeError);
                         CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                         return;);
    if (iWAVParser)
    {
        if (iWAVParser->InitWavParser(iFilename, &iFileServer) != PVWAVPARSER_OK)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFWAVFFParserNode::DoInit Error attempting to initialize the WAV parser"));
            OSCL_DELETE(iWAVParser);
            iWAVParser = NULL;
            iRet = PVMFFailure;
            CommandComplete(iInputCommands, aCmd, iRet);
            return;
        }
        else if (!iWAVParser->RetrieveFileInfo(wavinfo))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFWAVFFParserNode::DoInit Error attempting to retrieve the WAV file info"));
            OSCL_DELETE(iWAVParser);
            iWAVParser = NULL;
            iRet = PVMFFailure;
        }
        else if (!verify_supported_format())
        {
            OSCL_DELETE(iWAVParser);
            iWAVParser = NULL;
            iRet = PVMFErrNotSupported;
        }
        else
        {
            if ((wavinfo.AudioFormat == PVWAV_ITU_G711_ALAW) || (wavinfo.AudioFormat == PVWAV_ITU_G711_ULAW))
            {
                if (iWAVParser->SetOutputToUncompressedPCM())
                {
                    wavinfo.AudioFormat = PVWAV_PCM_AUDIO_FORMAT;
                    wavinfo.BitsPerSample = 16;
                    wavinfo.BytesPerSample = 2;
                }
            }
            if (iSelectedTrackList.size() ==  0)
            {
                // save the port information in the selected track list
                PVWAVFFNodeTrackPortInfo trackportinfo;
                trackportinfo.iTrackId = 0;  // Only support 1 track
                trackportinfo.iTag = PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE;
                trackportinfo.iPort = iOutPort;
                trackportinfo.iClockConverter = NULL;
                trackportinfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED;
                trackportinfo.iTrackDataMemoryPool = NULL;
                trackportinfo.iMediaDataImplAlloc = NULL;
                trackportinfo.iMediaDataMemPool = NULL;
                trackportinfo.iNode = this;
                iSelectedTrackList.push_back(trackportinfo);
            }

            int error;
            OSCL_TRY(error, InitializeTrackStructure());
            OSCL_FIRST_CATCH_ANY(error,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFWAVFFParserNode::DoInitNode: Error - initializing tracks"));
                                 OSCL_DELETE(iOutPort);
                                 iOutPort = NULL;
                                 iSelectedTrackList.erase(iSelectedTrackList.begin());
                                 OSCL_DELETE(iWAVParser);
                                 iWAVParser = NULL;
                                 iRet = PVMFFailure;
                                 SetState(EPVMFNodeError);
                                 CommandComplete(iInputCommands, aCmd, iRet);
                                 return;);


            if (iOutPort && iOutPort->IsConnected())
            {
                // set the new parameters on the connected port

                OsclAny* temp = NULL;
                iOutPort->iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

                if (!config)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFWAVFFParserNode::DoInitNode: Error - Peer port does not support capability interface"));

                    iRet = PVMFFailure;
                    SetState(EPVMFNodeError);
                    CommandComplete(iInputCommands, aCmd, iRet);
                    return;
                }
                PVMFStatus status = PVMFSuccess;

                status = NegotiateSettings(config);

                if (status != PVMFSuccess)
                {
                    iRet = PVMFFailure;
                    SetState(EPVMFNodeError);
                    CommandComplete(iInputCommands, aCmd, iRet);
                    return;
                }

            }

            // Add the available metadata keys to the vector
            iAvailableMetadataKeys.clear();

            if (wavinfo.NumSamples > 0 && wavinfo.SampleRate > 0)
            {
                leavecode = 0;
                OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_DURATION_KEY));
            }

            leavecode = 0;
            OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_NUMTRACKS_KEY));

            if (wavinfo.BitsPerSample > 0 && wavinfo.SampleRate > 0)
            {
                leavecode = 0;
                OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_TRACKINFO_BITRATE_KEY));
            }

            if (wavinfo.SampleRate > 0)
            {
                leavecode = 0;
                OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY));
            }

            if (wavinfo.NumChannels > 0)
            {
                leavecode = 0;
                OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY));
            }

            if (wavinfo.AudioFormat != PVWAV_UNKNOWN_AUDIO_FORMAT)
            {
                leavecode = 0;
                OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY));
            }

            if (wavinfo.BytesPerSample > 0)
            {
                leavecode = 0;
                OSCL_TRY(leavecode, iAvailableMetadataKeys.push_back(PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY));
            }

            nextState = EPVMFNodeInitialized;
            iRet = PVMFSuccess;
        }
    }
    else
    {
        iRet = PVMFErrNoMemory;
    }

    SetState(nextState);
    CommandComplete(iInputCommands, aCmd, iRet);
}

// verify that the format is supported
bool PVMFWAVFFParserNode::verify_supported_format()
{
    switch (wavinfo.AudioFormat)
    {
        case PVWAV_PCM_AUDIO_FORMAT:
        case PVWAV_ITU_G711_ALAW:
        case PVWAV_ITU_G711_ULAW:
            return true;

        default:
            break;
    }
    return false;
}

/**
 //Called by the command handler AO to do the node Prepare
 */
void PVMFWAVFFParserNode::DoPrepare(PVMFWAVFFNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
            //this node doesn't need to do anything to get ready
            //to start.
            SetState(EPVMFNodePrepared);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFWAVFFParserNode::DoPrepare: Error -- wrong state %d\n", iInterfaceState));
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

void PVMFWAVFFParserNode::DoStart(PVMFWAVFFNodeCommand& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
            SetState(EPVMFNodeStarted);
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    RunIfNotReady(1);
    CommandComplete(iInputCommands, aCmd, status);
}

void PVMFWAVFFParserNode::DoStop(PVMFWAVFFNodeCommand& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            // stop and reset position to beginning
            ResetAllTracks();

            // Reset the WAV FF to beginning
            if (iWAVParser)
            {
                iWAVParser->SeekPCMSample(0);
            }

            //clear msg queue
            if (iOutPort)
                iOutPort->ClearMsgQueues();
            // Clear scheduled port activities
            iPortActivityQueue.clear();

            //transition to Prepared state
            SetState(EPVMFNodePrepared);
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}


void PVMFWAVFFParserNode::DoPause(PVMFWAVFFNodeCommand& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
            SetState(EPVMFNodePaused);
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}

void PVMFWAVFFParserNode::DoFlush(PVMFWAVFFNodeCommand& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            //the flush is asynchronous.  move the command from
            //the input command queue to the current command, where
            //it will remain until the flush completes.

            int32 err;
            OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
            if (err != OsclErrNone)
            {
                CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
                return;
            }
            iInputCommands.Erase(&aCmd);
            break;

        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}


/**
 //A routine to tell if a flush operation is in progress.
 */
bool PVMFWAVFFParserNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

void PVMFWAVFFParserNode::DoReset(PVMFWAVFFNodeCommand& aCmd)
{
    ReleaseAllPorts();
    //discard any port activity events
    iPortActivityQueue.clear();
    CleanupFileSource();
    SetState(EPVMFNodeIdle);
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);

}

void PVMFWAVFFParserNode::DoCancelAllCommands(PVMFWAVFFNodeCommand& aCmd)
{
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
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
 //Called by the command handler AO to do the Cancel single command
 */
void PVMFWAVFFParserNode::DoCancelCommand(PVMFWAVFFNodeCommand& aCmd)
{
    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFWAVFFNodeCommandBase::Parse(id);

    //first check "current" command if any
    {
        PVMFWAVFFNodeCommand* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iCurrentCommand, *cmd, PVMFErrCancelled);
            //report cancel success
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            return;
        }
    }

    //next check input queue.
    {
        //start at element 1 since this cancel command is element 0.
        PVMFWAVFFNodeCommand* cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            //cancel the queued command
            CommandComplete(iInputCommands, *cmd, PVMFErrCancelled);
            //report cancel success
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            return;
        }
    }
    //if we get here the command isn't queued so the cancel fails.
    CommandComplete(iInputCommands, aCmd, PVMFFailure);
}


void PVMFWAVFFParserNode::InitializeTrackStructure()
{
    // compute the number samples and buffer size that will be used for each message
    trackdata_num_samples = (wavinfo.SampleRate * PVWAV_MSEC_PER_BUFFER) / 1000; // samples in buffer
    trackdata_bufsize = trackdata_num_samples * wavinfo.BytesPerSample * wavinfo.NumChannels;

    iSelectedTrackList[0].iClockConverter = OSCL_NEW(MediaClockConverter, (wavinfo.SampleRate));
    iSelectedTrackList[0].iTrackDataMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (PVWAVFF_MEDIADATA_POOLNUM));
    iSelectedTrackList[0].iMediaDataImplAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iSelectedTrackList[0].iTrackDataMemoryPool));
    iSelectedTrackList[0].iMediaDataMemPool = OSCL_NEW(PVMFMemPoolFixedChunkAllocator, ("WavFFPar", PVWAVFF_MEDIADATA_POOLNUM, PVWAVFF_MEDIADATA_CHUNKSIZE));
    if (iSelectedTrackList[0].iMediaDataMemPool)
    {
        iSelectedTrackList[0].iMediaDataMemPool->enablenullpointerreturn();
    }

}


void PVMFWAVFFParserNode::DoRequestPort(PVMFWAVFFNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoRequestPort() In"));

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;
    aCmd.PVMFWAVFFNodeCommandBase::Parse(tag, portconfig);

    //validate the tag...
    if (tag != PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE)
    {
        //bad port tag
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFWAVFFParserNode::DoRequestPort: Error - Invalid port tag"));
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }


    // Allocate a port based on the request
    // Return the pointer to the port in the command complete message
    iCmdRespPort = NULL;

    //check the tag
    if ((int32)aCmd.iParam1 == PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE)
    {

        if (iOutPort)
        {
            // only supporting one track
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFWAVFFParserNode::DoRequestPort: Only supporting one output port"));
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return;
        }

        iOutPort = OSCL_NEW(PVMFWAVFFParserOutPort, (PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE, this));
        if (!iOutPort)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFWAVFFParserNode::DoRequestPort: Error - Can't create output port"));
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return;
        }

        if (iSelectedTrackList.size() > 0)
        {
            iSelectedTrackList[0].iTag = PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE;
            iSelectedTrackList[0].iPort = iOutPort;
        }
        else if (iWAVParser)
        {
            // this is the case if the port is destroyed and re-created after init
            // save the port information in the selected track list
            PVWAVFFNodeTrackPortInfo trackportinfo;
            trackportinfo.iTrackId = 0;  // Only support 1 track
            trackportinfo.iTag = PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE;
            trackportinfo.iPort = iOutPort;
            trackportinfo.iClockConverter = NULL;
            trackportinfo.iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED;
            trackportinfo.iTrackDataMemoryPool = NULL;
            trackportinfo.iMediaDataImplAlloc = NULL;
            trackportinfo.iMediaDataMemPool = NULL;
            trackportinfo.iNode = this;
            iSelectedTrackList.push_back(trackportinfo);

            int error;
            OSCL_TRY(error, InitializeTrackStructure());
            OSCL_FIRST_CATCH_ANY(error, PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFWAVFFParserNode::RequestPort: Error - initializing tracks"));
                                 OSCL_DELETE(iOutPort);
                                 iOutPort = NULL;
                                 iSelectedTrackList.erase(iSelectedTrackList.begin());
                                 OSCL_DELETE(iWAVParser);
                                 iWAVParser = NULL;
                                 SetState(EPVMFNodeError);
                                 CommandComplete(iInputCommands, aCmd, PVMFFailure);
                                 return;);

        }

        iPortActivityQueue.reserve(PVMF_WAVFFPARSER_NODE_COMMAND_VECTOR_RESERVE);

        iCmdRespPort = iOutPort;

    }
    else
    {
        // don't support other types yet
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFWAVFFParserNode::DoRequestPort: Error - unsupported port type"));
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)iCmdRespPort);
}

void PVMFWAVFFParserNode::DoReleasePort(PVMFWAVFFNodeCommand& aCmd)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFWAVFFParserNode::DoReleasePort"));

    // search for the matching port address
    // disconnect it, if needed
    // cleanup the buffers associated with it
    // delete the port
    // set the address to NULL

    // Remove the selected track from the track list
    int32 i = 0;
    int32 maxtrack = iSelectedTrackList.size();
    while (i < maxtrack)
    {
        if (iSelectedTrackList[i].iPort == aCmd.iParam1)
        {
            // Found the element. So erase it
            iSelectedTrackList[i].iMediaData.Unbind();
            if (iSelectedTrackList[i].iPort)
            {
                iSelectedTrackList[i].iPort->Disconnect();
                OSCL_DELETE(((PVMFWAVFFParserOutPort*)iSelectedTrackList[i].iPort));
                iSelectedTrackList[i].iPort = NULL;
                iOutPort = NULL;
            }
            OSCL_DELETE(iSelectedTrackList[i].iClockConverter);
            OSCL_DELETE(iSelectedTrackList[i].iTrackDataMemoryPool);
            OSCL_DELETE(iSelectedTrackList[i].iMediaDataImplAlloc);
            OSCL_DELETE(iSelectedTrackList[i].iMediaDataMemPool);
            iSelectedTrackList.erase(iSelectedTrackList.begin() + i);
            break;
        }
        ++i;
    }

    if (i >= maxtrack)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrBadHandle);
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}


void PVMFWAVFFParserNode::ResetAllTracks()
{
    for (uint32 i = 0; i < iSelectedTrackList.size(); ++i)
    {
        iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED;
        iSelectedTrackList[i].iMediaData.Unbind();
        iSelectedTrackList[i].iSeqNum = 0;
    }
}

bool PVMFWAVFFParserNode::ReleaseAllPorts()
{

    while (!iSelectedTrackList.empty())
    {
        if (iSelectedTrackList[0].iPort)
        {
            iSelectedTrackList[0].iPort->Disconnect();
        }
        iSelectedTrackList[0].iMediaData.Unbind();
        OSCL_DELETE(iSelectedTrackList[0].iClockConverter);
        OSCL_DELETE(iSelectedTrackList[0].iTrackDataMemoryPool);
        OSCL_DELETE(iSelectedTrackList[0].iMediaDataImplAlloc);
        OSCL_DELETE(iSelectedTrackList[0].iMediaDataMemPool);
        iSelectedTrackList.erase(iSelectedTrackList.begin());
    }

    if (iOutPort)
    {
        OSCL_DELETE(iOutPort);
        iOutPort = NULL;
    }

    return true;
}

void PVMFWAVFFParserNode::CleanupFileSource()
{
    iAvailableMetadataKeys.clear();

    if (iWAVParser)
    {
        OSCL_DELETE(iWAVParser);
        iWAVParser = NULL;
    }
}


void PVMFWAVFFParserNode::CommandComplete(PVMFWAVFFNodeCmdQ& aCmdQ, PVMFWAVFFNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData, PVUuid* aEventUUID, int32* aEventCode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        int32 leavecode = 0;
        OSCL_TRY(leavecode, errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL)));
        if (leavecode == 0)
        {
            extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
        }
    }

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }
}


PVMFCommandId PVMFWAVFFParserNode::QueueCommandL(PVMFWAVFFNodeCommand& aCmd)
{
    if (IsAdded())
    {
        PVMFCommandId id;
        id = iInputCommands.AddL(aCmd);
        /* Wakeup the AO */
        RunIfNotReady();
        return id;
    }
    OSCL_LEAVE(OsclErrInvalidState);
    return -1;
}


bool PVMFWAVFFParserNode::HandleOutgoingQueueReady(PVMFPortInterface* aPortInterface)
{
    uint32 i = 0;
    while (i++ < iSelectedTrackList.size())
    {
        if (iSelectedTrackList[i].iPort == aPortInterface &&
                iSelectedTrackList[i].iState == PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA)
        {
            // Found the element and right state
            // re-send the data
            if (!SendTrackData(iSelectedTrackList[i]))
            {
                return false;
            }

            // Success in re-sending the data, and change state to getdata
            iSelectedTrackList[i].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
            return true;
        }
    }

    // No match found.
    return false;
}


/**
//////////////////////////////////////////////////
// Extension interface implementation
//////////////////////////////////////////////////
*/

void PVMFWAVFFParserNode::addRef()
{
    ++iExtensionRefCount;
}

void PVMFWAVFFParserNode::removeRef()
{
    --iExtensionRefCount;
}

PVMFStatus PVMFWAVFFParserNode::QueryInterfaceSync(PVMFSessionId aSession,
        const PVUuid& aUuid,
        PVInterface*& aInterfacePtr)
{
    OSCL_UNUSED_ARG(aSession);
    aInterfacePtr = NULL;
    if (queryInterface(aUuid, aInterfacePtr))
    {
        aInterfacePtr->addRef();
        return PVMFSuccess;
    }
    return PVMFErrNotSupported;
}

bool PVMFWAVFFParserNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (PVMF_DATA_SOURCE_INIT_INTERFACE_UUID == uuid)
    {
        PVMFDataSourceInitializationExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFDataSourceInitializationExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (PVMF_TRACK_SELECTION_INTERFACE_UUID == uuid)
    {
        PVMFTrackSelectionExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFTrackSelectionExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (KPVMFMetadataExtensionUuid == uuid)
    {
        PVMFMetadataExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (PvmfDataSourcePlaybackControlUuid == uuid)
    {
        PvmfDataSourcePlaybackControlInterface* myInterface = OSCL_STATIC_CAST(PvmfDataSourcePlaybackControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }
    return true;
}

PVMFStatus PVMFWAVFFParserNode::SetSourceInitializationData(OSCL_wString& aSourceURL, PVMFFormatType& aSourceFormat, OsclAny* aSourceData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::SetSourceInitializationData() called"));

    OSCL_UNUSED_ARG(aSourceData);

    if (aSourceFormat != PVMF_MIME_WAVFF)
    {
        return PVMFFailure;
    }

    iFilename = aSourceURL;
    return PVMFSuccess;
}

PVMFStatus PVMFWAVFFParserNode::SetClientPlayBackClock(PVMFMediaClock* aClientClock)
{
    OSCL_UNUSED_ARG(aClientClock);
    return PVMFSuccess;
}

PVMFStatus PVMFWAVFFParserNode::SetEstimatedServerClock(PVMFMediaClock* aClientClock)
{
    OSCL_UNUSED_ARG(aClientClock);
    return PVMFSuccess;
}

//From PVMFTrackSelectionExtensionInterface
PVMFStatus PVMFWAVFFParserNode::GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetMediaPresentationInfo() called"));

    // Check to make sure the WAV file has been parsed
    if (!iWAVParser)
    {
        return PVMFFailure;
    }

    uint32 duration_sec = wavinfo.NumSamples / wavinfo.SampleRate;
    uint32 duration_msec = wavinfo.NumSamples % wavinfo.SampleRate;
    uint32 duration = (duration_msec * 1000) / wavinfo.NumSamples + duration_sec * 1000 ;

    aInfo.setDurationValue(duration);
    // Current version of WAV parser is limited to 1 channel
    PVMFTrackInfo tmpTrackInfo;

    // set the port tag for this track
    tmpTrackInfo.setPortTag(PVMF_WAVFFPARSER_NODE_PORT_TYPE_SOURCE);

    // track id
    tmpTrackInfo.setTrackID(0);

    // check supported formats

    // bitrate
    tmpTrackInfo.setTrackBitRate(wavinfo.ByteRate*8 /* convert bytes to bits */);

    // timescale
    tmpTrackInfo.setTrackDurationTimeScale((uint64)wavinfo.SampleRate);

    // config info

    // mime type
    OSCL_FastString mime_type;

    if (wavinfo.AudioFormat == PVWAV_PCM_AUDIO_FORMAT)
    {
        if (wavinfo.BitsPerSample == 8)
        {
            mime_type = _STRLIT_CHAR(PVMF_MIME_PCM8);
        }
        else if (wavinfo.isLittleEndian)
        {
            mime_type = _STRLIT_CHAR(PVMF_MIME_PCM16);
        }
        else
        {
            mime_type = _STRLIT_CHAR(PVMF_MIME_PCM16_BE);
        }
    }
    else if (wavinfo.AudioFormat == PVWAV_ITU_G711_ALAW)
    {
        mime_type =  _STRLIT_CHAR(PVMF_MIME_ALAW);
    }
    else if (wavinfo.AudioFormat == PVWAV_ITU_G711_ULAW)
    {
        mime_type = _STRLIT_CHAR(PVMF_MIME_ULAW);
    }
    else
    {
        return PVMFFailure;
    }

    tmpTrackInfo.setTrackMimeType(mime_type);

    // add the track
    aInfo.addTrackInfo(tmpTrackInfo);

    return PVMFSuccess;
}

PVMFStatus PVMFWAVFFParserNode::SelectTracks(PVMFMediaPresentationInfo& aInfo)
{
    OSCL_UNUSED_ARG(aInfo);
    return PVMFSuccess;
}

///////////// Implementation of PVWAVFFNodeTrackPortInfo //////////////////////
void PVWAVFFNodeTrackPortInfo::freechunkavailable(OsclAny*)
{
    // Activate the parent node if necessary
    if (iNode)
    {
        if (iNode->IsAdded())
        {
            iNode->RunIfNotReady();
        }
    }
}


// From PvmfDataSourcePlaybackControlInterface
PVMFCommandId PVMFWAVFFParserNode::SetDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, PVMFTimestamp& aActualMediaDataTS, bool aSeekToSyncPoint, uint32 aStreamID, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFWAVParserNode::SetDataSourcePosition: aTargetNPT=%d, aSeekToSyncPoint=%d, aContext=0x%x",
                     aTargetNPT, aSeekToSyncPoint, aContext));

    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommand::Construct(aSessionId, PVWAVFF_NODE_CMD_SETDATASOURCEPOSITION, aTargetNPT, &aActualNPT,  &aActualMediaDataTS, aSeekToSyncPoint, aStreamID, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFWAVFFParserNode::QueryDataSourcePosition(PVMFSessionId aSessionId, PVMFTimestamp aTargetNPT, PVMFTimestamp& aActualNPT, bool aSeekToSyncPoint, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFWAVParserNode::QueryDataSourcePosition: aTargetNPT=%d, aSeekToSyncPoint=%d, aContext=0x%x",
                     aTargetNPT, aSeekToSyncPoint, aContext));

    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommand::Construct(aSessionId, PVWAVFF_NODE_CMD_QUERYDATASOURCEPOSITION, aTargetNPT, &aActualNPT, aSeekToSyncPoint, aContext);
    return QueueCommandL(cmd);
}

PVMFCommandId PVMFWAVFFParserNode::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aSeekPointBeforeTargetNPT,
        PVMFTimestamp& aSeekPointAfterTargetNPT,
        OsclAny* aContext,
        bool aSeekToSyncPoint)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFWAVParserNode::QueryDataSourcePosition: aTargetNPT=%d, aSeekToSyncPoint=%d, aContext=0x%x",
                     aTargetNPT, aSeekToSyncPoint,  aContext));
    OSCL_UNUSED_ARG(aSeekPointAfterTargetNPT);

    PVMFWAVFFNodeCommand cmd;
    // Construct not changed, aSeekPointBeforeTargetNPThas replace aAcutalNPT
    cmd.PVMFWAVFFNodeCommand::Construct(aSessionId, PVWAVFF_NODE_CMD_QUERYDATASOURCEPOSITION, aTargetNPT, &aSeekPointBeforeTargetNPT, aSeekToSyncPoint, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFWAVFFParserNode::SetDataSourceRate(PVMFSessionId aSessionId, int32 aRate, PVMFTimebase* aTimebase, OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::SetDataSourceRate() called"));

    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommand::Construct(aSessionId, PVWAVFF_NODE_CMD_SETDATASOURCERATE, aRate, aTimebase, aContext);
    return QueueCommandL(cmd);
}

void PVMFWAVFFParserNode::DoSetDataSourcePosition(PVMFWAVFFNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoSetDataSourcePosition() In"));

    uint32 targetNPT = 0;
    uint32* actualNPT = NULL;
    uint32* actualMediaDataTS = NULL;
    bool jumpToIFrame = false;
    uint32 streamID = 0;
    aCmd.PVMFWAVFFNodeCommand::Parse(targetNPT, actualNPT, actualMediaDataTS, jumpToIFrame, streamID);

    uint32 i = 0;
    for (i = 0; i < iSelectedTrackList.size(); ++i)
    {
        iSelectedTrackList[i].iSendBOS = true;
    }
    //save the stream id for next media segment
    iStreamID = streamID;


    *actualMediaDataTS = iSelectedTrackList[0].iClockConverter->get_converted_ts(COMMON_PLAYBACK_CLOCK_TIMESCALE);

    // see if targetNPT is greater than or equal to clip duration.
    uint32 duration_sec = wavinfo.NumSamples / wavinfo.SampleRate;
    uint32 duration_msec = wavinfo.NumSamples % wavinfo.SampleRate;
    uint32 duration = (duration_msec * 1000) / wavinfo.NumSamples + duration_sec * 1000 ;
    uint32 tempTargetNPT = targetNPT;
    if (tempTargetNPT >= duration)
    {
        // need to reset the tracks to zero.
        targetNPT = 0;
        // reset the track to zero and then report EOS for the tracks.
    }

    // compute the sample corresponding to the target NPT
    // NPT is in milliseconds so divide by 1000
    uint32 target_sec = targetNPT / 1000;
    uint32 target_millisec = targetNPT % 1000;
    uint32 target_sample = (target_millisec * wavinfo.SampleRate) / 1000 + target_sec * wavinfo.SampleRate;

    if (target_sample > wavinfo.NumSamples)
    {
        // limit it to the end of the clip
        target_sample = wavinfo.NumSamples;
    }

    {
        uint32 position_sec = target_sample / wavinfo.SampleRate;
        uint32 position_msec = target_sample % wavinfo.SampleRate;
        *actualNPT = (position_msec * 1000) / wavinfo.NumSamples + position_sec * 1000;
        if (tempTargetNPT >= duration)
        {
            // need to send duration in *actualNPT
            *actualNPT = duration;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode::DoSetDataSourcePosition() targetNPT=%d, target sample=%d, actualNPT=%d, actualMediaTS=%d", targetNPT, target_sample, *actualNPT, *actualMediaDataTS));


    PVWavParserReturnCode retval = iWAVParser->SeekPCMSample(target_sample);
    // reposition the file
    if (PVWAVPARSER_OK != retval)
    {
        PVUuid eventuuid;
        int32 eventcode;
        if (!MapWAVErrorCodeToEventCode(retval, eventuuid, eventcode))
        {
            eventuuid = PVMFFileFormatEventTypesUUID;
            eventcode = PVMFFFErrMisc;
        }
        CommandComplete(iInputCommands, aCmd, PVMFErrResource, NULL, &eventuuid, &eventcode);
    }
    else
    {
        if (tempTargetNPT >= duration)
        {
            // we need to send EOS for the tracks. Change the trackstate to EndOftrack.
            for (uint32 ii = 0; ii < iSelectedTrackList.size(); ++ii)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode::DoSetDataSourcePosition targetNPT >= duration , report BOS-EOS"));
                iSelectedTrackList[ii].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
            }
        }
        else
        {
            // reset the state of the track if they are currently at the end of the file
            for (uint32 ii = 0; ii < iSelectedTrackList.size(); ++ii)
            {
                if ((PVWAVFFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK == iSelectedTrackList[ii].iState) ||
                        (PVWAVFFNodeTrackPortInfo::TRACKSTATE_ENDOFTRACK == iSelectedTrackList[ii].iState))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFWAVFFParserNode::DoSetDataSourcePosition() reseting track state of track %d from endoftrack", ii));
                    iSelectedTrackList[ii].iState = PVWAVFFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
                }
            }
        }
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoSetDataSourcePosition() Out"));

}


void PVMFWAVFFParserNode::DoQueryDataSourcePosition(PVMFWAVFFNodeCommand& aCmd)
{
    uint32 targetNPT = 0;
    uint32* actualNPT = NULL;
    bool seektosyncpoint = false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::DoQueryDataSourcePosition() In"));

    aCmd.PVMFWAVFFNodeCommand::Parse(targetNPT, actualNPT, seektosyncpoint);
    if (actualNPT == NULL)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
        return;
    }

    // compute the sample corresponding to the target NPT
    // NPT is in milliseconds so divide by 1000
    uint32 target_sec = targetNPT / 1000;
    uint32 target_millisec = targetNPT % 1000;
    uint32 target_sample = (target_millisec * wavinfo.SampleRate) / 1000 + target_sec * wavinfo.SampleRate;

    if (target_sample > wavinfo.NumSamples)
    {
        // limit it to the end of the clip
        target_sample = wavinfo.NumSamples;
    }

    {
        uint32 position_sec = target_sample / wavinfo.SampleRate;
        uint32 position_msec = target_sample % wavinfo.SampleRate;
        *actualNPT = (position_msec * 1000) / wavinfo.NumSamples + position_sec * 1000;
    }

    CommandComplete(iInputCommands, aCmd, PVMFSuccess);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP4FFParserNode::DoQueryDataSourcePosition() Out"));
}


void PVMFWAVFFParserNode::DoSetDataSourceRate(PVMFWAVFFNodeCommand& aCmd)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoSetDataSourceRate() In"));

    // Retrieve the new rate
    int32 rate;
    PVMFTimebase* timebase = NULL;
    PVMFStatus cmdstatus = PVMFSuccess;
    aCmd.PVMFWAVFFNodeCommand::Parse(rate, timebase);

    if (timebase == NULL)
    {
        if (rate < MIN_WAVFFPARSER_RATE || rate > MAX_WAVFFPARSER_RATE)
        {
            // Limit to MIN and MAX RATES for now.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFWAVFFParserNode::DoSetDataSourceRate() Invalid playback rate %d valid range is [%d, %d]", rate, MIN_WAVFFPARSER_RATE, MAX_WAVFFPARSER_RATE));
            cmdstatus = PVMFErrNotSupported;
        }
    }
    else
    {
        // Allow outside timebase
    }

    // complete the command
    CommandComplete(iInputCommands, aCmd, cmdstatus);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoSetDataSourceRate() Out"));

}


bool PVMFWAVFFParserNode::MapWAVErrorCodeToEventCode(int32 aWAVErrCode, PVUuid& aEventUUID, int32& aEventCode)
{

    switch (aWAVErrCode)
    {
        case PVWAVPARSER_UNSUPPORTED_FORMAT:
            aEventUUID = PVMFFileFormatEventTypesUUID;
            aEventCode = PVMFFFErrFileRead;
            break;

        case PVWAVPARSER_READ_ERROR:
            aEventUUID = PVMFFileFormatEventTypesUUID;
            aEventCode = PVMFFFErrFileRead;
            break;

        case PVWAVPARSER_MISC_ERROR:
            aEventUUID = PVMFFileFormatEventTypesUUID;
            aEventCode = PVMFFFErrNotSupported;
            break;

        case PVWAVPARSER_OK:
            OSCL_ASSERT(false); // Should not pass this "error" code to this function

        default:
            return false;
    }

    return true;
}



bool PVMFWAVFFParserNode::SendEndOfTrackCommand(PVWAVFFNodeTrackPortInfo& aTrackPortInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::SendEndOfTrackCommand() In"));

    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();

    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

    sharedMediaCmdPtr->setStreamID(iStreamID);

    // Set timestamp
    uint32 timestamp = aTrackPortInfo.iClockConverter->get_converted_ts(COMMON_PLAYBACK_CLOCK_TIMESCALE);
    sharedMediaCmdPtr->setTimestamp(timestamp);

    // Set the sequence number
    sharedMediaCmdPtr->setSeqNum(aTrackPortInfo.iSeqNum++);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);

    if (aTrackPortInfo.iPort->QueueOutgoingMsg(mediaMsgOut) != PVMFSuccess)
    {
        // Output queue is busy, so wait for the output queue being ready
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFWAVFFParserNode::SendEndOfTrackCommand: Outgoing queue busy. "));
        return false;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::SendEndOfTrackCommand() Out"));
    return true;
}


bool PVMFWAVFFParserNode::SendBeginOfMediaStreamCommand(PVWAVFFNodeTrackPortInfo& aTrackPortInfo)
{
    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);

    uint32 timestamp = aTrackPortInfo.iClockConverter->get_converted_ts(COMMON_PLAYBACK_CLOCK_TIMESCALE);
    sharedMediaCmdPtr->setTimestamp(timestamp);

    uint32 seqNum = 0;
    sharedMediaCmdPtr->setSeqNum(seqNum);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
    mediaMsgOut->setStreamID(iStreamID);

    if (aTrackPortInfo.iPort->QueueOutgoingMsg(mediaMsgOut) != PVMFSuccess)
    {
        // Output queue is busy, so wait for the output queue being ready
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFWAVFFParserNode::SendBeginOfMediaStreamCommand: Outgoing queue busy. "));
        return false;
    }
    aTrackPortInfo.iSendBOS = false;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::SendBeginOfMediaStreamCommand() Out StreamId %d ", iStreamID));
    return true;
}


// Metadata handling

uint32 PVMFWAVFFParserNode::GetNumMetadataKeys(char* aQueryKeyString)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetNumMetadataKeys() called"));

    uint32 num_entries = 0;

    if (aQueryKeyString == NULL)
    {
        // No query key so just return all the available keys
        num_entries = iAvailableMetadataKeys.size();
    }
    else
    {
        // Determine the number of metadata keys based on the query key string provided
        for (uint32 i = 0; i < iAvailableMetadataKeys.size(); i++)
        {
            // Check if the key matches the query key
            if (pv_mime_strcmp(iAvailableMetadataKeys[i].get_cstr(), aQueryKeyString) >= 0)
            {
                num_entries++;
            }
        }
    }

    return num_entries;
}


uint32 PVMFWAVFFParserNode::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetNumMetadataValues() called"));

    uint32 numkeys = aKeyList.size();

    if (iWAVParser == NULL || numkeys == 0)
    {
        return 0;
    }

    // Count the number of metadata value entries based on the key list provided
    uint32 numvalentries = 0;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_DURATION_KEY) == 0 &&
                wavinfo.NumSamples > 0 && wavinfo.SampleRate > 0)
        {
            // Duration
            ++numvalentries;
        }
        else if (oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_NUMTRACKS_KEY) == 0)
        {
            // Number of tracks
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_BITRATE_KEY) == 0) &&
                 wavinfo.BitsPerSample > 0 && wavinfo.SampleRate > 0)
        {
            // Bitrate
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY) == 0) &&
                 wavinfo.NumChannels > 0)
        {
            // Number of channels
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY) == 0) &&
                 wavinfo.SampleRate > 0)
        {
            // Sampling rate
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY) == 0) &&
                 wavinfo.BitsPerSample > 0)
        {
            // Bits per sample
            ++numvalentries;
        }
        else if ((oscl_strcmp(aKeyList[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY) == 0) &&
                 wavinfo.AudioFormat != 0)
        {
            // Format
            ++numvalentries;
        }
    }

    return numvalentries;
}


PVMFCommandId PVMFWAVFFParserNode::GetNodeMetadataKeys(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, uint32 starting_index, int32 max_entries, char* query_key, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetNodeMetadataKeys() called"));

    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommand::Construct(aSessionId, PVWAVFF_NODE_CMD_GETNODEMETADATAKEY, aKeyList, starting_index, max_entries, query_key, aContext);
    return QueueCommandL(cmd);
}


PVMFCommandId PVMFWAVFFParserNode::GetNodeMetadataValues(PVMFSessionId aSessionId, PVMFMetadataList& aKeyList, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, uint32 starting_index, int32 max_entries, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::GetNodeMetadataValue() called"));

    PVMFWAVFFNodeCommand cmd;
    cmd.PVMFWAVFFNodeCommand::Construct(aSessionId, PVWAVFF_NODE_CMD_GETNODEMETADATAVALUE, aKeyList, aValueList, starting_index, max_entries, aContext);
    return QueueCommandL(cmd);
}

// From PVMFMetadataExtensionInterface
PVMFStatus PVMFWAVFFParserNode::ReleaseNodeMetadataKeys(PVMFMetadataList& ,
        uint32 ,
        uint32)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::ReleaseNodeMetadataKeys() called"));
    //nothing needed-- there's no dynamic allocation in this node's key list
    return PVMFSuccess;
}

// From PVMFMetadataExtensionInterface
PVMFStatus PVMFWAVFFParserNode::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 start,
        uint32 end)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::ReleaseNodeMetadataValues() called"));

    if (start > end || aValueList.size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFWAVFFParserNode::ReleaseNodeMetadataValues() Invalid start/end index"));
        return PVMFErrArgument;
    }

    if (end >= aValueList.size())
    {
        end = aValueList.size() - 1;
    }

    for (uint32 i = start; i <= end; i++)
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

                case PVMI_KVPVALTYPE_UINT32:
                case PVMI_KVPVALTYPE_UINT8:
                    // No memory to free for these valtypes
                    break;

                default:
                    // Should not get a value that wasn't created from here
                    break;
            }

            OSCL_ARRAY_DELETE(aValueList[i].key);
            aValueList[i].key = NULL;
        }
    }

    return PVMFSuccess;
}

PVMFStatus PVMFWAVFFParserNode::DoGetNodeMetadataKey(PVMFWAVFFNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoGetNodeMetadataKey() In"));

    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key;

    aCmd.PVMFWAVFFNodeCommand::Parse(keylistptr, starting_index, max_entries, query_key);

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

    for (uint32 lcv = 0; lcv < iAvailableMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            // No query key so this key is counted
            ++num_entries;
            if (num_entries > starting_index)
            {
                // Past the starting index so copy the key
                PVMFStatus status = PushBackMetadataKeys(keylistptr, lcv);
                if (PVMFErrNoMemory == status)
                {
                    return status;
                }
                num_added++;
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
                    // Past the starting index so copy the key
                    PVMFStatus status = PushBackMetadataKeys(keylistptr, lcv);
                    if (PVMFErrNoMemory == status)
                    {
                        return status;
                    }
                    num_added++;
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



PVMFStatus PVMFWAVFFParserNode::DoGetNodeMetadataValue(PVMFWAVFFNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::DoGetMetadataValue() In"));

    if (iWAVParser == NULL)
    {
        return PVMFErrInvalidState;
    }

    PVMFMetadataList* keylistptr_in = NULL;
    PVMFMetadataList* keylistptr = NULL;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    aCmd.PVMFWAVFFNodeCommand::Parse(keylistptr_in, valuelistptr, starting_index, max_entries);

    // Check the parameters
    if (keylistptr_in == NULL || valuelistptr == NULL)
    {
        return PVMFErrArgument;
    }

    keylistptr = keylistptr_in;
    //If numkeys is one, just check to see if the request
    //is for ALL metadata
    if (keylistptr_in->size() == 1)
    {
        if (oscl_strncmp((*keylistptr)[0].get_cstr(),
                         PVWAVMETADATA_ALL_METADATA_KEY,
                         oscl_strlen(PVWAVMETADATA_ALL_METADATA_KEY)) == 0)
        {
            //use the complete metadata key list
            keylistptr = &iAvailableMetadataKeys;
        }
    }
    uint32 numkeys = keylistptr->size();

    if (starting_index > (numkeys - 1) || numkeys <= 0 || max_entries == 0)
    {
        // Don't do anything
        return PVMFErrArgument;
    }

    uint32 numvalentries = 0;
    int32 numentriesadded = 0;
    for (uint32 lcv = 0; lcv < numkeys; lcv++)
    {
        int32 leavecode = 0;
        PvmiKvp KeyVal;
        KeyVal.key = NULL;
        uint32 KeyLen = 0;

        if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_DURATION_KEY) == 0 &&
                wavinfo.NumSamples > 0 && wavinfo.SampleRate > 0)
        {
            // Duration
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_DURATION_KEY) + 1; // for "duration;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32;"
                KeyLen += oscl_strlen(PVWAVMETADATA_TIMESCALE1000) + 1; // for "timescale=1000" and NULL terminator

                // Allocate memory for the string
                leavecode = CreateNewArray(KeyVal.key, KeyLen);
                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_DURATION_KEY, oscl_strlen(PVWAVMETADATA_DURATION_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_TIMESCALE1000, oscl_strlen(PVWAVMETADATA_TIMESCALE1000));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value- use uint64 in case of large sample count
                    {
                        uint32 duration_sec = wavinfo.NumSamples / wavinfo.SampleRate;
                        uint32 duration_msec = wavinfo.NumSamples % wavinfo.SampleRate;
                        uint32 duration = (duration_msec * 1000) / wavinfo.NumSamples + duration_sec * 1000 ;
                        KeyVal.value.uint32_value = duration;
                    }
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if (oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_NUMTRACKS_KEY) == 0)
        {
            // Number of tracks
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_NUMTRACKS_KEY) + 1; // for "num-tracks;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = CreateNewArray(KeyVal.key, KeyLen);
                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_NUMTRACKS_KEY, oscl_strlen(PVWAVMETADATA_NUMTRACKS_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value
                    KeyVal.value.uint32_value = 1;  // Currently only one track WAV files are supported
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_BITRATE_KEY) == 0) &&
                 wavinfo.BitsPerSample > 0 && wavinfo.SampleRate > 0)
        {
            // Bitrate
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_TRACKINFO_BITRATE_KEY) + 1; // for "track-info/bit-rate;"
                KeyLen += oscl_strlen(PVWAVMETADATA_INDEX0) + 1; // for "index=0;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = CreateNewArray(KeyVal.key, KeyLen);

                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_TRACKINFO_BITRATE_KEY, oscl_strlen(PVWAVMETADATA_TRACKINFO_BITRATE_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_INDEX0, oscl_strlen(PVWAVMETADATA_INDEX0));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value
                    KeyVal.value.uint32_value = wavinfo.BitsPerSample * wavinfo.SampleRate; // Convert to bitrate in bits per sec
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY) == 0) &&
                 wavinfo.NumChannels > 0)
        {
            // Number of channels
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY) + 1; // for "track-info/audio/channels;"
                KeyLen += oscl_strlen(PVWAVMETADATA_INDEX0) + 1; // for "index=0;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = CreateNewArray(KeyVal.key, KeyLen);
                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY, oscl_strlen(PVWAVMETADATA_TRACKINFO_AUDIO_CHANNELS_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_INDEX0, oscl_strlen(PVWAVMETADATA_INDEX0));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value
                    KeyVal.value.uint32_value = wavinfo.NumChannels;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY) == 0) &&
                 wavinfo.SampleRate > 0)
        {
            // Sampling rate
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY) + 1; // for "track-info/sample-rate;"
                KeyLen += oscl_strlen(PVWAVMETADATA_INDEX0) + 1; // for "index=0;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = CreateNewArray(KeyVal.key, KeyLen);
                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY, oscl_strlen(PVWAVMETADATA_TRACKINFO_SAMPLERATE_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_INDEX0, oscl_strlen(PVWAVMETADATA_INDEX0));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value
                    KeyVal.value.uint32_value = wavinfo.SampleRate;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY) == 0) &&
                 wavinfo.BitsPerSample > 0)
        {
            // Bits per sample
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY) + 1; // for "track-info/audio/bits-per-sample;"
                KeyLen += oscl_strlen(PVWAVMETADATA_INDEX0) + 1; // for "index=0;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR) + 1; // for "uint32" and NULL terminator

                // Allocate memory for the string
                leavecode = CreateNewArray(KeyVal.key, KeyLen);

                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY, oscl_strlen(PVWAVMETADATA_TRACKINFO_AUDIO_BITSPERSAMPLE_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_INDEX0, oscl_strlen(PVWAVMETADATA_INDEX0));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_UINT32_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value
                    KeyVal.value.uint32_value = wavinfo.BitsPerSample;
                    // Set the length and capacity
                    KeyVal.length = 1;
                    KeyVal.capacity = 1;
                }
                else
                {
                    // Memory allocation failed
                    KeyVal.key = NULL;
                    break;
                }
            }
        }
        else if ((oscl_strcmp((*keylistptr)[lcv].get_cstr(), PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY) == 0) &&
                 wavinfo.AudioFormat != 0)
        {
            // Format
            // Increment the counter for the number of values found so far
            ++numvalentries;

            // Create a value entry if past the starting index
            if (numvalentries > starting_index)
            {
                KeyLen = oscl_strlen(PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY) + 1; // for "track-info/audio/format;"
                KeyLen += oscl_strlen(PVWAVMETADATA_INDEX0) + 1; // for "index=0;"
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR); // for "valtype="
                KeyLen += oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR) + 1; // for "char*" and NULL terminator

                uint32 valuelen = 0;
                switch (wavinfo.AudioFormat)
                {
                    case PVWAV_PCM_AUDIO_FORMAT:
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_PCM)) + 1; // Value string plus one for NULL terminator
                        break;

                    case PVWAV_ITU_G711_ALAW:
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_ALAW)) + 1; // Value string plus one for NULL terminator
                        break;

                    case PVWAV_ITU_G711_ULAW:
                        valuelen = oscl_strlen(_STRLIT_CHAR(PVMF_MIME_ULAW)) + 1; // Value string plus one for NULL terminator
                        break;

                    default:
                        // Should not enter here
                        OSCL_ASSERT(false);
                        valuelen = 1;
                        break;
                }

                // Allocate memory for the strings
                leavecode = CreateNewArray(KeyVal.key, KeyLen);
                if (0 == leavecode)
                {
                    leavecode = CreateNewArray(KeyVal.value.pChar_value, valuelen);
                }

                if (leavecode == 0)
                {
                    // Copy the key string
                    oscl_strncpy(KeyVal.key, PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY, oscl_strlen(PVWAVMETADATA_TRACKINFO_AUDIO_FORMAT_KEY) + 1);
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_INDEX0, oscl_strlen(PVWAVMETADATA_INDEX0));
                    oscl_strncat(KeyVal.key, PVWAVMETADATA_SEMICOLON, oscl_strlen(PVWAVMETADATA_SEMICOLON));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_STRING_CONSTCHAR));
                    oscl_strncat(KeyVal.key, PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR, oscl_strlen(PVMI_KVPVALTYPE_CHARPTR_STRING_CONSTCHAR));
                    KeyVal.key[KeyLen-1] = 0;
                    // Copy the value
                    switch (wavinfo.AudioFormat)
                    {
                        case PVWAV_PCM_AUDIO_FORMAT:
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_PCM), valuelen);
                            break;

                        case PVWAV_ITU_G711_ALAW:
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_ALAW), valuelen);
                            break;

                        case PVWAV_ITU_G711_ULAW:
                            oscl_strncpy(KeyVal.value.pChar_value, _STRLIT_CHAR(PVMF_MIME_ULAW), valuelen);
                            break;

                        default:
                            // Should not enter here
                            OSCL_ASSERT(false);
                            break;
                    }
                    KeyVal.value.pChar_value[valuelen-1] = 0;
                    // Set the length and capacity
                    KeyVal.length = valuelen;
                    KeyVal.capacity = valuelen;
                }
                else
                {
                    // Memory allocation failed so clean up
                    if (KeyVal.key)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.key);
                        KeyVal.key = NULL;
                    }
                    if (KeyVal.value.pChar_value)
                    {
                        OSCL_ARRAY_DELETE(KeyVal.value.pChar_value);
                    }
                    break;
                }
            }
        }

        if (KeyVal.key != NULL)
        {
            leavecode = PushBackKeyVal(valuelistptr, KeyVal);
            if (leavecode != 0)
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

    return PVMFSuccess;
}


PVMFStatus  PVMFWAVFFParserNode::NegotiateSettings(PvmiCapabilityAndConfig* configInterface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFWAVFFParserNode::NegotiateSettings() In"));

    if (!iWAVParser)
    {
        // settings aren't initialized so defer this for now
        return PVMFSuccess;
    }


    // setting the format, sample rate, and number of channels
    PvmiKvp kvp;
    PvmiKvp* retKvp = NULL; // for return value

#define WAVFF_MAX_PARAM_KEYLEN 128

    char buf[WAVFF_MAX_PARAM_KEYLEN];
    oscl_strncpy(buf, MOUT_AUDIO_FORMAT_KEY, WAVFF_MAX_PARAM_KEYLEN);
    buf[WAVFF_MAX_PARAM_KEYLEN-1] = 0;

    // set the format
    kvp.key = buf;

#define WAVFF_MAX_PARAM_VALLEN 128
    char valbuf[WAVFF_MAX_PARAM_VALLEN];
    const char* valptr;

    if (wavinfo.AudioFormat == PVWAV_PCM_AUDIO_FORMAT)
    {
        if (wavinfo.BitsPerSample == 8)
        {
            valptr = PVMF_MIME_PCM8;
        }
        else if (wavinfo.isLittleEndian)
        {
            valptr = PVMF_MIME_PCM16;
        }
        else
        {
            valptr = PVMF_MIME_PCM16_BE;
        }
    }
    else if (wavinfo.AudioFormat == PVWAV_ITU_G711_ALAW)
    {
        valptr = PVMF_MIME_ALAW;
    }
    else if (wavinfo.AudioFormat == PVWAV_ITU_G711_ULAW)
    {
        valptr = PVMF_MIME_ULAW;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFWAVFFParserNode::NegotiateSettings() - unknown/unsupported format %d", wavinfo.AudioFormat));
        return PVMFFailure;
    }

    oscl_strncpy(valbuf, valptr, WAVFF_MAX_PARAM_VALLEN);
    valbuf[WAVFF_MAX_PARAM_VALLEN-1] = 0;

    kvp.length = kvp.capacity = oscl_strlen(valbuf);

    kvp.value.pChar_value = valbuf;

    int32 err = 0;
    retKvp = NULL;
    OSCL_TRY(err, configInterface->setParametersSync(0, &kvp, 1, retKvp););

    if (err != OsclErrNone || retKvp)
    {
        return PVMFFailure;
    }

    kvp.length = kvp.capacity = 0;

    // set the sample rate
    kvp.value.uint32_value = wavinfo.SampleRate;

    oscl_strncpy(buf, MOUT_AUDIO_SAMPLING_RATE_KEY, WAVFF_MAX_PARAM_KEYLEN);
    buf[WAVFF_MAX_PARAM_KEYLEN-1] = 0;

    kvp.key = buf;

    err = 0;
    retKvp = NULL;
    OSCL_TRY(err, configInterface->setParametersSync(0, &kvp, 1, retKvp););

    if (err != OsclErrNone || retKvp)
    {
        return PVMFFailure;
    }

    // set the number of channels

    kvp.value.uint32_value = wavinfo.NumChannels;

    oscl_strncpy(buf, MOUT_AUDIO_NUM_CHANNELS_KEY, WAVFF_MAX_PARAM_KEYLEN);
    buf[WAVFF_MAX_PARAM_KEYLEN-1] = 0;

    kvp.key = buf;

    err = 0;
    retKvp = NULL;
    OSCL_TRY(err, configInterface->setParametersSync(0, &kvp, 1, retKvp););

    if (err != OsclErrNone || retKvp)
    {
        return PVMFFailure;
    }

    return PVMFSuccess;
}


int32 PVMFWAVFFParserNode::PushBackPortActivity(PVMFPortActivity &aActivity)
{
    int32 err = OsclErrNone;
    OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
    return err;
}

int32 PVMFWAVFFParserNode::CreateNewArray(char*& aPtr, int32 aLen)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode,
             aPtr = OSCL_ARRAY_NEW(char, aLen););
    return leavecode;
}

int32 PVMFWAVFFParserNode::PushBackKeyVal(Oscl_Vector<PvmiKvp, OsclMemAllocator>*& aValueListPtr, PvmiKvp &aKeyVal)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, (*aValueListPtr).push_back(aKeyVal));
    return leavecode;
}

PVMFStatus PVMFWAVFFParserNode::PushBackMetadataKeys(PVMFMetadataList *&aKeyListPtr, uint32 aLcv)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, aKeyListPtr->push_back(iAvailableMetadataKeys[aLcv]));
    OSCL_FIRST_CATCH_ANY(leavecode,	PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFRMFFParserNode::DoGetMetadataKeys() Memory allocation failure when copying metadata key"));return PVMFErrNoMemory);

    return PVMFSuccess;
}

