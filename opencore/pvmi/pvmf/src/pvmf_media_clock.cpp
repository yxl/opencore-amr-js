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

#include "pvmf_media_clock.h"

OSCL_EXPORT_REF PVMFMediaClock::PVMFMediaClock() : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVMFMediaClock"),
        iState(STOPPED),
        iClockTimebase(NULL)
{
    iLogger = PVLogger::GetLoggerObject("PVMFMediaClock");
    iLatestRunningClockTime = 0;
    iLatestRunningTimebaseTime = 0;
    iStartTimebaseTickValue = 0;
    iStartClockTime = 0;
    iPauseClockTime = 0;
    iLastAdjustObsTimebaseTime = 0;
    iAdjustmentTimebaseTime = 0;
    iStartNPT = 0;
    iIsNPTPlayBackDirectionBackwards = 0;
    iStartMediaClockTS = 0;
    iClockUnit = PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC;
    iPreviousClockUnit = PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC;
    iActiveTimersCount = 0;
    iTimerIDCount = 1;
    OsclThread::GetId(iOrigThreadID);
    AddToScheduler();
    iMutex = OSCL_NEW(OsclMutex, ());
    iMutex->Create();

    //initialize this to real time
    iLastTimebaseRate = REALTIME_PLAYBACK_RATE;

    iIsTimebaseCountBased = false;
}

OSCL_EXPORT_REF PVMFMediaClock::~PVMFMediaClock()
{
    Reset();
    iMutex->Close();
    if (iMutex)
        OSCL_DELETE(iMutex);
    RemoveFromScheduler();
}

void PVMFMediaClock::CleanCallbackInfImplObjects()
{
    for (uint32 ii = 0;ii < iMediaClockSetCallbackObjects.size();ii++)
    {
        if (iMediaClockSetCallbackObjects[ii]->iNotificationInterfaceDestroyedCallback)
        {
            (iMediaClockSetCallbackObjects[ii]->iNotificationInterfaceDestroyedCallback)->NotificationsInterfaceDestroyed();
        }
        OSCL_DELETE(iMediaClockSetCallbackObjects[ii]);
        iMediaClockSetCallbackObjects.erase(&iMediaClockSetCallbackObjects[ii]);
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFMediaClock::ConstructMediaClockNotificationsInterface(PVMFMediaClockNotificationsInterface*& aIface,
        PVMFMediaClockNotificationsObsBase& aNotificationInterfaceDestroyedCallback,
        uint32 aLatency)
{

    PVMFMediaClockNotificationsInterfaceImpl* ifaceImpl = NULL;

    if (RUNNING == iState)
    {
        return PVMFErrInvalidState;
    }

    ifaceImpl = OSCL_NEW(PVMFMediaClockNotificationsInterfaceImpl, (this, aLatency, aNotificationInterfaceDestroyedCallback));
    aIface = OSCL_STATIC_CAST(PVMFMediaClockNotificationsInterface*, ifaceImpl);

    if (aIface)
    {
        iMediaClockSetCallbackObjects.push_back(ifaceImpl);
        AdjustLatenciesOfSinks();
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF void PVMFMediaClock::DestroyMediaClockNotificationsInterface(PVMFMediaClockNotificationsInterface* aInf)
{
    if (!aInf)
    {
        return;
    }

    //Cancel all active callbacks created from this interface object.
    if (!iTimersPriQueue.empty())
    {
        Oscl_Vector<PVMFMediaClockTimerQueueElement, OsclMemAllocator> qVector = iTimersPriQueue.vec();
        for (uint32 ii = 0;ii < qVector.size();ii++)
        {
            if (qVector[ii].pInterfaceObject == aInf)
            {
                CommonCancelCallback(qVector[ii].callBackID, false, false);
            }
        }
    }

    if (!iIsNPTPlayBackDirectionBackwards)
    {
        if (!iTimersPriQueueNPT.empty())
        {
            Oscl_Vector<PVMFMediaClockTimerQueueElement, OsclMemAllocator> qVector = iTimersPriQueueNPT.vec();
            for (uint32 ii = 0;ii < qVector.size();ii++)
            {
                if (qVector[ii].pInterfaceObject == aInf)
                {
                    CommonCancelCallback(qVector[ii].callBackID, false, true);
                }
            }
        }
    }
    else
    {
        if (!iTimersPriQueueNPTBackwards.empty())
        {
            Oscl_Vector<PVMFMediaClockTimerQueueElement, OsclMemAllocator> qVector = iTimersPriQueueNPTBackwards.vec();
            for (uint32 ii = 0;ii < qVector.size();ii++)
            {
                if (qVector[ii].pInterfaceObject == aInf)
                {
                    CommonCancelCallback(qVector[ii].callBackID, false, true);
                }
            }

        }
    }



    //Destroy the interface
    PVMFMediaClockNotificationsInterfaceImpl* ifaceImpl = NULL;

    ifaceImpl = OSCL_STATIC_CAST(PVMFMediaClockNotificationsInterfaceImpl*, aInf);
    for (uint32 ii = 0;ii < iMediaClockSetCallbackObjects.size();ii++)
    {
        if (iMediaClockSetCallbackObjects[ii] == ifaceImpl)
        {
            if (iMediaClockSetCallbackObjects[ii]->iNotificationInterfaceDestroyedCallback)
            {
                (iMediaClockSetCallbackObjects[ii]->iNotificationInterfaceDestroyedCallback)->NotificationsInterfaceDestroyed();
            }
            OSCL_DELETE(iMediaClockSetCallbackObjects[ii]);
            iMediaClockSetCallbackObjects.erase(&iMediaClockSetCallbackObjects[ii]);
        }
    }
}

OSCL_EXPORT_REF bool PVMFMediaClock::SetClockTimebase(PVMFTimebase& aTimebase)
{
    // Clock timebase can only be set during stopped or paused states
    if (iState == RUNNING)
    {
        return false;
    }

    // Save the clock timebase object pointer
    iClockTimebase = &aTimebase;

    if ((iClockTimebase->GetRate() != iLastTimebaseRate) &&
            iActiveTimersCount != 0)
    {
        //start scheduling afresh
        AdjustScheduling();
    }

    //Update timebase rate
    iLastTimebaseRate = iClockTimebase->GetRate();

    //If this is a counting timebase, then set the timebase
    //observer to this clock, so that we'll get the count update
    //notices.
    if (aTimebase.GetCountTimebase())
    {
        aTimebase.GetCountTimebase()->SetClockObserver(this);
        iIsTimebaseCountBased = true;
    }
    else
    {
        iIsTimebaseCountBased = false;
    }

    //Notify observers that the timebase is updated.
    ClockTimebaseUpdated();

    return true;
}
void PVMFMediaClock::AdjustLatenciesOfSinks()
{
    uint32 size = iMediaClockSetCallbackObjects.size();
    uint32 ii = 0;

    if (!size)
        return;

    //find the minimum and maximum latencies
    uint32 minLatency = iMediaClockSetCallbackObjects[0]->iLatency;
    uint32 maxLatency = iMediaClockSetCallbackObjects[0]->iLatency;
    for (ii = 0;ii < size - 1;ii++)
    {
        if (iMediaClockSetCallbackObjects[ii+1]->iLatency > iMediaClockSetCallbackObjects[ii]->iLatency)
        {
            minLatency = iMediaClockSetCallbackObjects[ii]->iLatency;
        }
        else
        {
            minLatency = iMediaClockSetCallbackObjects[ii+1]->iLatency;
        }

        if (iMediaClockSetCallbackObjects[ii+1]->iLatency > iMediaClockSetCallbackObjects[ii]->iLatency)
        {
            maxLatency = iMediaClockSetCallbackObjects[ii+1]->iLatency;
        }
        else
        {
            maxLatency = iMediaClockSetCallbackObjects[ii]->iLatency;
        }
    }

    //set adjusted-latencies and latency-delays
    for (ii = 0;ii < size;ii++)
    {
        iMediaClockSetCallbackObjects[ii]->iAdjustedLatency =
            iMediaClockSetCallbackObjects[ii]->iLatency - minLatency;

        iMediaClockSetCallbackObjects[ii]->iLatencyDelayForClockStartNotification =
            maxLatency - iMediaClockSetCallbackObjects[ii]->iLatency;
    }

}

OSCL_EXPORT_REF bool PVMFMediaClock::Start()
{
    bool overflowFlag = false;
    // Can only start from stopped or paused states
    if (iState == RUNNING)
    {
        return false;
    }


    uint32 tbval = 0;


    // Save the clock timebase value to the appropriate
    // variable and update the iLatest... values.
    if (iState == STOPPED)
    {
        // Retrieve the current time value from the clock timebase
        if (iClockTimebase != NULL)
        {
            iClockTimebase->GetCurrentTick32(iStartTimebaseTickValue, overflowFlag);
        }

        //can reuse overflowFlag as result would be virtually same
        GetScaledTimebaseTickCount(tbval, overflowFlag);
        // Starting from stop
        UpdateLatestTimes(iStartClockTime, tbval);
    }
    else
    {
        GetScaledTimebaseTickCount(tbval, overflowFlag);
        // Resuming from pause
        UpdateLatestTimes(iPauseClockTime, tbval);
    }

    // Change to running state
    SetClockState(RUNNING);

    //Restart callback scheduling
    AdjustScheduling();

    return true;
}

OSCL_EXPORT_REF bool PVMFMediaClock::Pause()
{
    bool overflowFlag = false;
    // Can only pause during running state
    if (iState != RUNNING)
    {
        return false;
    }

    // Save the current time
    uint32 tbval = 0;
    GetCurrentTime32(iPauseClockTime, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, tbval);
    UpdateLatestTimes(iPauseClockTime, tbval);

    // Change to paused state
    SetClockState(PAUSED);

    //Cancel any scheduled Run
    Cancel();
    return true;
}

OSCL_EXPORT_REF bool PVMFMediaClock::Stop()
{
    // Can only stop when running or paused
    if (iState == STOPPED)
    {
        return false;
    }

    // Reset the time values
    uint32 tmp = 0;
    UpdateLatestTimes(tmp, tmp);
    iStartClockTime = tmp;
    iPauseClockTime = tmp;
    iLastAdjustObsTimebaseTime = tmp;
    iAdjustmentTimebaseTime = tmp;
    iStartTimebaseTickValue = tmp;

    // Change to stopped state
    SetClockState(STOPPED);

    //Clear all callback queues
    ClearAllQueues();
    return true;
}

OSCL_EXPORT_REF void PVMFMediaClock::GetStartTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits)
{
    aOverflow = false;
    // Convert to the requested time units and return the start time
    FromClockUnit(iStartClockTime, aTime, aUnits, aOverflow);
}

OSCL_EXPORT_REF bool PVMFMediaClock::SetStartTime32(uint32& aTime, PVMFMediaClock_TimeUnits aUnits, bool& aOverFlow)
{
    aOverFlow = false;

    // Can only set start time while stopped
    if (iState != STOPPED)
    {
        return false;
    }

    iPreviousClockUnit = iClockUnit;
    // set clock units to usec if units arg is usec. Otherwise, default is msec
    (PVMF_MEDIA_CLOCK_USEC == aUnits) ? iClockUnit = PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC :
            iClockUnit = PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC;

    if (iPreviousClockUnit != iClockUnit)
    {
        AdjustClockInternalsToNewUnits(aOverFlow);
    }

    // Convert to clock units and set the start time
    bool overflowFlag1 = false;
    ToClockUnit(aTime, aUnits, iStartClockTime, overflowFlag1);

    aOverFlow = aOverFlow | overflowFlag1;

    //start scheduling afresh
    AdjustScheduling();
    return true;
}

OSCL_EXPORT_REF PVMFMediaClockAdjustTimeStatus PVMFMediaClock::AdjustClockTime32(uint32& aClockTime, uint32& aTimebaseTime, uint32& aAdjustedTime, PVMFMediaClock_TimeUnits aUnits, bool& aOverFlow)
{
    aOverFlow = false;

    // Clock can only be adjusted when it is running
    if (iState != RUNNING)
    {
        return PVMF_MEDIA_CLOCK_ADJUST_ERR_INVALID_STATE;
    }

    // Check if the adjustment's observed time is later than the last one
    uint32 temp = 0;
    if (PVTimeComparisonUtils::IsEarlier(aTimebaseTime, iLastAdjustObsTimebaseTime, temp) && (temp != 0))
    {
        // Old data so don't use it for adjustment
        return PVMF_MEDIA_CLOCK_ADJUST_ERR_INVALID_TIMEBASE_TIME;
    }

    iPreviousClockUnit = iClockUnit;

    // set clock units to usec if units arg is usec. Otherwise, default is msec
    (PVMF_MEDIA_CLOCK_USEC == aUnits) ? iClockUnit = PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC :
            iClockUnit = PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC;

    if (iPreviousClockUnit != iClockUnit)
    {
        AdjustClockInternalsToNewUnits(aOverFlow);
    }

    // Convert the observed clock and adjustment time to usec so it can be compared
    uint32 adjusttime, clocktime;
    bool overflowFlag1 = false, overflowFlag2 = false;
    ToClockUnit(aClockTime, aUnits, clocktime, overflowFlag1);
    ToClockUnit(aAdjustedTime, aUnits, adjusttime, overflowFlag2);

    // Make sure the adjustment's clock and timebase times are before current time
    uint32 currenttime = 0;
    uint32 tbval = 0;
    bool overflowFlag3 = false, overflowFlag4 = false;

    // Get the current timebase time
    GetScaledTimebaseTickCount(tbval, overflowFlag4);

    // Get the current clock time in clock units
    GetCurrentTime32(currenttime, overflowFlag3, iClockUnit == PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC ? PVMF_MEDIA_CLOCK_USEC :
                     PVMF_MEDIA_CLOCK_MSEC);

    aOverFlow = aOverFlow | overflowFlag1 | overflowFlag2 | overflowFlag3 | overflowFlag4;

    if (PVTimeComparisonUtils::IsEarlier(tbval, aTimebaseTime, temp) && (temp != 0))
    {
        // Observed timebase time cannot be later than the current timebase time
        return PVMF_MEDIA_CLOCK_ADJUST_ERR_INVALID_TIMEBASE_TIME;
    }

    if (clocktime > currenttime)
    {
        // Observed clock time cannot be later than the current clock time
        return PVMF_MEDIA_CLOCK_ADJUST_ERR_CORRUPT_CLOCK_TIME;
    }

    // Make the adjustment
    return AdjustClock(clocktime, aTimebaseTime, adjusttime, currenttime, tbval);

}

OSCL_EXPORT_REF bool PVMFMediaClock::Reset()
{
    bool tmpFlag = true;

    //stop clock if its not already stopped
    if (STOPPED != iState)
    {
        tmpFlag = Stop();
    }

    // remove all ClockObservers
    iClockObservers.clear();

    // remove interface objects. This will also remove all ClockStateObservers.
    CleanCallbackInfImplObjects();

    return tmpFlag;
}

OSCL_EXPORT_REF void PVMFMediaClock::GetCurrentTick32(uint32& aTimebaseTickCount, bool& aOverflow)
{
    aOverflow = false;

    if (iClockTimebase != NULL)
    {
        iClockTimebase->GetCurrentTick32(aTimebaseTickCount, aOverflow);
    }
    else
    {
        aTimebaseTickCount = 0;
    }
}

OSCL_EXPORT_REF void PVMFMediaClock::GetTimebaseResolution(uint32& aResolution)
{
    if (iClockTimebase)
    {
        // Retrieve the resolution from the timebase being used
        iClockTimebase->GetTimebaseResolution(aResolution);
    }
    else
    {
        // No timebase so set to 0
        aResolution = 0;
    }
}

OSCL_EXPORT_REF int32 PVMFMediaClock::GetRate(void)
{
    if (iClockTimebase)
    {
        // Retrieve the playback rate from the timebase being used
        return iClockTimebase->GetRate();
    }
    else
    {
        // No timebase so return realtime rate
        return REALTIME_PLAYBACK_RATE;
    }
}

OSCL_EXPORT_REF void PVMFMediaClock::GetCurrentTime32(uint32& aClockTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits)
{
    uint32 temp = 0;

    GetCurrentTime32(aClockTime, aOverflow, aUnits, temp);
}

OSCL_EXPORT_REF void PVMFMediaClock::GetCurrentTime32(uint32& aClockTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits, uint32& aTimebaseTime)
{
    // Get and return the current timebase value

    bool overflowFlag1 = false, overflowFlag2 = false, overflowFlag3 = false;

    aOverflow = false;

    // Get the current timebase time
    GetScaledTimebaseTickCount(aTimebaseTime, aOverflow);

    // Determine and return the current clock time
    if (iState == STOPPED)
    {
        // Return the specified start time
        FromClockUnit(iStartClockTime, aClockTime, aUnits, overflowFlag3);
    }
    else if (iState == PAUSED)
    {
        // Returned the paused time
        FromClockUnit(iPauseClockTime, aClockTime, aUnits, overflowFlag3);
    }
    else
    {
        // Running state
        uint32 currenttime;

        // Determine current clock time including any adjustment
        GetAdjustedRunningClockTime(currenttime, aTimebaseTime);

        // Convert to requested time units
        FromClockUnit(currenttime, aClockTime, aUnits, overflowFlag3);
    }

    aOverflow = aOverflow | overflowFlag1 | overflowFlag2 | overflowFlag3;
}

OSCL_EXPORT_REF bool PVMFMediaClock::QueryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMFMediaClockControlInterfaceUuid)
    {
        PVMFMediaClockControlInterface* myInterface =
            OSCL_STATIC_CAST(PVMFMediaClockControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFMediaClockAccessInterfaceUuid)
    {
        PVMFMediaClockAccessInterface* myInterface =
            OSCL_STATIC_CAST(PVMFMediaClockAccessInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFMediaClockNPTClockPositionAccessInterfaceUuid)
    {
        PVMFMediaClockNPTClockPositionAccessInterface* myInterface =
            OSCL_STATIC_CAST(PVMFMediaClockNPTClockPositionAccessInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }

    return true;
}

OSCL_EXPORT_REF void PVMFMediaClock::SetClockObserver(PVMFMediaClockObserver& aObserver)
{
    iClockObservers.push_back(&aObserver);
}

OSCL_EXPORT_REF void PVMFMediaClock::RemoveClockObserver(PVMFMediaClockObserver& aObserver)
{
    for (uint32 i = 0;i < iClockObservers.size();i++)
    {
        if (iClockObservers[i] == &aObserver)
            iClockObservers.erase(&iClockObservers[i]);
    }
}

void PVMFMediaClock::SetClockState(PVMFMediaClockState aState)
{
    // Change the clock state
    iState = aState;

    // Notify observers

    //If this is clock start, we need to send start notification after adjusting latency
    if (RUNNING == iState)
    {
        for (uint32 ii = 0;ii < iMediaClockSetCallbackObjects.size();ii++)
        {
            if (iMediaClockSetCallbackObjects[ii]->iClockStateObserver != NULL)
            {
                if (0 == iMediaClockSetCallbackObjects[ii]->iLatencyDelayForClockStartNotification)
                {
                    (iMediaClockSetCallbackObjects[ii]->iClockStateObserver)->ClockStateUpdated();
                }
                else
                {
                    //Queue notification
                    QueueClockStartNotificationEvent(iMediaClockSetCallbackObjects[ii]->iLatencyDelayForClockStartNotification,
                                                     iMediaClockSetCallbackObjects[ii]->iClockStateObserver);
                }
            }
        }
    }
    else
    {
        for (uint32 ii = 0;ii < iMediaClockSetCallbackObjects.size();ii++)
        {
            if (iMediaClockSetCallbackObjects[ii]->iClockStateObserver != NULL)
            {
                (iMediaClockSetCallbackObjects[ii]->iClockStateObserver)->ClockStateUpdated();
            }
        }
    }
}

void PVMFMediaClock::GetScaledTimebaseTickCount(uint32& aScaledTickCount, bool& aOverFlow)
{
    uint32 temp = 0;
    aOverFlow = false;

    if (iClockTimebase != NULL)
    {
        iClockTimebase->GetCurrentTick32(temp, aOverFlow);
    }

    PVTimeComparisonUtils::IsEarlier(iStartTimebaseTickValue, temp, aScaledTickCount);
}

void PVMFMediaClock::UpdateLatestTimes(uint32 aTime, uint32 aTimebaseVal)
{
    // Set the latest time values
    iLatestRunningClockTime = aTime;
    iLatestRunningTimebaseTime = aTimebaseVal;
}

void PVMFMediaClock::AdjustClockInternalsToNewUnits(bool& aOverFlow)
{

    uint32 temp = 0;

    aOverFlow = false;

    // Change the units
    if (PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC == iPreviousClockUnit)
    {
        ToClockUnit(iLatestRunningClockTime, PVMF_MEDIA_CLOCK_USEC, temp, aOverFlow);
        iLatestRunningClockTime = temp;

        ToClockUnit(iStartClockTime, PVMF_MEDIA_CLOCK_USEC, temp, aOverFlow);
        iStartClockTime = temp;

        ToClockUnit(iPauseClockTime, PVMF_MEDIA_CLOCK_USEC, temp, aOverFlow);
        iPauseClockTime = temp;
    }
    else if (PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC == iPreviousClockUnit)
    {
        ToClockUnit(iLatestRunningClockTime, PVMF_MEDIA_CLOCK_MSEC, temp, aOverFlow);
        iLatestRunningClockTime = temp;

        ToClockUnit(iStartClockTime, PVMF_MEDIA_CLOCK_MSEC, temp, aOverFlow);
        iStartClockTime = temp;

        ToClockUnit(iPauseClockTime, PVMF_MEDIA_CLOCK_MSEC, temp, aOverFlow);
        iPauseClockTime = temp;
    }
}

#define OSCL_DISABLE_WARNING_CONV_POSSIBLE_LOSS_OF_DATA
#include "osclconfig_compiler_warnings.h"

void PVMFMediaClock::ToClockUnit(uint32& aSrcVal, PVMFMediaClock_TimeUnits aSrcUnits, uint32& aDestVal, bool& aOverFlow)
{
    uint32 multconst = 1;

    aOverFlow = false;

    switch (iClockUnit)
    {
        case PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC:
        {
            if (PVMF_MEDIA_CLOCK_USEC == aSrcUnits)
            {
                aDestVal = aSrcVal / 1000;
            }
            else
            {
                // Determine the multiplier constant for the specified units
                switch (aSrcUnits)
                {
                    case PVMF_MEDIA_CLOCK_SEC:
                        multconst = 1000;
                        break;

                    case PVMF_MEDIA_CLOCK_MIN:
                        multconst = 60000;
                        break;

                    case PVMF_MEDIA_CLOCK_HOUR:
                        multconst = 0x36EE80;
                        break;

                    case PVMF_MEDIA_CLOCK_DAY:
                        multconst = 0x5265C00;
                        break;

                    case PVMF_MEDIA_CLOCK_MSEC:
                    default:
                        break;
                }

                // Convert value to clock units
                uint64 time64 = (uint64)(aSrcVal * multconst);

                //There is a chance that Tickcount did not wrap around but aTime value does
                if (time64 > (uint64)(0xFFFFFFFF))
                {
                    aOverFlow = true;
                }
                aDestVal = Oscl_Int64_Utils::get_uint64_lower32(time64);
            }
        }
        break;

        case PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC:
        {
            // Determine the multiplier constant for the specified units
            switch (aSrcUnits)
            {
                case PVMF_MEDIA_CLOCK_MSEC:
                    multconst = 1000;
                    break;

                case PVMF_MEDIA_CLOCK_SEC:
                    multconst = 1000000;
                    break;

                case PVMF_MEDIA_CLOCK_MIN:
                    multconst = 60000000;
                    break;

                case PVMF_MEDIA_CLOCK_HOUR:
                    multconst = 0xD693A400;
                    break;

                case PVMF_MEDIA_CLOCK_DAY:
                {
                    uint64 temp = UINT64_HILO(0x14, 0x1DD76000);
                    multconst = Oscl_Int64_Utils::get_uint64_lower32(temp);
                }
                break;

                case PVMF_MEDIA_CLOCK_USEC:
                default:
                    break;
            }

            // Convert value to clock units
            uint64 time64 = (uint64)(aSrcVal * multconst);
            //There is a chance that Tickcount did not wrap around but aTime value does
            if (time64 > (uint64)(0xFFFFFFFF))
            {
                aOverFlow = true;
            }
            aDestVal = Oscl_Int64_Utils::get_uint64_lower32(time64);
        }
        break;

        default:
        {
            break;
        }
    }

}

void PVMFMediaClock::FromClockUnit(uint32& aClockUnitVal, uint32& aDstVal,
                                   PVMFMediaClock_TimeUnits aDstUnits, bool& aOverflow)
{

    uint32 divconst = 1;

    aOverflow = false;

    switch (iClockUnit)
    {
        case PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC:
        {
            if (PVMF_MEDIA_CLOCK_USEC == aDstUnits)
            {
                //detect overflow;
                uint64 time64 = (uint64)(aClockUnitVal * 1000);
                if (time64 > (uint64)(0xFFFFFFFF))
                {
                    aOverflow = true;
                }
                aDstVal = Oscl_Int64_Utils::get_uint64_lower32(time64);
            }
            else
            {
                // Determine the multiplier constant for the specified units
                switch (aDstUnits)
                {
                    case PVMF_MEDIA_CLOCK_SEC:
                        divconst = 1000;
                        break;

                    case PVMF_MEDIA_CLOCK_MIN:
                        divconst = 60000;
                        break;

                    case PVMF_MEDIA_CLOCK_HOUR:
                        divconst = 3600000;
                        break;

                    case PVMF_MEDIA_CLOCK_DAY:
                        divconst = 86400000;
                        break;

                    case PVMF_MEDIA_CLOCK_MSEC:
                    default:
                        break;
                }

                // Convert value to desired units
                aDstVal = aClockUnitVal / divconst;
            }
        }
        break;

        case PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC:
        {
            // Determine the multiplier constant for the specified units
            switch (aDstUnits)
            {
                case PVMF_MEDIA_CLOCK_MSEC:
                    divconst = 1000;
                    break;

                case PVMF_MEDIA_CLOCK_SEC:
                    divconst = 1000000;
                    break;

                case PVMF_MEDIA_CLOCK_MIN:
                    divconst = 60000000;
                    break;

                case PVMF_MEDIA_CLOCK_HOUR:
                    divconst = 0xD693A400;
                    break;

                case PVMF_MEDIA_CLOCK_DAY:
                {
                    uint64 temp = UINT64_HILO(0x14, 0x1DD76000);
                    divconst = Oscl_Int64_Utils::get_uint64_lower32(temp);
                }
                break;

                case PVMF_MEDIA_CLOCK_USEC:
                default:
                    break;
            }
            // Convert value to desired units
            aDstVal = aClockUnitVal / divconst;

        }
        break;

        default:
        {
            break;
        }
    }
}

void PVMFMediaClock::ConvertTickcountToClockUnits(uint32 aTickcount, uint32& aTimeValue, bool& aOverflowFlag)
{
    uint32 tbval = aTickcount;

    aOverflowFlag = false;

    //Convert tickCount value to msecs
    uint32 usecPerTick = 0;
    GetTimebaseResolution(usecPerTick);

    if (usecPerTick)
    {
        tbval = tbval * (usecPerTick / 1000);
    }
    else /*timebase is not set*/
    {
        tbval = 0;
    }

    ToClockUnit(tbval, PVMF_MEDIA_CLOCK_MSEC, aTimeValue, aOverflowFlag);
}
void PVMFMediaClock::ToUSec(uint32& aSrcVal, PVMFMediaClock_TimeUnits aSrcUnits, uint32& aUSecVal, bool& aOverFlow)
{
    uint32 multconst = 1;

    aOverFlow = false;


    // Determine the multiplier constant for the specified units
    switch (aSrcUnits)
    {
        case PVMF_MEDIA_CLOCK_MSEC:
            multconst = 1000;
            break;

        case PVMF_MEDIA_CLOCK_SEC:
            multconst = 1000000;
            break;

        case PVMF_MEDIA_CLOCK_MIN:
            multconst = 60000000;
            break;

        case PVMF_MEDIA_CLOCK_HOUR:
            multconst = 0xD693A400;
            break;

        case PVMF_MEDIA_CLOCK_DAY:
        {
            uint64 temp = UINT64_HILO(0x14, 0x1DD76000);
            multconst = Oscl_Int64_Utils::get_uint64_lower32(temp);
        }
        break;

        case PVMF_MEDIA_CLOCK_USEC:
        default:
            break;
    }

    // Convert value to clock units
    uint64 time64 = (uint64)(aSrcVal * multconst);
    //There is a chance that Tickcount did not wrap around but aTime value does
    if (time64 > (uint64)(0xFFFFFFFF))
    {
        aOverFlow = true;
    }

    aUSecVal = Oscl_Int64_Utils::get_uint64_lower32(time64);
}

PVMFMediaClockAdjustTimeStatus PVMFMediaClock::AdjustClock(uint32& aObsTime, uint32& aObsTimebase, uint32& aAdjTime,
        uint32& aCurrentTime, uint32& aCurrentTimebase)
{
    // In this implementation, don't allow adjustments to be made with
    // data older than when the last adjustment was made
    uint32 temp = 0;
    if (PVTimeComparisonUtils::IsEarlier(aObsTimebase, iAdjustmentTimebaseTime, temp) && (temp != 0))
    {
        return PVMF_MEDIA_CLOCK_ADJUST_ERR_INVALID_TIMEBASE_TIME;
    }

    // Make the adjustment
    if (aAdjTime > aObsTime)
    {
        // Adjusted time is ahead so move ahead

        // Save the observed timebase time of the adjusted time
        iLastAdjustObsTimebaseTime = aObsTimebase;
        UpdateLatestTimes(aAdjTime, aObsTimebase);

        // Set the latest adjustment time as the current timebase time
        iAdjustmentTimebaseTime = aCurrentTimebase;
    }
    else if (aAdjTime < aObsTime)
    {
        // Adjusted time is before the current time

        // Save the observed timebase time of the adjusted time
        iLastAdjustObsTimebaseTime = aObsTimebase;

        // Set the latest clock time to the current clock time
        // Set the latest timebase time to (current timebase time) + ((observed clock time) - (adjusted time))
        uint32 offsettimebase = 0;
        uint32 usecPerTick = 0;

        GetTimebaseResolution(usecPerTick);

        //calculate ticks in offsettbtime
        if (PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC == iClockUnit)
        {
            uint32 usecInOffset = (aObsTime - aAdjTime) * 1000;

            //Calculate number of ticks
            offsettimebase = usecInOffset / usecPerTick;
        }
        else if (PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC == iClockUnit)
        {
            offsettimebase = (aObsTime - aAdjTime) / usecPerTick;
        }

        UpdateLatestTimes(aCurrentTime, aCurrentTimebase + offsettimebase);
        iAdjustmentTimebaseTime = aCurrentTimebase;
    }
    else
    {
        // Since there is no adjustment, do nothing
    }

    //Start fresh scheduling
    AdjustScheduling();

    ClockAdjusted();
    return PVMF_MEDIA_CLOCK_ADJUST_SUCCESS;
}

void PVMFMediaClock::GetAdjustedRunningClockTime(uint32& aDstTime, uint32& aTimebaseVal)
{
    uint32 delta = 0;
    // Current time is (latest clock time)+(current timebase time - latest timebase time)
    aDstTime = iLatestRunningClockTime;

    // If backward adjustment occurs, iLatestRunningTimebaseTime might be greater than
    // the current value. To avoid negative values and clock going back, only
    // add the diff if current timebase value is greater. This makes the clock "freeze" until
    // the difference has elapsed
    if (PVTimeComparisonUtils::IsEarlier(iLatestRunningTimebaseTime, aTimebaseVal, delta) && (delta != 0))
    {
        //convert delta to clock units
        uint32 deltaTime = 0;
        bool overflowFlag = false;
        ConvertTickcountToClockUnits(delta, deltaTime, overflowFlag);
        aDstTime += deltaTime;
    }
}

void PVMFMediaClock::ClockCountUpdated()
{
    //Calling Run will fire any ready timers.
    Run();

    //notify all observers that the clock count was updated.
    for (uint32 i = 0;i < iClockObservers.size();i++)
        iClockObservers[i]->ClockCountUpdated();
}

void PVMFMediaClock::ClockAdjusted()
{
    //notify all observers that the clock was adjusted
    for (uint32 i = 0;i < iClockObservers.size();i++)
        iClockObservers[i]->ClockAdjusted();
}

void PVMFMediaClock::ClockTimebaseUpdated()
{
    //notify all observers that the clock timebase was updated.
    for (uint32 i = 0;i < iClockObservers.size();i++)
    {
        PVMFMediaClockObserver* obs = iClockObservers[i];
        obs->ClockTimebaseUpdated();
    }
    //reset timebase history.
    iLastAdjustObsTimebaseTime = 0;
    iAdjustmentTimebaseTime = 0;
    iStartTimebaseTickValue = 0;
}

PVMFStatus PVMFMediaClock::SetCallbackCommon(uint32 aAbsoluteTime,
        uint32 aWindow,
        PVMFMediaClockNotificationsObs* aCallback,
        bool aThreadLock,
        const OsclAny* aContextData,
        uint32& aCallBackID,
        const OsclAny* aInterfaceObject,
        uint32 aCurrentTime, bool aIsNPT)
{
    uint32 delta = 0;

    if (NULL == aCallback)
    {
        return PVMFErrArgument;
    }

    //absolute time should be later than current time.
    //upper limit of 30 min for timer

    if (!aIsNPT || (aIsNPT && !iIsNPTPlayBackDirectionBackwards))
    {
        if (PVTimeComparisonUtils::IsEarlier(aCurrentTime + MSECS_IN_30_MINS, aAbsoluteTime, delta) ||
                PVTimeComparisonUtils::IsEarlier(aAbsoluteTime, aCurrentTime, delta))
        {
            return PVMFErrArgument;
        }
    }
    else    /*If this is NPT and clock direction is backwards. So, conditions will be opposite.*/
    {
        if (PVTimeComparisonUtils::IsEarlier(aAbsoluteTime, aCurrentTime + MSECS_IN_30_MINS, delta) ||
                PVTimeComparisonUtils::IsEarlier(aCurrentTime, aAbsoluteTime, delta))
        {
            return PVMFErrArgument;
        }
    }


    if (aThreadLock)
        iMutex->Lock();

    aCallBackID = iTimerIDCount++;
    //insert the timer in the queue
    //
    PVMFMediaClockTimerQueueElement timerQueueElement;

    timerQueueElement.contextData = aContextData;
    timerQueueElement.pInterfaceObject = aInterfaceObject;
    timerQueueElement.timeOut = aAbsoluteTime;
    timerQueueElement.callBackID = aCallBackID;
    timerQueueElement.isNPTTimer = aIsNPT;
    timerQueueElement.window = aWindow;
    timerQueueElement.obs = aCallback;
    if (!aIsNPT)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "PVMFMediaClock::SetCallbackCommon Setting regular callback for time %d at time %d",
                         aAbsoluteTime, aCurrentTime));

        iTimersPriQueue.push(timerQueueElement);

        //Adjust scheduling if the element inserted is the topmost element
        if ((iTimersPriQueue.top()).callBackID == (iTimerIDCount - 1))
        {
            AdjustScheduling(false, aCurrentTime);
        }
    }
    else
    {
        if (!iIsNPTPlayBackDirectionBackwards)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFMediaClock::SetCallbackCommon Setting NPT callback for time %d at time %d",
                             aAbsoluteTime, aCurrentTime));

            iTimersPriQueueNPT.push(timerQueueElement);
            if ((iTimersPriQueueNPT.top()).callBackID == (iTimerIDCount - 1))
            {
                AdjustScheduling(true, aCurrentTime);
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFMediaClock::SetCallbackCommon Setting backwards NPT callback for time %d at time %d",
                             aAbsoluteTime, aCurrentTime));

            iTimersPriQueueNPTBackwards.push(timerQueueElement);
            if ((iTimersPriQueueNPT.top()).callBackID == (iTimerIDCount - 1))
            {
                AdjustScheduling(true, aCurrentTime);
            }
        }

    }
    iActiveTimersCount++;

    if (aThreadLock)
        iMutex->Unlock();

    return PVMFSuccess;
}

PVMFStatus PVMFMediaClock::SetCallbackAbsoluteTime(
    uint32 aAbsoluteTime,
    uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID,
    /*IN*/  const OsclAny* aInterfaceObject
)
{
    uint32 currentTime = 0;
    bool overFlowFlag = false;

    GetCurrentTime32(currentTime, overFlowFlag, PVMF_MEDIA_CLOCK_MSEC);

    return SetCallbackCommon(aAbsoluteTime, aWindow, aCallback,
                             aThreadLock, aContextData, aCallBackID, aInterfaceObject, currentTime, false);

}

void PVMFMediaClock::CalculateRunLTimerValue(bool aIsNPT, uint32 aCurrentTime, int32& aDelta)
{
    int32 nptDelta = 0;
    int32 delta = 0;

    if (iTimersPriQueueNPT.size() ||
            iTimersPriQueueNPTBackwards.size())
    {
        uint32 temp = 0;
        GetNPTClockPosition(temp);

        if (!iIsNPTPlayBackDirectionBackwards)
        {
            if (iTimersPriQueueNPT.size())
            {
                nptDelta = iTimersPriQueueNPT.top().timeOut - temp;
            }
        }
        else
        {
            if (iTimersPriQueueNPTBackwards.size())
            {
                nptDelta = temp - iTimersPriQueueNPT.top().timeOut;
            }
        }

        if (!iTimersPriQueue.size())
        {
            aDelta = nptDelta;
            return;
        }
    }

    if (iTimersPriQueue.size())
    {
        bool overFlowFlag = false;

        uint32 currentTime = 0;

        if (!aIsNPT)
        {
            currentTime = aCurrentTime;
        }
        else
        {
            GetCurrentTime32(currentTime, overFlowFlag, PVMF_MEDIA_CLOCK_MSEC);
        }


        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "PVMFMediaClock::CalculateRunLTimerValue currtime %d top.timeOut %d", currentTime, iTimersPriQueue.top().timeOut));

        delta = iTimersPriQueue.top().timeOut - currentTime;

        if (!iTimersPriQueueNPT.size()
                && !iTimersPriQueueNPTBackwards.size())
        {
            aDelta = delta;
            return;
        }
    }

    aDelta = delta < nptDelta ? delta : nptDelta;
}

void PVMFMediaClock::AdjustScheduling(bool aIsNPT, uint32 aCurrentTime)
{

    //If timebase is count-based, no need for scheduling.
    if (iIsTimebaseCountBased)
    {
        return;
    }
    //Make sure current thread context is same on which AddToScheduler() was called

    TOsclThreadId tempThreadID;
    OsclThread::GetId(tempThreadID);

    if (!OsclThread::CompareId(tempThreadID, iOrigThreadID))
    {
        OsclError::Leave(OsclErrThreadContextIncorrect);
    }

    uint32 currentTime = 0;
    bool overFlowFlag = false;

    // A fresh RunIfInactive() will be called
    Cancel();

    //get current time if argument aCurrentTime is 0
    if (!aIsNPT)
    {
        if (aCurrentTime)
        {
            currentTime = aCurrentTime;
        }
        else
        {
            GetCurrentTime32(currentTime, overFlowFlag, PVMF_MEDIA_CLOCK_MSEC);
        }
    }
    else
    {
        aCurrentTime ? currentTime = aCurrentTime : GetNPTClockPosition(currentTime);
    }

    int32 deltaTime = 1;

    //if queues are empty, no need to schedule
    if (iTimersPriQueue.size() != 0 ||
            iTimersPriQueueNPT.size() != 0 ||
            iTimersPriQueueNPTBackwards.size() != 0)
    {
        CalculateRunLTimerValue(aIsNPT, currentTime, deltaTime);
        if (deltaTime >= 0)
        {
            //Adjust for rate
            if ((iClockTimebase != NULL) && (iClockTimebase->GetRate() != 0)
                    && (REALTIME_PLAYBACK_RATE != iClockTimebase->GetRate()))
            {
                //Support rate upto 0.1 resolution
                uint32 convNumerator = 10;
                uint32 conversionNumber =
                    (iClockTimebase->GetRate() * convNumerator) / REALTIME_PLAYBACK_RATE;

                if (conversionNumber != 0)
                {
                    deltaTime = (deltaTime * convNumerator) / conversionNumber ;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFMediaClock::AdjustScheduling ERROR: Timebase rate corrupted"));
                }
            }

            RunIfNotReady((uint32)deltaTime*1000);

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFMediaClock::AdjustScheduling Timer set for %d msecs wall clock time", deltaTime));
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFMediaClock::AdjustScheduling Late callback detected", deltaTime));
            //this means callback is already late. fire asap
            RunIfNotReady(0);
        }
    }
}

PVMFStatus PVMFMediaClock::SetCallbackDeltaTime(
    /*IN*/  uint32 aDeltaTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID,
    /*IN*/  const OsclAny* aInterfaceObject)
{
    uint32 currentTime = 0;
    bool overFlowFlag = false;

    GetCurrentTime32(currentTime, overFlowFlag, PVMF_MEDIA_CLOCK_MSEC);

    return SetCallbackCommon(currentTime + aDeltaTime, aWindow, aCallback,
                             aThreadLock, aContextData, aCallBackID, aInterfaceObject, currentTime, false);
}

PVMFStatus PVMFMediaClock::CommonCancelCallback(uint32 aCallbackID, bool aThreadLock, bool aIsNPT)
{
    bool flag = 0;
    int elementRemoved = 0;
    PVMFStatus retVal;

    if (aThreadLock)
        iMutex->Lock();

    if (!aIsNPT)
    {
        if (!iTimersPriQueue.empty())
        {
            if ((iTimersPriQueue.top()).callBackID == (aCallbackID))
            {
                flag = 1;
            }

            //Remove element from queue by ID
            PVMFMediaClockTimerQueueElement timerQueueElement;
            timerQueueElement.callBackID = aCallbackID;

            elementRemoved = iTimersPriQueue.remove(timerQueueElement);

            if (elementRemoved && flag)
            {
                AdjustScheduling();
            }
        }
    }
    else
    {
        if (!iIsNPTPlayBackDirectionBackwards)
        {
            if (!iTimersPriQueueNPT.empty())
            {
                if ((iTimersPriQueueNPT.top()).callBackID == (aCallbackID))
                {
                    flag = 1;
                }

                //Remove element from queue by ID
                PVMFMediaClockTimerQueueElement timerQueueElement;
                timerQueueElement.callBackID = aCallbackID;

                elementRemoved = iTimersPriQueueNPT.remove(timerQueueElement);
            }
        }
        else
        {
            if (!iTimersPriQueueNPTBackwards.empty())
            {
                if ((iTimersPriQueueNPTBackwards.top()).callBackID == (aCallbackID))
                {
                    flag = 1;
                }

                //Remove element from queue by ID
                PVMFMediaClockTimerQueueElement timerQueueElement;
                timerQueueElement.callBackID = aCallbackID;

                elementRemoved = iTimersPriQueueNPTBackwards.remove(timerQueueElement);
            }
        }

        if (elementRemoved && flag)
        {
            /*When scheduling is for NPT, make sure to pass isNPT flag i.e. 'true' */
            AdjustScheduling(true);
        }

    }

    if (elementRemoved)
    {
        iActiveTimersCount--;
        retVal = PVMFSuccess;
    }
    else
    {
        retVal = PVMFErrBadHandle;
    }
    if (aThreadLock)
        iMutex->Unlock();

    return retVal;
}

PVMFStatus PVMFMediaClock::CancelCallback(
    /*IN*/  uint32 aCallbackID, bool aThreadLock)
{
    return CommonCancelCallback(aCallbackID, aThreadLock, false);
}

PVMFStatus PVMFMediaClock::SetNPTCallbackAbsoluteTime(
    /*IN*/  uint32 aAbsoluteTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID,
    /*IN*/  const OsclAny* aInterfaceObject)
{

    uint32 currentTime = 0;

    GetNPTClockPosition(currentTime);

    return SetCallbackCommon(aAbsoluteTime, aWindow, aCallback,
                             aThreadLock, aContextData, aCallBackID, aInterfaceObject, currentTime, true);

}

PVMFStatus PVMFMediaClock::SetNPTCallbackDeltaTime(
    /*IN*/  uint32 aDeltaTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID,
    /*IN*/  const OsclAny* aInterfaceObject)
{
    uint32 currentTime = 0;

    GetNPTClockPosition(currentTime);

    return SetCallbackCommon(currentTime + aDeltaTime, aWindow, aCallback,
                             aThreadLock, aContextData, aCallBackID, aInterfaceObject, currentTime, true);

}

PVMFStatus PVMFMediaClock::CancelNPTCallback(
    /*IN*/  uint32 aCallbackID, bool aThreadLock)
{
    return CommonCancelCallback(aCallbackID, aThreadLock, true);
}

void PVMFMediaClock::ClearAllQueues()
{
    PVMFMediaClockTimerQueueElement topTimerElement;

    while (iTimersPriQueue.size() != 0)
    {
        topTimerElement = iTimersPriQueue.top();
        iTimersPriQueue.pop();
        iActiveTimersCount--;
        topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID,
                                             PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW, 0,
                                             topTimerElement.contextData, PVMFErrCallbackClockStopped);
    }

    if (!iIsNPTPlayBackDirectionBackwards)
    {
        while (iTimersPriQueueNPT.size() != 0)
        {
            topTimerElement = iTimersPriQueueNPT.top();
            iTimersPriQueueNPT.pop();
            iActiveTimersCount--;
            topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID,
                                                 PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW, 0,
                                                 topTimerElement.contextData, PVMFErrCallbackClockStopped);
        }
    }
    else
    {
        while (iTimersPriQueueNPTBackwards.size() != 0)
        {
            topTimerElement = iTimersPriQueueNPTBackwards.top();
            iTimersPriQueueNPTBackwards.pop();
            iActiveTimersCount--;
            topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID,
                                                 PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW, 0,
                                                 topTimerElement.contextData, PVMFErrCallbackClockStopped);
        }
    }

}

void PVMFMediaClock::ClearPresentNPTQueue()
{
    PVMFMediaClockTimerQueueElement topTimerElement;

    if (!iIsNPTPlayBackDirectionBackwards)
    {
        while (iTimersPriQueueNPT.size() != 0)
        {
            topTimerElement = iTimersPriQueueNPT.top();
            iTimersPriQueueNPT.pop();
            iActiveTimersCount--;
            topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID,
                                                 PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW, 0,
                                                 topTimerElement.contextData, PVMFErrCallbackHasBecomeInvalid);
        }
    }
    else
    {
        while (iTimersPriQueueNPTBackwards.size() != 0)
        {
            topTimerElement = iTimersPriQueueNPTBackwards.top();
            iTimersPriQueueNPTBackwards.pop();
            iActiveTimersCount--;
            topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID,
                                                 PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW, 0,
                                                 topTimerElement.contextData, PVMFErrCallbackHasBecomeInvalid);
        }
    }
}

void PVMFMediaClock::UpdateNPTClockPosition(
    /*IN*/  uint32 aStartNPT,
    /*IN*/  bool aIsPlayBackDirectionBackwards)
{
    bool overflow = false;

    iStartNPT = aStartNPT;

    GetCurrentTime32(iStartMediaClockTS, overflow, PVMF_MEDIA_CLOCK_MSEC);

    if (iIsNPTPlayBackDirectionBackwards != aIsPlayBackDirectionBackwards)
    {
        //NPT clock direction has changed. So invalidate all existing timers
        ClearPresentNPTQueue();
    }

    iIsNPTPlayBackDirectionBackwards = aIsPlayBackDirectionBackwards;

    /*reschedule. Send argument as true so that this function knows that
     scheduling is being done for NPT.*/
    AdjustScheduling(true);
}

PVMFStatus PVMFMediaClock::GetNPTClockPosition(
    /*OUT*/ uint32& aCurrentPosition)
{
    uint32 currentTime = 0;
    bool overflow = false;

    GetCurrentTime32(currentTime, overflow, PVMF_MEDIA_CLOCK_MSEC);

    if (overflow)
    {
        return PVMFErrOverflow;
    }

    if (iIsNPTPlayBackDirectionBackwards)
    {
        aCurrentPosition = iStartNPT - (currentTime - iStartMediaClockTS);
    }
    else
    {
        aCurrentPosition = iStartNPT + (currentTime - iStartMediaClockTS);
    }
    return PVMFSuccess;
}

void PVMFMediaClock::ClearNPTClockPosition()
{
    iStartNPT = 0;
    iStartMediaClockTS = 0;
    iIsNPTPlayBackDirectionBackwards = 0;
}

void PVMFMediaClock::QueueClockStartNotificationEvent(uint32 aDelta, PVMFMediaClockStateObserver *aClockStateObserver)
{
    PVMFMediaClockStartNotificationEventElement element;
    uint32 queuedEventID;
    PVMFStatus status = PVMFFailure;

    element.clockStateObserver = aClockStateObserver;

    //Set a callback on the clock to set this event
    status = SetCallbackDeltaTime(aDelta, 0, (PVMFMediaClockNotificationsObs*)this, false,
                                  &iClockStartNotificationEventQueue, queuedEventID, this);

    element.eventID = queuedEventID;
    if (PVMFSuccess == status)
    {
        iClockStartNotificationEventQueue.push_back(element);
    }

}

PVMFStatus PVMFMediaClock::QueueNPTClockTransitionEvent(uint32 aMediaClockPosition, uint32 aStartNPT,
        bool aIsPlayBackDirectionBackwards, uint32 aWindow, uint32& aClockTransitionID)
{
    PVMFMediaClockNPTTransitionEventElement element;
    PVMFStatus status = PVMFFailure;

    element.isPlayBackDirectionBackwards = aIsPlayBackDirectionBackwards;
    element.mediaClockPosition = aMediaClockPosition;
    element.startNPT = aStartNPT;
    element.window = aWindow;

    //Set a callback on the clock to set this event
    status = SetCallbackAbsoluteTime(element.mediaClockPosition, element.window, (PVMFMediaClockNotificationsObs*)this, false,
                                     &iNPTTransitionEventQueue, aClockTransitionID, this);

    element.eventID = aClockTransitionID;
    if (PVMFSuccess == status)
    {
        iNPTTransitionEventQueue.push_back(element);
    }

    return status;
}

PVMFStatus PVMFMediaClock::CancelNPTClockTransitionEvent(uint32 aClockTransitionEventID)
{
    PVMFStatus status;

    status = CancelCallback(aClockTransitionEventID, false);

    if (PVMFSuccess != status)
    {
        return status;
    }

    for (uint32 ii = 0;ii < iNPTTransitionEventQueue.size();ii++)
    {
        if (iNPTTransitionEventQueue[ii].eventID == aClockTransitionEventID)
            iNPTTransitionEventQueue.erase(&iNPTTransitionEventQueue[ii]);
    }
    return PVMFSuccess;
}

void PVMFMediaClock::ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 delta,
                                     const OsclAny* aContextData, PVMFStatus aStatus)
{

    OSCL_UNUSED_ARG(aTimerAccuracy);
    OSCL_UNUSED_ARG(delta);
    if (aStatus != PVMFSuccess)
    {
        /*This means the clock was stopped*/
        return;
    }

    //if event is NPT transition
    if (aContextData == (void*)&iNPTTransitionEventQueue)
    {
        for (uint32 ii = 0;ii < iNPTTransitionEventQueue.size();ii++)
        {
            if (iNPTTransitionEventQueue[ii].eventID == aCallBackID)
            {
                UpdateNPTClockPosition(iNPTTransitionEventQueue[ii].startNPT,
                                       iNPTTransitionEventQueue[ii].isPlayBackDirectionBackwards);

                iNPTTransitionEventQueue.erase(&iNPTTransitionEventQueue[ii]);
            }
        }
    }
    //if event is clock-start notification
    else if (aContextData == (void*)&iClockStartNotificationEventQueue)
    {
        for (uint32 ii = 0;ii < iClockStartNotificationEventQueue.size();ii++)
        {
            if (iClockStartNotificationEventQueue[ii].eventID == aCallBackID)
            {
                (iClockStartNotificationEventQueue[ii].clockStateObserver)->ClockStateUpdated();
                iClockStartNotificationEventQueue.erase(&iClockStartNotificationEventQueue[ii]);
            }
        }
    }

}

void PVMFMediaClock::NotificationsInterfaceDestroyed()
{
    //do nothing
}

void PVMFMediaClock::Run()
{
    uint32 currentTime = 0;
    bool overFlowFlag = false;
    uint32 delta = 0;

    PVMFMediaClockTimerQueueElement topTimerElement;
    PVTimeComparisonUtils::MediaTimeStatus status;

    /*Caution: Both loops below should always be similar. Any update needed for regular callback handling
    loop should also be checked for NPT loop ..and vice versa*/

    if (iTimersPriQueue.size())
    {
        topTimerElement = iTimersPriQueue.top();

        GetCurrentTime32(currentTime, overFlowFlag, PVMF_MEDIA_CLOCK_MSEC);

        status = PVTimeComparisonUtils::CheckTimeWindow(topTimerElement.timeOut, currentTime, topTimerElement.window,
                 topTimerElement.window, delta);

        while (iTimersPriQueue.size() &&
                status != PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "PVMFMediaClock::Run Timer for regular callback currentTime - %d callbackTime - %d callbackMargin - %d queue size - %d status - %d", currentTime,
                             topTimerElement.timeOut, topTimerElement.window, iTimersPriQueue.size(), status));

            switch (status)
            {
                case PVTimeComparisonUtils::MEDIA_EARLY_WITHIN_WINDOW:
                case PVTimeComparisonUtils::MEDIA_ONTIME_WITHIN_WINDOW:
                case PVTimeComparisonUtils::MEDIA_LATE_WITHIN_WINDOW:
                case PVTimeComparisonUtils::MEDIA_LATE_OUTSIDE_WINDOW:
                {
                    iTimersPriQueue.pop();
                    iActiveTimersCount--;
                    topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID, status, delta,
                                                         topTimerElement.contextData, PVMFSuccess);
                }
                break;

                case PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW:
                    //If callback is early, just schedule fresh
                    break;

                default:
                {
                    //@@error
                }
            }

            //check if more timers fall within the window.
            topTimerElement = iTimersPriQueue.top();

            GetCurrentTime32(currentTime, overFlowFlag, PVMF_MEDIA_CLOCK_MSEC);

            status = PVTimeComparisonUtils::CheckTimeWindow(topTimerElement.timeOut, currentTime, topTimerElement.window,
                     topTimerElement.window, delta);
        }

        AdjustScheduling(false, currentTime);
    }

    // Check NPT timers now

    //normal NPT clock
    if (!iIsNPTPlayBackDirectionBackwards)
    {
        if (iTimersPriQueueNPT.size())
        {
            topTimerElement = iTimersPriQueueNPT.top();

            GetNPTClockPosition(currentTime);

            status = PVTimeComparisonUtils::CheckTimeWindow(topTimerElement.timeOut, currentTime, topTimerElement.window,
                     topTimerElement.window, delta);

            while (iTimersPriQueueNPT.size() &&
                    status != PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "PVMFMediaClock::Run Timer for NPT callback currentTime - %d callbackTime - %d callbackMargin - %d queue size - %d status - %d", currentTime,
                                 topTimerElement.timeOut, topTimerElement.window, iTimersPriQueueNPT.size(), status));

                switch (status)
                {
                    case PVTimeComparisonUtils::MEDIA_EARLY_WITHIN_WINDOW:
                    case PVTimeComparisonUtils::MEDIA_ONTIME_WITHIN_WINDOW:
                    case PVTimeComparisonUtils::MEDIA_LATE_WITHIN_WINDOW:
                    case PVTimeComparisonUtils::MEDIA_LATE_OUTSIDE_WINDOW:
                    {
                        iTimersPriQueueNPT.pop();
                        iActiveTimersCount--;
                        topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID, status, delta,
                                                             topTimerElement.contextData, PVMFSuccess);
                    }
                    break;

                    case PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW:
                        //If callback is early, just schedule fresh
                        break;

                    default:
                    {
                        //@@error
                    }
                }

                //check if more timers fall within the window.
                topTimerElement = iTimersPriQueueNPT.top();

                GetNPTClockPosition(currentTime);

                status = PVTimeComparisonUtils::CheckTimeWindow(topTimerElement.timeOut, currentTime, topTimerElement.window,
                         topTimerElement.window, delta);
            }

            AdjustScheduling(true, currentTime);
        }
    }
    else  /*When direction of NPT clock is backwards. Just use the other queue and reverse calculations*/
    {
        if (iTimersPriQueueNPTBackwards.size())
        {
            topTimerElement = iTimersPriQueueNPTBackwards.top();

            GetNPTClockPosition(currentTime);

            status = PVTimeComparisonUtils::CheckTimeWindow(topTimerElement.timeOut, currentTime, topTimerElement.window,
                     topTimerElement.window, delta);

            while (iTimersPriQueueNPTBackwards.size() &&
                    status != PVTimeComparisonUtils::MEDIA_LATE_OUTSIDE_WINDOW)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "PVMFMediaClock::Run Timer for Backwards NPT callback currentTime - %d callbackTime - %d callbackMargin - %d queue size - %d status - %d", currentTime,
                                 topTimerElement.timeOut, topTimerElement.window, iTimersPriQueueNPTBackwards.size(), status));

                switch (status)
                {
                    case PVTimeComparisonUtils::MEDIA_EARLY_WITHIN_WINDOW:
                    {
                        iTimersPriQueueNPT.pop();
                        iActiveTimersCount--;
                        topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID, PVTimeComparisonUtils::MEDIA_LATE_WITHIN_WINDOW, delta,
                                                             topTimerElement.contextData, PVMFSuccess);
                    }
                    break;

                    case PVTimeComparisonUtils::MEDIA_ONTIME_WITHIN_WINDOW:
                    case PVTimeComparisonUtils::MEDIA_LATE_WITHIN_WINDOW:
                    {
                        iTimersPriQueueNPT.pop();
                        iActiveTimersCount--;
                        topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID, PVTimeComparisonUtils::MEDIA_EARLY_WITHIN_WINDOW, delta,
                                                             topTimerElement.contextData, PVMFSuccess);
                    }
                    break;
                    case PVTimeComparisonUtils::MEDIA_EARLY_OUTSIDE_WINDOW:
                    {
                        iTimersPriQueueNPT.pop();
                        iActiveTimersCount--;
                        topTimerElement.obs->ProcessCallBack(topTimerElement.callBackID, PVTimeComparisonUtils::MEDIA_LATE_OUTSIDE_WINDOW, delta,
                                                             topTimerElement.contextData, PVMFSuccess);
                    }
                    break;

                    default:
                    {
                        //@@error
                    }
                }

                //check if more timers fall within the window.
                topTimerElement = iTimersPriQueueNPTBackwards.top();

                GetNPTClockPosition(currentTime);

                status = PVTimeComparisonUtils::CheckTimeWindow(topTimerElement.timeOut, currentTime, topTimerElement.window,
                         topTimerElement.window, delta);
            }

            AdjustScheduling(true, currentTime);
        }
    }
}

OSCL_EXPORT_REF PVMFMediaClockNotificationsInterfaceImpl::PVMFMediaClockNotificationsInterfaceImpl(PVMFMediaClock *aClock,
        uint32 aLatency,
        PVMFMediaClockNotificationsObsBase& aNotificationInterfaceDestroyedCallback)
{
    iContainer = aClock;
    iLatency = aLatency;
    iNotificationInterfaceDestroyedCallback = &aNotificationInterfaceDestroyedCallback;
    iAdjustedLatency = 0;
    iClockStateObserver = NULL;
    iLatencyDelayForClockStartNotification = 0;
}

OSCL_EXPORT_REF PVMFMediaClockNotificationsInterfaceImpl::~PVMFMediaClockNotificationsInterfaceImpl()
{
    //check if vectors need to be destroyed
}

OSCL_EXPORT_REF PVMFStatus PVMFMediaClockNotificationsInterfaceImpl::SetCallbackAbsoluteTime(
    /*IN*/  uint32 aAbsoluteTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID)
{
    if (iContainer)
    {
        return iContainer->SetCallbackAbsoluteTime(aAbsoluteTime - iAdjustedLatency, aWindow, aCallback, aThreadLock, aContextData,
                aCallBackID, this);
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFMediaClockNotificationsInterfaceImpl::SetCallbackDeltaTime(
    /*IN*/  uint32 aDeltaTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID)
{
    if (iContainer)
    {
        return iContainer->SetCallbackDeltaTime(aDeltaTime - iAdjustedLatency, aWindow, aCallback, aThreadLock, aContextData,
                                                aCallBackID, this);
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFMediaClockNotificationsInterfaceImpl::CancelCallback(
    /*IN*/  uint32 aCallbackID, bool aThreadLock)
{
    if (iContainer)
    {
        return iContainer->CancelCallback(aCallbackID, aThreadLock);
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFMediaClockNotificationsInterfaceImpl::SetNPTCallbackAbsoluteTime(
    /*IN*/  uint32 aAbsoluteTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID)
{
    if (iContainer)
    {
        return iContainer->SetNPTCallbackAbsoluteTime(aAbsoluteTime - iAdjustedLatency, aWindow, aCallback, aThreadLock, aContextData,
                aCallBackID, this);
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFMediaClockNotificationsInterfaceImpl::SetNPTCallbackDeltaTime(
    /*IN*/  uint32 aDeltaTime,
    /*IN*/  uint32 aWindow,
    /*IN*/  PVMFMediaClockNotificationsObs* aCallback,
    /*IN*/  bool aThreadLock,
    /*IN*/  const OsclAny* aContextData,
    /*OUT*/ uint32& aCallBackID)
{
    if (iContainer)
    {
        return iContainer->SetNPTCallbackDeltaTime(aDeltaTime - iAdjustedLatency, aWindow, aCallback, aThreadLock, aContextData,
                aCallBackID, this);
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFMediaClockNotificationsInterfaceImpl::CancelNPTCallback(
    /*IN*/  uint32 aCallbackID, bool aThreadLock)
{
    if (iContainer)
    {
        return iContainer->CancelNPTCallback(aCallbackID, aThreadLock);
    }
    else
    {
        return PVMFFailure;
    }
}

OSCL_EXPORT_REF void PVMFMediaClockNotificationsInterfaceImpl::SetClockObserver(PVMFMediaClockObserver& aObserver)
{
    if (iContainer)
    {
        iContainer->SetClockObserver(aObserver);
    }
}

OSCL_EXPORT_REF void PVMFMediaClockNotificationsInterfaceImpl::RemoveClockObserver(PVMFMediaClockObserver& aObserver)
{
    if (iContainer)
    {
        iContainer->RemoveClockObserver(aObserver);
    }
}

OSCL_EXPORT_REF void PVMFMediaClockNotificationsInterfaceImpl::SetClockStateObserver(PVMFMediaClockStateObserver& aObserver)
{
    iClockStateObserver = &aObserver;
}

OSCL_EXPORT_REF void PVMFMediaClockNotificationsInterfaceImpl::RemoveClockStateObserver(PVMFMediaClockStateObserver& aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
    iClockStateObserver = NULL;
}

OSCL_EXPORT_REF void PVMFTimebase_Tickcount::GetCurrentTick32(uint32& aTimebaseTickCount, bool& aOverflow)
{
    uint32 currenttickcount = OsclTickCount::TickCount();

    aOverflow = false;

    // Check to see if the tickcount wrapped around
    if (iPrevTickcount > currenttickcount)
    {
        aOverflow = true;
    }

    aTimebaseTickCount = currenttickcount;

    // Save the current tickcount for next comparison
    iPrevTickcount = currenttickcount;
}

OSCL_EXPORT_REF void PVMFTimebase_Tickcount::GetCurrentTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits)
{
    uint32 currenttickcount = OsclTickCount::TickCount();

    aOverflow = false;

    // Check to see if the tickcount wrapped around
    if (iPrevTickcount > currenttickcount)
    {
        aOverflow = true;
    }

    if (PVMF_MEDIA_CLOCK_USEC == aUnits)
    {
        uint64 time64 = (uint64)(currenttickcount * iMicrosecPerTick);
        //There is a chance that Tickcount did not wrap around but aTime value does
        if (time64 > (uint64)(0xFFFFFFFF))
        {
            aOverflow = true;
        }
        aTime = Oscl_Int64_Utils::get_uint64_lower32(time64);
    }
    else                                  /*convert time to millsecs*/
    {
        aTime = OsclTickCount::TicksToMsec(currenttickcount);
        uint32 divConst = 1;

        switch (aUnits)
        {
            case PVMF_MEDIA_CLOCK_SEC:
                divConst = 1000;
                break;

            case PVMF_MEDIA_CLOCK_MIN:
                divConst = 60000;
                break;

            case PVMF_MEDIA_CLOCK_HOUR:
                divConst = 3600000;
                break;

            case PVMF_MEDIA_CLOCK_DAY:
                divConst = 86400000;
                break;

            case PVMF_MEDIA_CLOCK_MSEC:
            default:
                break;
        }

        aTime = aTime / divConst;
    }

    // Save the current tickcount for next comparison
    iPrevTickcount = currenttickcount;
}
