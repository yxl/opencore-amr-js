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
#ifndef PVMF_NODES_SYNC_CONTROL_H_INCLUDED
#define PVMF_NODES_SYNC_CONTROL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PVMF_TIMESTAMP_H_INCLUDED
#include "pvmf_timestamp.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif

#define PvmfNodesSyncControlUuid PVUuid(0xd4d82607,0xbca5,0x43e3,0x88,0x30,0xa3,0x1b,0x36,0x0a,0xb5,0xf2)

// Forward declarations
class PVMFMediaClock;

/**
 * Configuration interface used by data sink nodes to synchronize rendering of media data
 * and handle repositioning.
 */
class PvmfNodesSyncControlInterface : public PVInterface
{
    public:
        /**
         * Sets the clock to which media data is synchronized.
         *
         * @param aClock Clock object to which media data is synchronized.
         * @return Completion status.
         */
        virtual PVMFStatus SetClock(PVMFMediaClock* aClock) = 0;

        /**
         * Notifies the data sink node that clock rate is changing. Also allows
         * the underlying data sink node to tell whether the new rate can be supported
         * via the return code.
         *
         * @param aRate The new playback rate expressed in millipercent of "real-time" playback
         * @return PVMFSuccess if the data sink can handle the new rate. PVMFFailure if it cannot
         **/
        virtual PVMFStatus ChangeClockRate(int32 aRate) = 0;

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
        virtual PVMFStatus SetMargins(int32 aEarlyMargin, int32 aLateMargin) = 0;

        /**
         * Notification that the clock has started
         *
         * The clock reference passed in by SetClock() can be controlled outside of
         * the node. This lets the controller of the clock notify the node when the clock
         * has started progressing
         *
         * @return None
         */
        virtual void ClockStarted(void) = 0;

        /**
         * Notification that the clock has stopped
         *
         * The clock reference passed in by SetClock() can be controlled outside of
         * the node. This lets the controller of the clock notify the node when the clock
         * has stopped progressing due to Pause() or Stop() being called on the clock.
         *
         * @return None
         */
        virtual void ClockStopped(void) = 0;

        /**
         * Skip processing media data until a specified timestamp
         *
         * Media data with timestamp before the specified starting and resume timestamp will be skipped.  Default
         * behaviour is to drop all skipped data, where SyncMediaData API will return -1 for all timestamps
         * before the starting/resume time.  If aRenderSkippedData is set to true, SyncMediaData API will return
         * 0 for all timestamps after the starting time but before the resume time to indicate forced rendering of all skipped data.
         * This method is asynchronous and the completion of this command will be sent through the
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aSessionId Session ID returned when creating a session with the node
         * @param aResumeTimestamp Timestamp at which normal evaluation of timestamp against the clock will resume. This timestamp
         *                         must be greater than or equal to aStartingTimestamp
         * @param aPlayBackPositionContinuous When set to true, this is meant to notify the data sink that
         *        this skip media data call does not relate to a change in source position. Or in
         *        other words source is continuous. Example: Smart Forward Repositioning (SFR). In this case the source node
         *        remains in the same position but sinks are asked to skip thru media data segments.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId SkipMediaData(PVMFSessionId aSessionId,
                                            PVMFTimestamp aResumeTimestamp,
                                            uint32 aStreamID = 0,
                                            bool aPlayBackPositionContinuous = false,
                                            OsclAny* aContext = NULL) = 0;
};

#endif // PVMF_NODES_SYNC_CONTROL_H_INCLUDED




