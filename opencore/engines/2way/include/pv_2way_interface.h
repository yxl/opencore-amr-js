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
*  Name        : pv_2way_interface.h
*  Part of     :
*  Interface   :
*  Description : Interface class and supporting definitions for the PV2Way SDK
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_2WAY_INTERFACE_H_INCLUDED
#define PV_2WAY_INTERFACE_H_INCLUDED


//  INCLUDES
#ifndef PV_COMMON_TYPES_H_INCLUDED
#include "pv_common_types.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef PVT_COMMON_H_INCLUDED
#include "pvt_common.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVLOGGER_ACCESSORIES_H_INCLUDED
#include "pvlogger_accessories.h"
#endif

#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

#ifndef PVT_2WAY_BASIC_TYPES_H_INCLUDED
#include "pv_2way_basic_types.h"
#endif

#ifndef PV_2WAY_H324M_TYPES_H_INCLUDED
#include "pv_2way_h324m_types.h"
#endif


// CONSTANTS

// DATA TYPES

// CLASS DECLARATION

/**
 * CPV2WayInterface Class
 *
 * CPV2WayInterface is the interface to the pv2way SDK, which
 * allows initialization, control, and termination of a two-way (3g-324m, SIP) terminal.
 * The application is expected to contain and maintain a pointer to the
 * CPV2WayInterface instance at all times that a call is active.
 * The CPV2WayFactory factory class is to be used to create and
 * delete instances of this class
 **/
class CPV2WayInterface
{
    public:
        /**
         * Object destructor function
         * Releases Resources prior to destruction
         **/
        virtual ~CPV2WayInterface() {};

        /**
         * Returns version information about the SDK
         *
         * @param aSDKInfo
         *         A reference to a PVSDKInfo structure which contains the product label and date
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *          PVMFErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId GetSDKInfo(PVSDKInfo &aSDKInfo,
                OsclAny* aContextData = NULL) = 0;

        /**
         * Returns information about all modules currently used by the SDK.
         *
         * @param aSDKModuleInfo
         *         A reference to a PVSDKModuleInfo structure which contains the number of modules currently used by
         *         pv2way SDK and the PV UID and description string for each module. The PV UID and description string
         *         for modules will be returned in one string buffer allocated by the client. If the string buffer
         *         is not large enough to hold the all the module's information, the information will be written
         *         up to the length of the buffer and truncated.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *          PVMFErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId GetSDKModuleInfo(PVSDKModuleInfo &aSDKModuleInfo,
                OsclAny* aContextData = NULL) = 0;


        /**
         * This function is valid only in the EIdle state.  It is a no-op when
         * invoked in any other state.  It causes the pv2way to transition
         * to the ESetup state.  The terminal remains in the EInitializing state during
         * the transition.
         *
         * While initializing, the pv2way tries to allocate system resources needed
         * for a two-way call.  If it fails for some reason,
         * and the pv2way reverts to the EIdle state.  All the resources are de-allocated.
         *
         *
         * @param aInitInfo
         *         A reference to a CPV2WayInitInfo structure which contains the capabilities of the applications sinks
         *          and sources to handle compressed and uncompressed formats.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *			PVMFErrArgument if more tx and rx codecs are set than engine can handle, or the mandatory codecs are not in the list.
         *          PVMFErrNotSupported if the format of the sources/sinks is incomtible with what the SDK can handle
         *          PVMFErrInvalidState if invoked in the incorrect state
         *          PVMFErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId Init(PV2WayInitInfo& aInitInfo,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This function is valid only in the ESetup and EInitializing state.  It is a
         * no-op when invoked in the EIdle state and returns PVMFErrInvalidState
         * if invoked in any other state.
         *
         * It causes the pv2way to transition back to the EIdle state.  The
         * terminal remains in the EResetting state during the transition.
         *
         * While resetting, the pv2way de-allocates all resources resources that
         * had been previously allocated.  When it completes, ResetComplete is called
         * and the pv2way reverts to the EIdle state.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *          PVMFErrInvalidState if invoked in the incorrect state
         *          PVMFErrNoMemory if the SDK failed to allocate memory during this operation
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId Reset(OsclAny* aContextData = NULL) = 0;

        /**
         * This function allows the user to specify the media source for an outgoing track.
         * Sources should be added after the PVT_INDICATION_OUTGOING_TRACK is received which specifies the format type
         * and the unique track id.  The format type is indicated using the PV2WayTrackInfoInterface extension interface in the
         * PVAsyncInformationalEvent.
         * Data sources could be of the following types:
         * a)raw media sources like camera, microphone etc.
         * b)sources of compressed data like file, combined capture and encode devices.
         *
         	 * @param aTrackId
         *          The outgoing track id
         * @param aDataSource
         *          Reference to the data source for this track
         * @param aContextData
         *          Optional opaque data that will be passed back to the user with the command response
         * @leave	This method can leave with one of the following error codes
         *          PVMFErrNotSupported if the format of the sources/sinks is incomtible with what the SDK can handle
         *          KPVErrInvalidState if invoked in the incorrect state
         *          KErrNoMemory if the SDK failed to allocate memory during this operation
         * @return A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVCommandId AddDataSource(PVTrackId aTrackId,
                PVMFNodeInterface& aDataSource,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This function unbinds a previously added source.
         *
         * @param aDataSource pointer to the media source node
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId RemoveDataSource(PVMFNodeInterface& aDataSource,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This function allows the user to specify the media sink for an incoming track.
         * AddDataSinkL can be called only for established incoming tracks identified by a unique
         * track id.
         * Incoming tracks are initiated by the peer and their establishment is indicated using the
         * PVT_INDICATION_INCOMING_TRACK notification which provides the media type and a unique track id.
         * The format type is indicated using the PV2WayTrackInfoInterface extension interface in the
         * PVAsyncInformationalEvent.
         * Data sinks could be of the following types:
         * a)raw media sinks like video display sinks for RGB and YUV formats, audio rendering sinks for PCM.
         * b)sources of compressed data like file, combined decode and render devices.
         *
         *
         * @param aTrackId
         *          Indicates the unique track id to be associated with this sink.
         * @param aDataSink The data sink to be added
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @return A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId AddDataSink(PVTrackId aTrackId,
                PVMFNodeInterface& aDataSink,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This function unbinds a previously added sink.
         *
         * @param aDataSink pointer to the media sink node
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId RemoveDataSink(PVMFNodeInterface& aDataSink,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This function can be invoked only in the ESetup state.  The terminal starts connecting with the remote
         * terminal based on the specified options and capabilities.
         * Incoming tracks may be opened before ConnectL completes and will be indicated via the
         * PVT_INDICATION_INCOMING_TRACK event.
         *
         * @param aOptions
         *         Optional additional information for call setup.
         * @param aCommServer
         *		   An optional pointer to a comm server to provide comm source and sink end-points.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId Connect(const PV2WayConnectOptions& aOptions,
                PVMFNodeInterface* aCommServer = NULL,
                OsclAny* aContextData = NULL) = 0;

        /**
         * The Disconnect call is valid only when invoked in the EConnecting, and
         * EConnected states.  It causes the terminal to transition to the
         * EDisconnecting state.  All the media tracks both incoming and outgoing
         * will be closed on invoking Disconnect. On completion, the terminal
         * goes to the ESetup state. The statistics of the previous call shall
         * still be available until Connect is invoked again.
         *
         * It is a no-op when called in any other state.
         *
         * The post disconnect option specifies what this terminal wishes to do
         * after the data call is terminated, whether it wants to disconnect the line
         * or continue the call as a voice only call.
         *
         * This is an asynchronous request.
         *
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId Disconnect(OsclAny* aContextData = NULL) = 0;

        /**
         * This function returns the current state of the pv2way.
         * Application may use this info for updating display or determine if the
         * pv2way is ready for the next request.
         *
         * @param aState
         *         Reflects the state of the PV 2Way engine when the command was received.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns value indicating the current pv2way state
         **/
        OSCL_IMPORT_REF virtual PVCommandId GetState(PV2WayState& aState,
                OsclAny* aContextData = NULL) = 0;

        /**
         * For an incoming track this function  pauses sending media to the sink (output device) and stops the sink.
         *
         * For outgoing, it pauses the sending of media from the source and stops the source.
         *
         * @param aDirection Specifies the direction of the track - incoming or outgoing
         * @param aTrackId Specifies which track is to be paused.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId Pause(PV2WayDirection aDirection,
                PVTrackId aTrackId,
                OsclAny* aContextData = NULL) = 0;

        /**
         * Resume a previously paused incoming or outgoing track.  For incoming,
         * this function starts resumes playing out the media to the appropriate
         * sink based on the current settings.  For outgoing it resumes encoding
         * and sending media from the source.
         *
         * @param aDirection Specifies the direction of the track - incoming or outgoing
         * @param aTrackId Specifies which track is to be paused.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId Resume(PV2WayDirection aDirection,
                PVTrackId aTrackId,
                OsclAny* aContextData = NULL) = 0;


        /**
         * Allows a logging appender to be attached at some point in the
         * logger tag tree.  The location in the tag tree is specified by the
         * input tag string.  A single appender can be attached multiple times in
         * the tree, but it may result in duplicate copies of log messages if the
         * appender is not attached in disjoint portions of the tree.
         * A logging appender is responsible for actually writing the log message
         * to its final location (e.g., memory, file, network, etc).
         * This API can be called anytime after creation of the terminal.
         *
         * @param aTag Specifies the logger tree tag where the appender should be attached.
         * @param aAppender The log appender to attach.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId SetLogAppender(const char* aTag,
                OsclSharedPtr<PVLoggerAppender>& aAppender,
                OsclAny* aContextData = NULL) = 0;


        /**
         * Allows a logging appender to be removed from the logger tree at the
         * point specified by the input tag.  The input tag cannot be NULL.
         *
         * @param aTag Specifies the logger tree tag where the appender should be removed.
         * @param aAppender The log appender to remove.  Must be a reference to the same object that was set.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId RemoveLogAppender(const char* aTag,
                OsclSharedPtr<PVLoggerAppender>& aAppender,
                OsclAny* aContextData = NULL) = 0;


        /**
         * Allows the logging level to be set for the logging node specified by the
         * tag.  A larger log level will result in more messages being logged.  A message
         * will only be logged if its level is LESS THAN or equal to the current log level.
         * The set_subtree flag will allow an entire subtree, with the specified tag as the root,
         * to be reset to the specified value.
         *
         * @param aTag Specifies the logger tree tag where the log level should be set.
         * @param aLevel Specifies the log level to set.
         * @param aSetSubtree Specifies whether the entire subtree with aTag as the root should
         *                      be reset to the log level.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId SetLogLevel(const char* aTag,
                int32 aLevel,
                bool aSetSubtree = false,
                OsclAny* aContextData = NULL) = 0;


        /**
         * Allows the logging level to be queried for a particular logging tag.
         * A larger log level will result in more messages being logged.
         *
         * In the asynchronous response, this should return the log level along with an
         * indication of where the level was inherited (i.e., the ancestor tag).
         *
         * @param aTag Specifies the logger tree tag where the log level should be retrieved.
         * @param aLogInfo an output parameter which will be filled in with the log level information.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId GetLogLevel(const char* aTag,
                int32& aLogInfo,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to allow for extensibility of the pv2way interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the system, they are added
         * to the UUIDs array.
         *
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type.  This functionality can be turned off.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids An array to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         **/
        OSCL_IMPORT_REF virtual PVCommandId QueryUUID(const PvmfMimeString& aMimeType,
                Oscl_Vector<PVUuid, BasicAlloc>& aUuids,
                bool aExactUuidsOnly = false,
                OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to allow for extensibility of the pv2way interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         * TBD:  Define the UIID, InterfacePtr structures
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         * @exception not_supported
         *                   leaves if the specified interface id is not supported.
         **/
        OSCL_IMPORT_REF virtual PVCommandId QueryInterface(const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                OsclAny* aContext = NULL) = 0;

        /**
         * This API is to allow the user to cancel all pending requests.  The current request being
         * processed, if any, will also be aborted.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         **/
        OSCL_IMPORT_REF virtual PVCommandId CancelAllCommands(OsclAny* aContextData = NULL) = 0;
};



#endif //


