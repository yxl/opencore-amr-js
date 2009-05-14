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
*  Name        : pv_player_interface.h
*  Part of     :
*  Interface   :
*  Description : Interface class and supporting definitions for the pvPlayer engine
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_PLAYER_INTERFACE_H_INCLUDED
#define PV_PLAYER_INTERFACE_H_INCLUDED


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

#ifndef PV_PLAYER_EVENTS_H_INCLUDED
#include "pv_player_events.h"
#endif

#ifndef PV_PLAYER_DATASOURCE_H_INCLUDED
#include "pv_player_datasource.h"
#endif

#ifndef PV_PLAYER_DATASINK_H_INCLUDED
#include "pv_player_datasink.h"
#endif

#ifndef PVMI_KVP_H_INCLUDED
#include "pvmi_kvp.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif


// CLASS DECLARATION
/**
 * PVPlayerInterface is the interface to the pvPlayer SDK, which
 * allows control of a multimedia playback engine.
 * The PVPlayerFactory factory class is to be used to create and
 * delete instances of this object
 **/
class PVPlayerInterface
{
    public:

        /**
         * Object destructor function
         * Releases all resources prior to destruction
         **/
        virtual ~PVPlayerInterface() {};

        /**
         * Returns information about all modules currently used by pvPlayer SDK.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aSDKModuleInfo
         *         A reference to a PVSDKModuleInfo structure which contains the number of modules currently used by
         *         pvPlayer SDK and the PV UUID and description string for each module. The PV UUID and description string
         *         for modules will be returned in one string buffer allocated by the client. If the string buffer
         *         is not large enough to hold the all the module's information, the information will be written
         *         up to the length of the buffer and truncated.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows a logging appender to be attached at some point in the
         * logger tag tree.  The location in the tag tree is specified by the
         * input tag string.  A single appender can be attached multiple times in
         * the tree, but it may result in duplicate copies of log messages if the
         * appender is not attached in disjoint portions of the tree.
         * A logging appender is responsible for actually writing the log message
         * to its final location (e.g., memory, file, network, etc).
         * This API can be called anytime after creation of pvPlayer.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aTag
         *         Specifies the logger tree tag where the appender should be attached.
         * @param aAppender
         *         The log appender to attach.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId SetLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows a logging appender to be removed from the logger tree at the
         * point specified by the input tag.  If the input tag is NULL then the
         * appender will be removed from locations in the tree.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aTag
         *         Specifies the logger tree tag where the appender should be removed.
         *         Can be NULL to remove at all locations.
         * @param aAppender
         *         The log appender to remove.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId RemoveLogAppender(const char* aTag, OsclSharedPtr<PVLoggerAppender>& aAppender, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows the logging level to be set for the logging node specified by the
         * tag.  A larger log level will result in more messages being logged.  A message
         * will only be logged if its level is LESS THAN or equal to the current log level.
         * The aSetSubtree flag will allow an entire subtree, with the specified tag as the root,
         * to be reset to the specified value.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aTag
         *         Specifies the logger tree tag where the log level should be set.
         * @param aLevel
         *         Specifies the log level to set.
         * @param aSetSubtree
         *         Specifies whether the entire subtree with aTag as the root should
         *         be reset to the log level.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree = false, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows the logging level to be queried for a particular logging tag.
         * A larger log level will result in more messages being logged.
         * In the asynchronous response, this should return the log level along with an
         * indication of where the level was inherited (i.e., the ancestor tag).
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aTag
         *         Specifies the logger tree tag where the log level should be retrieved.
         * @param aLogInfo
         *         An output parameter which will be filled in with the log level information.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @exception This method can leave with one of the following error codes
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId GetLogLevel(const char* aTag, PVLogLevelInfo& aLogInfo, const OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to allow for extensibility of the pvPlayer interface.
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
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command ID for asynchronous completion
         **/
        virtual PVCommandId QueryUUID(const PvmfMimeString& aMimeType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly = false, const OsclAny* aContextData = NULL) = 0;
        /**
         * This API is to allow for extensibility of the pvPlayer interface.
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
         * This API is to allow user of the SDK to cancel any specific command which is pending on pvPlayer.
         * If the request is to cancel a command which still has to be processed pvPlayer will just remove
         * the command from its queue of commands to be processed. If the request is to cancel a command that
         * is ongoing then player will attempt to interrupt the ongoing command. The state of player after a cancel
         * can vary. So the user of pvPlayerSDK must always query for state before issuing any subsequent
         * commands.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aCancelCmdId
         *          Command Id to be cancelled.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId CancelCommand(PVCommandId aCancelCmdId, const OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to allow the user to cancel all pending requests in pvPlayer. The current request being
         * processed, if any, will also be aborted. The user of PV-SDK should get the state of
         * PVPlayer Engine after the command completes and before issuing any other command.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId CancelAllCommands(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function returns the current state of pvPlayer.
         * Application may use this info for updating display or determine if the
         * pvPlayer is ready for the next request.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aState
         *         A reference to a PVPlayerState. Upon successful completion of this command,
         *         it will contain the current state of pvPlayer.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetPVPlayerState(PVPlayerState& aState, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function returns the current state of pvPlayer as a synchronous command.
         * Application may use this info for updating display or determine if the
         * pvPlayer is ready for the next request.
         *
         * @param aState
         *         A reference to a PVPlayerState. Upon successful completion of this command,
         *         it will contain the current state of pvPlayer.
         * @returns Status indicating whether the command succeeded or not.
         **/
        virtual PVMFStatus GetPVPlayerStateSync(PVPlayerState& aState) = 0;

        /**
         * This function allows a player data source to be specified for playback. This function must be called
         * when pvPlayer is in PVP_STATE_IDLE state and before calling Init. The specified data source must be a valid PVPlayerDataSource to
         * be accepted for use in playback.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aDataSource
         *          Reference to the player data source to be used for playback
         * @param aContextData
         *          Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *          OsclErrNotSupported if the format of the source is incompatible with what the SDK can handle
         *          OsclErrInvalidState if invoked in the incorrect state
         *          OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @return A unique command id for asynchronous completion
         */
        virtual PVCommandId AddDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function switches pvPlayer from PVP_STATE_IDLE state to the PVP_STATE_INITIALIZED state.
         * During the transition, pvPlayer is in the PVP_STATE_INITIALIZING transitional state and
         * the data source is being initialized to obtain metadata and track information of the source media.
         * If initialization fails, pvPlayer will revert to PVP_STATE_IDLE state and the data source
         * will be closed.
         * The Command should only be called in PVP_STATE_IDLE.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Init(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function makes a request to return the list of all or segment of available metadata keys in the current pvPlayer state.
         * The metadata key list is dynamic and can change during the course of pvPlayer usage.
         * The list can be used to retrieve the metadata values with GetMetadataValues function.
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
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetMetadataKeys(PVPMetadataList& aKeyList, int32 aStartingIndex = 0, int32 aMaxEntries = -1,
                                            char* aQueryKey = NULL, const OsclAny* aContextData = NULL) = 0;

        /**
         * The function makes a request to return the metadata value(s) specified by the passed in metadata key list.
         * If the requeted metadata value is unavailable or the metadata key is invalid, the returned list will not contain
         * a KVP entry for the key. Note that value indexed in the returned aValueList does not necessary match the same index into
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
         * @param aMetadataValuesCopiedInCallBack
         *	       Boolean to let engine know if metadata values are copied by User of SDK in command complete callback.
         *         By default the SDK assumes this to be the case. If this argument is set to false by the caller,
         *         then SDK assumes that user will call ReleaseMetaDataValues at a later point.
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetMetadataValues(PVPMetadataList& aKeyList, int32 aStartingValueIndex, int32 aMaxValueEntries, int32& aNumAvailableValueEntries,
                                              Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, const OsclAny* aContextData = NULL, bool aMetadataValuesCopiedInCallBack = true) = 0;

        /**
         * The function makes a request to release the metadata value(s) specified by the passed in metadata value list.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.If a GetMetaDataValues were called in
         * PVP_STATE_INITIALIZED state, then corresponding ReleaseMetaDataValues must be called before Reset.
         * If a GetMetaDataValues were called in PVP_STATE_PREPARED, PVP_STATE_STARTED, PVP_STATE_PAUSED states,
         * then corresponding ReleaseMetaDataValues must be called before Stop.
         *
         * @param aValueList
         *         Reference to a vector of KVP to place the specified metadata values
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId ReleaseMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function allows a player data sink to be specified for playback. This function must be called
         * when pvPlayer is in PVP_STATE_INITIALIZED state. The specified data sink must be a valid PVPlayerDataSink to
         * be accepted for use in playback.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aDataSink
         *         The player data sink to be used for playback.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrNotSupported if the format of the sink is incompatible with what the SDK can handle
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @return A unique command id for asynchronous completion
         **/
        virtual PVCommandId AddDataSink(PVPlayerDataSink& aDataSink, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function sets the begin and end positions for the new playback range or changes the end position of the
         * current playback range. This function must be called when pvPlayer is in PVP_STATE_INITIALIZED, PVP_STATE_PREPARED,
         * PVP_STATE_STARTED, or PVP_STATE_PAUSED state. The specified positions must be between beginning of clip and
         * clip duration. The units of position is specified in the passed-in parameter PVPPlaybackPosition.
         * If either of the positions is indeterminate, use the indeterminate flag in PVPPlaybackPosition structure.
         * The queued playback range can be done using aQueueRange flag which is Not Supported as of now by PV-SDK.
         * This function will overwrite any previous playback range info. The only exception is the changing of end position
         * for the current playback range during playback.
         * Command if called in player state as PVP_STATE_INITIALISED or PVP_STATE_PAUSED, will complete in one Engine AO run without actually
         * changing the position. The change in position will come into affect when Prepare or Resume respectively is called on Engine by the app.
         * If reposition request is not honored by the source node during Prepare or Resume, engine will continue to complete Prepare or Resume
         * but will send an informational event "PVMFInfoChangePlaybackPositionNotSupported" to the app informing that the SetPlaybackRange request
         * could not be honored.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aBeginPos
         *         Beginning position for the new playback range
         * @param aEndPos
         *         Ending position for the new playback range.
         * @param aQueueRange
         *         Input flag to tell pvPlayer to queue the new playback range (Set/true) or use the
         *         new playback range right away (Reset/false)
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetPlaybackRange(PVPPlaybackPosition aBeginPos, PVPPlaybackPosition aEndPos, bool aQueueRange, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function retrieves the playback range information for the current or queued playback range.
         * The user can choose which playback range by the aQueued flag. This function can be called when pvPlayer is in
         * PVP_STATE_INITIALIZED, PVP_STATE_PREPARED, PVP_STATE_STARTED, or PVP_STATE_PAUSED state.
         * The units of position is specified in the passed-in PVPlaybackPosition parameters which will be filled in when the command completes.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aBeginPos
         *         Reference to place the begin position for the playback range
         * @param aEndPos
         *         Reference to place the end position for the playback range
         * @param aQueued
         *         Input flag to choose inof of which playback range to return. Set(true)for queued range. Reset(false) for current range.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetPlaybackRange(PVPPlaybackPosition &aBeginPos, PVPPlaybackPosition &aEndPos, bool aQueued, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function allows querying of the current playback position. The playback position units
         * will be in the one specified by the passed-in reference to PVPPlaybackPosition. If no units
         * are specified, the units will default to milliseconds.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aPos
         *         Reference to place the current playback position
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetCurrentPosition(PVPPlaybackPosition &aPos, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function allows the setting of the playback rate. The playback rate can be set
         * as millipercent of "real-time" playback rate. For example, 100000 means 1X "real-time", 400000
         * means 4X, 25000 means 0.25X, and -100000 means 1X backward.
         * The playback rate can also be modified by specifying the timebase to use for the playback
         * clock. This is accomplished by  setting the aRate parameter to 0 and passing in a pointer
         * to an PVMFTimebase.
         * This function can be called when pvPlayer is in PVP_STATE_PREPARED, PVP_STATE_STARTED, or PVP_STATE_PAUSED state.
         * Changing to or from an outside timebase is only allowed in PVP_STATE_PREPARED.
         * Command if called in player state PVP_STATE_PAUSED with a direction change, will complete in one Engine AO run without actually
         * changing the direction. The change in direction will come into affect when Resume is called on Engine by the app. If the request
         * is not honored by the source node during Resume, engine will continue to complete Resume but will send an informational event
         * "PVMFInfoChangePlaybackPositionNotSupported" to the app informing that the SetPlaybackRate request could not be honored.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aRate
         *         The playback rate specified as millipercent of "real-time".
         *         A millipercent is 1/1000 of a percent. So 2X = 200% of realtime is
         *         200,000 millipercent. The motivation is to povide precision with an integer parameter.
         *         Negative rates specify backward playback.
         *         The valid range of absolute value of playback rates will be limited to the
         *         minimum and maximum returned by GetPlaybackMinMaxRate().
         * @param aTimebase
         *         Reference to an PVMFTimebase which will be used to drive the playback clock. aRate must be
         *         set to 0, 1X, or -1X to use the timebase.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrArgument if rate or timebase is invalid
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId SetPlaybackRate(int32 aRate, PVMFTimebase* aTimebase = NULL, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function retrieves the current playback rate setting. If the playback rate is set as a millipercent of "real-time"
         * playback rate, then aRate will be filled in with the milliperecent value when this command completes
         * successfully. If the playback rate is set by an outside timebase, aRate will be set to 0 and aTimebase pointer
         * will point to the PVMFTimebase being used when the command completes successfully.
         * This function can be called when pvPlayer is in
         * PVP_STATE_PREPARED, PVP_STATE_STARTED, or PVP_STATE_PAUSED state.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aRate
         *         A reference to an integer which will be filled in with the current playback rate expressed
         *         as millipercent of "real-time" playback rate. If an outside timebase is being used, aRate would
         *         be set to 0.
         * @param aTimebase
         *         Reference to an PVMFTimebase pointer which will be valid if an outside timebase is being used
         *         for the playback clock.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetPlaybackRate(int32& aRate, PVMFTimebase*& aTimebase, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function retrieves the minimum and maximum playback rate expressed as  a millipercent of "real-time"
         * playback rate.
         * This function can be called anytime between pvPlayer instantiation and destruction.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aMinRate
         *         A reference to an integer which will be filled in with the minimum playback rate allowed expressed
         *         as millipercent of "real-time" playback rate.
         * @param aMaxRate
         *         A reference to an integer which will be filled in with the maximum playback rate allowed expressed
         *         as millipercent of "real-time" playback rate.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId GetPlaybackMinMaxRate(int32& aMinRate, int32& aMaxRate, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function allows querying of the current playback position as a synchronous command. The playback position units
         * will be in the one specified by the passed-in reference to PVPPlaybackPosition. If no units
         * are specified, the units will default to milliseconds.
         *
         * @param aPos
         *         Reference to place the current playback position
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         * @returns Status indicating whether the command succeeded or not.
         **/
        virtual PVMFStatus GetCurrentPositionSync(PVPPlaybackPosition &aPos) = 0;

        /**
         * This functions prepares pvPlayer for playback. pvPlayer connects the data source with the data sinks and starts
         * the data source to queue the media data for playback(e.g. for 3GPP streaming, fills the jitter buffer).
         * pvPlayer also checks to make sure each component needed for
         * playback is ready and capable. When successful, pvPlayer will be in PVP_STATE_PREPARED state,
         * The command should be called only in PVP_STATE_INITIALISED.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Prepare(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function kicks off the actual playback. Media data are sent out from the data source to the data sink(s).
         * pvPlayer will transition to PVP_STATE_STARTED state after playback starts successfully.
         * The command should be called only in PVP_STATE_PREPARED.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Start(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function pauses the currently ongoing playback. pvPlayer must be in PVP_STATE_STARTED state
         * to call this function. When pause successfully completes, pvPlayer will be in PVP_STATE_PAUSED state.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Pause(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function resumes the currently paused playback. pvPlayer must be in
         * PVP_STATE_PAUSED state to call this function. When resume successfully completes,
         * pvPlayer will be in PVP_STATE_STARTED state.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Resume(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function stops the current playback and transitions pvPlayer to the PVP_STATE_INITIALIZED
         * state. During the transition, data transmission from data source to all data sinks are
         * terminated. Also all connections between data source and data sinks
         * are torn down.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Stop(const OsclAny* aContextData = NULL) = 0;

        /**
        * This function may be used to close and unbind a data sink that has been previously added.
        * This function must be called when pvPlayer is in PVP_STATE_INITIALIZED state. If the data sink
        * is in use for playback, Stop must be called first to stop the playback and free the data sink.
        * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
        * callback handler will be called when this command request completes.
        *
        * @param aDataSink
        *         Reference to the data sink to be removed
        * @param aContextData
        *         Optional opaque data that will be passed back to the user with the command response
        * @leave This method can leave with one of the following error codes
        *         OsclErrBadHandle if the passed in sink parameter is invalid
        *         OsclErrInvalidState if invoked in the incorrect state
        *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
        * @returns A unique command id for asynchronous completion
        **/
        virtual PVCommandId RemoveDataSink(PVPlayerDataSink& aDataSink, const OsclAny* aContextData = NULL) = 0;

        /**
         * This function cleans up resources used for playback to transition pvPlayer to PVP_STATE_IDLE state.
         * While processing this command, pvPlayer is in the PVP_STATE_RESETTING state.
         * If any data sinks are still referenced by pvPlayer when this function is called, the data sinks
         * will be closed and removed from pvPlayer during the Reset.
         * If already in PVP_STATE_IDLE state, then nothing will occur.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId Reset(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function may be used to close and unbind a data source that has been previously added.
         * This function must be called when pvPlayer is in PVP_STATE_IDLE state. If the data source
         * has already been initialized, Reset must be called first.
         * This command request is asynchronous. PVCommandStatusObserver's CommandCompleted()
         * callback handler will be called when this command request completes.
         *
         * @param aDataSource
         *         Reference to the data source to be removed.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave This method can leave with one of the following error codes
         *         OsclErrBadHandle if the passed in sink parameter is invalid
         *         OsclErrInvalidState if invoked in the incorrect state
         *         OsclErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        virtual PVCommandId RemoveDataSource(PVPlayerDataSource& aDataSource, const OsclAny* aContextData = NULL) = 0;

        /**
         * Returns SDK version information about pvPlayer.
         *
         * @param aSDKInfo
         *         A reference to a PVSDKInfo structure which contains product name, supported hardware platform,
         *         supported software platform, version, part number, and PV UUID. These fields will contain info
         *        .for the currently instantiated pvPlayer engine when this function returns success.
         *
         **/
        OSCL_IMPORT_REF static void GetSDKInfo(PVSDKInfo& aSDKInfo);

};




#endif // PV_PLAYER_INTERFACE_H_INCLUDED
