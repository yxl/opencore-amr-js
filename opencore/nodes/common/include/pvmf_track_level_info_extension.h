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
#ifndef PVMF_TRACK_LEVEL_INFO_EXTENSION_H_INCLUDED
#define PVMF_TRACK_LEVEL_INFO_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif


struct PVMFSampleNumTS
{
    int32 iSampleNumber;
    PVMFTimestamp iTimestamp;
};

typedef Oscl_Vector<PVMFSampleNumTS, OsclMemAllocator> PVMFSampleNumTSList;


#define PVMF_TRACK_LEVEL_INFO_INTERFACE_MIMETYPE "pvxxx/pvmf/pvmftracklevelinfointerface"
#define PVMF_TRACK_LEVEL_INFO_INTERFACE_UUID PVUuid(0xb8a4c3ad,0x4c08,0x4962,0xbb,0x6b,0x0d,0x62,0x37,0x63,0xbd,0x22)

class PVMFTrackLevelInfoExtensionInterface : public PVInterface
{
    public:
        virtual void addRef() = 0;
        virtual void removeRef() = 0;
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
        * Retrieves the list of all available tracks from a source node as a list of track info. The returned
        * track info list is needed to call other methods from this extension interface.
        *
        * @param aTracks, a reference to a vector of PVMFTrackInfo which will be filled in with track info
        * of currently available tracks
        * @return PVMFSuccess if successfully retrieved else PVMFFailure for errors.
        */
        virtual PVMFStatus GetAvailableTracks(Oscl_Vector<PVMFTrackInfo, OsclMemAllocator>& aTracks) = 0;

        /**
        * Provides the timestamp for a specified sample number in a specific track
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the list of PVMFTrackInfo returned
        * by GetAvailableTracks() function. The sample number to timestamp conversion would be performed on the track
        * specified by this parameter
        * @param aSampleNum, the sample number in the specified track to return the timestamp. The sample number is
        * in 0-based index.
        * @param aTimestamp, a reference to PVMFTimestamp which will be filled with timestamp value for the specified sample
        * number in the specified track if this function completes successfully
        *
        * @return PVMFSuccess if timestamp is returned for the specified sample number, PVMFErrInvalidState if the information
        * is not available, else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetTimestampForSampleNumber(PVMFTrackInfo& aTrackInfo, uint32 aSampleNum, PVMFTimestamp& aTimestamp) = 0;

        /**
        * Provides the sample number that is closest and before the specified timestamp in a specific track
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the PVMFMediaPresentationInfo returned
        * by GetAvailableTracks() function. The timestamp to sample number conversion would be performed on the track
        * specified by this parameter
        * @param aTimestamp, the timestamp in the specified track to return the sample number for.
        * in 0-based index.
        * @param aSampleNum, a reference to an integer which will be filled with the sample number (0-based index) value closest but before
        * the specified timestamp in the specified track if this function completes successfully. If the timestamp is greater than
        * the duration of the specified track, then the number for the last sample in the track will be returned.
        *
        * @return PVMFSuccess if sample number is returned for the specified timestamp, PVMFErrInvalidState if the information
        * is not available, else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetSampleNumberForTimestamp(PVMFTrackInfo& aTrackInfo, PVMFTimestamp aTimestamp, uint32& aSampleNum) = 0;

        /**
        * Returns the number of synchronization samples in a specific track
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the PVMFMediaPresentationInfo returned
        * by GetAvailableTracks() function. The number of synchronization samples would be returned for the track
        * specified by this parameter
        * @param aNumSyncSamples, a reference to an integer which will be filled with the number of synchronization samples
        * in the specified track. If all the samples in the track are synchronizaton samples, then the integer would be set to -1.
        *
        * @return PVMFSuccess if the number of sync samples is returned for the specified track, PVMFErrInvalidState if the information
        * is not available, else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetNumberOfSyncSamples(PVMFTrackInfo& aTrackInfo, int32& aNumSyncSamples) = 0;

        /**
        * Returns the list of synchronization sample information in a specific track
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the PVMFMediaPresentationInfo returned
        * by GetAvailableTracks() function. The synchronization sample info would be returned for the track
        * specified by this parameter
        * @param aList, a reference to a vector of PVMFSampleNumTS which will be filled with information about synchronization samples
        * in the specified track.
        * @param aStartIndex, the starting index in the synchronization sample list to fill aList with. (0-based index). If aStartIndex
        * is not specified, aList will be filled from the first synchronization sample in the track
        * @param aMaxEntries, the number of synchronization sample infos to fill in aList. If this parameter is not specified, this
        * function will put info on all synchronization samples for the specified track in aList.
        *
        * @return PVMFSuccess if the number of sync samples is returned for the specified track, PVMFErrInvalidState if the information
        * is not available, PVMFErrArgument if aStartIndex and/or aMaxEntries are invalid, PVMFErrNoMemory if memory allocation failure
        * occurs else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetSyncSampleInfo(PVMFTrackInfo& aTrackInfo, PVMFSampleNumTSList& aList, uint32 aStartIndex = 0, int32 aMaxEntries = -1) = 0;

        /**
        * Returns the list of synchronization sample information in a specific track, based on a target timestamp.
        * This API attemps to return closest sync sample information both before and after a given target time.
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the PVMFMediaPresentationInfo returned
        * by GetAvailableTracks() function. The synchronization sample info would be returned for the track
        * specified by this parameter
        * @param aTargetTimeInMS, a reference to a target time in ms. The closest synchronization samples to this time would
        * be returned
        * @param aList, a reference to a vector of PVMFSampleNumTS which will be filled with information about synchronization
        * samples in the specified track. This list contains closest synchronization samples, to specified aTargetTimeInMS,
        * both before and after. If the list contains just one element then it typically means there are no sync samples
        * after the specified target time.
        * @param aHowManySamples, the  offset from base index in the synchronization sample list,if value is X then it will allow
        * to return X previous and X next synchronization samples from base index. If aHowManySamples
        * is not specified, aList will be filled from the one previous and one next synchronization sample from after base index in the track
        *
        * @return PVMFSuccess if the number of sync samples is returned for the specified track, PVMFErrInvalidState if the information
        * is not available, PVMFErrArgument if aTargetTimeInMS is invalid, PVMFErrNoMemory if memory allocation failure
        * occurs else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetSyncSampleInfo(PVMFSampleNumTSList& aList, PVMFTrackInfo& aTrackInfo, int32 aTargetTimeInMS, uint32 aHowManySamples = 1) = 0;

        /**
        * Provides the timestamp of a specific track for a specified data position
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the list of PVMFTrackInfo returned
        * by GetAvailableTracks() function. The data position to timestamp conversion would be performed on the track
        * specified by this parameter
        * @param aDataPosition, the data position in the source to return the timestamp for the specified track. The data
        * position is number of bytes from beginning.
        * @param aTimestamp, a reference to PVMFTimestamp which will be filled with timestamp value in the specified track
        * for the specified data position if this function completes successfully
        *
        * @return PVMFSuccess if timestamp is returned for the specified data position, PVMFErrInvalidState if the information
        * is not available, else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetTimestampForDataPosition(PVMFTrackInfo& aTrackInfo, uint32 aDataPosition, PVMFTimestamp& aTimestamp) = 0;

        /**
        * Provides the data position for the specified timestamp in a specific track
        *
        * @param aTrackInfo, a reference to a PVMFTrackInfo chosen from the PVMFMediaPresentationInfo returned
        * by GetAvailableTracks() function. The timestamp to data position conversion would be performed on the track
        * specified by this parameter
        * @param aTimestamp, the timestamp in the specified track to return the data position.
        * @param aDataPosition, a reference to an unsigned integer which will be filled with the data position
        * (bytes from beginning) closest but before the specified timestamp in the specified track if this function
        * completes successfully. If the timestamp is greater than the duration of the specified track, then the
        * data position for the last sample in the track will be returned.
        *
        * @return PVMFSuccess if data position is returned for the specified timestamp, PVMFErrInvalidState if the information
        * is not available, else PVMFFailure for any other errors
        */
        virtual PVMFStatus GetDataPositionForTimestamp(PVMFTrackInfo& aTrackInfo, PVMFTimestamp aTimestamp, uint32& aDataPosition) = 0;
};

#endif // PVMF_TRACK_LEVEL_INFO_EXTENSION_H_INCLUDED

