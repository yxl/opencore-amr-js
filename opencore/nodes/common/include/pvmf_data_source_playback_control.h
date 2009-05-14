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
#ifndef PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED
#define PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
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
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif


#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#define PVMF_DATA_SOURCE_PLAYBACK_CONTROL_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmfdatasourceplaybackcontrolinterface"
#define PvmfDataSourcePlaybackControlUuid PVUuid(0x4d0ff812,0x4fe1,0x4407,0xb9,0x11,0x6b,0x07,0x3a,0xa3,0x43,0x20)

typedef enum _PVMFDataSourcePositionMode
{
    PVMF_SET_DATA_SOURCE_POSITION_MODE_UNKNOWN = -1,
    PVMF_SET_DATA_SOURCE_POSITION_MODE_NOW = 0,
    PVMF_SET_DATA_SOURCE_POSITION_END_OF_CURRENT_PLAY_ELEMENT = 1,
    PVMF_SET_DATA_SOURCE_POSITION_MODE_END_OF_CURRENT_PLAY_SESSION = 2,
}PVMFDataSourcePositionMode;

class PVMFDataSourcePositionParams
{
    public:
        PVMFDataSourcePositionParams()
        {
            iMode = PVMF_SET_DATA_SOURCE_POSITION_MODE_UNKNOWN;
            iPlayElementIndex = -1;
            iTargetNPT = 0;
            iActualNPT = 0;
            iActualMediaDataTS = 0;
            iSeekToSyncPoint = true;
            iPlaylistUri = NULL;
        };

        PVMFDataSourcePositionMode iMode;
        int32                      iPlayElementIndex;
        PVMFTimestamp              iTargetNPT;
        PVMFTimestamp              iActualNPT;
        PVMFTimestamp              iActualMediaDataTS;
        bool                       iSeekToSyncPoint;
        uint32                     iStreamID;
        char*					   iPlaylistUri;
};


// Forward declaration
class PVMFPortInterface;

/**
 * Configuration interface to control data source nodes for playback
 */
class PvmfDataSourcePlaybackControlInterface : public PVInterface
{
    public:
        /**
         * Asynchronous method to set the position of a data source to a new location.
         *
         * The data source will be repositioned to continue providing media data at the
         * specified destination timestamp.  In the case where tracks with synchronization points are
         * among the source data types provided, there is an option to reposition the
         * data source to the nearest synchronization point before the specified destination timestamp.
         * The actual timestamp of the first media data after repositioning will be written to the
         * aActualTimestamp parameter. Source data for all data tracks will be repositioned to the
         * actual destination timestamp and the user might need to have knowledge of the difference
         * between the specified destination timestamp and the actual one for rendering purposes.
         * This method is asynchronous and the completion of this command will be sent through the
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aTargetNPT Target normal-play-time timestamp in milliseconds of the location where the
         *                   data source will be repositioned to.
         * @param aActualNPT The actual normal-play-time timestamp after repositioning will be saved
         *                   to this parameter.
         * @param aActualMediaDataTS The media data timestamp corresponding to the actual NPT time. This
         *                           will be the timestamp of the first media data after repositioning.
         * @param aSeekToSyncPoint If data source provides tracks with synchronization points, enabling this option
         *                      will reposition the data source to the nearest synchronization point before the
         *                      specified destination.
         * @param aStreamID Player engine informs the source and sink about the streamID that will apply for the
         *                  next playback segment since there needs to be a coordination between the two.
         *                  In some cases the engine may want the source to resend the BOS with the same
         *                  streamID again.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                PVMFTimestamp& aActualMediaDataTS,
                bool aSeekToSyncPoint = true,
                uint32 aStreamID = 0,
                OsclAny* aContext = NULL) = 0;


        /**
         * Asynchronous method to set the position of a data source to a new location.
         *
         * The data source will be repositioned to continue providing media data at the
         * specified destination timestamp.  In the case where tracks with synchronization points are
         * among the source data types provided, there is an option to reposition the
         * data source to the nearest synchronization point before the specified destination timestamp.
         * @param aSessionId The assigned node session ID to use for this request
         *
         * @param aPVMFDataSourcePositionParams
         * All the repositioning parameters are contained in PVMFDataSourcePositionParams.
         * This is a container class for:
         * #iMode - Playback positon mode indicates when the provided playback position is expected to take effect.
         * 1) Now => implies that pause any current playback session and start playback from the new position
         * 2) End of current play element => A play session can be composed of multiple play elements (say a
         * playlist session) and the provided playlist position is to take effect once the current play
         * element is complete (say reposition to song 7 after the current song is done)
         * 3) End of current play session => This playback position is to take effect once the entire current
         * play session is done.
         * #iPlayElementIndex - Typically provided in case of a seek within a playsession
         * with multiple elements. This is the index ON which the position info provided above applies.
         * Say go to 30 seconds into play element 7. If this value is -1 it means current
         * play element.
         * #iTargetNPT Target normal-play-time timestamp in milliseconds of the location where the
         *             data source will be repositioned to.
         * #iActualNPT The actual normal-play-time timestamp after repositioning will be saved
         *             to this parameter.
         * #iActualTimestamp - The actual timestamp of the first media data after repositioning will be written
         * to the iActualTimestamp parameter. Source data for all data tracks will be repositioned to the
         * actual destination timestamp and the user might need to have knowledge of the difference
         * between the specified destination timestamp and the actual one for rendering purposes.
         * #iSeekToSyncPoint If data source provides tracks with synchronization points, enabling this option
         *                   will reposition the data source to the nearest synchronization point before the
         *                   specified destination.
         *
         * This method is asynchronous and the completion of this command will be sent through the
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVMFCommandId SetDataSourcePosition(PVMFSessionId aSessionId,
                PVMFDataSourcePositionParams& aPVMFDataSourcePositionParams,
                OsclAny* aContext = NULL)
        {
            /* Derived classes can override this method if so desired */
            OSCL_UNUSED_ARG(aSessionId);
            OSCL_UNUSED_ARG(aPVMFDataSourcePositionParams);
            OSCL_UNUSED_ARG(aContext);

            OSCL_LEAVE(OsclErrNotSupported);
            return 0;
        }

        /**
         * Asynchronous method to query the position of a data source to a new location without changing the position
         *
         * In the case where tracks with synchronization point are
         * among the source data types provided, there is an option to return the
         * nearest synchronization point before the specified destination timestamp.
         * This method is asynchronous and the completion of this command will be sent through the
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aTargetNPT Target normal-play-time timestamp in milliseconds of the location where the
         *                   data source should be if actually repositioning.
         * @param aActualNPT The actual normal-play-time timestamp after repositioning if it actually occurs
         * @param aSeekToSyncPoint If data source provides tracks with synchronization points, enabling this option
         *                      will set the actual NPT to a synchronization point before the target NPT.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aActualNPT,
                bool aSeekToSyncPoint = true,
                OsclAny* aContext = NULL
                                                     ) = 0;

        /**
         * Asynchronous method to query the position of a data source to a new location without changing the position
         *
         * This overload of QueryDataSourcePosition adds two function arguments, aSyncBeforeTargetNPT and aSyncBeforeTargetNPT
         * The idea is to let engine make a decision for repositioning, instead of node.
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aTargetNPT Target normal-play-time timestamp in milliseconds of the location where the
         *                   data source should be if actually repositioning.
         * @param aSyncBeforeTargetNPT The timestamp of nearest sync point before aTargetNPT
         * @param aSyncAfterTargetNPT The timestamp of nearest sync point before aTargetNPT
         * @param aSeekToSyncPoint If data source provides tracks with synchronization points, enabling this option
         *                      will set the actual NPT to a synchronization point before the target NPT.
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVMFCommandId QueryDataSourcePosition(PVMFSessionId aSessionId,
                PVMFTimestamp aTargetNPT,
                PVMFTimestamp& aSyncBeforeTargetNPT,
                PVMFTimestamp& aSyncAfterTargetNPT,
                OsclAny* aContext = NULL,
                bool aSeekToSyncPoint = true
                                                     ) = 0;

        /**
         * Asynchronous method to set the playback rate of the data source
         *
         * The playback rate is specified as millipercent of the "real-time" playback rate (e.g. 100000=1X,
         * 400000=4X, -50000=0.5X backward) or as an outside timebase. If the specified rate is not
         * supported by the data source, the command should complete with an error status.
         * This method is asynchronous and the completion of this command will be sent through the
         * PVMFNodeCmdStatusObserver of the node implementing this interface.
         *
         * @param aSessionId The assigned node session ID to use for this request
         * @param aRate The new playback rate expressed as millipercent of "real-time" playback rate
         * @param aTimebase The timebase specified to use for the playback clock
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVMFCommandId SetDataSourceRate(PVMFSessionId aSessionId,
                                                int32 aRate,
                                                PVMFTimebase* aTimebase = NULL,
                                                OsclAny* aContext = NULL) = 0;

        /**
         * Synchronous method to compute skip timestamp based on actual and target NPT values
         * Not all source nodes need implement this. Some source node (viz. 3GPP RTSP Streaming)
         * are "realtime" sources and are constrained by remote server's datatransfer rate,which
         * typically is realtime. In those cases it makes sense to start rendering from actualNPT
         * immaterial how engine's repositioning tunables are configured.
         *
         * @param aTargetNPT Target normal-play-time timestamp in milliseconds of the location where the
         *                   data source will be repositioned to, as provided in a prior SetDataSourcePosition
         *                   call.
         * @param aActualNPT The actual normal-play-time timestamp after repositioning as provided by the
         *                   source node in a prior SetDataSourcePosition call.
         * @param aActualMediaDataTS The media data timestamp corresponding to the actual NPT time. This
         *                           will be the timestamp of the first media data after repositioning, as
         *                           provided by the source node.
         * @param aSkipTimeStamp Computed by source node based on aTargetNPT, aActualNPT and aActualMediaDataTS.
         * @param aStartNPT Computed by source node based on aTargetNPT, aActualNPT and aActualMediaDataTS.
         * @returns PVMFErrNotSupported if the source node does not care about skip timestamps, else one
         * of PVMF return codes.
         **/
        virtual PVMFStatus ComputeSkipTimeStamp(PVMFTimestamp aTargetNPT,
                                                PVMFTimestamp aActualNPT,
                                                PVMFTimestamp aActualMediaDataTS,
                                                PVMFTimestamp& aSkipTimeStamp,
                                                PVMFTimestamp& aStartNPT)
        {
            OSCL_UNUSED_ARG(aTargetNPT);
            OSCL_UNUSED_ARG(aActualNPT);
            OSCL_UNUSED_ARG(aActualMediaDataTS);
            OSCL_UNUSED_ARG(aSkipTimeStamp);
            OSCL_UNUSED_ARG(aStartNPT);
            return PVMFErrNotSupported;
        }

        /**
         * Synchronous method to notify the Target NPT to the Source Node. Some source node
         * (viz. 3GPP RTSP Streaming) will need to know the Target NPT before Prepare is
         * called on the node to support PipeLining feature, in which the Source node
         * will send a SETUP and PLAY request back to back to the streaming server's supporting
         * PipeLining mode.
         *
         * @param aTargetNPT Target normal-play-time timestamp in milliseconds of the location where the
         *                   data source will be repositioned to.
         */
        virtual PVMFStatus NotifyTargetPositionSync(PVMFTimestamp aTargetNPT)
        {
            OSCL_UNUSED_ARG(aTargetNPT);
            return PVMFErrNotSupported;
        }
};

#endif // PVMF_DATA_SOURCE_PLAYBACK_CONTROL_H_INCLUDED




