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
 * @file pvmf_jb_session_duration_timer.cpp
 * @brief Session Duration timer to Jitter Buffer
 */
#ifndef PVMF_JB_SESSION_DURATION_TIMER_H_INCLUDED
#include "pvmf_jb_session_duration_timer.h"
#endif

#ifndef PVMF_JITTER_BUFFER_COMMON_INTERNAL_H
#include "pvmf_jitter_buffer_common_internal.h"
#endif

////////////////////////////////////////////////////////////////////////////
PvmfJBSessionDurationTimer::PvmfJBSessionDurationTimer(PvmfJBSessionDurationTimerObserver* aObserver)
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PvmfJBSessionDurationTimer"),
        iCurrentMonitoringIntervalInMS(0),
        iSessionDurationInMS(0),
        iElapsedSessionDurationInMS(0),
        iObserver(aObserver),
        iStarted(false),
        iTimerStartTimeInMS(0),
        iMonitoringIntervalElapsed(0),
        iEstimatedServerClock(NULL),
        iEstimatedServClockValAtLastCancel(0),
        iExpectedEstimatedServClockValAtSessionEnd(0)
{
    ipLogger = PVLogger::GetLoggerObject("PvmfJBSessionDurationTimer");
    ipClockLoggerSessionDuration = PVLogger::GetLoggerObject("clock.streaming_manager.sessionduration");
    AddToScheduler();
    iClock.SetClockTimebase(iClockTimeBase);
}


////////////////////////////////////////////////////////////////////////////
PvmfJBSessionDurationTimer::~PvmfJBSessionDurationTimer()
{
    Stop();
    iEstimatedServerClock = NULL;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfJBSessionDurationTimer::Start()
{
    PVMF_JB_LOGINFO((0, "PvmfJBSessionDurationTimer::Start"));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Start"));
    if ((iSessionDurationInMS > 0) && (iCurrentMonitoringIntervalInMS > 0))
    {
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Start - SessionDurationInMS = %d", iSessionDurationInMS));
        iClock.Start();
        uint32 timebase32 = 0;
        iTimerStartTimeInMS = 0;
        iMonitoringIntervalElapsed = 0;
        bool overflowFlag = false;
        iClock.GetCurrentTime32(iTimerStartTimeInMS, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
        /* Compute expected estimated serv clock value when duration expires */
        if (iEstimatedServerClock != NULL)
        {
            iExpectedEstimatedServClockValAtSessionEnd = iEstimatedServClockValAtLastCancel;
            uint32 currEstServClk32;
            currEstServClk32 = iExpectedEstimatedServClockValAtSessionEnd;
            PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Start - CurrEstServClock  = %d", currEstServClk32));
            uint32 remainingSessionDuration32 = 0;
            remainingSessionDuration32 = iSessionDurationInMS - iElapsedSessionDurationInMS;
            iExpectedEstimatedServClockValAtSessionEnd += remainingSessionDuration32;

            PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Start - ExpectedEstimatedServClockValAtSessionEnd = %d", iExpectedEstimatedServClockValAtSessionEnd));
        }
        RunIfNotReady(iCurrentMonitoringIntervalInMS*1000);
        iStarted = true;
        return PVMFSuccess;
    }
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfJBSessionDurationTimer::setSessionDurationInMS(uint32 duration)
{
    PVMF_JB_LOGINFO((0, "PvmfJBSessionDurationTimer::setMaxInactivityDurationInMS"));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::setMaxInactivityDurationInMS"));
    iSessionDurationInMS = duration;
    iElapsedSessionDurationInMS = 0;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfJBSessionDurationTimer::Stop()
{
    PVMF_JB_LOGINFO((0, "PvmfJBSessionDurationTimer::Stop"));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Stop"));
    OsclTimerObject::Cancel();
    iStarted = false;
    iSessionDurationInMS = 0;
    iClock.Stop();
    iTimerStartTimeInMS = 0;
    iMonitoringIntervalElapsed = 0;
    iExpectedEstimatedServClockValAtSessionEnd = 0;
    iEstimatedServClockValAtLastCancel = 0;
    iElapsedSessionDurationInMS = 0;
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PvmfJBSessionDurationTimer::Cancel()
{
    PVMF_JB_LOGINFO((0, "PvmfJBSessionDurationTimer::Cancel"));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Cancel"));
    iStarted = false;
    uint32 timebase32 = 0;
    uint32 cancelTime = 0;
    bool overflowFlag = false;
    iClock.GetCurrentTime32(cancelTime, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    iMonitoringIntervalElapsed = (cancelTime - iTimerStartTimeInMS);
    iEstimatedServClockValAtLastCancel = 0;
    if (iEstimatedServerClock != NULL)
    {
        uint32 timebase32 = 0;
        iEstimatedServerClock->GetCurrentTime32(iEstimatedServClockValAtLastCancel, overflowFlag,
                                                PVMF_MEDIA_CLOCK_MSEC, timebase32);
    }

    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Cancel - EstimatedServClockValAtLastCancel = %d", iEstimatedServClockValAtLastCancel));
    iClock.Stop();
    iTimerStartTimeInMS = 0;
    OsclTimerObject::Cancel();
    return PVMFSuccess;
}


void PvmfJBSessionDurationTimer::EstimatedServerClockUpdated()
{
    if (iEstimatedServerClock != NULL)
    {
        uint32 timebase32 = 0;
        uint32 estServClock = 0;
        bool overflowFlag = false;

        iEstimatedServerClock->GetCurrentTime32(estServClock, overflowFlag,
                                                PVMF_MEDIA_CLOCK_MSEC, timebase32);
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::EstimatedServerClockUpdated - CurrEstServClock = %2d", estServClock));
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::EstimatedServerClockUpdated - ExpectedEstServClock = %2d", iExpectedEstimatedServClockValAtSessionEnd));
        if (estServClock >= iExpectedEstimatedServClockValAtSessionEnd)
        {
            this->Cancel();
            iObserver->PVMFJBSessionDurationTimerEvent();
        }
    }
}

////////////////////////////////////////////////////////////////////////////
void PvmfJBSessionDurationTimer::Run()
{
    PVMF_JB_LOGINFO((0, "PvmfJBSessionDurationTimer::Run"));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PvmfJBSessionDurationTimer::Run"));

    if (!iStarted)
        return;

    if (!iObserver)
    {
        PVMF_JB_LOGERROR((0, "PvmfJBSessionDurationTimer::Run: Error - Observer not set"));
        return;
    }

    uint32 timebase32 = 0;
    uint32 cancelTime = 0;
    bool overflowFlag = false;

    iClock.GetCurrentTime32(cancelTime, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    iMonitoringIntervalElapsed = (cancelTime - iTimerStartTimeInMS);
    iClock.Stop();
    iTimerStartTimeInMS = 0;
    iObserver->PVMFJBSessionDurationTimerEvent();
    /*
     * Do not reschudule the AO here. Observer would reschedule this AO
     * once it is done processing the timer event.
     */
}

