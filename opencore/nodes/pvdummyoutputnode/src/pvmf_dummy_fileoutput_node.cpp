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
 *
 * @file pvmf_dummy_fileoutput_node.cpp
 * @brief Simple dummy file output node. Does not Write incoming data to any
 *   file
 *
 */

#include "pvmf_dummy_fileoutput_inport.h"
#include "pvmf_dummy_fileoutput_node.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pvmf_timedtext.h"

////////////////////////////////////////////////////////////////////////////
PVMFDummyFileOutputNode::PVMFDummyFileOutputNode(int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFDummyFileOutputNode")
        , iCmdIdCounter(0)
        , iInPort(NULL)
        , iFileOpened(0)
        , iFirstMediaData(false)
        , iLogger(NULL)
        , iFormat(PVMF_MIME_FORMAT_UNKNOWN)
        , iExtensionRefCount(0)
        , iMaxFileSizeEnabled(false)
        , iMaxDurationEnabled(false)
        , iMaxFileSize(0)
        , iMaxDuration(0)
        , iFileSize(0)
        , iFileSizeReportEnabled(false)
        , iDurationReportEnabled(false)
        , iFileSizeReportFreq(0)
        , iDurationReportFreq(0)
        , iNextFileSizeReport(0)
        , iNextDurationReport(0)
        , iClock(NULL)
        , iEarlyMargin(DEFAULT_EARLY_MARGIN)
        , iLateMargin(DEFAULT_LATE_MARGIN)
{
    ConstructL();
    int32 err;
    OSCL_TRY(err,

             //Create the input command queue.  Use a reserve to avoid lots of
             //dynamic memory allocation.
             iInputCommands.Construct(PVMF_FILE_OUTPUT_NODE_COMMAND_ID_START, PVMF_FILE_OUTPUT_NODE_COMMAND_VECTOR_RESERVE);

             //Create the "current command" queue.  It will only contain one
             //command at a time, so use a reserve of 1.
             iCurrentCommand.Construct(0, 1);

             //Create the port vector.
             iPortVector.Construct(PVMF_FILE_OUTPUT_NODE_PORT_VECTOR_RESERVE);

             //Set the node capability data.
             //This node can support an unlimited number of ports.
             iCapability.iCanSupportMultipleInputPorts = false;
             iCapability.iCanSupportMultipleOutputPorts = false;
             iCapability.iHasMaxNumberOfPorts = true;
             iCapability.iMaxNumberOfPorts = 1;
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
    ChangeNodeState(EPVMFNodeCreated);
}

////////////////////////////////////////////////////////////////////////////
PVMFDummyFileOutputNode::~PVMFDummyFileOutputNode()
{
    //thread logoff
    if (IsAdded())
        RemoveFromScheduler();

    //Cleanup allocated interfaces


    //Cleanup allocated ports
    if (iInPort)
    {
        OSCL_DELETE(((PVMFDummyFileOutputInPort*)iInPort));
        iInPort = NULL;
    }

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


    if (iAlloc)
    {
        OSCL_DELETE(iAlloc);
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::ConstructL()
{
    iAlloc = (Oscl_DefAlloc*)(new PVMFFileOutputAlloc());
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFDummyFileOutputNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:ThreadLogoff"));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
                RemoveFromScheduler();
            iLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code

        default:
            return PVMFErrInvalidState;
            // break;	This break statement was removed to avoid compiler warning for Unreachable Code
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::CloseOutputFile()
{
    // Close output file
    if (iFileOpened)
    {
        iOutputFile.Close();
        iFs.Close();
        iFileOpened = 0;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Init"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Prepare"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:GetCapability"));
    iCapability.iInputFormatCapability.clear();

    if (iFormat != PVMF_MIME_FORMAT_UNKNOWN)
    {
        // Format is already set, so return only that one
        iCapability.iInputFormatCapability.push_back(iFormat);
    }
    else
    {
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMR_IETF);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_M4V);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM8);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM16);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_YUV420);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_ADTS);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_H2632000);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_3GPP_TIMEDTEXT);
    }
    aNodeCapability = iCapability;
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:QueryUUID"));

    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:QueryInterface"));

    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:RequestPort"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:ReleasePort"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFPortIter* PVMFDummyFileOutputNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:GetPorts"));

    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Start"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}



////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Stop"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
/**
//Queue an asynchronous node command
*/
PVMFCommandId PVMFDummyFileOutputNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Flush"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Pause"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:Reset"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:CancelAllCommands"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}



////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:CancelCommand"));
    PVMFDummyFileOutputNodeCommand cmd;
    cmd.PVMFDummyFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::removeRef()
{
    if (iExtensionRefCount > 0)
        --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
bool PVMFDummyFileOutputNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMFDummyFileOutputNodeConfigUuid)
    {
        PVMFDummyFileOutputNodeConfigInterface* myInterface = OSCL_STATIC_CAST(PVMFDummyFileOutputNodeConfigInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == PvmfComposerSizeAndDurationUuid)
    {
        PvmfComposerSizeAndDurationInterface* myInterface = OSCL_STATIC_CAST(PvmfComposerSizeAndDurationInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        ++iExtensionRefCount;
    }
    else if (uuid == PvmfNodesSyncControlUuid)
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
PVMFStatus PVMFDummyFileOutputNode::SetMaxDuration(bool aEnable, uint32 aMaxDurationMilliseconds)
{
    iMaxDurationEnabled = aEnable;
    if (iMaxDurationEnabled)
    {
        iMaxDuration = aMaxDurationMilliseconds;
    }
    else
    {
        iMaxDuration = 0;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::GetMaxDurationConfig(bool& aEnable, uint32& aMaxDurationMilliseconds)
{
    aEnable = iMaxDurationEnabled;
    aMaxDurationMilliseconds = iMaxDuration;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::SetDurationProgressReport(bool aEnable, uint32 aReportFrequency)
{
    iDurationReportEnabled = aEnable;
    if (iDurationReportEnabled)
    {
        iDurationReportFreq = aReportFrequency;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::GetDurationProgressReportConfig(bool& aEnable, uint32& aReportFrequency)
{
    aEnable = iDurationReportEnabled;
    aReportFrequency = iDurationReportFreq;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::SetClock(PVMFMediaClock* aClock)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::SetClock: aClock=0x%x", aClock));

    iClock = aClock;
    if (iInPort)
    {
        return ((PVMFDummyFileOutputInPort*)iInPort)->SetClock(aClock);
    }

    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::ChangeClockRate(int32 aRate)
{
    OSCL_UNUSED_ARG(aRate);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::ChangeClockRate: aRate=%d", aRate));

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::SetMargins(int32 aEarlyMargin, int32 aLateMargin)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::SetMargins: aEarlyMargin=%d, aLateMargin=%d", aEarlyMargin, aLateMargin));

    iEarlyMargin = aEarlyMargin;
    iLateMargin = aLateMargin;
    if (iInPort)
    {
        return ((PVMFDummyFileOutputInPort*)iInPort)->SetMargins(aEarlyMargin, aLateMargin);
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::ClockStarted(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::ClockStarted"));

    if (iInPort)
    {
        ((PVMFDummyFileOutputInPort*)iInPort)->Start();
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::ClockStopped(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::ClockStopped"));

    if (iInPort)
    {
        ((PVMFDummyFileOutputInPort*)iInPort)->Pause();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFDummyFileOutputNode::SkipMediaData(PVMFSessionId aSessionId,
        PVMFTimestamp aResumeTimestamp,
        uint32 aStreamID,
        bool aPlayBackPositionContinuous,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::SkipMediaData: aResumeTimestamp=%d, aContext=0x%x",
                     aResumeTimestamp, aContext));

    if (!iInPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileOutputNode::SkipMediaData: Error - Input port has not been created"));
        OSCL_LEAVE(OsclErrNotReady);
        return 0;
    }

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodeInitialized:
        case EPVMFNodePaused:
            return ((PVMFDummyFileOutputInPort*)iInPort)->SkipMediaData(aSessionId, aResumeTimestamp, aStreamID, aPlayBackPositionContinuous, aContext);

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFDummyFileOutputNode::SkipMediaData: Error - Wrong state"));
            OSCL_LEAVE(OsclErrInvalidState);
            return 0;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::Run()
{
    if (!iInputCommands.empty())
    {
        if (ProcessCommand(iInputCommands.front()))
        {
            //note: need to check the state before re-scheduling
            //since the node could have been reset in the ProcessCommand
            //call.
            if (iInterfaceState != EPVMFNodeCreated)
                RunIfNotReady();
            return;
        }
    }

    // Process port activity
    if (!iPortActivityQueue.empty()
            && (iInterfaceState == EPVMFNodeStarted || FlushPending()))
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
        return;
    }

    //If we get here we did not process any ports or commands.
    //Check for completion of a flush command...
    if (FlushPending()
            && iPortActivityQueue.empty())
    {
        SetState(EPVMFNodePrepared);
        iInPort->ResumeInput();
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFSuccess);
        RunIfNotReady();
    }
}

void PVMFDummyFileOutputNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFDummyFileOutputNode::PortActivity: port=0x%x, type=%d",
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

        case PVMF_PORT_ACTIVITY_DELETED:
            //Report port deleted info event to the node.
            ReportInfoEvent(PVMFInfoPortDeleted
                            , (OsclAny*)aActivity.iPort);
            //Purge any port activity events already queued
            //for this port.
            {
                for (uint32 i = 0;i < iPortActivityQueue.size();)
                {
                    if (iPortActivityQueue[i].iPort == aActivity.iPort)
                        iPortActivityQueue.erase(&iPortActivityQueue[i]);
                    else
                        i++;
                }
            }
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            //nothing needed.
            break;
        default:
            break;
    }
}

/////////////////////////////////////////////////////
// Port Processing routines
/////////////////////////////////////////////////////

void PVMFDummyFileOutputNode::QueuePortActivity(const PVMFPortActivity &aActivity)
{
    //queue a new port activity event
    int32 err;
    OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFDummyFileOutputNode::PortActivity: Error - iPortActivityQueue.push_back() failed", this));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
    }
    else
    {
        //wake up the AO to process the port activity event.
        RunIfNotReady();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::ProcessIncomingData(PVMFSharedMediaDataPtr aMediaData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::ProcessIncomingData()"));

    PVMFStatus status = PVMFSuccess;

    OsclRefCounterMemFrag frag;
    uint32 numFrags = aMediaData->getNumFragments();
    OsclRefCounterMemFrag formatSpecificInfo;
    aMediaData->getFormatSpecificInfo(formatSpecificInfo);

    for (uint32 i = 0; (i < numFrags) && status == PVMFSuccess; i++)
    {
        aMediaData->getMediaFragment(i, frag);
        switch (iInterfaceState)
        {
            case EPVMFNodeStarted:

                break;

            case EPVMFNodeInitialized:
                // Already stopped. Ignore incoming data.
                break;

            default:
                // Wrong state
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFDummyFileOutputNode::ProcessIncomingData: Error - Wrong state"));
                status = PVMFFailure;
                break;
        }
    }

    return status;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::SendDurationProgress(uint32 aTimestamp)
{

    if (iDurationReportEnabled &&
            aTimestamp >= iNextDurationReport)
    {
        iNextDurationReport = aTimestamp - (aTimestamp % iDurationReportFreq) + iDurationReportFreq;
        ReportInfoEvent(PVMF_COMPOSER_DURATION_PROGRESS, (OsclAny*)aTimestamp);
    }
    return PVMFSuccess;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::SendFileSizeProgress()
{
    if (iFileSizeReportEnabled &&
            iFileSize >= iNextFileSizeReport)
    {
        iNextFileSizeReport = iFileSize - (iFileSize % iFileSizeReportFreq) + iFileSizeReportFreq;
        ReportInfoEvent(PVMF_COMPOSER_FILESIZE_PROGRESS, (OsclAny*)iFileSize);
    }
    return PVMFSuccess;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::CheckMaxFileSize(uint32 aFrameSize)
{
    if (iMaxFileSizeEnabled)
    {
        if ((iFileSize + aFrameSize) >= iMaxFileSize)
        {
            // Change state to initialized
            ChangeNodeState(EPVMFNodeInitialized);

            // Clear all pending port activity
            ClearPendingPortActivity();

            // Report max file size event
            ReportInfoEvent(PVMF_COMPOSER_MAXFILESIZE_REACHED, NULL);
            return PVMFSuccess;
        }

        return PVMFPending;
    }
    return PVMFErrNotSupported;
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::CheckMaxDuration(uint32 aTimestamp)
{
    if (iMaxDurationEnabled)
    {
        if (aTimestamp >= iMaxDuration)
        {
            // Change state to initialized
            ChangeNodeState(EPVMFNodeInitialized);

            // Clear all pending port activity
            ClearPendingPortActivity();

            // Report max duration event
            ReportInfoEvent(PVMF_COMPOSER_MAXDURATION_REACHED, NULL);
            return PVMFSuccess;
        }

        return PVMFPending;
    }
    return PVMFErrNotSupported;
}


//////////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::ClearPendingPortActivity()
{
    // index starts at 1 because the current command (i.e. iCmdQueue[0]) will be
    // erased inside Run

    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }
}

void PVMFDummyFileOutputNode::ChangeNodeState(TPVMFNodeInterfaceState aNewState)
{
    iInterfaceState = aNewState;
}

//////////////////////////////////////////////////////////////////////////////////////////
void PVMFDummyFileOutputNode::CommandComplete(PVMFDummyFileOutputNodeCmdQ& aCmdQ, PVMFDummyFileOutputNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFDummyFileOutputNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

PVMFCommandId PVMFDummyFileOutputNode::QueueCommandL(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}


/////////////////////////////////////////////////////
bool PVMFDummyFileOutputNode::ProcessPortActivity()
{//called by the AO to process a port activity message
    //Pop the queue...
    PVMFPortActivity activity(iPortActivityQueue.front());
    iPortActivityQueue.erase(&iPortActivityQueue.front());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFDummyFileOutputNode::ProcessPortActivity: port=0x%x, type=%d",
                     this, activity.iPort, activity.iType));

    int32 err = OsclErrNone;

    PVMFStatus status = PVMFSuccess;
    switch (activity.iType)
    {
        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            status = ProcessIncomingMsg(activity.iPort);
            //if there is still data, queue another port activity event.
            //tbd-- do we need to do this even if port is busy?
            if (activity.iPort->IncomingMsgQueueSize() > 0)
            {
                OSCL_TRY(err, iPortActivityQueue.push_back(activity););
            }
            break;
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
        default:
            return false;
            // break;	This statement was removed to avoid compiler warning for Unreachable Code
    }

    return true;
}

/////////////////////////////////////////////////////
PVMFStatus PVMFDummyFileOutputNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one buffer off the port's
    //incoming data queue.  This routine will dequeue and
    //dispatch the data.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFDummyFileOutputNode::ProcessIncomingMsg: aPort=0x%x", this, aPort));

    if (aPort->GetPortTag() != PVMF_DUMMY_FILE_OUTPUT_NODE_PORT_TYPE_SINK)
    {
        return PVMFFailure;
    }
    PVMFSharedMediaMsgPtr msg;
    PVMFStatus status = aPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFDummyFileOutputNode::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this));
        return status;
    }
    /*
       INFORMATION!!!
       The FileOutputNode is generally used by the engine unit tests as SinkNode
       For now, most of the unit tests have OBSOLETED the use of FileOutputNode,
       But still some of the tests are using the FileOutputNode in place of,
       MIO (RefFileOutput).

       Since the usage FileOutputNode is not defined yet, we are adding support for
       BOS Message as a NO-OP so that the node should be able to handle Any and all
       the BOS Messages gracefully.

       IMPORTANT!!!,
       For Complete support of BOS in the FileOutputNode, we need to make more changes.
       Those changes will be done only once the life scope of FileOutputNode is defined.
    */
    if (msg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVFileOutputNode::ProcessIncomingMsg BOS Recieved"));
        return PVMFSuccess;
    }

    // Transfer to the port's sync queue to do synchronization
    // This is temporary until data is directly saved to the sync queue
    uint32 dropped;
    uint32 skipped;
    status = ((PVMFDummyFileOutputInPort*)aPort)->iDataQueue.QueueMediaData(msg, &dropped, &skipped);
    if (dropped > 0)
    {
        PVMFNodeInterface::ReportInfoEvent(PVMFInfoDataDiscarded);
    }

    if (status == PVMFErrNoMemory)
    {
        return PVMFFailure;
    }
    else
    {
        return PVMFSuccess;
    }
}

bool PVMFDummyFileOutputNode::ProcessCommand(PVMFDummyFileOutputNodeCommand& aCmd)
{
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

        default://unknown command type
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            break;
    }

    return true;
}

/**
//A routine to tell if a flush operation is in progress.
*/
bool PVMFDummyFileOutputNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

void PVMFDummyFileOutputNode::DoQueryUuid(PVMFDummyFileOutputNodeCommand& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFDummyFileOutputNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node
    //Match against custom interface1...
    if (*mimetype == PVMF_FILE_OUTPUT_NODE_CUSTOM1_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_FILE_OUTPUT_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_BASEMIMETYPE))
    {

        //PVUuid uuid(PVMF_SOCKET_NODE_CUSTOM1_UUID);
        uuidvec->push_back(PVMFDummyFileOutputNodeConfigUuid);
        uuidvec->push_back(PvmfComposerSizeAndDurationUuid);
        uuidvec->push_back(PvmfNodesSyncControlUuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFDummyFileOutputNode::DoQueryInterface(PVMFDummyFileOutputNodeCommand&  aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::DoQueryInterface"));

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFDummyFileOutputNodeCommandBase::Parse(uuid, ptr);

    if (queryInterface(*uuid, *ptr))
    {
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

void PVMFDummyFileOutputNode::DoInit(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVMFStatus iRet = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            //this node doesn't need to do anything to get ready
            //to start.
            SetState(EPVMFNodeInitialized);
            break;
        case EPVMFNodeInitialized:
            break;
        default:
            iRet = PVMFErrInvalidState;
            break;
    }
    CommandComplete(iInputCommands, aCmd, iRet);
}

/**
//Called by the command handler AO to do the node Prepare
*/
void PVMFDummyFileOutputNode::DoPrepare(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVMFStatus iRet = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodeInitialized:
            //this node doesn't need to do anything to get ready
            //to start.
            SetState(EPVMFNodePrepared);
            break;
        case EPVMFNodePrepared:
            break;
        default:
            iRet = PVMFErrInvalidState;
            break;
    }
    CommandComplete(iInputCommands, aCmd, iRet);
}

void PVMFDummyFileOutputNode::DoStart(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVMFStatus status = PVMFSuccess;
    switch (iInterfaceState)
    {
        case EPVMFNodePrepared:
        case EPVMFNodePaused:
        {
            if (!iClock)
            {
                // If not using sync clock, start processing incoming data
                ((PVMFDummyFileOutputInPort*)iInPort)->Start();
            }
            if (!iFileOpened)
            {
                if (iFs.Connect() != 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFDummyFileOutputNode::DoStart: iFs.Connect Error."));
                    status = PVMFErrNoResources;
                    break;
                }

                if (0 != iOutputFile.Open(iOutputFileName.get_cstr(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, iFs))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFDummyFileOutputNode::DoStart: iOutputFile.Open Error."));
                    status = PVMFErrNoResources;
                    break;
                }

                iFileOpened = 1;

                iFirstMediaData = true;
            }
            SetState(EPVMFNodeStarted);

            break;
        }
        case EPVMFNodeStarted:
            status = PVMFSuccess;
            break;
        default:
            status = PVMFErrInvalidState;
            break;
    }

    CommandComplete(iInputCommands, aCmd, status);
}

void PVMFDummyFileOutputNode::DoStop(PVMFDummyFileOutputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            // Stop data source
            if (iInPort)
            {

                ((PVMFDummyFileOutputInPort*)iInPort)->Stop();
                CloseOutputFile();
            }

            // Clear queued messages in ports
            uint32 i;
            for (i = 0; i < iPortVector.size(); i++)
                iPortVector[i]->ClearMsgQueues();

            // Clear scheduled port activities
            iPortActivityQueue.clear();

            //transition to Initialized state
            SetState(EPVMFNodePrepared);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        case EPVMFNodePrepared:
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

void PVMFDummyFileOutputNode::DoFlush(PVMFDummyFileOutputNodeCommand& aCmd)
{
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

            //Notify all ports to suspend their input
            {
                for (uint32 i = 0;i < iPortVector.size();i++)
                    iPortVector[i]->SuspendInput();
            }

            // Stop data source
            break;

        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

void PVMFDummyFileOutputNode::DoPause(PVMFDummyFileOutputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        {
            // Pause data source
            if (!iClock)
            {
                // If not using sync clock, pause processing of incoming data
                ((PVMFDummyFileOutputInPort*)iInPort)->Pause();
            }

            SetState(EPVMFNodePaused);
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
            break;
        }
        case EPVMFNodePaused:
        {
            CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        }
        break;
        default:
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            break;
    }
}

void PVMFDummyFileOutputNode::DoReset(PVMFDummyFileOutputNodeCommand& aCmd)
{
    if (IsAdded())
    {
        if (iInPort)
        {
            OSCL_DELETE(((PVMFDummyFileOutputInPort*)iInPort));
            iInPort = NULL;
        }

        //logoff & go back to Created state.
        SetState(EPVMFNodeIdle);
        ThreadLogoff();
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        OSCL_LEAVE(OsclErrInvalidState);
    }
}


void PVMFDummyFileOutputNode::DoRequestPort(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::DoRequestPort"));
    //This node supports port request from any state

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;

    aCmd.PVMFDummyFileOutputNodeCommandBase::Parse(tag, portconfig);

    //validate the tag...
    switch (tag)
    {
        case PVMF_DUMMY_FILE_OUTPUT_NODE_PORT_TYPE_SINK:
            break;
        default:
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFDummyFileOutputNode::DoRequestPort: Error - Invalid port tag"));
            CommandComplete(iInputCommands, aCmd, PVMFFailure);
            return;
    }

    if (iInPort)
    {
        // it's been taken for now, so reject this request
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
        return;
    }

    // Create and configure output port
    int32 err;
    PVMFFormatType fmt = PVMF_MIME_FORMAT_UNKNOWN;
    if (portconfig)
    {
        fmt = portconfig->get_str();
    }

    if (iFormat != PVMF_MIME_FORMAT_UNKNOWN &&
            iFormat != fmt)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileOutputNode::DoRequestPort: Error - Format not supported (format was preset)"));
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
        return;
    }

    OSCL_TRY(err, iInPort = OSCL_NEW(PVMFDummyFileOutputInPort, (tag, this)););
    if (err != OsclErrNone || !iInPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFDummyFileOutputNode::DoRequestPort: Error - PVMFDummyFileOutputInPort::Create() failed"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    ((PVMFDummyFileOutputInPort*)iInPort)->SetClock(iClock);
    ((PVMFDummyFileOutputInPort*)iInPort)->SetMargins(iEarlyMargin, iLateMargin);

    //if format was provided in mimestring, set it now.
    if (portconfig)
    {
        PVMFFormatType fmt = portconfig->get_str();
        if (fmt != PVMF_MIME_FORMAT_UNKNOWN
                && ((PVMFDummyFileOutputInPort*)iInPort)->IsFormatSupported(fmt))
        {
            ((PVMFDummyFileOutputInPort*)iInPort)->iFormat = fmt;
            ((PVMFDummyFileOutputInPort*)iInPort)->FormatUpdated();
        }
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)iInPort);
}

void PVMFDummyFileOutputNode::DoReleasePort(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::DoReleasePort"));

    if (iInPort)
    {
        OSCL_DELETE(((PVMFDummyFileOutputInPort*)iInPort));
        iInPort = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        return;
    }
    CommandComplete(iInputCommands, aCmd, PVMFFailure);
}

void PVMFDummyFileOutputNode::DoCancelAllCommands(PVMFDummyFileOutputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFDummyFileOutputNode::DoCancelAllCommands"));
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

void PVMFDummyFileOutputNode::DoCancelCommand(PVMFDummyFileOutputNodeCommand& aCmd)
{
    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFDummyFileOutputNodeCommandBase::Parse(id);

    //first check "current" command if any
    {
        PVMFDummyFileOutputNodeCommand* cmd = iCurrentCommand.FindById(id);
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
        PVMFDummyFileOutputNodeCommand* cmd = iInputCommands.FindById(id, 1);
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
