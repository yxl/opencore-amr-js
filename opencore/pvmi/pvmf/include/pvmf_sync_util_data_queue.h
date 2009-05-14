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
 * @file pvmf_sync_util_data_queue.h
 * @brief Utility class to synchronize processing of media msg to a specified clock.
 */

#ifndef PVMF_SYNC_UTIL_DATA_QUEUE_H_INCLUDED
#define PVMF_SYNC_UTIL_DATA_QUEUE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#include "pvmf_media_msg.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif
#ifndef PVMF_SYNC_UTIL_H_INCLUDED
#include "pvmf_sync_util.h"
#endif

/** Default data queue reserve size */
const int32 DEFAULT_QUEUE_RESERVE_SIZE = 10;

// Forward declaration
class PvmfSyncUtilDataQueue;

/**
 * Observer class for PvmfSyncUtilDataQueue.  User of PvmfSyncUtilDataQueue class must implement this observer
 * interface to listen to events from the utility.
 */
class PvmfSyncUtilDataQueueObserver
{
    public:
        /**
         * Notifies the observer to call PvmfSyncUtilDataQueue::DequeueMediaData at the specified
         * time to retrieve the media msg and process it.
         *
         * @param aDataQueue Pointer to the PvmfSyncUtilDataQueue object to be processed at the specified time.
         * @param aTimeMilliseconds Number of milliseconds to the time to process the media data.
         */
        virtual void ScheduleProcessData(PvmfSyncUtilDataQueue* aDataQueue, uint32 aTimeMilliseconds) = 0;

        /**
         * Notifies the observer of completing SkipMediaData call. The first media data after the specified
         * resume timestamp is at the head of the queue.
         */
        virtual void SkipMediaDataComplete() = 0;

        virtual ~PvmfSyncUtilDataQueueObserver() {}
};

/**
 * Utility class to synchronize media message to a specified clock. PvmfSyncUtilDataQueue checks the timestamp
 * of incoming message against the specified clock, and decide whether to drop the message if it arrived
 * too late, or schedule the data to be processed at a later time that is synchronized to the
 * specified clock.
 */
class PvmfSyncUtilDataQueue: public PVMFMediaClockObserver
{
    public:
        /**
         * Constructor
         * @param aObserver Observer to the utility
         * @param name Optional name for this queue.  If a name is provided, datapath
         *   logging will be enabled for the queue.
         */
        OSCL_IMPORT_REF PvmfSyncUtilDataQueue(PvmfSyncUtilDataQueueObserver* aObserver, PvmfSyncUtil* aUtil, char*name = NULL);

        /** Destructor */
        OSCL_IMPORT_REF virtual ~PvmfSyncUtilDataQueue() ;

        /**
         * Sets the reserve size of the media message queue.
         *
         * @param aReserveSize Number of media msg reserved on the data queue.
         * @return PVMFErrNoMemory if there is a failure in reserving the necessary memory. Else PVMFSuccess.
         */
        OSCL_IMPORT_REF PVMFStatus ReserveDataQueue(uint32 aReserveSize);

        /**
         * Queue incoming media message to be processed.
         *
         * Incoming media message will be added to the data queue and a PvmfSyncUtilDataQueueObserver::ScheduleProcessData
         * call will be made to schedule the data to be processed when necessary.  If the media message is
         * determined to be late, the data will be dropped and will not be scheduled for processing.
         *
         * @param aMediaMsg Incoming media message to be queued and processed.
         * @param aDropped Optional output parameter to hold the number of late frames dropped.
         * @param aSkipped Optional output parameter to hold the number of frames skipped.
         * @param aFront Optional input parameter, when set the data will go to the front of the queue
         *    rather than at the end.
         * @return PVMFErrNoMemory if the incoming data cannot be added to the queue. PVMFFailure if the
         *         media message is late and is dropped.  PVMFSuccess if the data is successfully scheduled
         *         to be processed at a later time that is synchronized to the clock.
         */
        OSCL_IMPORT_REF PVMFStatus QueueMediaData(PVMFSharedMediaMsgPtr aMediaMsg, uint32*aDropped = NULL, uint32*aSkipped = NULL, bool aFront = false);

        /**
         * Retrieve media message to be processed.
         *
         * Returns the first queued media message if it has a timestamp that lies between the current time of
         * the clock plus and minus the specified late and early margins.  If it is too late to process
         * the first queued media message, the data will be dropped and continue to the next media message. This
         * continues until media message that is on time is found, or media message is scheduled to be processed
         * later, or all queued media message are dropped.  If it is too early to process the first queued
         * media message, it will be rescheduled to be processed at a later time.
         *
         * @param aMediaMsg Output parameter to hold the retrieved media message if the method completed successfully.
         * @param aDropped Optional output parameter to hold the number of late frames dropped.
         * @param aSkipped Optional output parameter to hold the number of frames skipped.
         * @return PVMFSuccess if there is media message for processing at the current time.  PVMFPending if
         *         media message is rescheduled to be processed at a later time.  PVMFFailure if there is no
         *         media message for processing at the current time.
         */
        OSCL_IMPORT_REF PVMFStatus DequeueMediaData(PVMFSharedMediaMsgPtr& aMediaMsg, uint32* aDropped = NULL, uint32* aSkipped = NULL);

        /**
         * Skip processing media message until a specified timestamp
         *
         * Media message with timestamp before the specified resume timestamp will be skipped.  Default
         * behaviour is to drop all skipped message, but if aRenderSkippedData is set to true, DequeueMediaData
         * will return all media message before the resume time forces rendering of all skipped message. This
         * is an asynchronous method and the observer will be notified through SkipMediaDataComplete when
         * the first media message after skipping is at the head of data queue.
         *
         * @param aResumeTimestamp Timestamp at which normal evaluation of timestamp against the clock will resume.
         * @param aRenderSkippedData Force rendering of skipped message.
         * @return PVMFPending if skipping has started and pending completion.
         */
        OSCL_IMPORT_REF PVMFStatus SkipMediaData(PVMFTimestamp aResumeTimestamp, bool aRenderSkippedData = false);

        /**
         * Cancel a SkipMediaData that is in progress and do not make the SkipMediaDataComplete
         * callback.  If not SkipMediaData is in progress, ignore the command.
         *
         */
        OSCL_IMPORT_REF void CancelSkipMediaData();

        /**
         * Clear buffered media message in data queue
         *
         * @return Completion status
         */
        OSCL_IMPORT_REF PVMFStatus Clear();

        /**
         * Set frame drop mode.  Default is enabled (late frames are dropped).
         *
         */
        OSCL_IMPORT_REF void SetLateFrameDropMode(bool);


        /**
         * For direct access to the media message queue.
         */
        Oscl_Vector<PVMFSharedMediaMsgPtr, OsclMemAllocator> &DataQueue()
        {
            return iDataQueue;
        }

        /**
         * For direct access to the synchronization function
         */
        OSCL_IMPORT_REF PVMFStatus SynchronizeData(uint32* , uint32*);


        /**
         * To enable datapath logging on this queue
         */
        OSCL_IMPORT_REF void SetName(const char*);

        OSCL_IMPORT_REF PVMFStatus SetClock(PVMFMediaClock*);
        OSCL_IMPORT_REF PVMFStatus SetClockForFrameStep(PVMFMediaClock*);

        //From PVMFMediaClockObserver
        OSCL_IMPORT_REF void ClockTimebaseUpdated();
        OSCL_IMPORT_REF void ClockCountUpdated();
        OSCL_IMPORT_REF void ClockAdjusted();
        OSCL_IMPORT_REF void NotificationsInterfaceDestroyed();

    private:


        PvmfSyncUtilDataQueueObserver* iObserver;
        PvmfSyncUtil* iSyncUtil;
        Oscl_Vector<PVMFSharedMediaMsgPtr, OsclMemAllocator> iDataQueue;
        PVLogger* iLogger;
        bool iLateFrameDropEnable;
        OSCL_StackString<20> iName;
        PVLogger* iDatapathLogger;
        void LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char* msg);
        void LogMediaMsgInfo(PVMFSharedMediaMsgPtr aMediaMsg, const char* msg, uint32);
        PVMFMediaClock* iClock;
        PVMFMediaClockNotificationsInterface* iClockNotificationsInf;
        bool iClockOwner;
        PVMFStatus DoSetClock(PVMFMediaClock*, bool);
        void PassClockToSyncUtil();

        //for frame-stepping.
        int32 iClockFrameCount;
        int32 iSyncFrameCount;
        PVMFStatus FrameStep();
        bool FrameStepMode();
        bool FrameSyncMode();
        void FrameStepClkAdjust(PVMFTimestamp aTimestamp);

        /* Diagnostic log related */
        PVLogger* iDiagnosticsLogger;
        bool iDiagnosticsLogged;
        void LogDiagnostics();
        uint32 iDropFrameCount;

        OsclErrorTrapImp* iOsclErrorTrapImp;

};

#endif // PVMF_SYNC_UTIL_DATA_QUEUE_H_INCLUDED





