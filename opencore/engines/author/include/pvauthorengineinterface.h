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
#ifndef PVAUTHORENGINEINTERFACE_H_INCLUDED
#define PVAUTHORENGINEINTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

// Forward declarations
class PVMFNodeInterface;
class PVLoggerAppender;
template<class T, class Alloc> class Oscl_Vector;

/**
 * An enumeration of the major states of the pvAuthor Engine.
 */
enum PVAEState
{
    PVAE_STATE_IDLE = 0,
    PVAE_STATE_OPENED,
    PVAE_STATE_INITIALIZED,
    PVAE_STATE_RECORDING,
    PVAE_STATE_PAUSED,
    PVAE_STATE_ERROR
};

/**
 * Enumeration of errors from pvAuthor Engine.
 */
enum PVAEErrorEvent
{
    // TBD
    PVAE_ENCODE_ERROR
};

/**
 * Enumeration of informational events from pvAuthor Engine.
 */
enum PVAEInfoEvent
{
    // TBD
    PVAE_OUTPUT_PROGRESS
};


/**
 * PVAuthorEngineInterface
 **/
class PVAuthorEngineInterface
{
    public:
        /**
         * Destructor.
         */
        virtual ~PVAuthorEngineInterface() {};

        /**
         * Allows a logging appender to be attached at some point in the
         * logger tag tree.  The location in the tag tree is specified by the
         * input tag string.  A single appender can be attached multiple times in
         * the tree, but it may result in duplicate copies of log messages if the
         * appender is not attached in disjoint portions of the tree.
         * A logging appender is responsible for actually writing the log message
         * to its final location (e.g., memory, file, network, etc).
         * Currently this API is NOT SUPPORTED.
         *
         * @param aTag Specifies the logger tree tag where the appender should be attached.
         * @param aAppender The log appender to attach.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId SetLogAppender(const char* aTag, PVLoggerAppender& aAppender, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows a logging appender to be removed from the logger tree at the
         * point specified by the input tag.  If the input tag is NULL then the
         * appender will be removed from locations in the tree.
         * Currently this API is NOT SUPPORTED.
         *
         * @param aTag Specifies the logger tree tag where the appender should be removed.
         *             Can be NULL to remove at all locations.
         * @param aAppender The log appender to remove.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         *
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId RemoveLogAppender(const char* aTag, PVLoggerAppender& aAppender, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows the logging level to be set for the logging node specified by the
         * tag.  A larger log level will result in more messages being logged.  A message
         * will only be logged if its level is LESS THAN or equal to the current log level.
         * The set_subtree flag will allow an entire subtree, with the specified tag as the root,
         * to be reset to the specified value.
         * Currently this API is NOT SUPPORTED.
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
         */
        virtual PVCommandId SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree = false, const OsclAny* aContextData = NULL) = 0;

        /**
         * Allows the logging level to be queried for a particular logging tag.
         * A larger log level will result in more messages being logged.
         *
         * In the asynchronous response, this should return the log level along with an
         * indication of where the level was inherited (i.e., the ancestor tag).
         * Currently this API is NOT SUPPORTED.
         *
         * @param aTag Specifies the logger tree tag where the log level should be retrieved.
         * @param aLogInfo An output parameter which will be filled in with the log level information.
         * @param aContextData
         *         Optional opaque data that will be passed back to the user with the command response
         * @exception memory_error leaves on memory allocation error.
         *
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId GetLogLevel(const char* aTag, PVLogLevelInfo& aLogInfo, const OsclAny* aContextData = NULL) = 0;

        /**
         * Opens an authoring session.
         *
         * This command is valid only when pvAuthor engine is in PVAE_STATE_IDLE state. Upon completion
         * of this method, pvAuthor engine will be in PVAE_STATE_OPENED state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @return Unique command ID to identify this command in command response
         */
        virtual PVCommandId Open(const OsclAny* aContextData = NULL) = 0;

        /**
         * Closes an authoring session.
         *
         * All resources added and allocated to the authoring session will be released.
         *
         * This command is valid only when pvAuthor engine is in PVAE_STATE_OPENED state and Upon
         * completion of this command, pvAuthor Engine will be in PVAE_STATE_IDLE state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @return Unique command ID to identify this command in command response
         */
        virtual PVCommandId Close(const OsclAny* aContextData = NULL) = 0;

        /**
         * Adds a media source to be used as input to an authoring session.
         *
         * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state.
         * This command does not change the pvAuthor Engine engine state.
         *
         * @param aDataSource Reference to the data source
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @return Unique command ID to identify this command in command response
         */
        virtual PVCommandId AddDataSource(const PVMFNodeInterface& aDataSource, const OsclAny* aContextData = NULL) = 0;

        /**
         * Unbinds a previously added data source.
         *
          * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state.
         * This command does not change the pvAuthor Engine engine state.
         *
         * @param aDataSource Reference to the data source to be removed
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId RemoveDataSource(const PVMFNodeInterface& aDataSource, const OsclAny* aContextData = NULL) = 0;

        /**
         * Selects an output composer by specifying its MIME type.
         *
         * pvAuthor engine will use the most suitable output composer of the specified MIME type available
         * in the authoring session. This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED
         * state. This command does not change the pvAuthor Engine state.
         *
         * Upon completion of this command, opaque data to indentify the selected composer is provided in the
         * callback.  The user needs to use this opaque data to identify the composer when calling AddMediaTrack(),
         * AddDataSink().  A configuration interface for the selected composer will be
         * saved to the PVInterface pointer provided in aConfigInterface parameter.  User should call
         * queryInterface to query for the configuration interfaces supported by the composer. When
         * configuration is complete or before calling Reset(), user must call
         * removeRef on the PVInterface object to remove its reference to the object.
         *
         * @param aComposerType MIME type of output composer to be used
         * @param aConfigInterface Pointer to configuration object for the selected composer will be
         * saved to this parameter upon completion of this call
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId SelectComposer(const PvmfMimeString& aComposerType,
                                           PVInterface*& aConfigInterface,
                                           const OsclAny* aContextData = NULL) = 0;

        /**
         * Selects an output composer by specifying its Uuid.
         *
         * pvAuthor engine the composer of the specified Uuid in the authoring session.
         * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state. This command does
         * not change the pvAuthor Engine state.
         *
         * Upon completion of this command, opaque data to indentify the selected composer is provided in the
         * callback.  The user needs to use this opaque data to identify the composer when calling AddMediaTrack(),
         * AddDataSink().  A configuration interface for the selected composer will be
         * saved to the PVInterface pointer provided in aConfigInterface parameter.  User should call
         * queryInterface to query for the configuration interfaces supported by the composer. When
         * configuration is complete or before calling Reset(), user must call
         * removeRef on the PVInterface object to remove its reference to the object.
         *
         * @param aComposerUuid Uuid of output composer to be used
         * @param aConfigInterface Pointer to configuration object for the selected composer will be
         * saved to this parameter upon completion of this call
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId SelectComposer(const PVUuid& aComposerUuid,
                                           PVInterface*& aConfigInterface,
                                           const OsclAny* aContextData = NULL) = 0;

        /**
         * Add a media track to the specified composer.
         *
         * The source data of this media track will come from the specified data source. pvAuthor engine
         * will select the most suitable available encoder of the specified type. A media track will be
         * added to the specified composer, and encoded data will be written to the composer during the
         * authoring session.
         *
         * A configuration object for the selected composer will be saved to the PVInterface pointer
         * provided in aConfigInterface parameter.  User should call queryInterface to query for the
         * configuration interfaces supported by the encoder. Before calling Reset(), user must call
         * removeRef on the PVInterface object to remove its reference to the object.
         *
         * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state. The referenced
         * data source and composer must be already added before this method is called.
         * This command does not change the pvAuthor Engine engine state.
         *
         * @param aDataSource Data source node to provide input data
         * @param aEncoderType MIME type of encoder to encode the source data
         * @param aComposer Opaque data to identify the composer in which a media track will be added.
         * @param aConfigInterface Pointer to configuration object for the selected encoder will be
         * saved to this parameter upon completion of this call
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId AddMediaTrack(const PVMFNodeInterface& aDataSource,
                                          const PvmfMimeString& aEncoderType,
                                          const OsclAny* aComposer,
                                          PVInterface*& aConfigInterface,
                                          const OsclAny* aContextData = NULL) = 0;

        /**
         * Add a media track to the specified composer.
         *
         * The source data of this media track will come from the specified data source. pvAuthor engine
         * will encoder of the specified Uuid to encode the source data. A media track will be
         * added to the specified composer, and encoded data will be written to the composer during the
         * authoring session.
         *
         * A configuration object for the selected composer will be saved to the PVInterface pointer
         * provided in aConfigInterface parameter.  User should call queryInterface to query for the
         * configuration interfaces supported by the encoder. Before calling Reset(), user must call
         * removeRef on the PVInterface object to remove its reference to the object.
         *
         * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state. The referenced
         * data source and composer must be already added before this method is called.
         * This command does not change the pvAuthor Engine engine state.
         *
         * @param aDataSource Data source node to provide input data
         * @param aEncoderUuid Uuid of encoder to encode the source data
         * @param aComposer Opaque data to identify the composer in which a media track will be added.
         * @param aConfigInterface Pointer to configuration object for the selected encoder will be
         * saved to this parameter upon completion of this call
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId AddMediaTrack(const PVMFNodeInterface& aDataSource,
                                          const PVUuid& aEncoderUuid,
                                          const OsclAny* aComposer,
                                          PVInterface*& aConfigInterface,
                                          const OsclAny* aContextData = NULL) = 0;

        /**
         * Adds a media sink where output data from the specified composer will be written to.
         * Currently this API does not cause any action as it is not relevant.
         *
         * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state. The
         * referenced composer must be previously selected.
         *
         * This command does not change the pvAuthor Engine engine state.
         *
         * @param aDataSink Reference to the data sink to be used
         * @param aComposer Opaque data identifying the composer to which the data sink will connect to.
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId AddDataSink(const PVMFNodeInterface& aDataSink,
                                        const OsclAny* aComposer,
                                        const OsclAny* aContextData = NULL) = 0;

        /**
         * Removes a previously added data sink.
         * Currently this API does not cause any action as it is not relevant.

         * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state.
         * This command does not change the pvAuthor Engine engine state.
         *
         * @param aDataSink Reference to the data sink to be removed
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId RemoveDataSink(const PVMFNodeInterface& aDataSink, const OsclAny* aContextData = NULL) = 0;


        /**
         * Initialize an authoring session.
         *
         * Upon calling this method, no more data sources and sinks can be added to the
         * session.  Also, all configuration settings will be locked and cannot
         * be modified until the session is reset by calling Reset().  Resources for the
         * session will allocated and initialized to the configuration settings specified.
          * This command is valid only when pvAuthor Engine is in PVAE_STATE_OPENED state.
         *
         * Upon completion of this command, pvAuthor Engine will be in PVAE_STATE_INITIALIZED
         * state, and the authoring session is ready to start.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId Init(const OsclAny* aContextData = NULL) = 0;

        /**
         * Reset an initialized authoring session.
         *
         * The authoring session will be stopped and all composers and encoders selected for
         * the session will be removed.  All data sources and sinks will be reset but will
         * continue to be available for authoring the next output clip.
         *
         * User must call removeRef() to remove its reference to any PVInterface objects
         * received from SelectComposer() or AddMediaTrack() or QueryInterface() APIs
         * before calling this method.  This method would fail otherwise.
         *
         * This method can be called from ANY state but PVAE_STATE_IDLE. Upon completion of this command, pvAuthor
         * Engine will be in PVAE_STATE_OPENED state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId Reset(const OsclAny* aContextData = NULL) = 0;

        /**
         * Start the authoring session.
         *
         * pvAuthor Engine will begin to receive source data, encode them to the specified format
         * and quality, and send the output data to the specified data sinks.
         * This function is valid only in the PVAE_STATE_INITIALIZED state.
         *
         * Upon completion of this command, pvAuthor Engine will be in PVAE_STATE_RECORDING state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId Start(const OsclAny* aContextData = NULL) = 0;

        /**
         * Pause the authoring session.
         *
         * The authoring session will be paused and no encoded output data will be sent to
         * the data sink. This function is valid only in the PVAE_STATE_RECORDING state.
         *
         * Upon completion of this command, pvAuthor Engine will be in PVAE_STATE_PAUSED state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId Pause(const OsclAny* aContextData = NULL) = 0;

        /**
         * Resume a paused authoring session.
         *
         * The authoring session will be resumed and pvAuthor Engine will resume sending encoded
         * output data to the data sinks. This function is valid only in the PVAE_STATE_PAUSED state.
         *
         * Upon completion of this command, pvAuthor Engine will be in PVAE_STATE_RECORDING state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId Resume(const OsclAny* aContextData = NULL) = 0;

        /**
         * Stops an authoring session.
         *
         * The authoring session will be stopped and pvAuthor Engine will stop receiving source data
         * from the data sources, and no further encoded data will be sent to the data sinks.
         * This function is valid only in the PVAE_STATE_RECORDING and PVAE_STATE_PAUSED states.
         *
         * Upon completion of this command, pvAuthor Engine will be in PVAE_STATE_INITIALIZED state.
         *
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId Stop(const OsclAny* aContextData = NULL) = 0;

        /**
         * This function returns the current state of the pvAuthor Engine.
         * Application may use this info for updating display or determine if the pvAuthor Engine is
         * ready for the next command.
         *
         * @param aState Output parameter to hold state information
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for synchronous completion
         */
        virtual PVAEState GetPVAuthorState() = 0;

        /**
         * Discover the UUIDs of interfaces associated with the specified MIME type and node
         *
         * This API is to allow for extensibility of the pvAuthor Engine interface. User can query for
         * all UUIDs associated with a particular MIME type. The UUIDs will be added to the aUuids
         * vector provided by the user.
         * Currently this API is NOT SUPPORTED.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids A vector to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId QueryUUID(const PvmfMimeString& aMimeType,
                                      Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                      bool aExactUuidsOnly = false,
                                      const OsclAny* aContextData = NULL) = 0;

        /**
         * This API is to allow for extensibility of the pvAuthor engine interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         * TBD:  Define the UIID, InterfacePtr structures
         *
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         * @param aContextData Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId QueryInterface(const PVUuid& aUuid,
                                           PVInterface*& aInterfacePtr,
                                           const OsclAny* aContextData = NULL) = 0;

        /**
         * Returns information about all modules currently used by the SDK.
         * Currently this API is NOT SUPPORTED.
         *
         * @param aSDKModuleInfo A reference to a PVSDKModuleInfo structure which contains
         * the number of modules currently used by pvAuthor Engine and the PV UID and description
         * string for each module. The PV UID and description string for modules will be
         * returned in one string buffer allocated by the client. If the string buffer is not
         * large enough to hold the all the module's information, the information will be written
         * up to the length of the buffer and truncated.
         * @param aContextData Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId GetSDKModuleInfo(PVSDKModuleInfo& aSDKModuleInfo, const OsclAny* aContextData = NULL) = 0;

        /**
         * Cancel all pending requests. The current request being processed, if any, will also be aborted.
         * PVAE_CMD_CANCEL_ALL_COMMANDS will be passed to the command observer on completion.
         * Currently this API is NOT SUPPORTED.
         *
         * @param aContextData Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         */
        virtual PVCommandId CancelAllCommands(const OsclAny* aContextData = NULL) = 0;

        /**
         * Returns SDK version information about author engine.
         *
         * @param aSDKInfo
         *         A reference to a PVSDKInfo structure which contains product name, supported hardware platform,
         *         supported software platform, version, part number, and PV UUID. These fields will contain info
         *        .for the currently instantiated pvPlayer engine when this function returns success.
         *
         **/
        OSCL_IMPORT_REF static void GetSDKInfo(PVSDKInfo& aSDKInfo);
};

#endif // PVAUTHORENGINE_H_INCLUDED


