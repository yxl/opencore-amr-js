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
 * @file pvmf_fileoutput_node.cpp
 * @brief Simple file output node. Writes incoming data to specified
 * file
 *
 */

#include "pvmf_fileoutput_inport.h"
#include "pvmf_fileoutput_node.h"
#include "pvlogger.h"
#include "oscl_error_codes.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"
#include "pvmf_timedtext.h"

////////////////////////////////////////////////////////////////////////////
PVMFFileOutputNode::PVMFFileOutputNode(int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFFileOutputNode")
        , iCmdIdCounter(0)
        , iInPort(NULL)
        , iFileHandle(NULL)
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
PVMFFileOutputNode::~PVMFFileOutputNode()
{
    //thread logoff
    if (IsAdded())
        RemoveFromScheduler();

    //Cleanup allocated interfaces


    //Cleanup allocated ports
    if (iInPort)
    {
        OSCL_DELETE(((PVMFFileOutputInPort*)iInPort));
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
void PVMFFileOutputNode::ConstructL()
{
    iAlloc = (Oscl_DefAlloc*)(new PVMFFileOutputAlloc());
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
                AddToScheduler();
            iLogger = PVLogger::GetLoggerObject("PVMFFileOutputNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
        default:
            return PVMFErrInvalidState;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:ThreadLogoff"));

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

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::CloseOutputFile()
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
PVMFCommandId PVMFFileOutputNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Init"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_INIT, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Prepare"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PREPARE, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:GetCapability"));
    iCapability.iInputFormatCapability.clear();

    if (iFormat != PVMF_MIME_FORMAT_UNKNOWN)
    {
        // Format is already set, so return only that one
        iCapability.iInputFormatCapability.push_back(iFormat);
    }
    else
    {
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMR_IETF);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_AMRWB_IETF);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_M4V);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM8);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM16);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_YUV420);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_ADTS);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_H2631998);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_H2632000);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_RAW);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO_MP4);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_H264_VIDEO);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_PCM);
        iCapability.iInputFormatCapability.push_back(PVMF_MIME_3GPP_TIMEDTEXT);
    }
    aNodeCapability = iCapability;
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:QueryUUID"));

    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYUUID, aMimeType, aUuids, aExactUuidsOnly, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:QueryInterface"));

    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_QUERYINTERFACE, aUuid, aInterfacePtr, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:RequestPort"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_REQUESTPORT, aPortTag, aPortConfig, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:ReleasePort"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RELEASEPORT, aPort, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFPortIter* PVMFFileOutputNode::GetPorts(const PVMFPortFilter* aFilter)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:GetPorts"));

    OSCL_UNUSED_ARG(aFilter);//port filter is not implemented.
    iPortVector.Reset();
    return &iPortVector;
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Start"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_START, aContext);
    return QueueCommandL(cmd);
}



////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Stop"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_STOP, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
/**
//Queue an asynchronous node command
*/
PVMFCommandId PVMFFileOutputNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Flush"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_FLUSH, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Pause"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_PAUSE, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:Reset"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_RESET, aContext);
    return QueueCommandL(cmd);
}


////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:CancelAllCommands"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELALLCOMMANDS, aContext);
    return QueueCommandL(cmd);
}



////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:CancelCommand"));
    PVMFFileOutputNodeCommand cmd;
    cmd.PVMFFileOutputNodeCommandBase::Construct(s, PVMF_GENERIC_NODE_CANCELCOMMAND, aCmdId, aContext);
    return QueueCommandL(cmd);
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::addRef()
{
    ++iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::removeRef()
{
    if (iExtensionRefCount > 0)
        --iExtensionRefCount;
}

////////////////////////////////////////////////////////////////////////////
bool PVMFFileOutputNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PvmfFileOutputNodeConfigUuid)
    {
        PvmfFileOutputNodeConfigInterface* myInterface = OSCL_STATIC_CAST(PvmfFileOutputNodeConfigInterface*, this);
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
PVMFStatus PVMFFileOutputNode::SetOutputFileName(const OSCL_wString& aFileName)
{
    if (iInterfaceState != EPVMFNodeIdle
            && iInterfaceState != EPVMFNodeInitialized
            && iInterfaceState != EPVMFNodeCreated
            && iInterfaceState != EPVMFNodePrepared)
        return false;

    iOutputFileName = aFileName;
    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetOutputFileDescriptor(const OsclFileHandle* aFileHandle)
{
    if (iInterfaceState != EPVMFNodeIdle
            && iInterfaceState != EPVMFNodeInitialized
            && iInterfaceState != EPVMFNodeCreated
            && iInterfaceState != EPVMFNodePrepared)
        return false;

    iOutputFile.SetPVCacheSize(0);
    iOutputFile.SetAsyncReadBufferSize(0);
    iOutputFile.SetNativeBufferSize(0);
    iOutputFile.SetLoggingEnable(false);
    iOutputFile.SetSummaryStatsLoggingEnable(false);
    iOutputFile.SetFileHandle((OsclFileHandle*)aFileHandle);

    //call open
    int32 retval = iOutputFile.Open(_STRLIT_CHAR("dummy"),
                                    Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY,
                                    iFs);

    if (retval == 0)
    {
        iFileOpened = 1;
        iFirstMediaData = true;
        return PVMFSuccess;
    }
    return PVMFFailure;
}
////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetMaxFileSize(bool aEnable, uint32 aMaxFileSizeBytes)
{
    iMaxFileSizeEnabled = aEnable;
    if (iMaxFileSizeEnabled)
    {
        iMaxFileSize = aMaxFileSizeBytes;
    }
    else
    {
        iMaxFileSize = 0;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::GetMaxFileSizeConfig(bool& aEnable, uint32& aMaxFileSizeBytes)
{
    aEnable = iMaxFileSizeEnabled;
    aMaxFileSizeBytes = iMaxFileSize;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetMaxDuration(bool aEnable, uint32 aMaxDurationMilliseconds)
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
void PVMFFileOutputNode::GetMaxDurationConfig(bool& aEnable, uint32& aMaxDurationMilliseconds)
{
    aEnable = iMaxDurationEnabled;
    aMaxDurationMilliseconds = iMaxDuration;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetFileSizeProgressReport(bool aEnable, uint32 aReportFrequency)
{
    iFileSizeReportEnabled = aEnable;
    if (iFileSizeReportEnabled)
    {
        iFileSizeReportFreq = aReportFrequency;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::GetFileSizeProgressReportConfig(bool& aEnable, uint32& aReportFrequency)
{
    aEnable = iFileSizeReportEnabled;
    aReportFrequency = iFileSizeReportFreq;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetDurationProgressReport(bool aEnable, uint32 aReportFrequency)
{
    iDurationReportEnabled = aEnable;
    if (iDurationReportEnabled)
    {
        iDurationReportFreq = aReportFrequency;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::GetDurationProgressReportConfig(bool& aEnable, uint32& aReportFrequency)
{
    aEnable = iDurationReportEnabled;
    aReportFrequency = iDurationReportFreq;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetClock(PVMFMediaClock* aClock)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::SetClock: aClock=0x%x", aClock));

    iClock = aClock;
    if (iInPort)
    {
        return ((PVMFFileOutputInPort*)iInPort)->SetClock(aClock);
    }

    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::ChangeClockRate(int32 aRate)
{
    OSCL_UNUSED_ARG(aRate);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::ChangeClockRate: aRate=%d", aRate));

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SetMargins(int32 aEarlyMargin, int32 aLateMargin)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::SetMargins: aEarlyMargin=%d, aLateMargin=%d", aEarlyMargin, aLateMargin));

    iEarlyMargin = aEarlyMargin;
    iLateMargin = aLateMargin;
    if (iInPort)
    {
        return ((PVMFFileOutputInPort*)iInPort)->SetMargins(aEarlyMargin, aLateMargin);
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::ClockStarted(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::ClockStarted"));

    if (iInPort)
    {
        ((PVMFFileOutputInPort*)iInPort)->Start();
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::ClockStopped(void)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::ClockStopped"));

    if (iInPort)
    {
        ((PVMFFileOutputInPort*)iInPort)->Pause();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFFileOutputNode::SkipMediaData(PVMFSessionId aSessionId,
        PVMFTimestamp aResumeTimestamp,
        uint32 aStreamID,
        bool aPlayBackPositionContinuous,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::SkipMediaData: aResumeTimestamp=%d, aContext=0x%x",
                     aResumeTimestamp, aContext));

    if (!iInPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFFileOutputNode::SkipMediaData: Error - Input port has not been created"));
        OSCL_LEAVE(OsclErrNotReady);
        return 0;
    }

    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodeInitialized:
        case EPVMFNodePaused:
            return ((PVMFFileOutputInPort*)iInPort)->SkipMediaData(aSessionId, aResumeTimestamp, aStreamID, aPlayBackPositionContinuous, aContext);

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFFileOutputNode::SkipMediaData: Error - Wrong state"));
            OSCL_LEAVE(OsclErrInvalidState);
            return 0;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::Run()
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

void PVMFFileOutputNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFFileOutputNode::PortActivity: port=0x%x, type=%d",
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

void PVMFFileOutputNode::QueuePortActivity(const PVMFPortActivity &aActivity)
{
    //queue a new port activity event
    int32 err;
    OSCL_TRY(err, iPortActivityQueue.push_back(aActivity););
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFFileOutputNode::PortActivity: Error - iPortActivityQueue.push_back() failed", this));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
    }
    else
    {
        //wake up the AO to process the port activity event.
        RunIfNotReady();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::ProcessIncomingData(PVMFSharedMediaDataPtr aMediaData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::ProcessIncomingData()"));

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
                if (iFirstMediaData)
                {
                    status = WriteFormatSpecificInfo(formatSpecificInfo.getMemFragPtr(), formatSpecificInfo.getMemFragSize());
                    if (status != PVMFSuccess)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFFileOutputNode::ProcessIncomingData: Error - WriteFormatSpecificInfo failed"));
                        return PVMFFailure;
                    }
                }

                if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_3GPP_TIMEDTEXT)
                {
                    PVMFTimedTextMediaData* textmediadata = (PVMFTimedTextMediaData*)(frag.getMemFragPtr());
                    // Output the text sample entry
                    if (textmediadata->iTextSampleEntry.GetRep() != NULL)
                    {
                        PVMFTimedTextMediaData* textmediadata = (PVMFTimedTextMediaData*)(frag.getMemFragPtr());
                        // Output the text sample entry
                        if (textmediadata->iTextSampleEntry.GetRep() != NULL)
                        {
                            // @TODO Write out the text sample entry in a better format
                            status = WriteData((OsclAny*)(textmediadata->iTextSampleEntry.GetRep()), sizeof(PVMFTimedTextSampleEntry));
                            if (status == PVMFFailure)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                                (0, "PVMFFileOutputNode::ProcessIncomingData: Error - WriteData failed for text sample entry"));
                                return PVMFFailure;
                            }
                        }
                        // Write out the raw text sample
                        status = WriteData((OsclAny*)(textmediadata->iTextSample), textmediadata->iTextSampleLength);
                        if (status == PVMFFailure)
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                            (0, "PVMFFileOutputNode::ProcessIncomingData: Error - WriteData failed for text sample entry"));
                            return PVMFFailure;
                        }
                    }
                }
                else
                {
                    status = WriteData(frag, aMediaData->getTimestamp());
                    if (status == PVMFFailure)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                        (0, "PVMFFileOutputNode::ProcessIncomingData: Error - WriteData failed"));
                        return PVMFFailure;
                    }
                }

                break;

            case EPVMFNodeInitialized:
                // Already stopped. Ignore incoming data.
                break;

            default:
                // Wrong state
                PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFFileOutputNode::ProcessIncomingData: Error - Wrong state"));
                status = PVMFFailure;
                break;
        }
    }

    return status;
}
//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::WriteFormatSpecificInfo(OsclAny* aPtr, uint32 aSize)
{
    PVMFStatus status = PVMFSuccess;

    if (!iFileOpened)
    {
        if (iFs.Connect() != 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: iFs.Connect Error."));
            status = PVMFErrNoResources;
            return status;
        }

        if (0 != iOutputFile.Open(iOutputFileName.get_cstr(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, iFs))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: iOutputFile.Open Error."));
            status = PVMFErrNoResources;
            return status;
        }

        iFileOpened = 1;

        iFirstMediaData = true;
    }

    if (iFirstMediaData)
    {
        // Add the amr header if required
        if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_AMR_IETF)
        {
            // Check if the incoming data has "#!AMR\n" string
            if (aSize < AMR_HEADER_SIZE ||
                    oscl_strncmp((const char*)aPtr, AMR_HEADER, AMR_HEADER_SIZE) != 0)
            {
                // AMR header not found, add AMR header to file first
                status = WriteData((OsclAny*)AMR_HEADER, AMR_HEADER_SIZE);
                if (status != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: Error - WriteData failed"));
                    return status;
                }
            }
            iFirstMediaData = false;
        }
        // Add the amr-wb header if required
        else if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_AMRWB_IETF)
        {
            // Check if the incoming data has "#!AMR-WB\n" string
            if (aSize < AMRWB_HEADER_SIZE ||
                    oscl_strncmp((const char*)aPtr, AMRWB_HEADER, AMRWB_HEADER_SIZE) != 0)
            {
                // AMR header not found, add AMR header to file first
                status = WriteData((OsclAny*)AMRWB_HEADER, AMRWB_HEADER_SIZE);
                if (status != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: Error - WriteData failed"));
                    return status;
                }
            }
            iFirstMediaData = false;
        }
        else if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_M4V)
        {
            if (aSize > 0)
            {
                status = WriteData(aPtr, aSize);
                if (status != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: Error - WriteData failed"));
                    return status;
                }
            }
            iFirstMediaData = false;
        }
        else if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_PCM8)
        {
            if (aSize > 0)
            {
                status = WriteData(aPtr, aSize);
                if (status != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: Error - WriteData failed"));
                    return status;
                }
            }
            iFirstMediaData = false;
        }
        else if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_PCM16)
        {
            if (aSize > 0)
            {
                status = WriteData(aPtr, aSize);
                if (status != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: Error - WriteData failed"));
                    return status;
                }
            }
            iFirstMediaData = false;
        }
        else if (((PVMFFileOutputInPort*)iInPort)->iFormat == PVMF_MIME_3GPP_TIMEDTEXT)
        {
            if (aSize > 0)
            {
                // TODO Write out the text track level info in some formatted way
                status = WriteData(aPtr, aSize);
                if (status != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::WriteFormatSpecificInfo: Error - WriteData failed"));
                    return status;
                }
            }
            iFirstMediaData = false;
        }
        else
        {
            iFirstMediaData = false;
        }
    }
    return status;
}


//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::SendDurationProgress(uint32 aTimestamp)
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
PVMFStatus PVMFFileOutputNode::SendFileSizeProgress()
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
PVMFStatus PVMFFileOutputNode::CheckMaxFileSize(uint32 aFrameSize)
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
PVMFStatus PVMFFileOutputNode::CheckMaxDuration(uint32 aTimestamp)
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
PVMFStatus PVMFFileOutputNode::WriteData(OsclAny* aData, uint32 aSize)
{
    if (!aData || aSize == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFFileOutputNode::WriteData: Error - Invalid data or data size"));
        return PVMFFailure;
    }

    switch (CheckMaxFileSize(aSize))
    {
        case PVMFFailure:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFFileOutputNode::WriteData: Error - CheckMaxFileSize failed"));
            return PVMFFailure;

        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFFileOutputNode::WriteData: Maxmimum file size reached"));
            return PVMFSuccess;

        default:
            break;
    }

    int32 wlength = 0;
    if ((wlength = iOutputFile.Write(aData, sizeof(uint8), aSize)) != (int32)aSize)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFFileOutputNode::WriteData: Error - File write failed"));
        ReportInfoEvent(PVMFInfoProcessingFailure);
        return PVMFFailure;
    }
    else
    {
        iOutputFile.Flush();
    }

    iFileSize += wlength;
    return SendFileSizeProgress();
}

//////////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::WriteData(OsclRefCounterMemFrag aMemFrag, uint32 aTimestamp)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::WriteData: aTimestamp=%d", aTimestamp));

    switch (CheckMaxDuration(aTimestamp))
    {
        case PVMFFailure:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFFileOutputNode::WriteData: Error - CheckMaxDuration failed"));
            return PVMFFailure;

        case PVMFSuccess:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFFileOutputNode::WriteData: Maxmimum duration reached"));
            return PVMFSuccess;

        default:
            break;
    }

    if (WriteData(aMemFrag.getMemFragPtr(), aMemFrag.getMemFragSize()) == PVMFSuccess)
        return SendDurationProgress(aTimestamp);
    else
        return PVMFFailure;
}

//////////////////////////////////////////////////////////////////////////////////
void PVMFFileOutputNode::ClearPendingPortActivity()
{
    // index starts at 1 because the current command (i.e. iCmdQueue[0]) will be erased inside Run
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }
}

void PVMFFileOutputNode::ChangeNodeState(TPVMFNodeInterfaceState aNewState)
{
    iInterfaceState = aNewState;
}

/***********************************/
void PVMFFileOutputNode::CommandComplete(PVMFFileOutputNodeCmdQ& aCmdQ, PVMFFileOutputNodeCommand& aCmd, PVMFStatus aStatus, OsclAny* aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFFileOutputNode:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                    , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    //create response
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aEventData);
    PVMFSessionId session = aCmd.iSession;

    //Erase the command from the queue.
    aCmdQ.Erase(&aCmd);

    //Report completion to the session observer.
    ReportCmdCompleteEvent(session, resp);
}

PVMFCommandId PVMFFileOutputNode::QueueCommandL(PVMFFileOutputNodeCommand& aCmd)
{
    PVMFCommandId id;

    id = iInputCommands.AddL(aCmd);

    //wakeup the AO
    RunIfNotReady();

    return id;
}


/////////////////////////////////////////////////////
bool PVMFFileOutputNode::ProcessPortActivity()
{//called by the AO to process a port activity message
    //Pop the queue...
    PVMFPortActivity activity(iPortActivityQueue.front());
    iPortActivityQueue.erase(&iPortActivityQueue.front());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFFileOutputNode::ProcessPortActivity: port=0x%x, type=%d",
                     this, activity.iPort, activity.iType));

    int32 err = OsclErrNone;

    PVMFStatus status = PVMFSuccess;
    switch (activity.iType)
    {
        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            status = ProcessIncomingMsg(activity.iPort);
            //if there is still data, queue another port activity event.
            if (activity.iPort->IncomingMsgQueueSize() > 0)
            {
                OSCL_TRY(err, iPortActivityQueue.push_back(activity););
            }
            break;
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
        default:
            return false;
    }

    return true;
}

/////////////////////////////////////////////////////
PVMFStatus PVMFFileOutputNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one buffer off the port's
    //incoming data queue.  This routine will dequeue and
    //dispatch the data.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFFileOutputNode::ProcessIncomingMsg: aPort=0x%x", this, aPort));

    if (aPort->GetPortTag() != PVMF_FILE_OUTPUT_NODE_PORT_TYPE_SINK)
    {
        return PVMFFailure;
    }
    PVMFSharedMediaMsgPtr msg;
    PVMFStatus status = aPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFFileOutputNode::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this));
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
                        (0, "PVFileOutputNode::ProcessIncomingMsg BOS Received"));
        return PVMFSuccess;
    }

    // Transfer to the port's sync queue to do synchronization
    // This is temporary until data is directly saved to the sync queue
    uint32 dropped;
    uint32 skipped;
    status = ((PVMFFileOutputInPort*)aPort)->iDataQueue.QueueMediaData(msg, &dropped, &skipped);
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

bool PVMFFileOutputNode::ProcessCommand(PVMFFileOutputNodeCommand& aCmd)
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
bool PVMFFileOutputNode::FlushPending()
{
    return (iCurrentCommand.size() > 0
            && iCurrentCommand.front().iCmd == PVMF_GENERIC_NODE_FLUSH);
}

void PVMFFileOutputNode::DoQueryUuid(PVMFFileOutputNodeCommand& aCmd)
{
    //This node supports Query UUID from any state

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFFileOutputNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    //Try to match the input mimetype against any of
    //the custom interfaces for this node
    //Match against custom interface1...
    if (*mimetype == PVMF_FILE_OUTPUT_NODE_CUSTOM1_MIMETYPE
            //also match against base mimetypes for custom interface1,
            //unless exactmatch is set.
            || (!exactmatch && *mimetype == PVMF_FILE_OUTPUT_NODE_MIMETYPE)
            || (!exactmatch && *mimetype == PVMF_BASEMIMETYPE))
    {
        uuidvec->push_back(PvmfFileOutputNodeConfigUuid);
        uuidvec->push_back(PvmfComposerSizeAndDurationUuid);
        uuidvec->push_back(PvmfNodesSyncControlUuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFFileOutputNode::DoQueryInterface(PVMFFileOutputNodeCommand&  aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::DoQueryInterface"));

    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFFileOutputNodeCommandBase::Parse(uuid, ptr);

    if (queryInterface(*uuid, *ptr))
    {
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }
}

void PVMFFileOutputNode::DoInit(PVMFFileOutputNodeCommand& aCmd)
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
void PVMFFileOutputNode::DoPrepare(PVMFFileOutputNodeCommand& aCmd)
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

void PVMFFileOutputNode::DoStart(PVMFFileOutputNodeCommand& aCmd)
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
                ((PVMFFileOutputInPort*)iInPort)->Start();
            }
            if (!iFileOpened)
            {
                if (iFs.Connect() != 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::DoStart: iFs.Connect Error."));
                    status = PVMFErrNoResources;
                    break;
                }

                if (0 != iOutputFile.Open(iOutputFileName.get_cstr(), Oscl_File::MODE_READWRITE | Oscl_File::MODE_BINARY, iFs))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFFileOutputNode::DoStart: iOutputFile.Open Error."));
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

void PVMFFileOutputNode::DoStop(PVMFFileOutputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        case EPVMFNodePaused:
            // Stop data source
            if (iInPort)
            {

                ((PVMFFileOutputInPort*)iInPort)->Stop();
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

void PVMFFileOutputNode::DoFlush(PVMFFileOutputNodeCommand& aCmd)
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

void PVMFFileOutputNode::DoPause(PVMFFileOutputNodeCommand& aCmd)
{
    switch (iInterfaceState)
    {
        case EPVMFNodeStarted:
        {
            // Pause data source
            if (!iClock)
            {
                // If not using sync clock, pause processing of incoming data
                ((PVMFFileOutputInPort*)iInPort)->Pause();
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

void PVMFFileOutputNode::DoReset(PVMFFileOutputNodeCommand& aCmd)
{
    if (IsAdded())
    {
        if (iInPort)
        {
            OSCL_DELETE(((PVMFFileOutputInPort*)iInPort));
            iInPort = NULL;
        }

        //logoff & go back to Created state.
        SetState(EPVMFNodeIdle);
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        OSCL_LEAVE(OsclErrInvalidState);
    }
}


void PVMFFileOutputNode::DoRequestPort(PVMFFileOutputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::DoRequestPort"));
    //This node supports port request from any state

    //retrieve port tag.
    int32 tag;
    OSCL_String* portconfig;

    aCmd.PVMFFileOutputNodeCommandBase::Parse(tag, portconfig);

    //validate the tag...
    switch (tag)
    {
        case PVMF_FILE_OUTPUT_NODE_PORT_TYPE_SINK:
            break;
        default:
            //bad port tag
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFFileOutputNode::DoRequestPort: Error - Invalid port tag"));
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
                        (0, "PVMFFileOutputNode::DoRequestPort: Error - Format not supported (format was preset)"));
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
        return;
    }

    OSCL_TRY(err, iInPort = OSCL_NEW(PVMFFileOutputInPort, (tag, this)););
    if (err != OsclErrNone || !iInPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFFileOutputNode::DoRequestPort: Error - PVMFFileOutputInPort::Create() failed"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }

    ((PVMFFileOutputInPort*)iInPort)->SetClock(iClock);
    ((PVMFFileOutputInPort*)iInPort)->SetMargins(iEarlyMargin, iLateMargin);

    //if format was provided in mimestring, set it now.
    if (portconfig)
    {
        PVMFFormatType fmt = portconfig->get_str();
        if (fmt != PVMF_MIME_FORMAT_UNKNOWN
                && ((PVMFFileOutputInPort*)iInPort)->IsFormatSupported(fmt))
        {
            ((PVMFFileOutputInPort*)iInPort)->iFormat = fmt;
            ((PVMFFileOutputInPort*)iInPort)->FormatUpdated();
        }
    }

    //Return the port pointer to the caller.
    CommandComplete(iInputCommands, aCmd, PVMFSuccess, (OsclAny*)iInPort);
}

void PVMFFileOutputNode::DoReleasePort(PVMFFileOutputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::DoReleasePort"));

    if (iInPort)
    {
        OSCL_DELETE(((PVMFFileOutputInPort*)iInPort));
        iInPort = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        return;
    }
    CommandComplete(iInputCommands, aCmd, PVMFFailure);
}

void PVMFFileOutputNode::DoCancelAllCommands(PVMFFileOutputNodeCommand& aCmd)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFFileOutputNode::DoCancelAllCommands"));
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

void PVMFFileOutputNode::DoCancelCommand(PVMFFileOutputNodeCommand& aCmd)
{
    //extract the command ID from the parameters.
    PVMFCommandId id;
    aCmd.PVMFFileOutputNodeCommandBase::Parse(id);

    //first check "current" command if any
    {
        PVMFFileOutputNodeCommand* cmd = iCurrentCommand.FindById(id);
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
        PVMFFileOutputNodeCommand* cmd = iInputCommands.FindById(id, 1);
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
