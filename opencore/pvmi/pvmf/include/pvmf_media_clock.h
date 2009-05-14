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
 *  @file pvmf_media_clock.h
 *  @brief Provides a time clock that can be paused and resumed,
 *	  set the start time, adjusted based on outside source feedback,
 *    and accepts user specified source for the free running clock.
 *
 */

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#define PVMF_MEDIA_CLOCK_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif

#ifndef OSCL_INT64_UTILS_H_INCLUDED
#include "oscl_int64_utils.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#ifndef TIME_COMPARISON_UTILS_H_INCLUDED
#include "time_comparison_utils.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#define MSECS_IN_30_MINS 0x1B7740


#define PVMFMediaClockControlInterfaceUuid PVUuid(0xca20319a,0x33f9,0x484f,0x8d,0x1c,0xa5,0x1e,0x23,0x4c,0xe6,0x03)
#define PVMFMediaClockAccessInterfaceUuid PVUuid(0xca20319a,0x33f9,0x484f,0x8d,0x1c,0xa5,0x1e,0x23,0x4c,0xe6,0x04)
#define PVMFMediaClockNPTClockPositionAccessInterfaceUuid PVUuid(0xca20319a,0x33f9,0x484f,0x8d,0x1c,0xa5,0x1e,0x23,0x4c,0xe6,0x05)

//playback rate unit is milli-percent of real time
#define REALTIME_PLAYBACK_RATE 100000

class PVMFMediaClock;


/*
 * Enum for the time units used in OSCL Media Clock
 */
enum PVMFMediaClock_TimeUnits
{
    PVMF_MEDIA_CLOCK_USEC   = 0,
    PVMF_MEDIA_CLOCK_MSEC   = 1,
    PVMF_MEDIA_CLOCK_SEC    = 2,
    PVMF_MEDIA_CLOCK_MIN    = 3,
    PVMF_MEDIA_CLOCK_HOUR   = 4,
    PVMF_MEDIA_CLOCK_DAY    = 5
};

/*
* Enum for return error codes for AdjustClock32() API
*/
enum PVMFMediaClockAdjustTimeStatus
{
    PVMF_MEDIA_CLOCK_ADJUST_SUCCESS,
    PVMF_MEDIA_CLOCK_ADJUST_ERR_INVALID_STATE,             // If adjustment is attempted when clock is not running.
    PVMF_MEDIA_CLOCK_ADJUST_ERR_INVALID_TIMEBASE_TIME,     // If Adjustment is older than latest adjustment
    PVMF_MEDIA_CLOCK_ADJUST_ERR_CORRUPT_CLOCK_TIME       // If Clock time arg passed is later than current time
};


class PVMFMediaClockNotificationsObsBase
{
    public:
        /**
         * This event happens when the clock has been Reset or destroyed and notification
         * interface object that observer is using has been destroyed. Observer should
         * set its pointer to PVMFMediaClockNotificationsInterface object as NULL.
         */
        virtual void NotificationsInterfaceDestroyed() = 0;
};

/**
    PVMFMediaClockNotificationsObs is an observer class for PVMFMediaClock for Callbacks.
    ProcessCallBack() is called when the timer expires.
*/
class PVMFMediaClockNotificationsObs : public virtual PVMFMediaClockNotificationsObsBase
{
    public:
        /**
         * This callback function is called when a callback expires or has become invalid.
         * @param callBackID Callback ID of the timer that has expired
         *				   				   Units is msec.
         * @param aTimerAccuracy Accuracy of timer. Value is from enum PVTimeComparisonUtils::MediaTimeStatus
         * @param aDelta delta of scheduled callback time and actual time of callback
         * @param aContextData Context data passed while setting the timer
         * @param aStatus Status of timer. Value can be PVMFSuccess, PVMFErrCallbackClockStopped
         *                or PVMFErrCallbackHasBecomeInvalid. aStatus is PVMFErrCallbackHasBecomeInvalid
         *                if the direction of NPT Clock has changed. aStatus can be PVMFErrCallbackHasBecomeInvalid
         *                only for a NPT callback.
         * @return	NONE
         *
         */
        virtual void ProcessCallBack(uint32 callBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 aDelta,
                                     const OsclAny* aContextData, PVMFStatus aStatus) = 0;

        /**
         * This callback function is called when the notification interface
         * being used by the object has been destroyed.
         */
        virtual void NotificationsInterfaceDestroyed() = 0;
};

/**
    PVMFMediaClockObserver is an observer class for PVMFMediaClock.  Modules
    can optionally register themselves as clock observers.  There
    can be multiple observers for a single clock.
*/
class PVMFMediaClockObserver : public virtual PVMFMediaClockNotificationsObsBase
{
    public:

        /**
         * This callback function is called when the timebase for this clock
         * has been updated
         */
        virtual void ClockTimebaseUpdated() = 0;

        /**
         * This callback function is called for counting timebases only, when the
         * count has been updated.
         */
        virtual void ClockCountUpdated() = 0;

        /**
         * This callback function is called when the clock has been adjusted.
         */
        virtual void ClockAdjusted() = 0;

        virtual ~PVMFMediaClockObserver() {}
};

/**
    PVMFMediaClockStateObserver is an observer class for PVMFMediaClock.  Modules
    can optionally register themselves as clock state observers.  There
    can be multiple observers for a single clock.
*/
class PVMFMediaClockStateObserver : public virtual PVMFMediaClockNotificationsObsBase
{
    public:
        /**
         * This callback function is called when the clock state changes.
         */
        virtual void ClockStateUpdated() = 0;

        virtual ~PVMFMediaClockStateObserver() {}
};

/**
    PVMFCountTimebase is an extension to the standard timebase to allow
    controlled stepping rather than continuous flow timebase.
*/
class PVMFCountTimebase
{
    public:
        /**
         * Set a new value for the count.  Will trigger a ClockCountUpdated callback
         * to all observers of the clock in which this timebase resides.
         * @param aCount (input): the new count
         */
        virtual void SetCount(int32 aCount) = 0;

        /**
         * Read current value of the count.
         * @param aCount (output): the current count
         */
        virtual void GetCount(int32& aCount) = 0;

        virtual ~PVMFCountTimebase() {}
    private:
        /**
         * Each PVMFCountTimebase will be contained within an PVMFMediaClock
         * class.  That PVMFMediaClock instance will set itself as the observer of
         * the PVMFCountTimebase.  To get notices from the timebase, modules
         * can register through the SetClockObserver method of the PVMFMediaClock.
         */
        friend class PVMFMediaClock;
        virtual void SetClockObserver(PVMFMediaClockObserver* aObserver) = 0;
};

/**
    PVMFTimebase is a base class to obtain the timebase clock time.
    Common source of the timebase clock is the system tickcount which is implemented
    as PVMFTimebase_Tickcount further below. PVMFTimebase is expected to return the time
    in units of microseconds even if the timebase itself does not have the resolution of microseconds.
*/
class PVMFTimebase
{
    public:

        virtual ~PVMFTimebase() {}

        /**
         * Gets the timebase clock's smallest time resolution in microseconds
         * @param aResolution: unsigned 32-bit value for the clock resolution
         */
        virtual void GetTimebaseResolution(uint32& aResolution) = 0;

        /**
         * Returns the timebase clock's smallest time resolution in microseconds
         * @param aResolution: unsigned 32-bit value for the clock resolution
         */

        /**
         * Read current value of the count.
         * @param aResolution: unsigned 32-bit value. Playback rate unit is milli-percent of real time
         */
        virtual int32 GetRate(void) = 0;

        /**
         *   Returns current tickcount. PVMFMediaClock sources this value from the timebase. If no
         *   timebase is set, aTimebaseTickCount is set to 0.
         *   @param aTime: a reference to an unsigned 32-bit integer to return the current time
         *   @param aUnits: the requested time units for aTime
         *   @param aTimebaseTime: a reference to an unsigned 32-bit integer to return the timebase time
         */
        virtual void GetCurrentTick32(uint32& aTimebaseTickCount, bool& aOverflow) = 0;

        /**
        *    This API returns pointer to clock's timebase if timebase being used is a PVMFCountTimebase
        *    object. Otherwise, NULL is returned. PVMFCountTimebase is used for a stepping clock.
        *    @return pointer to PVMFCountTimebase implementation, or NULL if not supported.
        */
        virtual PVMFCountTimebase* GetCountTimebase() = 0;

};

/*
PVMFMediaClockControlInterface interface class is implemented by PVMFMediaClock.
This class contains methods for controlling PVMFMediaClock
*/
class PVMFMediaClockControlInterface: public PVInterface
{
    public:
        /**
        *    Starts the clock from the start time or
        *    resumes the clock from the last paused time. Clock goes to RUNNING state.
        *    @return true if the clock is resumed or started, false otherwise
        */

        virtual OSCL_IMPORT_REF bool Start() = 0;

        /**
        *    Pauses the running clock. Saves the clock time when pausing as the paused time.
        *    Clock goes to PAUSED state. Returns true if the clock is paused, false otherwise.
        *    Will trigger a ClockStateUpdated notice to all state observers of this clock.
        *    @return true if the clock is paused, false otherwise
        */
        virtual OSCL_IMPORT_REF bool Pause() = 0;

        /**
        *    Stops the running or paused clock and start time is reset to 0. Clock goes to STOPPED state.
        *    Will trigger a ClockStateUpdated notice to all state observers of this clock. State
        *    observers and clock observers are not automatically removed when clock is stopped. However,
        *    any active callbacks will be fired with error code PVMFErrCallbackClockStopped.
        *
        *    @return true if the clock is stopped, false otherwise
        */
        virtual OSCL_IMPORT_REF bool Stop() = 0;

        /**
        *    Sets the starting clock time with unsigned 32-bit integer in the specified time units
        *    while in STOPPED state. Clock's internal timekeeping units are changed to usecs if aUnits
        *    is usecs. Otherwise, internal units remain msecs(default). High probability of overflow if
        *    units are given as usecs.
        *    @param aTime: a reference to an unsigned 32-bit integer to set the start time
        *    @param aUnits: the time units of aTime
        *    @param aOverFlow: true if operation resulted in overflow, false otherwise
        *    @return true if start time was set, false otherwise
        */
        virtual OSCL_IMPORT_REF bool SetStartTime32(uint32& aTime, PVMFMediaClock_TimeUnits aUnits, bool& aOverFlow) = 0;

        /**
        *   Adjusts the clock time with unsigned 32-bit integer in the specified time units while in
        *   RUNNING state. For backward adjustments, clock freezes internally till the adjusted time.
        *   @param aClockTime: a reference to an unsigned 32-bit integer to the observation clock time
        *   @param aTimebaseTime: a reference to an unsigned 32-bit integer to the observation timebase time
        *   @param aAdjustedTime: a reference to an unsigned 32-bit integer to the adjusted clock time
        *   @param aUnits: the time units of aClockTime and aAdjustedTime
        *   @return true if AdjustClockTime32 is successful
        */
        virtual OSCL_IMPORT_REF PVMFMediaClockAdjustTimeStatus AdjustClockTime32(uint32& aClockTime, uint32& aTimebaseTime, uint32& aAdjustedTime, PVMFMediaClock_TimeUnits aUnits, bool& aOverFlow) = 0;

        /**
        *   Stops the running or paused clock and start time is reset to 0. Clock goes to STOPPED state.
        *   Will trigger a ClockStateUpdated notice to all state observers of this clock.Clock observers
        *   and clock state observers are automatically removed when clock is Reset. Any active callbacks
        *   will be fired with error code PVMFErrCallbackClockStopped. All PVMFMediaClockNotificationsInterface
        *   objects are deleted.
        *   @return true if reset is successful.
        */
        virtual OSCL_IMPORT_REF bool Reset() = 0;

        void addRef() {}
        void removeRef() {}
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            OSCL_UNUSED_ARG(uuid);
            OSCL_UNUSED_ARG(iface);
            return false;
        }
};

/*
PVMFMediaClockAccessInterface interface class is implemented by PVMFMediaClock.
This class contains method for accessing start time from PVMFMediaClock.
*/
class PVMFMediaClockAccessInterface: public PVInterface
{
    public:

        /**
        *   Gets the starting clock time as an unsigned 32-bit integer in the specified time units
        *   @param aTime: a reference to an unsigned 32-bit integer to copy the start time
        *   @param aOverflow: a reference to a flag which is set if time value cannot fit in unsigned 32-bit integer
        *   @param aUnits: the requested time units for aTime
        */
        virtual OSCL_IMPORT_REF void GetStartTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits) = 0;

        /**
        *   Gets the starting clock time as an unsigned 32-bit integer in the specified time units
        *   @param aClockTime: a reference to an unsigned 32-bit integer to return current time in specified time units
        *   @param aOverflow: a reference to a flag which is set if time value cannot fit in unsigned 32-bit integer
        *   @param aUnits: the requested time units for aTime
        */
        virtual OSCL_IMPORT_REF void GetCurrentTime32(uint32& aClockTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits) = 0;

        /**
        *   This API is used to get current clock time in time units specified. If the value does not fit
        *   in 32 bit uint (aTime), aOverflow flag is set. This API also provides current timebase tickcount.
        *   This is not the absolute tickcount value from timebase. PVMFMediaClock uses scaled version of
        *   timebase tickcount by subtracting the tickcount value at clock start time from the current tickcount value.
        *   Thus, the clock starts from tickcount value 0 and probability of overflow is reduced.
        *   @param aClockTime: a reference to an unsigned 32-bit integer to return current time in specified time units
        *   @param aOverflow: a reference to a flag which is set if time value cannot fit in unsigned 32-bit integer
        *   @param aUnits: the requested time units for aTime
        *   @param aTimebaseTime: uint32 value used to return current (scaled) timebase tickcount
        */
        virtual OSCL_IMPORT_REF void GetCurrentTime32(uint32& aClockTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits, uint32& aTimebaseTime) = 0;

        void addRef() {}
        void removeRef() {}
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            OSCL_UNUSED_ARG(uuid);
            OSCL_UNUSED_ARG(iface);
            return false;
        }
};

/*
PVMFMediaClockNotificationsInterface interface is implemented by
PVMFMediaClockNotificationsInterfaceImpl class. This interface contains all callbacks related methods.
This class has methods related to both regular callbacks and NPT callbacks.
*/

class PVMFMediaClockNotificationsInterface
{
    public:

        /*!*********************************************************************
         **
         ** Function:    SetCallbackAbsoluteTime
         **
         ** Synopsis:	Set a callback timer specifying an absolute time in clock for timer expiry.
         **
         ** Arguments :
         ** @param		[absoluteTime]	-- absolute time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         **
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus SetCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID) = 0;

        /*!*********************************************************************
         **
         ** Function:    SetCallbackDeltaTime
         **
         ** Synopsis:	Set a callback timer specifying a delta time from current time for timer expiry.
         **
         ** Arguments :
         ** @param		[deltaTime]		-- delta time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         **
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus SetCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32 &aCallBackID) = 0;

        /*!*********************************************************************
         **
         ** Function:    CancelCallback
         **
         ** Synopsis:	Cancel callback timer set with SetCallBackDeltaTime() or SetCallbackAbsoluteTime()
         **
         ** Arguments :
         ** @param		[callbackID]	-- timer ID returned by SetCallBackDeltaTime()
         **									or SetCallbackAbsoluteTime()
         ** @param  :   [aThreadLock]   -- whether this call needs to be threadsafe
         ** Returns:
         ** @return		PVMFStatus
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus CancelCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock) = 0;

        /*!*********************************************************************
         **
         ** Function:    SetNPTCallbackAbsoluteTime
         **
         ** Synopsis:	Set a callback timer specifying an absolute time in clock for timer expiry.
         **
         ** Arguments :
         ** @param		[absoluteTime]	-- absolute time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         **
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/

        virtual OSCL_IMPORT_REF PVMFStatus SetNPTCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID) = 0;

        /*!*********************************************************************
         **
         ** Function:    SetNPTCallbackDeltaTime
         **
         ** Synopsis:	Set a callback timer specifying a delta time from current time for timer expiry.
         **
         ** Arguments :
         ** @param		[deltaTime]		-- delta time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         **
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus SetNPTCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID) = 0;

        /*!*********************************************************************
         **
         ** Function:    CancelCallback
         **
         ** Synopsis:	Cancel callback timer set with SetCallBackDeltaTime() or SetCallbackAbsoluteTime()
         **
         ** Arguments :
         ** @param		[callbackID]	-- timer ID returned by SetCallBackDeltaTime()
         **									or SetCallbackAbsoluteTime()
         ** @param  :   [aThreadLock]   -- whether this call needs to be threadsafe
         ** Returns:
         ** @return		PVMFStatus
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus CancelNPTCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock) = 0;

        /**
        *   This API sets the object passed as a ClockObserver. The object's callback
        *   functions (ClockTimebaseUpdated(),ClockCountUpdated(),ClockAdjusted() )
        *   will be called on corresponding events.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void SetClockObserver(PVMFMediaClockObserver& aObserver) = 0;

        /**
        *   This API removes aObserver obeject from the list of ClockObservers if aObserver
        *   is set as an observer.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void RemoveClockObserver(PVMFMediaClockObserver& aObserver) = 0;

        /**
        *   This API sets the object passed as a ClockStateObserver. The object's callback
        *   function ClockStateUpdated() will be called on clock state change.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void SetClockStateObserver(PVMFMediaClockStateObserver& aObserver) = 0;

        /**
        *   Removes an observer for this clock.  If the observer is not registered, this
        *   call does nothing.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void RemoveClockStateObserver(PVMFMediaClockStateObserver& aObserver) = 0;

        virtual ~PVMFMediaClockNotificationsInterface() {}
};

/*
PVMFMediaClockNotificationsImplInterface class is implemented by PVMFMediaClock. This class contains
corresponding PVMFMediaClock side functions of interface PVMFMediaClockNotifications.
*/

class PVMFMediaClockNotificationsImplInterface
{
    public:

        /*!*********************************************************************
         **
         ** Function:    SetCallbackAbsoluteTime
         **
         ** Synopsis:	Set a callback timer specifying an absolute time in clock for timer expiry.
         **
         ** Arguments :
         ** @param		[absoluteTime]	-- absolute time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         ** @param		[aInterfaceObject]	-- self pointer of interface object which calls this function
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus SetCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject) = 0;

        /*!*********************************************************************
         **
         ** Function:    SetCallbackDeltaTime
         **
         ** Synopsis:	Set a callback timer specifying a delta time from current time for timer expiry.
         **
         ** Arguments :
         ** @param		[deltaTime]		-- delta time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         ** @param		[aInterfaceObject]	-- self pointer of interface object which calls this function
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus SetCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32 &aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject) = 0;

        /*!*********************************************************************
         **
         ** Function:    CancelCallback
         **
         ** Synopsis:	Cancel callback timer set with SetCallBackDeltaTime() or SetCallbackAbsoluteTime()
         **
         ** Arguments :
         ** @param		[callbackID]	-- timer ID returned by SetCallBackDeltaTime()
         **									or SetCallbackAbsoluteTime()
         ** @param  :   [aThreadLock]   -- whether this call needs to be threadsafe
         ** Returns:
         ** @return		PVMFStatus
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus CancelCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock) = 0;

        /*!*********************************************************************
         **
         ** Function:    SetNPTCallbackAbsoluteTime
         **
         ** Synopsis:	Set a callback timer specifying an absolute time in clock for timer expiry.
         **
         ** Arguments :
         ** @param		[absoluteTime]	-- absolute time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         ** @param		[aInterfaceObject]	-- self pointer of interface object which calls this function
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/

        virtual OSCL_IMPORT_REF PVMFStatus SetNPTCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject) = 0;

        /*!*********************************************************************
         **
         ** Function:    SetNPTCallbackDeltaTime
         **
         ** Synopsis:	Set a callback timer specifying a delta time from current time for timer expiry.
         **
         ** Arguments :
         ** @param		[deltaTime]		-- delta time in clock when callBack should be called.
         **				   				   Units is msec.
         ** @param		[window]		-- Error tolerance available in callback time. If T is the desired
         **								   callback time and w is the allowed tolerance window, then callback
         **								   can come between T-w to T+w time.
         ** @param		[aObserver]     -- observer object to be called on timeout.
         ** @param		[threadLock]	-- If threadLock is true, callback will to be threadsafe otherwise
         **								   not. Making callback threadsafe might add overheads.
         ** @param      [aContextData]  -- context pointer that will be returned back with the callback.
         ** @param		[callBackID]	-- ID used to identify the timer for cancellation
         ** @param		[aInterfaceObject]	-- self pointer of interface object which calls this function
         ** Returns:
         ** @return		PVMFStatus		-- success or error code
         **
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus SetNPTCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject) = 0;

        /*!*********************************************************************
         **
         ** Function:    CancelCallback
         **
         ** Synopsis:	Cancel callback timer set with SetCallBackDeltaTime() or SetCallbackAbsoluteTime()
         **
         ** Arguments :
         ** @param		[callbackID]	-- timer ID returned by SetCallBackDeltaTime()
         **									or SetCallbackAbsoluteTime()
         ** @param  :   [aThreadLock]   -- whether this call needs to be threadsafe
         ** Returns:
         ** @return		PVMFStatus
         **
         ** Notes:
         **
         **********************************************************************/
        virtual OSCL_IMPORT_REF PVMFStatus CancelNPTCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock) = 0;

        /**
        *   Adds an observer for this clock.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void SetClockObserver(PVMFMediaClockObserver& aObserver) = 0;

        /**
        *   Removes an observer for this clock.  If the observer is not registered, this
        *   call does nothing.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void RemoveClockObserver(PVMFMediaClockObserver& aObserver) = 0;

        /**
        *   Sets an observer for this clock.  May leave if memory allocation fails.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void SetClockStateObserver(PVMFMediaClockStateObserver& aObserver) = 0;

        /**
        *   Removes an observer for this clock.  If the observer is not registered, this
        *   call does nothing.
        *   @param aObserver: the observer implemenation
        */
        virtual OSCL_IMPORT_REF void RemoveClockStateObserver(PVMFMediaClockStateObserver& aObserver) = 0;

        virtual ~PVMFMediaClockNotificationsImplInterface() {}
};


/*
PVMFMediaClockNPTClockPositionAccessInterface interface class is implemented by PVMFMediaClock.
This class contains methods for accessing and updating NPT clock position values
*/

class PVMFMediaClockNPTClockPositionAccessInterface: public PVInterface
{

    public:

        /**
        *   NPT clock position is not same as playback clock position. PVMFMediaClock
        *   can store the mapping between the two clocks. UpdateNPTClockPosition() API is
        *   called by the user to update change in NPT clock position. User can change the
        *   direction of NPT by specifying aIsPlayBackDirectionBackwards flag.
        *   @param aStartNPT: uint32 value containing new startNPT value
        *   @param aIsPlayBackDirectionBackwards: Should be set as false if
        *          NPT playback direction is forward from this point and should
        *          be set as true if NPT clock direction is backwards.
        */
        virtual void UpdateNPTClockPosition(
            /*IN*/  uint32 aStartNPT,
            /*IN*/  bool aIsPlayBackDirectionBackwards) = 0;

        /**
        *   Gets NPT clock position
        *   @param aCurrentPosition: uint32 value to store the current NPT clock position.
        *   @return PVMFSuccess/PVMFErrArgument. PVMFErrArgument is returned when there is
        *           overflow i.e. 32 bit aCurrentPosition is not enough to store current NPT
        *           clock value or if there is an internal overflow.
        */
        virtual PVMFStatus GetNPTClockPosition(
            /*OUT*/ uint32& aCurrentPosition) = 0;

        /**
        * 	This API clears the NPT clock mapping and resets internal variables to 0.
        */
        virtual void ClearNPTClockPosition() = 0;

        /**
        *   This API is used to schedule a NPT clock transition(time change) in future. All
        *   parameters are described above.
        *   @param aMediaClockPosition: Absolute media clock position when NPT clock change
        *                               should take place.
        *   @param aStartNPT: new startNPT value to be set
        *   @param aIsPlayBackDirectionBackwards: flag specifying if NPT clock should run
        *                                         backwards after NPT time update.
        *   @param aWindow: margin window for scheduling the time change. i.e. NPT clock
        *                   time change can be scheduled within window [aMediaClockPosition-aWindow,
        *                   aMediaClockPosition+aWindow].
        *   @param aClockTransitionEventID: clockTransitionID returned to user. This can be used
        *                                   to cancel a transition after it has been set.
        *   @return PVMFSuccess/PVMFFailure
        */
        virtual PVMFStatus QueueNPTClockTransitionEvent(uint32 aMediaClockPosition, uint32 aStartNPT,
                bool aIsPlayBackDirectionBackwards, uint32 aWindow, uint32& aClockTransitionEventID) = 0;

        /**
        *   This API is used to cancel a NPTClockTransition event set by using QueueNPTClockTransitionEvent()
        *   @param aClockTransitionEventID: ID returned from QueueNPTClockTransitionEvent()function.
        *   @return PVMFSuccess/PVMFFailure
        */
        virtual PVMFStatus CancelNPTClockTransitionEvent(uint32 aClockTransitionEventID) = 0;

        void addRef() {}
        void removeRef() {}
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            OSCL_UNUSED_ARG(uuid);
            OSCL_UNUSED_ARG(iface);
            return false;
        }
};

/*
PVMFMediaClockNotificationsInterfaceImpl is the implementation class for
PVMFMediaClockNotificationsInterface.
*/

class PVMFMediaClockNotificationsInterfaceImpl: public PVMFMediaClockNotificationsInterface
{

    public:

        //constructor

        OSCL_IMPORT_REF PVMFMediaClockNotificationsInterfaceImpl(PVMFMediaClock *, uint32 aLatency,
                PVMFMediaClockNotificationsObsBase& aNotificationInterfaceDestroyedCallback);

        /**
            The default destructor
        */
        OSCL_IMPORT_REF ~PVMFMediaClockNotificationsInterfaceImpl();

        //From PVMFMediaClockNotificationsInterface

        OSCL_IMPORT_REF PVMFStatus SetCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID);

        OSCL_IMPORT_REF PVMFStatus SetCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID);

        OSCL_IMPORT_REF PVMFStatus CancelCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock);

        OSCL_IMPORT_REF PVMFStatus SetNPTCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID);

        OSCL_IMPORT_REF PVMFStatus SetNPTCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID);

        OSCL_IMPORT_REF PVMFStatus CancelNPTCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock);

        OSCL_IMPORT_REF void SetClockObserver(PVMFMediaClockObserver& aObserver);

        OSCL_IMPORT_REF void RemoveClockObserver(PVMFMediaClockObserver& aObserver);

        OSCL_IMPORT_REF void SetClockStateObserver(PVMFMediaClockStateObserver& aObserver);

        OSCL_IMPORT_REF void RemoveClockStateObserver(PVMFMediaClockStateObserver& aObserver);

        //End PVMFMediaClockNotificationsInterface

    protected:

        /*PVMFMediaClock will have a vector of PVMFMediaClockNotificationsInterfaceImpl
        instances. PVMFMediaClock will access iLatency from this class*/
        friend class PVMFMediaClock;
        PVMFMediaClockStateObserver *iClockStateObserver;
        uint32 iLatency;
        //this value is iLatency - min latency among all objects present.
        uint32 iAdjustedLatency;
        //This value is the delay after which clock start notification will be sent.
        uint32 iLatencyDelayForClockStartNotification;
        PVMFMediaClock *iContainer;
        PVMFMediaClockNotificationsObsBase* iNotificationInterfaceDestroyedCallback;
};

/*This structure represents one callback element inserted in PVMFMediaClock's internal timer queues */
class PVMFMediaClockTimerQueueElement
{
    public:
        /**
         * Equality comparison for use with OsclPriorityQueue
         */
        bool operator==(const PVMFMediaClockTimerQueueElement &other) const
        {
            return callBackID == other.callBackID;
        }
        uint32 timeOut;
        uint32 callBackID;
        bool isNPTTimer;
        uint32 window;
        PVMFMediaClockNotificationsObs* obs;
        const OsclAny* contextData;
        const OsclAny* pInterfaceObject;
} ;


/*This structure represents one NPT Clock transition event */
typedef struct _PVMFMediaClockNPTTransitionEventElement
{
    uint32 mediaClockPosition;
    uint32 startNPT;
    bool isPlayBackDirectionBackwards;
    uint32 window;
    uint32 eventID;
} PVMFMediaClockNPTTransitionEventElement;

/*This structure represents one clock start notification event */
typedef struct _PVMFMediaClockStartNotificationEventElement
{
    PVMFMediaClockStateObserver *clockStateObserver;
    uint32 eventID;
} PVMFMediaClockStartNotificationEventElement;

/*This is the comparison class supplied to PVMFMediaClock's priority queues */
class PVMFMediaClockTimerQueueCompareLess
{
    public:
        int compare(PVMFMediaClockTimerQueueElement& a, PVMFMediaClockTimerQueueElement& b) const
        {
            uint32 delta = 0;
            return PVTimeComparisonUtils::IsEarlier(b.timeOut, a.timeOut, delta);
        }
};

/*This comparison class is used for backwards NPT priority queue. This queue needs to maintain timers in
Descending fashion*/
class PVMFMediaClockTimerQueueCompareLessForNPTBackwards
{
    public:
        int compare(PVMFMediaClockTimerQueueElement& a, PVMFMediaClockTimerQueueElement& b) const
        {
            uint32 delta = 0;
            return PVTimeComparisonUtils::IsEarlier(a.timeOut, b.timeOut, delta);
        }
};


class PVMFMediaClock :  public OsclTimerObject,
            public PVMFTimebase,
            public PVMFMediaClockObserver,
            public PVMFMediaClockControlInterface,
            public PVMFMediaClockAccessInterface,
            public PVMFMediaClockNPTClockPositionAccessInterface,
            public PVMFMediaClockNotificationsImplInterface,
            public PVMFMediaClockNotificationsObs /*Media clock uses itself to set callback*/
{

    public:

        /**
            The default constructor initializes the clock to 0 and goes to STOPPED state
        */
        OSCL_IMPORT_REF PVMFMediaClock();

        /**
            The default destructor
        */
        OSCL_IMPORT_REF ~PVMFMediaClock();

        /**
        *    Sets the timebase to use for this clock.
        *    Will trigger an ClockTimebaseUpdated notice to all observers of this clock.
        *    The clock timebase can only be set while in STOPPED and PAUSED states.
        *    @param aTimebase: a reference to an PVMFTimebase-derived object
        *    @return true if the new clock timebase has been accepted, false otherwise
        */
        OSCL_IMPORT_REF bool SetClockTimebase(PVMFTimebase& aTimebase);

        /**
        *    This API constructs a PVMFMediaClockNotificationsInterface object. This API does not
        *    work when clock is in running state and if called while clock is running, it returns
        *    PVMFErrInvalidState. Object created using this API must be explicitly destroyed using
        *    DestroyMediaClockNotificationsInterface().
        *    @param aIface: PVMFMediaClockNotificationsInterface* variable to store reference to
        *                   constructed notifications object.
        *    @param aNotificationInterfaceDestroyedCallback: Implementation of callback function
        *                                                    which would be called when the interface
        *                                                    is destroyed.
        *    @param aLatency: uint32 variable containing latency value associated with the module.
        *    @return true if the new clock timebase has been accepted, false otherwise
        */
        OSCL_IMPORT_REF PVMFStatus ConstructMediaClockNotificationsInterface(PVMFMediaClockNotificationsInterface*& aIface,
                PVMFMediaClockNotificationsObsBase &aNotificationInterfaceDestroyedCallback, uint32 aLatency = 0);

        /*  This API destroys the PVMFMediaClockNotificationsInterface object created
        *   by ConstructMediaClockNotificationsInterface().
        *   @param aIface: PVMFMediaClockNotificationsInterface* variable containing reference to
        *   object to be destroyed.
        */
        OSCL_IMPORT_REF void DestroyMediaClockNotificationsInterface(PVMFMediaClockNotificationsInterface* aInf);



        //	From PVMFTimebase

        OSCL_IMPORT_REF void GetCurrentTick32(uint32& aTimebaseTickCount, bool& aOverflow);

        PVMFCountTimebase* GetCountTimebase()
        {
            if (iClockTimebase)
                return iClockTimebase->GetCountTimebase();
            return NULL;
        }

        OSCL_IMPORT_REF void GetTimebaseResolution(uint32& aResolution);


        OSCL_IMPORT_REF int32 GetRate(void);


        // From PVMFMediaClockControlInterface

        OSCL_IMPORT_REF bool Start();

        OSCL_IMPORT_REF bool Pause();

        OSCL_IMPORT_REF bool Stop();

        OSCL_IMPORT_REF bool SetStartTime32(uint32& aTime, PVMFMediaClock_TimeUnits aUnits, bool& aOverFlow);

        OSCL_IMPORT_REF PVMFMediaClockAdjustTimeStatus AdjustClockTime32(uint32& aClockTime, uint32& aTimebaseTime, uint32& aAdjustedTime, PVMFMediaClock_TimeUnits aUnits, bool& aOverFlow);

        OSCL_IMPORT_REF bool Reset();

        // End PVMFMediaClockControlInterface

        // From PVMFMediaClockAccessInterface

        OSCL_IMPORT_REF void GetStartTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits);
        OSCL_IMPORT_REF void GetCurrentTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits, uint32& aTimebaseTime);
        OSCL_IMPORT_REF void GetCurrentTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits);

        // End PVMFMediaClockAccessInterface


        OSCL_IMPORT_REF bool QueryInterface(const PVUuid& uuid, PVInterface*& iface);

        // From PVMFMediaClockNPTClockPositionAccessInterface

        void UpdateNPTClockPosition(
            /*IN*/  uint32 aStartNPT,
            /*IN*/  bool aIsPlayBackDirectionBackwards);

        PVMFStatus GetNPTClockPosition(
            /*OUT*/ uint32& aCurrentPosition);

        void ClearNPTClockPosition();

        PVMFStatus QueueNPTClockTransitionEvent(uint32 aMediaClockPosition, uint32 aStartNPT,
                                                bool aIsPlayBackDirectionBackwards, uint32 aWindow, uint32& aClockTransitionEventID);

        PVMFStatus CancelNPTClockTransitionEvent(uint32 aClockTransitionEventID);
        // End PVMFMediaClockNPTClockPositionAccessInterface

        /*
         * Enum for PVMFMediaClock's internal states
         */
        enum PVMFMediaClockState
        {
            STOPPED,
            RUNNING,
            PAUSED
        };

        /*Returns the current state of MediaClock*/
        PVMFMediaClockState GetState()
        {
            return iState;
        }


    protected:

        friend class PVMFMediaClockNotificationsInterfaceImpl;

        //start PVMFMediaClockNotificationsImplInterface functions
        /**
            Sets an observer for this clock.  May leave if memory allocation fails.
            @param aObserver: the observer implemenation
        */
        OSCL_IMPORT_REF void SetClockObserver(PVMFMediaClockObserver& aObserver);

        /**
            Removes an observer for this clock.  If the observer is not registered, this
            call does nothing.
            @param aObserver: the observer implemenation
        */
        OSCL_IMPORT_REF void RemoveClockObserver(PVMFMediaClockObserver& aObserver);

        /**
            Sets an observer for this clock.  May leave if memory allocation fails.
            @param aObserver: the observer implemenation
        */
        void SetClockStateObserver(PVMFMediaClockStateObserver& aObserver)
        {/*Dummy function*/
            OSCL_UNUSED_ARG(aObserver);
        }
        /**
            Removes an observer for this clock.  If the observer is not registered, this
            call does nothing.
            @param aObserver: the observer implemenation
        */
        void RemoveClockStateObserver(PVMFMediaClockStateObserver& aObserver)
        {/*Dummy function*/
            OSCL_UNUSED_ARG(aObserver);
        }


        PVMFStatus SetCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject);

        PVMFStatus SetCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject);

        PVMFStatus CancelCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock);

        PVMFStatus SetNPTCallbackAbsoluteTime(
            /*IN*/  uint32 aAbsoluteTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject);

        PVMFStatus SetNPTCallbackDeltaTime(
            /*IN*/  uint32 aDeltaTime,
            /*IN*/  uint32 aWindow,
            /*IN*/  PVMFMediaClockNotificationsObs* aObs,
            /*IN*/  bool aThreadLock,
            /*IN*/  const OsclAny* aContextData,
            /*OUT*/ uint32& aCallBackID,
            /*IN*/  const OsclAny* aInterfaceObject);

        PVMFStatus CancelNPTCallback(
            /*IN*/  uint32 aCallbackID, bool aThreadLock);

        //End PVMFMediaClockNotificationsInterface
        /**
            Changes the clock's state to the specified state
            @param aState: the new state to change to
        */
        void SetClockState(PVMFMediaClockState aState);
        /**
            Updates the iLatestTime and iLatestSourceVal to specified values
            @param aTime: the new iLatestTime value to change to
            @param aSourceVal: the new iLatestSourceVal value to change to
        */
        void UpdateLatestTimes(uint32 aTime, uint32 aSourceVal);

        /* Gets delta between iStartTimebaseTickValue and current tickcount*/
        void GetScaledTimebaseTickCount(uint32& aScaledTickCount, bool& aOverFlow);
        /**
            Updates the iLatestRunningClockTime and iLatestRunningTimebaseTime to new units
        */
        void AdjustClockInternalsToNewUnits(bool& aOverFlow);
        /**
            Converts a time value in the specified time units to microseconds
            @param aSrcVal: unsigned 32-bit time value in units specified by aSrcUnits
            @param aSrcUnits: time units of aSrcVal
            @param aUSecVal: reference to unsigned 32-bit integer to store the microsecond time value
        */
        void ToClockUnit(uint32& aSrcVal, PVMFMediaClock_TimeUnits aSrcUnits, uint32& aDestVal, bool& aOverFlow);
        /**
            Converts a millisecond time value to the specified time units
            @param aUSecVal: unsigned 32-bit integer in microsecond time value
            @param aDstVal: reference to unsigned 32-bit integer which will contain aUSecVal in the
            specified aDstUnits time units
            @param aDstUnits: requested time units for aDstVal
        */

        void FromClockUnit(uint32& aClockUnitVal, uint32& aDstVal,
                           PVMFMediaClock_TimeUnits aDstUnits, bool& aOverFlow);

        /**
            Converts a time value in the specified time units to microseconds
            @param aSrcVal: unsigned 32-bit time value in units specified by aSrcUnits
        	@param aSrcUnits: time units of aSrcVal
        	@param aUSecVal: reference to unsigned 32-bit integer to store the microsecond time value
        */
        void ToUSec(uint32& aSrcVal, PVMFMediaClock_TimeUnits aSrcUnits,
                    uint32& aUSecVal, bool& aOverflow);

        /**
            Converts timebase ticks to clock units
            @param aDelta: unsigned 32-bit tickcount value
            @param aDeltaTime: reference to unsigned 32-bit integer to store time
        */
        void ConvertTickcountToClockUnits(uint32 aTickcount, uint32& aTimeValue, bool& aOverflowFlag);
        /**
            Updates the internal clock parameters based on the adjustment information provided.
            This function can be overridden in the derived classes to allow variety in the adjustment algorithm
            @param aObsTime: unsigned 32-bit integer in microsecond for the observed clock time
            @param aObsTimebase: unsigned 32-bit integer in microsecond for the observed timebase time
            @param aAdjTime: unsigned 32-bit integer in microsecond for the adjusted clock time
            @param aCurrentTime: unsigned 32-bit integer in microsecond for the current clock time
            @param aCurrentTimebase: unsigned 32-bit integer in microsecond for the current timebase time
        */
        virtual PVMFMediaClockAdjustTimeStatus AdjustClock(uint32& aObsTime, uint32& aObsTimebase, uint32& aAdjTime,
                uint32& aCurrentTime, uint32& aCurrentTimebase);

        /**
            Returns the adjusted current clock time when the clock is running
            This function can be overridden in the derived classes to allow variety in the adjustment algorithm
            @param aDstTime: unsigned 32-bit integer in microseconds to output the adjusted current clock time
            @param aTimebaseVal: unsigned 32-bit integer in microseconds of the current timebase time
        */
        virtual void GetAdjustedRunningClockTime(uint32& aDstTime, uint32& aTimebaseVal);

        //Possible units for time keeping
        enum PVMFMediaClock_ClockUnit
        {
            PVMF_MEDIA_CLOCK_CLOCKUNIT_USEC = 0,
            PVMF_MEDIA_CLOCK_CLOCKUNIT_MSEC = 1
        };

        PVLogger* iLogger;


        // Timebase time is stored as unsigned 32-bit value in microseconds
        uint32 iLatestRunningClockTime;           // Last reference clock time due to starting/resuming, pausing, and adjustment
        uint32 iLatestRunningTimebaseTime;        // Timebase time corresponding to the latest running clock time
        uint32 iStartTimebaseTickValue;           // Timebase tick value at clock start time.
        uint32 iStartClockTime;                   // Starting clock time. Set by the SetStartTime...() APIs
        uint32 iPauseClockTime;                   // Clock time when Pause() API is called.
        uint32 iLastAdjustObsTimebaseTime;           // The observed timebase time corresponding to the adjusted time passed in
        uint32 iAdjustmentTimebaseTime;           // The timebase time of the last successful AdjustClockTime...() call
        PVMFMediaClock_ClockUnit iClockUnit;      // unit of above values in clock implementation
        PVMFMediaClock_ClockUnit iPreviousClockUnit; // unit of above values in clock implementation

        PVMFMediaClockState iState;               // Internal state of the clock

        PVMFTimebase* iClockTimebase;             // Pointer to this clock's timebase

        //vector of clock observers.
        Oscl_Vector<PVMFMediaClockObserver*, OsclMemAllocator> iClockObservers;

        //from PVMFMediaClockObserver, for callbacks from an PVMFCountTimebase.
        void ClockCountUpdated();
        void ClockTimebaseUpdated();
        void ClockAdjusted();

        // for NPT mapping
        bool iIsNPTPlayBackDirectionBackwards;
        PVMFTimestamp iStartNPT;
        PVMFTimestamp iStartMediaClockTS;

        //for npt transition queueing
        Oscl_Vector<PVMFMediaClockNPTTransitionEventElement, OsclMemAllocator> iNPTTransitionEventQueue;

        //for clock-start notification event queuing
        Oscl_Vector<PVMFMediaClockStartNotificationEventElement, OsclMemAllocator> iClockStartNotificationEventQueue;

        //vector of PVMFMediaClockNotificationsInterfaceImpl objects. Each object represents a session
        Oscl_Vector<PVMFMediaClockNotificationsInterfaceImpl*, OsclMemAllocator> iMediaClockSetCallbackObjects;

        //callback related functions, members

        OsclPriorityQueue < PVMFMediaClockTimerQueueElement, OsclMemAllocator, Oscl_Vector<PVMFMediaClockTimerQueueElement, OsclMemAllocator>,
        PVMFMediaClockTimerQueueCompareLess > iTimersPriQueue;

        //Mutex for multithreading support
        OsclMutex *iMutex;

        //For keeping count of active timers
        uint32 iActiveTimersCount;

        //For keeping track of timer ID count.
        uint32 iTimerIDCount;

        //This stores the current thread ID when clock is constructed.
        TOsclThreadId iOrigThreadID;

        // Variables to be used when timebase is swapped.
        int32 iLastTimebaseRate;

        bool iIsTimebaseCountBased;

        //common setCallback() for regular and NPT timers
        PVMFStatus SetCallbackCommon(uint32 aDeltaTime, uint32 aWindow,
                                     PVMFMediaClockNotificationsObs* aCallback, bool aThreadLock, const OsclAny* aContextData,
                                     uint32& aCallBackID, const OsclAny* aInterfaceObject, uint32 aCurrentTime, bool aIsNPT);

        //common CancelCallback() for regular and NPT timers
        PVMFStatus CommonCancelCallback(uint32 aCallbackID, bool aThreadLock, bool aIsNPT);

        //This function adjusts latencies of all PVMFMediaClockNotificationsInterfaceImpl objects
        //stored in the vector by subtracting largest common latency from all.
        void AdjustLatenciesOfSinks();

        /**
        This is a generic function for doing fresh scheduling of PVMFMediaClock object
        This function is called when a new callback is created. a callback is cancelled or
        when clock state changes.
        @param aIsNPT: signifies if aCurrentTime is NPT time
        @param aCurrentTime: This is passed for optimization. More often than not, the calling
        function has the current time value. If no argument is supplied, GetCurrentTime() is called.
        */
        void AdjustScheduling(bool aIsNPT = false, uint32 aCurrentTime = 0);

        //for AO
        void Run();

        //Destoys all callback interface objects created. This is called from the destructor.
        void CleanCallbackInfImplObjects();

        /**
        This function calculates the optimum time period after which Run() should be called.
        @param aIsNPT: signifies if aCurrentTime is NPT time
        @param aCurrentTime: This is passed for optimization as calling function has current time value.
        @param aDelta: Time value to be passed in RunIfNotActive()
        */
        void CalculateRunLTimerValue(bool aIsNPT, uint32 aCurrentTime, int32& aDelta);

        //NPT callbacks related members
        OsclPriorityQueue < PVMFMediaClockTimerQueueElement, OsclMemAllocator, Oscl_Vector<PVMFMediaClockTimerQueueElement, OsclMemAllocator>,
        PVMFMediaClockTimerQueueCompareLess > iTimersPriQueueNPT;

        OsclPriorityQueue < PVMFMediaClockTimerQueueElement, OsclMemAllocator, Oscl_Vector<PVMFMediaClockTimerQueueElement, OsclMemAllocator>,
        PVMFMediaClockTimerQueueCompareLessForNPTBackwards > iTimersPriQueueNPTBackwards;

        //For flushing out queue when playback direction changes. This is done with NPT clock
        void ClearPresentNPTQueue();

        void ClearAllQueues();

        //From PVMFMediaClockNotificationsObs interface
        void ProcessCallBack(uint32 aCallBackID, PVTimeComparisonUtils::MediaTimeStatus aTimerAccuracy, uint32 delta,
                             const OsclAny* acontextData, PVMFStatus aStatus);
        void NotificationsInterfaceDestroyed();

        void QueueClockStartNotificationEvent(uint32 aDelta, PVMFMediaClockStateObserver *aClockStateObserver);
};

/**
    PVMFTimebase_Tickcount is PVMFTimebase-derived class which uses
    the OSCL's system tickcount as the timebase. This class is provided
    as the default PVMFTimebase that is available on any platform with OSCL support.
*/
class PVMFTimebase_Tickcount : public PVMFTimebase
{
    public:
        /**
            Constructor. Retrieves the constant to convert OSCL tickcount value to microseconds
        */
        PVMFTimebase_Tickcount()
        {
            iMicrosecPerTick = OsclTickCount::TickCountPeriod();
            iPrevTickcount = 0;
        }

        /**
            Destructor
        */
        ~PVMFTimebase_Tickcount()
        {
        }

        // From PVMFTimebase
        /**
            Returns the OSCL tickcount's time resolution in microseconds
            Implementation of virtual function from PVMFTimebase
            @param aResolution: On function completion, contains OSCL tickcount resolution
        */
        void GetTimebaseResolution(uint32& aResolution)
        {
            aResolution = iMicrosecPerTick;
        }
        int32 GetRate(void)
        {
            return REALTIME_PLAYBACK_RATE;
        }
        OSCL_IMPORT_REF void GetCurrentTick32(uint32& aTimebaseTickCount, bool& aOverflow);
        OSCL_IMPORT_REF void GetCurrentTime32(uint32& aTime, bool& aOverflow, PVMFMediaClock_TimeUnits aUnits);

        PVMFCountTimebase* GetCountTimebase()
        {
            return NULL;
        }

    protected:
        uint32 iMicrosecPerTick;
        uint32 iPrevTickcount;

};

/**
    PVMFTimebase_Count is PVMFTimebase-derived class that can be used to
    implement a simple count-based timebase.
*/
class PVMFTimebase_Count : public PVMFTimebase, public PVMFCountTimebase
{
    public:
        /**
            Constructor.
        */
        PVMFTimebase_Count()
        {
            iCurrentCount = 0;
            iObserver = NULL;
        }

        /**
            Destructor
        */
        ~PVMFTimebase_Count()
        {
        }

        // From PVMFTimebase
        /**
            Returns the OSCL tickcount's time resolution in microseconds
            Implementation of virtual function from PVMFTimebase
            @param aResolution: On function completion, contains OSCL tickcount resolution
        */
        void GetTimebaseResolution(uint32& aResolution)
        {
            aResolution = 0;                      //not meaningful for a count-based timebase.
        }

        int32 GetRate(void)
        {
            return 0;       //not meaningful for a count-based timebase.
        }
        /**
            Returns the current clock time as unsigned 32-bit integer object in the specified time units
            @param aTime: a reference to an unsigned 32-bit integer to return the current time
            @param aUnits: the requested time units for aTime
            @param aTimebaseTime: a reference to an unsigned 32-bit integer to return the timebase time
        */
        void GetCurrentTick32(uint32& aTick, bool& aOverflow)
        {
            //not meaningful for a count-based timebase.
            aTick = 0;
            aOverflow = false;
        }
        /**
            Returns the PVMFCountTimebase implementation pointer
        */
        PVMFCountTimebase* GetCountTimebase()
        {
            return this;
        }

        //From PVMFCountTimebase
        /**
            Used to adjust the current count.
            @param aCount (input): new count value.
        */
        void SetCount(int32 aCount)
        {
            iCurrentCount = aCount;
            if (iObserver)
            {
                iObserver->ClockCountUpdated();
            }
        }

        /**
            Used to retreive the current count.
            @param aCount (output): new count value.
        */
        void GetCount(int32& aCount)
        {
            aCount = iCurrentCount;
        }

    protected:
        int32 iCurrentCount;
        PVMFMediaClockObserver* iObserver;

    private:
        friend class PVMFMediaClock;
        //From PVMFCountTimebase
        void SetClockObserver(PVMFMediaClockObserver* aObserver)
        {
            iObserver = aObserver;
        }

};
#endif                                            //PVMF_MEDIA_CLOCK_H_INCLUDED
