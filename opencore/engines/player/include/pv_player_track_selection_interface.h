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
#ifndef PV_PLAYER_TRACK_SELECTION_INTERFACE_H_INCLUDED
#define PV_PLAYER_TRACK_SELECTION_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

#define PV_PLAYER_TRACK_SELECTION_INTERFACE_MIMETYPE "pvxxx/pvplayer/track_selection_interface"
#define PVPlayerTrackSelectionInterfaceUuid PVUuid(0x960b3556,0xad6d,0x42bf,0xbc,0x3c,0xed,0x11,0x8d,0x50,0xd6,0x24)

class PVMFMediaPresentationInfo;

class PVMFTrackSelectionHelper
{
    public:
        /**
         * A synchronous method to obtain track selection inputs from the
         * user of pvPlayerSDK. If the user of pvPlayerSDK wishes to participate
         * in the track selection process then, the user of the SDK needs to provide
         * an implementation of this object. If provided, pvPlayerSDK will invoked the
         * SelectTracks(...) API as part of prepare.
         *
         *
         * @param aPlayableList [in] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the complete list of playable tracks.
         *
         * @param aPreferenceList [out] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the complete list of tracks that the user of SDK
         *                    wants to play. The implementor of this object would have to allocate
         *                    memory to populate the preference list.
         *
         * @returns PVMFSuccess if the wish list was successfully populated,
         *          PVMFErrNotSupported if user of SDK does not wish to provide inputs,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus SelectTracks(const PVMFMediaPresentationInfo& aPlayableList,
                                        PVMFMediaPresentationInfo& aPreferenceList) = 0;

        /**
         * A synchronous method to release the playable list of available tracks.
         * This API will be called by pvPlayerSDK to release the memory allocated
         * during SelectTracks.
         *
         * @param aList [in] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the preference list.
         *
         * @returns PVMFSuccess if the memory was successfully released,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus ReleasePreferenceList(PVMFMediaPresentationInfo& aPreferenceList) = 0;

};

/**
 * Track Selection interface for pvPlayer
 */
class PVPlayerTrackSelectionInterface : public PVInterface
{
    public:

        /**
         * A synchronous method to get the complete list of available tracks.
         * This API can be called anytime after a successful player init,
         * up until a reset.
         *
         * @param aList [out] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the complete list of available tracks. pvPlayerSDK
         *                    allocates memory while populating aList.
         *
         * @returns PVMFSuccess if the list was successfully populated,
         *          PVMFErrInvalidState if invoked before init complete or after reset,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus GetCompleteList(PVMFMediaPresentationInfo& aList) = 0;

        /**
         * A synchronous method to release the complete list of available tracks.
         * This API must be called to release the memory allocated during GetCompleteList.
         *
         * @param aList [in] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the complete list of available tracks.
         *
         * @returns PVMFSuccess if the memory was successfully released,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus ReleaseCompleteList(PVMFMediaPresentationInfo& aList) = 0;

        /**
         * A synchronous method to get the complete list of playable tracks.
         * The list of playable tracks is a subset (at times a proper subset) of
         * the complete list of available tracks.
         * This API can be called anytime after a successful player prepare,
         * up until a reset.
         *
         * @param aList [out] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the list of playable tracks. pvPlayerSDK
         *                    allocates memory while populating aList.
         *
         * @returns PVMFSuccess if the list was successfully populated,
         *          PVMFErrInvalidState if invoked before prepare complete or after reset,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus GetPlayableList(PVMFMediaPresentationInfo& aList) = 0;

        /**
         * A synchronous method to release the playable list of available tracks.
         * This API must be called to release the memory allocated during GetPlayableList.
         *
         * @param aList [in] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the complete list of playable tracks.
         *
         * @returns PVMFSuccess if the memory was successfully released,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus ReleasePlayableList(PVMFMediaPresentationInfo& aList) = 0;

        /**
         * A synchronous method to get the complete list of selected tracks.
         * The list of selected tracks is a subset (at times a proper subset) of
         * the list of playable tracks.
         * This API can be called anytime after a successful player prepare,
         * up until a reset.
         *
         * @param aList [out] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the list of selected tracks.pvPlayerSDK
         *                    allocates memory while populating aList.
         *
         * @returns PVMFSuccess if the list was successfully populated,
         *          PVMFErrInvalidState if invoked before prepare complete or after reset,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus GetSelectedList(PVMFMediaPresentationInfo& aList) = 0;

        /**
         * A synchronous method to release the playable list of available tracks.
         * This API must be called to release the memory allocated during GetSelectedList.
         *
         * @param aList [in] This is a data structure of type PVMFMediaPresentationInfo
         *                    that holds the complete list of selected tracks.
         *
         * @returns PVMFSuccess if the memory was successfully released,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus ReleaseSelectedList(PVMFMediaPresentationInfo& aList) = 0;

        /**
         * A synchronous method to register the track selection helper object.
         * This API can be called anytime after a successful player create,
         * up until delete. This API can be invoked with NULL to effectively
         * unregister a previously registered object.
         *
         * @param aObject [in] A pointer to track selection helper object,
         *                     can be NULL.
         *
         * @returns PVMFSuccess if the object was accepted by pvPlayerSDK,
         *          PVMFErrAlreadyExists if there is a previously registered object,
         *          PVMFFailure otherwise
         */
        virtual PVMFStatus RegisterHelperObject(PVMFTrackSelectionHelper* aObject) = 0;

};


#endif // PV_PLAYER_LICENSE_ACQUISITION_INTERFACE_H_INCLUDED



