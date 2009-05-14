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
 * @file pvmf_sync_util.h
 * @brief Utility class to synchronize processing of media data to a specified clock.
 */

#ifndef PVMF_SYNC_UTIL_H_INCLUDED
#define PVMF_SYNC_UTIL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif





#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

const int32 DEFAULT_EARLY_MARGIN = 200;
const int32 DEFAULT_LATE_MARGIN = 200;

typedef enum
{
    PVMF_SYNC_EARLY = 0,	/**< Timestamp is earlier than the current clock by more than the early margin.*/
    PVMF_SYNC_ON_TIME, 	/**< Timestamp is on time with regards to current clock. */
    PVMF_SYNC_LATE,			/**< Timestamp is later than the current clock by more than the late margin. */
    PVMF_SYNC_SKIPPED,		/**< Timestamp falls within the range of skipped media data timestamps. */
    PVMF_SYNC_SKIPPED_RENDER,/**< same as PVMF_SYNC_SKIPPED, but we want to render skipped data */
    PVMF_SYNC_SKIP_COMPLETE	/**< Timestamp is the first media data timestamp after the skipped range. */
} PvmfSyncStatus;

/**
 * Utility class to synchronize media data to a specified clock. PvmfSyncUtil checks the timestamp
 * of incoming data against the specified clock, and decide whether to drop the data if it arrived
 * too late, or schedule the data to be processed at a later time that is synchronized to the
 * specified clock.
 */
class PvmfSyncUtil
{
    public:
        /** Constructor */
        OSCL_IMPORT_REF PvmfSyncUtil();

        /** Destructor */
        ~PvmfSyncUtil() {}

        /**
         * Configure the criteria to determine whether media data is early or late.
         *
         * Media data is considered early if it has a timestamp earlier than the current time of the clock by
         * the early margin.  The media data will be scheduled to be processed at the later time that is
         * synchronized to the clock.  Media data is considered late if it has a timestamp later than the
         * current time of the clock by the late margin and will be dropped.
         *
         * @param aEarlyMargin Early margin in milliseconds.
         * @param aLateMargin Late margin in milliseconds.
         * @return Completion status
         */
        OSCL_IMPORT_REF PVMFStatus SetMargins(int32 aEarlyMargin, int32 aLateMargin);

        /**
         * Synchronize the timestamp of media data with the clock.
         *
         * The provided timestamp is checked against the clock and the utility determines whether the timestamp
         * is considered early, on-time or late.  If the timestamp is earlier than the current time on the clock
         * by more than the early margin, it is considered early, and this API will return the number of
         * milliseconds to the time to process the data.  If the timestamp is later than the current time on the
         * clock by more than the late margin, it is considered late and this API will return -1 to indicate
         * the data should be dropped.  Otherwise the timestamp is considered on-time and this API will return 0.
         *
         * @param aDataTimestamp Timestamp of media data in milliseconds.
         * @param aDuration Duration of the media data in milliseconds.
         * @param aMillisecondsEarly If return status is PVMF_SYNC_EARLY, the number of milliseconds to the
         *                           time to process the data is saved to this parameter
         *		If return is PVMF_SYNC_LATE, the amount by which the frame was late is returned.
         *		If return is PVMF_SYNC_SKIPPED, the delta from the frame TS to the skip mark is returned.
         * @return Synchronization status.
         */
        OSCL_IMPORT_REF PvmfSyncStatus SyncMediaData(PVMFTimestamp aDataTimestamp, uint32 aDuration, uint32& aMillisecondsEarly);

        /**
         * Skip processing media data until a specified timestamp
         *
         * Media data with timestamp before the specified resume timestamp will be skipped.  Default
         * behaviour is to drop all skipped data, where SyncMediaData API will return -1 for all timestamps
         * before the resume time.  If aRenderSkippedData is set to true, SyncMediaData API will return
         * 0 for all timestamps before the resume time to indicate forced rendering of all skipped data.
         *
         * @param aResumeTimestamp Timestamp at which normal evaluation of timestamp against the clock will resume.
         * @param aRenderSkippedData When set to true, SyncMediaData API will return 0 for all timestamps
         *        before the resume time.
         * @return Completion status.
         */
        OSCL_IMPORT_REF PVMFStatus SkipMediaData(PVMFTimestamp aResumeTimestamp, bool aRenderSkippedData = false);

        /**
         * Cancel a SkipMediaData that is in progress.  If no SkipMediaData
         * is in progress, ignore the command.
         *
         */
        OSCL_IMPORT_REF void CancelSkipMediaData();

        /**
        * A query for whether a skip is in progress
        */
        bool IsSkipping()
        {
            return iSkipMediaData;
        }

    private:
        friend class PvmfSyncUtilDataQueue;
        /**
         * Sets the clock to which media data is synchronized.
         *
         * @param aClock Clock object to which media data is synchronized.
         * @return Completion status.
         */
        OSCL_IMPORT_REF PVMFStatus SetClock(PVMFMediaClock* aClock);
        OSCL_IMPORT_REF PVMFStatus SetFrameStepClock(PVMFMediaClock* aClock);


    private:

        PVMFMediaClock* iClock;
        PVMFMediaClock* iFrameStepClock;
        int32 iEarlyMargin;
        int32 iLateMargin;

        // Repositioning
        bool iSkipMediaData;
        PVMFTimestamp iResumeTimestamp;
        bool iRenderSkippedData;

        PVLogger* iLogger;

};

#endif // PVMF_SYNC_UTIL_H_INCLUDED





