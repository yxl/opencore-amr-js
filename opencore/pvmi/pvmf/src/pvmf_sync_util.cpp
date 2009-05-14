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
* @file pvmf_sync_util.cpp
* @brief Utility class to synchronize processing of media data to a specified clock.
*/

#ifndef PVMF_SYNC_UTIL_H_INCLUDED
#include "pvmf_sync_util.h"
#endif

//set this to 1 to disable AV sync and frame dropping.
//this option is used for performance testing but must be "0" in production code.
#define DISABLE_AV_SYNC  0

OSCL_EXPORT_REF PvmfSyncUtil::PvmfSyncUtil() :
        iClock(NULL),
        iFrameStepClock(NULL),
        iEarlyMargin(DEFAULT_EARLY_MARGIN),
        iLateMargin(DEFAULT_LATE_MARGIN),
        iSkipMediaData(false),
        iResumeTimestamp(0),
        iRenderSkippedData(false)
{
    iLogger = PVLogger::GetLoggerObject("PvmfSyncUtil");
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtil::SetClock(PVMFMediaClock* aClock)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::SetClockAndTimebase: aClock=0x%x", aClock));

    iClock = aClock;

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtil::SetFrameStepClock(PVMFMediaClock* aClock)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::SetClockAndTimebase: SetFrameStepClock=0x%x", aClock));

    iFrameStepClock = aClock;

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtil::SetMargins(int32 aEarlyMargin, int32 aLateMargin)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::SetMargins: aEarlyMargin=%d, aLateMargin=%d", aEarlyMargin, aLateMargin));

    iEarlyMargin = aEarlyMargin;
    iLateMargin = aLateMargin;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PvmfSyncStatus PvmfSyncUtil::SyncMediaData(PVMFTimestamp aDataTimestamp, uint32 aDuration, uint32& aMillisecondsEarly)
{
#if DISABLE_AV_SYNC
    iClock = NULL;
#endif

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PvmfSyncUtil::SyncMediaData: aDataTimestamp=%d aDuration=%d", aDataTimestamp, aDuration));

    // TEMPORARY UNTIL DESIGN DEALING WITH DATA WITHOUT TIMESTAMP IS IMPLEMENTED
    if (aDataTimestamp == 0xFFFFFFFF)
    {
        // 0xFFFFFFFF is a special value meaning no timestamp. Change to time 0
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PvmfSyncUtil::SyncMediaData: aDataTimestamp is 0xFFFFFFFF so changing it to 0"));
        aDataTimestamp = 0;
    }
    // END TEMPORARY

    if (iSkipMediaData)
    {
        if ((aDataTimestamp + aDuration) >= iResumeTimestamp)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PvmfSyncUtil::SyncMediaData: SkipMediaData complete"));
            iSkipMediaData = false;
            return PVMF_SYNC_SKIP_COMPLETE;
        }
        else
        {
            if (iRenderSkippedData)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PvmfSyncUtil::SyncMediaData: Render skipped data"));
                return PVMF_SYNC_SKIPPED_RENDER;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "PvmfSyncUtil::SyncMediaData: Drop skipped data"));
                //return the skip interval
                aMillisecondsEarly = (iResumeTimestamp - aDataTimestamp);
                return PVMF_SYNC_SKIPPED;
            }
        }
    }
    else if (!iClock)
    {
        // No clock to synchronize to. Process data ASAP.
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtil::SyncMediaData: No clock available"));
        return PVMF_SYNC_ON_TIME;
    }

    // Get current playback clock time
    uint32 currentTime = 0;
    bool overflow = false;
    iClock->GetCurrentTime32(currentTime, overflow, PVMF_MEDIA_CLOCK_MSEC);

    // Check if in sync
    if (currentTime > (uint32)iLateMargin)
    {
        // Normal case
        // In sync if beginning or tail end of data is within the sync window
        if ((aDataTimestamp >= (currentTime - iLateMargin) && aDataTimestamp <= (currentTime + iEarlyMargin)) ||
                ((aDataTimestamp + aDuration) >= (currentTime - iLateMargin) && (aDataTimestamp + aDuration) <= (currentTime + iEarlyMargin)))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PvmfSyncUtil::SyncMediaData: In Sync Clock(%d), TS(%d)", currentTime, aDataTimestamp));
            return PVMF_SYNC_ON_TIME;
        }
    }
    else
    {
        // Special case when clock less than the late margin (to avoid substracting from 0 with unsigned int)
        // In sync if beginning is within the sync window
        if (aDataTimestamp <= (currentTime + iEarlyMargin))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PvmfSyncUtil::SyncMediaData: In Sync Clock(%d), TS(%d)", currentTime, aDataTimestamp));
            return PVMF_SYNC_ON_TIME;
        }
    }

    // Early if the beginning of the data is before
    // sync window
    if (aDataTimestamp > (currentTime + iEarlyMargin))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtil::SyncMediaData: Early data  Clock(%d), TS(%d) Dur(%d)", currentTime, aDataTimestamp, aDuration));
        aMillisecondsEarly = (aDataTimestamp - currentTime - iEarlyMargin);
        return PVMF_SYNC_EARLY;
    }
    else
    {
        // Late for all other cases
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PvmfSyncUtil::SyncMediaData: Late data  Clock(%d), TS(%d) Dur(%d)", currentTime, aDataTimestamp, aDuration));
        //return the lateness amount
        aMillisecondsEarly = ((currentTime + iEarlyMargin) - aDataTimestamp);
        return PVMF_SYNC_LATE; // Late frame
    }
}

OSCL_EXPORT_REF PVMFStatus PvmfSyncUtil::SkipMediaData(PVMFTimestamp aResumeTimestamp, bool aRenderSkippedData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PvmfSyncUtil::SkipMediaData: Resume(%d), Render(%d)", aResumeTimestamp, aRenderSkippedData));
    iSkipMediaData = true;
    iResumeTimestamp = aResumeTimestamp;
    iRenderSkippedData = aRenderSkippedData;
    return PVMFSuccess;
}

OSCL_EXPORT_REF void PvmfSyncUtil::CancelSkipMediaData()
{
    iSkipMediaData = false;
}







