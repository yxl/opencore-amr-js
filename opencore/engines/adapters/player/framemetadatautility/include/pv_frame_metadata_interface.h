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
/*
* ==============================================================================
*  Name        : pv_frame_metadata_interface.h
*  Part of     :
*  Interface   :
*  Description : Interface class and supporting definitions for the pvFrameAndMetadata utility
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_FRAME_METADATA_INTERFACE_H_INCLUDED
#define PV_FRAME_METADATA_INTERFACE_H_INCLUDED

// INCLUDES
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif

#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

#ifndef PV_PLAYER_TYPES_H_INCLUDED
#include "pv_player_types.h"
#endif

#ifndef PV_PLAYER_DATASOURCE_H_INCLUDED
#include "pv_player_datasource.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif


#define PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_ONLY           0x00000001
#define PV_FRAME_METADATA_INTERFACE_MODE_SOURCE_METADATA_AND_THUMBNAIL  0x00000002
#define PV_FRAME_METADATA_INTERFACE_MODE_ALL                            0x00000003


class PVFrameSelector
{
    public:
        enum PVFrameSelectMethod
        {
            SPECIFIC_FRAME,
            TIMESTAMP,
            AUTODETECT_FRAME
        };

        PVFrameSelectMethod iSelectionMethod;

        union
        {
            uint32 iFrameIndex;	   // Specific frame or max frame index for autodetect
            uint32 iTimeOffsetMilliSec; // Time of frame or max time offset for autodetect
        } iFrameInfo;
};

struct PVFrameBufferProperty
{
public:
    uint32 iFrameWidth;
    uint32 iFrameHeight;
    uint32 iDisplayWidth;
    uint32 iDisplayHeight;
};


/**
 * An enumeration of the major states of the pvFrameAndMetdata utility.
 **/
typedef enum
{
    /**
    	The state immediately after the pvFrameAndMetadata instance has been successfully created or instantiated.
    	pvFrameAndMetadata utility also returns to this state after successful completion of RemoveDataSource command.
    	The player data source can be added during this state.
    	No resources have been allocated yet.
    **/
    PVFM_STATE_IDLE	= 1,

    /**
    	pvFrameAndMetadata utility is in this state after successfully completing initialization of data source.
    	Video frames and metadata can be retrieved in this state.
    **/
    PVFM_STATE_INITIALIZED = 2,

    /**
    	pvFrameAndMetadata utility enters this state when it encounters an error. This is a transitional state and
    	after pvFrameAndMetadata performs error recovery, it should end up in PVFM_STATE_IDLE state.
    **/
    PVFM_STATE_ERROR = 3
} PVFrameAndMetadataState;


/**
 UUID for pvFrameAndMetadata error and information event type codes
 **/
#define PVFrameAndMetadataErrorInfoEventTypesUUID PVUuid(0x46fca5ac,0x5b57,0x4cc2,0x82,0xc3,0x03,0x10,0x60,0xb7,0xb5,0x98)

/**
 * An enumeration of error types from pvFrameAndMetadata
 **/
typedef enum
{
    /**
     pvFrameAndMetadata sends this error event when the player engine
     reports an error in response to a player API call
    **/
    PVFMErrPlayerEngine = 1024,

    /**
     pvFrameAndMetadata sends this error event when the MIO component for frame retrieval
     reports an error
    **/
    PVFMErrMIOComponent = 1025,

    /**
     pvFrameAndMetadata sends this error event when the specified source does not have a video track
     to retrieve a frame
    **/
    PVFMErrNoVideoTrack = 1026,

    /**
     Placeholder for the last pvFrameAndMetadata error event
     **/
    PVFMErrLast = 8191
} PVFMErrorEventType;


/**
 * An enumeration of informational event types from pvFrameAndMetadata
 **/
typedef enum
{
    /**
     pvPlayer sends this event when the playback has reached
     the specified end time and playback has been paused.
     **/
//	PVPlayerInfoEndTimeReached	= 8192,

    /**
     Placeholder for the last pvFrameAndMetadata informational event
     **/
    PVFMInfoLast = 10000

} PVFMInformationalEventType;


// CLASS DECLARATION
/**
 * PVFrameAndMetadataInterface is the interface to the pvFrameAndMetadata utility
 **/
class PVFrameAndMetadataInterface
{
    public:

        /**
         * Object destructor function
         * Releases all resources prior to destruction
         **/
        virtual ~PVFrameAndMetadataInterface() {};

        /**
         * This API is to allow for extensibility of the pvFrameAndMetadata interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the system, they are added
         * to the UUIDs array.
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type. This functionality can be turned off.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aMimeType
         *         The MIME type of the desired interfaces
         * @param aUuids
         *         An array to hold the discovered UUIDs
         * @param aExactUuidsOnly
         *         Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly = false, const OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to allow for extensibility of the pvFrameAndMetadata interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aUuid
         *         The UUID of the desired interface
         * @param aInterfacePtr
         *         A reference to the output pointer to the desired interface
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response

         * @exception This method can leave with one of the following error codes
         *         OsclErrNotSupported if the specified interface UUID is not supported
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId QueryInterface(const PVUuid& aUuid, PVInterface*& aInterfacePtr, const OsclAny* aContextData = NULL) = 0;


        /**
         * This API is to allow the user to cancel all pending requests in pvFrameAndMetadata utility. The current request being
         * processed, if any, will also be aborted. If a request being processed is cancelled, pvFrameAndMetadata utility will reset
         * everything and go to PVFM_STATE_IDLE state.
         * If this API is called when there is no request being processing (just pending requests or no pending requests),
         * the engine will not change the state and report this command to complete successfully.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId CancelAllCommands(const OsclAny* aContextData = NULL) = 0;


        /**
         * This function returns the current state of pvFrameAndMetadata utility.
         * Application may use this info for updating display or determine if the
         * pvFrameAndMetadata is ready for the next request.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aState
         *         A reference to a PVFrameAndMetadataState. Upon successful completion of this command,
         *         it will contain the current state of pvFrameAndMetadata utility.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetState(PVFrameAndMetadataState& aState, const OsclAny* aContextData = NULL) = 0;


        /**
         * This function returns the current state of pvFrameAndMetadata utility as a synchronous command.
         * Application may use this info for updating display or determine if the
         * pvFrameAndMetadata is ready for the next request.
         *
         * @param aState
         *         A reference to a PVFrameAndMetadataState. Upon successful completion of this command,
         *         it will contain the current state of pvFrameAndMetadata utility.
         * @returns Status indicating whether the command succeeded or not.
         **/
        virtual PVMFStatus GetStateSync(PVFrameAndMetadataState& aState) = 0;


        /**
         * This function allows a player data source to be specified to retrieve video frame and metadata. This function must be called
         * when pvFrameAndMetadata utility is in PVFM_STATE_IDLE state and before retrieving video frames and metadata. The specified
         * data source must be a valid PVPlayerDataSource to be accepted for use.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aDataSource
         *         Reference to the player data source to be used to retrieve video frames and metadata
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *         OsclErrNotSupported if the format of the source is incompatible with what the utility can handle
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @return A unique command ID for asynchronous completion
         */
        virtual PVCommandId AddDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData = NULL) = 0;


        /**
         * This function makes a request to return the list of all or segment of available metadata keys from the specified
         * data source.
         * The list can be used to retrieve the associated metadata values with GetMetadataValues function.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aKeyList
         *         Reference to a vector to place the metadata key list.
         * @param aStartingIndex
         *         Input parameter to specify the starting index for aKeyList. This parameter along with aMaxEntries
         *         allows us to retrieve the metadata key list in segments.
         * @param aMaxEntries
         *         Input parameter to specify the maximum number of entries to be added to aKeyList. If there is no limit, set to -1.
         * @param aQueryKey
         *         Input parameter to narrow down the list of requested keys. For example,
         *         "track-info/video" indicates all keys related to "track-info/video". for eg:
         *         "track-info/video/width" "track-info/video/height". A NULL value indicates that all
         *         keys are requested.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetMetadataKeys(PVPMetadataList& aKeyList, int32 aStartingIndex = 0, int32 aMaxEntries = -1,
                                            char* aQueryKey = NULL, const OsclAny* aContextData = NULL) = 0;


        /**
         * The function makes a request to return the metadata values specified by the passed in metadata key list.
         * If the requeted metadata value is unavailable or the metadata key is invalid, the returned list will not contain
         * a KVP entry for that key. Note that value indexed in the returned aValueList does not necessary match the same index into
         * the specified aKeyList since this command can return none or more than one KVP for a specified key.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aKeyList
         *         Reference to a list of metadata keys for which metadata values are requested.
         * @param aStartingValueIndex
         *         The starting index refers to the an index into the whole value list specified by the keys in aKeyList. This command
         *         would populate the aValueList starting from the specified index.
         * @param aMaxValueEntries
         *         Input parameter to specify the maximum number of entries to be added to aValueList. If there is no limit, set to -1.
         * @param aNumAvailableValueEntries
         *         Output parameter which will be filled with number of available values for the specified key list.
         * @param aValueList
         *         Reference to a vector of KVP to place the specified metadata values
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetMetadataValues(PVPMetadataList& aKeyList, int32 aStartingValueIndex, int32 aMaxValueEntries, int32& aNumAvailableValueEntries,
                                              Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, const OsclAny* aContextData = NULL) = 0;


        /**
         * The function returns the frame at the requested frame index.  In this
         * variant of the API, the caller provides the buffer.
         *
         * @param aFrameSelInfo
         *         The PVFrameSelector input parameter that specifies the
         *         frame of interest (i.e., whether to return a specific
         *         frame based on a frame index or time specification or
         *         the upper limit for the autodetection algorithm).
         * @param aProvidedFrameBuffer
         *         A pointer to the buffer provided by the caller
         *         to hold the output frame.
         * @param aBufferSize
         *         An input/output parameter with the capacity of the
         *         buffer on input.  The actual filled size will be
         *         returned when the function completes.
         * @param aBufferProp
         *         An output parameter which will contain information about the frame (e.g. width, height)
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetFrame(PVFrameSelector& aFrameSelInfo, uint8* aProvidedFrameBuffer, uint32& aBufferSize, PVFrameBufferProperty& aBufferProp, const OsclAny* aContextData = NULL) = 0;


        /**
         * The function returns the frame at the requested frame index.  In this
         * variant of the API, the pvFrameAndMetadata utility provides memory for
         * the frame buffer and returns it in the specified buffer pointer reference,
         *
         * @param aFrameSelInfo
         *         The PVFrameSelector input parameter that specifies the
         *         frame of interest (i.e., whether to return a specific
         *         frame based on a frame index or time specification or
         *         the upper limit for the autodetection algorithm).
         * @param aFrameBufferPtr
         *         An output parameter holding a double pointer to the
         *         buffer with output frame which will be provided by the utility.
         *         The buffer must be returned with ReturnBuffer API call.
         * @param aBufferSize
         *         An output parameter which will be set to the actual size of the frame buffer.
         * @param aBufferProp
         *         An output parameter which will contain information about the frame (e.g. width, height)
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetFrame(PVFrameSelector& aFrameSelInfo, uint8** aFrameBufferPtr, uint32& aBufferSize, PVFrameBufferProperty& aBufferProp, const OsclAny* aContextData = NULL) = 0;


        /**
         * The function returns the frame buffer allocated with the GetFrame API back to the utility
         *
         * @param aFrameBufferPtr
         *         An input parameter holding a pointer to the frame buffer provided by the utility.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrArgument if the specified frame buffer pointer is invalid
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId ReturnBuffer(uint8* aFrameBufferPtr, const OsclAny* aContextData = NULL) = 0;


        /**
         * This function may be used to close and unbind a player data source that has been previously added.
         * This function must be called when pvFrameAndMetadata utility is in PVFM_STATE_INITIALIZED state.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aDataSource
         *         Reference to the player data source to be removed.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrArgument if the passed in data source parameter is invalid
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the utility failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId RemoveDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to specify the pvFrameAndMetadata interface user's intent.
         * It allows a caller to specify upfront what their intent is, viz. just source metadata,
         * or all metadata (including codecs, sink etc), best thumbnail mode etc.
         * Specifiying the mode is optional. If this API is not called then default values
         * will be used. Default for metadata being all metadata. As for thumbnails if the
         * mode was not specified then whatever frame select mode is specified in GetFrame
         * API would be used.
         *
         * @param aMode
         *
         * @returns A status code on whether the intent was accepted or not
         * (PVMFSuccess, PVMFFailure, PVMFErrNotSupported, PVMFErrInvalidState, PVMFErrArgument)
         **/
        virtual PVMFStatus SetMode(uint32 aMode) = 0;

        /**
         * This method allows setting the maximum thumbnail size to be retrieved.
         * Any video streams with larger dimensions will be scaled down proportionally.
         * aWidth * aHeight * bpc should not exceed the size of the provided buffer.
         * @param aWidth
         * @param aHeight
         *
         * @returns A status code on whether the intent was accepted or not
         * (PVMFSuccess, PVMFFailure, PVMFErrNotSupported, PVMFErrInvalidState, PVMFErrArgument)
         **/
        virtual void SetThumbnailDimensions(uint32 aWidth, uint32 aHeight) = 0;
        virtual void GetThumbnailDimensions(uint32 &aWidth, uint32 &aHeight) = 0;
};

#endif // PV_FRAME_METADATA_INTERFACE_H_INCLUDED


