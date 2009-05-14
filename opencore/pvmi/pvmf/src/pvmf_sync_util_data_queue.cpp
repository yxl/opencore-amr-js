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
 * @file pvmf_sync_util_data_queue.cpp
 * @brief Utility class to synchronize processing of media data to a specified clock.
 */

#ifndef PVMF_SYNC_UTIL_DATA_QUEUE_H_INCLUDED
#include "pvmf_sync_util_data_queue.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#include "pvmf_media_msg_format_ids.h"

//set this to 1 to disable AV sync and frame dropping.
//this option is used for performance testing but must be "0" in production code.
#define DISABLE_AV_SYNC 0


#define LOGDATAPATH(x)	PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iDatapathLogger, PVLOGMSG_INFO, x);


#define LOGDIAGNOSTICS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_PROF,iDiagnosticsLogger,PVLOGMSG_INFO,m);


OSCL_EXPORT_REF PvmfSyncUtilDataQueue::PvmfSyncUtilDataQueue(PvmfSyncUtilDataQueueObserver* aObserver, PvmfSyncUtil* aUtil, char*name) :
        iObserver(aObserver),
        iSyncUtil(aUtil),
        iDataQueue(DEFAULT_QUEUE_RESERVE_SIZE),
        iDiagnosticsLogged(false),
        iDropFrameCount(0)
{
    iLogger = PVLogger::GetLoggerObject("PvmfSyncUtilDataQueue");
    iDiagnosticsLogger = PVLogger::GetLoggerObject("pvplayerdiagnostics.syncutil");
    iLateFrameDropEnable = true;
    iClock = NULL;
    iClockNotificationsInf = NULL;
    iSyncFrameCount = iClockFrameCount = 0;
    iClockOwner = false;
    SetName(name);
    iOsclErrorTrapImp = OsclErrorTrap::GetErrorTrapImp();
}


OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::SetName(const char*name)
{
    if (name
            && name[0] != '\0')
    {
        iName = name;
        iDatapathLogger = PVLogger::GetLoggerObject("datapath");
    }
    else
    {
        iDatapathLogger = NULL;
    }
}


OSCL_EXPORT_REF PvmfSyncUtilDataQueue::~PvmfSyncUtilDataQueue()
{
    LogDiagnostics();
    Clear();

    //remove ourself as observer of clock.
    if (iClockNotificationsInf && iClock)
    {
        iClockNotificationsInf->RemoveClockObserver(*this);
        iClock->DestroyMediaClockNotificationsInterface(iClockNotificationsInf);
    }
}

void PvmfSyncUtilDataQueue::PassClockToSyncUtil()
{
    if (!iSyncUtil)
        return;

    //clock or timebase has been updated-- decide how to update
    //the sync util clock.

    if (FrameSyncMode())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PvmfSyncUtil::PassClockToSyncUtil Enabling Clock in sync util"));
        iSyncUtil->SetClock(iClock);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PvmfSyncUtil::PassClockToSyncUtil Disabling Clock in sync util"));
        iSyncUtil->SetClock(NULL);
    }
}

OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::ClockTimebaseUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::ClockTimebaseUpdated"));

    if (!iClock)
        return;

    if (iClock->GetCountTimebase())
    {
        //Reset the frame step delta to zero.
        iClock->GetCountTimebase()->GetCount(iClockFrameCount);
        iSyncFrameCount = iClockFrameCount;
    }

    //reset the sync util clock if needed
    PassClockToSyncUtil();

    //wake up the observer since we might need to process data now.
    if (iObserver)
        iObserver->ScheduleProcessData(this, 0);
}

OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::ClockCountUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::ClockCountUpdated"));

    if (iClock
            && iClock->GetCountTimebase())
    {
        //read the new framecount
        iClock->GetCountTimebase()->GetCount(iClockFrameCount);

        //wakeup the observer since we might need to process data now
        if (iObserver)
            iObserver->ScheduleProcessData(this, 0);
    }
}

OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::NotificationsInterfaceDestroyed()
{
    iClockNotificationsInf = NULL;
}

OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::ClockAdjusted()
{
    uint32 clktime;
    uint32 tbtime;
    bool overflow;
    iClock->GetCurrentTime32(clktime, overflow, PVMF_MEDIA_CLOCK_MSEC, tbtime);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::ClockAdjusted, new value %d", (uint32)clktime));
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::SetClock(PVMFMediaClock* aClock)
{
    //set clock for 'sync always' mode.
    return DoSetClock(aClock, true);
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::SetClockForFrameStep(PVMFMediaClock* aClock)
{
    //set clock for 'sync only during frame step' mode.
    return DoSetClock(aClock, false);
}

PVMFStatus PvmfSyncUtilDataQueue::DoSetClock(PVMFMediaClock* aClock, bool aSyncAlways)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::DoSetClock, Clk 0x%x syncAlways %d", aClock, aSyncAlways));

    //remove ourself as observer of old clock, if any.
    if (iClockNotificationsInf && iClock)
    {
        iClockNotificationsInf->RemoveClockObserver(*this);
        iClock->DestroyMediaClockNotificationsInterface(iClockNotificationsInf);
        iClockNotificationsInf = NULL;
    }

    //save new clock.
    iClock = aClock;

    if (iClock)
    {
        iClock->ConstructMediaClockNotificationsInterface(iClockNotificationsInf, *this);
    }

    //set ourself as observer of new clock.
    if (iClockNotificationsInf)
    {
        iClockNotificationsInf->SetClockObserver(*this);
    }

    //make a note of initial clock timebase
    ClockTimebaseUpdated();

    //aSyncAlways==true means we're the clock owner, so should do both
    //sync and frame-step mode.
    //aSyncAlways==false means the MIO component owns the clock, so it
    //does sync mode and frame-step.  We need to sync logic during
    //frame-stepping, since the MIO may not have sync capability.
    iClockOwner = aSyncAlways;

    //pass new clock to sync util also.
    PassClockToSyncUtil();

    return PVMFFailure;//no sync util!
}

void PvmfSyncUtilDataQueue::LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char* msg)
//log media msg info, description, and associated q-depth.
{
    OSCL_UNUSED_ARG(aMediaMsg);
    OSCL_UNUSED_ARG(msg);
    if (!iDatapathLogger)
    {
        return;//unexpected call.
    }

    LOGDATAPATH
    ((0, "SYNC %s %s MediaMsg SeqNum %d, SId %d, TS %d, Dur %d, Q-depth %d/%d", iName.get_cstr()
      , msg
      , aMediaMsg->getSeqNum()
      , aMediaMsg->getStreamID()
      , aMediaMsg->getTimestamp()
      , aMediaMsg->getDuration()
      , iDataQueue.size()
      , iDataQueue.capacity()
     ));
}


void PvmfSyncUtilDataQueue::LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char* msg, uint32 time)
//log media msg info, time value, description, and associated q-depth.
{
    OSCL_UNUSED_ARG(aMediaMsg);
    OSCL_UNUSED_ARG(msg);
    OSCL_UNUSED_ARG(time);
    if (!iDatapathLogger)
    {
        return;//unexpected call.
    }

    LOGDATAPATH
    ((0, "SYNC %s %s Msec %d MediaMsg SeqNum %d, SId %d, TS %d, Dur %d, Q-depth %d/%d", iName.get_cstr()
      , msg
      , time
      , aMediaMsg->getSeqNum()
      , aMediaMsg->getStreamID()
      , aMediaMsg->getTimestamp()
      , aMediaMsg->getDuration()
      , iDataQueue.size()
      , iDataQueue.capacity()
     ));
}


OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::ReserveDataQueue(uint32 aReserveSize)
{
    if (aReserveSize <= iDataQueue.capacity())
    {
        return PVMFSuccess;
    }

    int32 err = OsclErrNone;
    OSCL_TRY(err,
             iDataQueue.reserve(aReserveSize);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         return PVMFErrNoMemory;
                        );

    return PVMFSuccess;
}


OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::QueueMediaData(PVMFSharedMediaMsgPtr aMediaMsg, uint32* aDropped, uint32* aSkipped, bool aFront)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PvmfSyncUtilDataQueue::QueueMediaData front %d", aFront));

    if (aDropped)
    {
        *aDropped = 0;
    }
    if (aSkipped)
    {
        *aSkipped = 0;
    }

    int32 err = OsclErrNone;
    if (aFront)
    {
        OSCL_TRY_NO_TLS(iOsclErrorTrapImp, err, iDataQueue.push_front(aMediaMsg););
    }
    else
    {
        OSCL_TRY_NO_TLS(iOsclErrorTrapImp, err, iDataQueue.push_back(aMediaMsg););
    }
    OSCL_FIRST_CATCH_ANY(err,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                         (0, "PvmfSyncUtilDataQueue::QueueMediaData: Error - No Memory"));
                         return PVMFErrNoMemory;
                        );

    if (iDatapathLogger)
    {
        LogMediaMsgInfo(aMediaMsg, "Data Q'd");
    }

    if (iDataQueue.size() == 1)
    {
        // Only schedule the next media data if the data queue was empty before the
        // current media data was added.  Otherwise, the data will be scheduled
        // later as the data queue is being processed.
        PVMFStatus status = SynchronizeData(aDropped, aSkipped);
        if (status == PVMFSuccess)
        {
            iObserver->ScheduleProcessData(this, 0);
        }

        return status;
    }
    else
    {
        return PVMFSuccess;
    }
}


OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::SetLateFrameDropMode(bool aDropFrame)
{
    iLateFrameDropEnable = aDropFrame;
}


OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::DequeueMediaData(PVMFSharedMediaMsgPtr& aMediaMsg, uint32* aDropped, uint32* aSkipped)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtilDataQueue::DequeueMediaData"));

    if (aDropped)
    {
        *aDropped = 0;
    }
    if (aSkipped)
    {
        *aSkipped = 0;
    }

    switch (SynchronizeData(aDropped, aSkipped))
    {
        case PVMFSuccess:
            //return current frame.
            aMediaMsg = iDataQueue[0];
            iDataQueue.erase(iDataQueue.begin());
            //log to datapath
            if (iDatapathLogger)
            {
                LogMediaMsgInfo(aMediaMsg, "Data De-Q'd");
            }

            return PVMFSuccess;

        case PVMFPending:
            return PVMFPending;

        default:
            return PVMFFailure;
    }
}


OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::SkipMediaData(PVMFTimestamp aResumeTimestamp, bool aRenderSkippedData)
{
    if (iSyncUtil->SkipMediaData(aResumeTimestamp, aRenderSkippedData) == PVMFSuccess)
    {
        return PVMFPending;
    }
    else
    {
        return PVMFFailure;
    }
}


OSCL_EXPORT_REF void PvmfSyncUtilDataQueue::CancelSkipMediaData()
{
    iSyncUtil->CancelSkipMediaData();
}


OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::Clear()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtilDataQueue::Clear"));

    LogDiagnostics();

    while (!iDataQueue.empty())
    {
        PVMFSharedMediaMsgPtr msg = iDataQueue.front();
        iDataQueue.erase(iDataQueue.begin());
        if (iDatapathLogger)
        {
            LogMediaMsgInfo(msg, "Data Cleared");
        }
    }
    return PVMFSuccess;
}

void PvmfSyncUtilDataQueue::FrameStepClkAdjust(PVMFTimestamp aTimestamp)
{
    //update the clock value for frame-step mode.

    //In normal playback, we'd be waiting until the clock and
    //timestamp to match before releasing a frame.
    //In frame-step mode, the clock is not advancing on its own, and we're
    //releasing frames based on commands regardless of clock time.
    //If we don't adjust the clock, the A/V will not stay sync'd, and we'll
    //have trouble later when resuming normal playback.
    //Therefore we explicitely set the clock time to the frame timestamp
    //each time a frame is released due to a frame step command.

    //if the frame timestamp is ahead of the clock, update the
    //clock to the timestamp.
    uint32 clktime;
    uint32 tbtime;
    bool overflow = 0;
    iClock->GetCurrentTime32(clktime, overflow, PVMF_MEDIA_CLOCK_MSEC, tbtime);
    if (aTimestamp > (PVMFTimestamp)clktime)
    {
        uint32 adjtime = aTimestamp;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtilDataQueue::FrameStepClkAdjust: from %d to %d", (uint32)clktime, (uint32)adjtime));
        PVMFMediaClockAdjustTimeStatus ok = iClock->AdjustClockTime32(clktime, tbtime, adjtime, PVMF_MEDIA_CLOCK_MSEC, overflow);
        if (PVMF_MEDIA_CLOCK_ADJUST_SUCCESS != ok)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PvmfSyncUtilDataQueue::FrameStepClkAdjust: from %d to %d FAILED", (uint32)clktime, (uint32)adjtime));
        }
    }
}

bool PvmfSyncUtilDataQueue::FrameStepMode()
{
    //apply the frame-step logic only when the clock is in frame-step mode,
    //and we're the clock owner.
    return(iClockOwner
           && iClock
           && iClock->GetCountTimebase());
}

bool PvmfSyncUtilDataQueue::FrameSyncMode()
{
    //apply frame sync logic when we are clock owner
    // or else not clock owner but in frame-step mode.
    return(iClockOwner
           || (iClock && iClock->GetCountTimebase()));
}

PVMFStatus PvmfSyncUtilDataQueue::FrameStep()
{
    //Decide whether or not to release the next media msg in frame-step mode.

    //Dropped, skipped, and skipped-but-rendered frames have already been handled
    //elsewhere.

    //release any media cmd ASAP, except EOS.
    if (iDataQueue[0]->getFormatID() >= PVMF_MEDIA_CMD_FORMAT_IDS_START
            && iDataQueue[0]->getFormatID() != PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtilDataQueue::FrameStep: Returning media cmd(id=%d)", iDataQueue[0]->getFormatID()));
        return PVMFSuccess;
    }

    //for media data or EOS cmd, release frames based on frame-step commands to
    //the timebase.

    if (iSyncFrameCount != iClockFrameCount)
    {
        //release the frame now.

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtilDataQueue::FrameStep: Release Frame Now TS %d", iDataQueue[0]->getTimestamp()));

        //Each time a frame is released, reduce the delta by 1, regardless
        //of whether it's forward or backward stepping.
        if (iSyncFrameCount < iClockFrameCount)
        {
            iSyncFrameCount++;
        }
        else
        {
            iSyncFrameCount--;
        }

        //adjust the clock to the frame timestamp.
        FrameStepClkAdjust(iDataQueue[0]->getTimestamp());

        return PVMFSuccess;
    }
    else
    {
        //wait on a frame-step command to the timebase

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtilDataQueue::FrameStep: Wait for Frame Step Cmd"));

        return PVMFPending;
    }
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtilDataQueue::SynchronizeData(uint32*aDropped, uint32*aSkipped)
{
    //This call evaluates the data queue.
    //It returns PVMFSuccess if it has determined that the frame or media cmd
    //on the top of the data queue should be sent downstream.


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtilDataQueue::SynchronizeData"));

    uint32 millisecondsEarly = 0;
    while (!iDataQueue.empty())
    {
        uint32 duration = 0;
        PVMFSharedMediaMsgPtr mediaMsg = iDataQueue[0];
        /* Retrieve duration only for media msgs, not for media cmds (viz. EOS, Reconfig etc) */
        if (mediaMsg->getFormatID() == PVMF_MEDIA_MSG_DATA_FORMAT_ID)
        {
            PVMFSharedMediaDataPtr mediaData;
            convertToPVMFMediaData(mediaData, mediaMsg);
            if (mediaData->getMarkerInfo() & PVMF_MEDIA_DATA_MARKER_INFO_DURATION_AVAILABLE_BIT)
            {
                duration = mediaMsg->getDuration();
            }
        }

        if (mediaMsg->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
        {
            PVMFSharedMediaCmdPtr mediaCmd;
            convertToPVMFMediaCmd(mediaCmd, mediaMsg);
            duration = mediaCmd->getDuration();
        }

        PvmfSyncStatus syncStatus =
            iSyncUtil->SyncMediaData(iDataQueue[0]->getTimestamp(),
                                     duration,
                                     millisecondsEarly);
        switch (syncStatus)
        {
            case PVMF_SYNC_ON_TIME:
                if (FrameStepMode())
                {
                    //frame-step mode, on-time.  frame-step rules apply.
                    return FrameStep();
                }
                else
                {
                    //normal playback, on-time.   release the frame now.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: On Time %d", iDataQueue[0]->getTimestamp()));
                    return PVMFSuccess;
                }
                // break;	This statement was removed to avoid compiler warning for Unreachable Code

            case PVMF_SYNC_EARLY:
                // For media command, ignore the synchronization except for EOS media command
                // which cannot be processed earlier than the specified timestamp
                if (iDataQueue[0]->getFormatID() >= PVMF_MEDIA_CMD_FORMAT_IDS_START &&
                        iDataQueue[0]->getFormatID() != PVMF_MEDIA_CMD_EOS_FORMAT_ID)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: Returning media cmd(id=%d) as being in sync,%s", iDataQueue[0]->getFormatID(), iName.get_cstr()));
                    return PVMFSuccess;
                }
                else if (FrameStepMode())
                {
                    //frame-step mode, early frame.  frame-step rules apply.
                    return FrameStep();
                }
                else
                {
                    //normal mode, early frame. Schedule re-processing after enough
                    //time has elapsed to make the frame on-time.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: Schedule to process TS %d %d ms later,%s", iDataQueue[0]->getTimestamp(), millisecondsEarly, iName.get_cstr()));
                    if (iDatapathLogger)
                    {
                        LogMediaMsgInfo(iDataQueue[0], "Early Frame, Delay", millisecondsEarly);
                    }
                    iObserver->ScheduleProcessData(this, millisecondsEarly);
                    return PVMFPending;
                }
                // break;	This statement was removed to avoid compiler warning for Unreachable Code

            case PVMF_SYNC_LATE:
                // For media command, ignore the synchronization
                if (iDataQueue[0]->getFormatID() >= PVMF_MEDIA_CMD_FORMAT_IDS_START)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: Returning media cmd(id=%d) as being in sync,%s", iDataQueue[0]->getFormatID(), iName.get_cstr()));
                    return PVMFSuccess;
                }
#if(DISABLE_AV_SYNC)
                else if (true)
                {
                    return PVMFSuccess;
                }
#endif
                else if (iLateFrameDropEnable)
                {
                    //dropping late frames.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: Drop late frame TS %d, Dur %d, delta %d, %s",
                                     iDataQueue[0]->getTimestamp(), iDataQueue[0]->getDuration(), millisecondsEarly, iName.get_cstr()));
                    PVMFSharedMediaMsgPtr msg = iDataQueue.front();
                    iDataQueue.erase(iDataQueue.begin());
                    if (iDatapathLogger)
                    {
                        LogMediaMsgInfo(msg, "Late Frame Dropped", millisecondsEarly);
                    }
                    if (aDropped)
                    {
                        (*aDropped)++;
                        iDropFrameCount++;
                    }
                    //continue looping...
                    break;
                }
                else if (FrameStepMode())
                {
                    //frame step mode, late frame, not dropping frames.
                    //frame-step rules apply.
                    return FrameStep();
                }
                else
                {
                    //normal mode, late frame, not dropping frames.  release this
                    //frame ASAP.
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: Process late frame ASAP, TS %d,%s", iDataQueue[0]->getTimestamp(), iName.get_cstr()));
                    return PVMFSuccess;
                }
                // break;	This statement was removed to avoid compiler warning for Unreachable Code

            case PVMF_SYNC_SKIPPED_RENDER:
            {
                //this is a skipped frame, but we are rendering skipped frames.
                //release the frame ASAP.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PvmfSyncUtilDataQueue::SynchronizeData: Render Skipped Data TS %d", iDataQueue[0]->getTimestamp()));

                PVMFSharedMediaMsgPtr mediaMsg = iDataQueue[0];
                if (mediaMsg->getFormatID() == PVMF_MEDIA_MSG_DATA_FORMAT_ID)
                {
                    PVMFSharedMediaDataPtr mediaData;
                    convertToPVMFMediaData(mediaData, mediaMsg);
                    uint32 markerinfo = mediaData->getMarkerInfo();
                    markerinfo |= PVMF_MEDIA_DATA_MARKER_INFO_NO_RENDER_BIT;
                    mediaData->setMarkerInfo(markerinfo);
                }
            }
            return PVMFSuccess;

            case PVMF_SYNC_SKIPPED:
                // For media command, ignore the synchronization and release
                // the cmd. (why not drop it?)
                if (iDataQueue[0]->getFormatID() >= PVMF_MEDIA_CMD_FORMAT_IDS_START)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: Returning media cmd(id=%d) as being in sync,%s", iDataQueue[0]->getFormatID(), iName.get_cstr()));
                    return PVMFSuccess;
                }
                //This is a skipped frame, and we are not rendering skipped frames.
                //Drop the frame.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PvmfSyncUtilDataQueue::SynchronizeData: Skipped frame TS %d Dur %d,%s",
                                 iDataQueue[0]->getTimestamp(), iDataQueue[0]->getDuration(), iName.get_cstr()));
                {
                    PVMFSharedMediaMsgPtr msg = iDataQueue.front();
                    iDataQueue.erase(iDataQueue.begin());
                    if (iDatapathLogger)
                    {
                        LogMediaMsgInfo(msg, "Frame Skipped", millisecondsEarly);
                    }
                }
                if (aSkipped)
                {
                    (*aSkipped)++;
                }
                //continue looping...
                break;

            case PVMF_SYNC_SKIP_COMPLETE:
                //Skipping is complete.  Notify observer, but hang onto the frame
                //to re-evaluate in next sync call.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PvmfSyncUtilDataQueue::SynchronizeData: Skip complete TS %d,%s", iDataQueue[0]->getTimestamp(), iName.get_cstr()));
                iObserver->SkipMediaDataComplete();
                return PVMFPending;
        }
    }

    //If we get here, the queue is now empty, and all data was dropped or skipped.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PvmfSyncUtilDataQueue::SynchronizeData: All data dropped or skipped,%s", iName.get_cstr()));
    return PVMFFailure;
}

void PvmfSyncUtilDataQueue::LogDiagnostics()
{
    if (iDiagnosticsLogged == false)
    {
        iDiagnosticsLogged = true;
        LOGDIAGNOSTICS((0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"));
        LOGDIAGNOSTICS((0, "TrackMimeType = %s", iName.get_cstr()));
        LOGDIAGNOSTICS((0, "PvmfSyncUtilDataQueue - Num Frames Dropped=%d", iDropFrameCount));
        iDropFrameCount = 0;
    }
}





