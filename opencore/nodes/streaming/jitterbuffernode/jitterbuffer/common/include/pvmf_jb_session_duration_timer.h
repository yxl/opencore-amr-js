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
#ifndef PVMF_JB_SESSION_DURATION_TIMER_H_INCLUDED
#define PVMF_JB_SESSION_DURATION_TIMER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif


/**
 * Observer class for the session duration timer AO
 */
class PvmfJBSessionDurationTimerObserver
{
    public:
        virtual ~PvmfJBSessionDurationTimerObserver() {}
        /**
         * A timer event, indicating that the timer has expired.
         */
        virtual void PVMFJBSessionDurationTimerEvent() = 0;
};

/**
 * Sessionduration timer object to Jitter Buffer node.
 * This object generates event when the session duration expires
 */
class PvmfJBSessionDurationTimer : public OsclTimerObject
{
    public:
        PvmfJBSessionDurationTimer(PvmfJBSessionDurationTimerObserver* aObserver);

        virtual ~PvmfJBSessionDurationTimer();

        /** Start Timer */
        PVMFStatus Start();

        PVMFStatus setSessionDurationInMS(uint32 duration);

        uint32 getSessionDurationInMS()
        {
            return iSessionDurationInMS;
        }

        /** Stop Timer events */
        PVMFStatus Stop();

        virtual PVMFStatus Cancel();

        bool IsTimerStarted()
        {
            return iStarted;
        }

        void SetEstimatedServerClock(PVMFMediaClock* aEstimatedServerClock)
        {
            iEstimatedServerClock = aEstimatedServerClock;
        }

        void EstimatedServerClockUpdated();

        uint32 GetExpectedEstimatedServClockValAtSessionEnd()
        {
            return iExpectedEstimatedServClockValAtSessionEnd;
        }

        void setCurrentMonitoringIntervalInMS(uint32 aCurrentMonitoringIntervalInMS)
        {
            iCurrentMonitoringIntervalInMS = aCurrentMonitoringIntervalInMS;
        }

        uint64 GetMonitoringIntervalElapsed()
        {
            return iMonitoringIntervalElapsed;
        }

        void UpdateElapsedSessionDuration(uint32 aElapsedTime)
        {
            iElapsedSessionDurationInMS += aElapsedTime;
        }

        uint32 GetElapsedSessionDurationInMS()
        {
            return iElapsedSessionDurationInMS;
        }

        uint32 GetEstimatedServClockValAtLastCancel()
        {
            return iEstimatedServClockValAtLastCancel;
        }

        void ResetEstimatedServClockValAtLastCancel()
        {
            iEstimatedServClockValAtLastCancel = 0;
            if (iEstimatedServerClock != NULL)
            {
                uint32 timebase32 = 0;
                bool overflowFlag = false;
                iEstimatedServerClock->GetCurrentTime32(iEstimatedServClockValAtLastCancel, overflowFlag,
                                                        PVMF_MEDIA_CLOCK_MSEC, timebase32);
            }
        }

    private:
        void Run();

        uint32 iCurrentMonitoringIntervalInMS;
        uint32 iSessionDurationInMS;
        uint32 iElapsedSessionDurationInMS;
        PvmfJBSessionDurationTimerObserver* iObserver;
        PVLogger* ipLogger;
        bool iStarted;

        PVMFMediaClock iClock;
        PVMFTimebase_Tickcount iClockTimeBase;
        uint32 iTimerStartTimeInMS;
        uint64 iMonitoringIntervalElapsed;

        PVMFMediaClock* iEstimatedServerClock;
        uint32 iEstimatedServClockValAtLastCancel;
        uint32 iExpectedEstimatedServClockValAtSessionEnd;

        PVLogger *ipClockLoggerSessionDuration;
};


#endif // PVMF_JB_SESSION_DURATION_TIMER_H_INCLUDED




