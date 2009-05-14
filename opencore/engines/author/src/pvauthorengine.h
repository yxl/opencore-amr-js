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
#ifndef PVAUTHORENGINE_H_INCLUDED
#define PVAUTHORENGINE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif
#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif
#ifndef PVAUTHORENGINEINTERFACE_H_INCLUDED
#include "pvauthorengineinterface.h"
#endif
#ifndef PVAE_NODE_UTILITY_H_INCLUDED
#include "pvaenodeutility.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#define K3gpComposerMimeType "/x-pvmf/ff-mux/3gp"
#define KAmrNbEncMimeType "/x-pvmf/audio/encode/amr-nb"
#define KAMRWbEncMimeType "/x-pvmf/audio/encode/amr-wb"
#define KH263EncMimeType "/x-pvmf/video/encode/h263"
#define KH264EncMimeType "/x-pvmf/video/encode/h264"
#define KMp4EncMimeType "/x-pvmf/video/encode/mp4"
#define KTextEncMimeType "/x-pvmf/text/encode/txt"

//MIME strings for .amr and .aac composer
#define KAMRNbComposerMimeType		"/x-pvmf/ff-mux/amr-nb"
#define KAMRWbComposerMimeType		"/x-pvmf/ff-mux/amr-wb"
#define KAACADIFComposerMimeType	"/x-pvmf/ff-mux/adif"
#define KAACADTSComposerMimeType	"/x-pvmf/ff-mux/adts"
#define KAACADIFEncMimeType			"/x-pvmf/audio/encode/aac/adif"
#define KAACADTSEncMimeType			"/x-pvmf/audio/encode/aac/adts"
#define KAACMP4EncMimeType          "/x-pvmf/audio/encode/X-MPEG4-AUDIO"
//end of changes

/**
 * Enumeration of types of asychronous commands that can be issued to PV Author Engine
 */
typedef enum
{
    PVAE_CMD_SET_LOG_APPENDER,
    PVAE_CMD_REMOVE_LOG_APPENDER,
    PVAE_CMD_SET_LOG_LEVEL,
    PVAE_CMD_GET_LOG_LEVEL,
    PVAE_CMD_OPEN,
    PVAE_CMD_CLOSE,
    PVAE_CMD_ADD_DATA_SOURCE,
    PVAE_CMD_REMOVE_DATA_SOURCE,
    PVAE_CMD_SELECT_COMPOSER,
    PVAE_CMD_ADD_MEDIA_TRACK,
    PVAE_CMD_ADD_DATA_SINK,
    PVAE_CMD_REMOVE_DATA_SINK,
    PVAE_CMD_INIT,
    PVAE_CMD_RESET,
    PVAE_CMD_START,
    PVAE_CMD_PAUSE,
    PVAE_CMD_RESUME,
    PVAE_CMD_STOP,
    PVAE_CMD_QUERY_UUID,
    PVAE_CMD_QUERY_INTERFACE,
    PVAE_CMD_GET_SDK_INFO,
    PVAE_CMD_GET_SDK_MODULE_INFO,
    PVAE_CMD_CANCEL_ALL_COMMANDS,
    PVAE_CMD_STOP_MAX_SIZE,
    PVAE_CMD_STOP_MAX_DURATION,
    PVAE_CMD_STOP_EOS_REACHED,
    PVAE_CMD_CAPCONFIG_SET_PARAMETERS
} PVAECmdType;

// Structure to hold the key string info for
// player engine's capability-and-config
struct PVAuthorKeyStringData
{
    char iString[64];
    PvmiKvpType iType;
    PvmiKvpValueType iValueType;
};

// The number of characters to allocate for the key string
#define PVAUTHORCONFIG_KEYSTRING_SIZE 128

// Key string info at the base level ("x-pvmf/author/")
#define PVAUTHORCONFIG_BASE_NUMKEYS 3
const PVAuthorKeyStringData PVAuthorConfigBaseKeys[PVAUTHORCONFIG_BASE_NUMKEYS] =
{
    //These are dummy keys, to be changed in future
    {"dummy1", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_INT32},
    {"dummy2", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_INT32},
    //It is necessary to have this key for the PVAuthorConfigProdInfoKeys to work
    //Do not change the position of this key in the array
    {"productinfo", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_INT32}
    //add more keys here
};

// Key string info at the productinfo level ("x-pvmf/author/productinfo/")
#define PVAUTHORCONFIG_PRODINFO_NUMKEYS 2
const PVAuthorKeyStringData PVAuthorConfigProdInfoKeys[PVAUTHORCONFIG_PRODINFO_NUMKEYS] =
{
    {"dummyprod1", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_INT32},
    {"dummyprod2", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_INT32}
};


/**
 * PVAuthorEngine Class
 */
class PVAuthorEngine : public PVAuthorEngineInterface,
            public OsclTimerObject,
            public PVMFNodeErrorEventObserver,
            public PVMFNodeInfoEventObserver,
            public PVAENodeUtilObserver,
            public PvmiCapabilityAndConfig
{
    public:
        static PVAuthorEngineInterface* Create(PVCommandStatusObserver* aCmdStatusObserver,
                                               PVErrorEventObserver *aErrorEventObserver, PVInformationalEventObserver *aInfoEventObserver);
        ~PVAuthorEngine();

        /////////////////////////////////////////////////////////
        // Pure virtuals from PVAuthorEngineInterface
        /////////////////////////////////////////////////////////
        OSCL_IMPORT_REF PVCommandId SetLogAppender(const char* aTag, PVLoggerAppender& aAppender,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId RemoveLogAppender(const char* aTag, PVLoggerAppender& aAppender,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId SetLogLevel(const char* aTag, int32 aLevel, bool aSetSubtree = false,
                                                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId GetLogLevel(const char* aTag, PVLogLevelInfo& aLogInfo, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Open(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Close(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId AddDataSource(const PVMFNodeInterface& aDataSource, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId RemoveDataSource(const PVMFNodeInterface& aDataSource, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId SelectComposer(const PvmfMimeString& aComposerType, PVInterface*& aConfigInterface,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId SelectComposer(const PVUuid& aComposerUuid, PVInterface*& aConfigInterface,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId AddMediaTrack(const PVMFNodeInterface& aDataSource, const PvmfMimeString& aEncoderType,
                const OsclAny* aComposer, PVInterface*& aConfigInterface,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId AddMediaTrack(const PVMFNodeInterface& aDataSource, const PVUuid& aEncoderUuid,
                const OsclAny* aComposer, PVInterface*& aConfigInterface,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId AddDataSink(const PVMFNodeInterface& aDataSink, const OsclAny* aComposer,
                                                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId RemoveDataSink(const PVMFNodeInterface& aDataSink, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Init(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Reset(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Start(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Pause(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Resume(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId Stop(const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId QueryUUID(const PvmfMimeString& aMimeType,
                                              Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                              bool aExactUuidsOnly = false,
                                              const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId QueryInterface(const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVAEState GetPVAuthorState();
        OSCL_IMPORT_REF PVCommandId GetSDKInfo(PVSDKInfo& aSDKInfo, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId GetSDKModuleInfo(PVSDKModuleInfo& aSDKModuleInfo, const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVCommandId CancelAllCommands(const OsclAny* aContextData = NULL);

        // Implement pure virtual from PVMFNodeErrorEventObserver
        void HandleNodeErrorEvent(const PVMFAsyncEvent& aEvent);

        // Implement pure virtual from PVMFNodeInfoEventObserver
        void HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent);

        // Implement pure virtuals from PVAENodeUtilObserver
        void NodeUtilCommandCompleted(const PVMFCmdResp& aResponse);
        void NodeUtilErrorEvent(const PVMFAsyncEvent& aEvent);
        // capability and configuration class implemented here
        // virtual functions of PvmiCapabilityAndConfig class implemented here

        void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        PVMFStatus getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        PVMFStatus releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);
        void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext, PvmiKvp* aParameters, int aNumParamElements);
        void DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP);
        PVMFCommandId setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements, PvmiKvp*& aRetKVP, OsclAny* aContext = NULL);
        uint32 getCapabilityMetric(PvmiMIOSession aSession);
        PVMFStatus verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int aNumElements);
        // functions used by cap-config class virtual functions
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);
        PVMFStatus DoQueryNodeCapConfig(char* aKeySubString, Oscl_Vector<PVInterface*, OsclMemAllocator>& aNodeCapConfigIF);
        void addRef();
        void removeRef();
    private:
        PVAuthorEngine();
        void Construct(PVCommandStatusObserver *aCmdObserver, PVErrorEventObserver *aErrorObserver,
                       PVInformationalEventObserver *aInfoObserver);

        // From OsclTimerObject
        void Run();

        // Command handling functions
        void Dispatch(PVEngineCommand& aCmd);
        void Dispatch(PVEngineAsyncEvent& aEvent);
        void PushCmdInFront(PVEngineCommand& aCmd);
        void CompleteEngineCommand(PVEngineCommand& aCmd, PVMFStatus aStatus,
                                   OsclAny* aResponseData = NULL, int32 aResponseDataSize = 0);
        PVMFStatus DoOpen(PVEngineCommand& aCmd);
        PVMFStatus DoClose(PVEngineCommand& aCmd);
        PVMFStatus DoAddDataSource(PVEngineCommand& aCmd);
        PVMFStatus DoRemoveDataSource(PVEngineCommand& aCmd);
        PVMFStatus DoSelectComposer(PVEngineCommand& aCmd);
        PVMFStatus DoAddMediaTrack(PVEngineCommand& aCmd);
        PVMFStatus DoAddDataSink(PVEngineCommand& aCmd);
        PVMFStatus DoRemoveDataSink(PVEngineCommand& aCmd);
        PVMFStatus DoInit(PVEngineCommand& aCmd);
        PVMFStatus DoReset(PVEngineCommand& aCmd);
        PVMFStatus DoStart(PVEngineCommand& aCmd);
        PVMFStatus DoPause(PVEngineCommand& aCmd);
        PVMFStatus DoResume(PVEngineCommand& aCmd);
        PVMFStatus DoStop(PVEngineCommand& aCmd);
        PVMFStatus DoQueryInterface(PVEngineCommand& aCmd);
        PVMFStatus DoStopMaxSizeDuration();

        // Registry function
        PVMFStatus QueryRegistry(const PvmfMimeString& aMimeType, PVUuid& aUuid);

        // Author engine state functions
        void SetPVAEState(PVAEState aState);
        PVAEState GetPVAEState()
        {
            return iState;
        }

        // Find the container that holds the specified node
        PVAENodeContainer* GetNodeContainer(PVAENodeContainerVector& aNodeContainerVector, PVMFNodeInterface* aNode);

        /**
         * Check if the specified format is supported
         */
        PVMFStatus IsCompressedFormatDataSource(PVAENodeContainer* aDataSrc, bool& aIsCompressedFormat);

        /**
         * Resets the graph.  All nodes will be disconnected.
         */
        void ResetGraph();

        /**
         * All nodes(except source nodes) will be destroyed.
         * All nodes are removed from node vectors.
         * Source node pools are destroyed when RemoveDataSource API is executed.
         */
        void ResetNodeContainers();
        /**
         * Query configuration interface for the specified encoder or composer node
         *
         * When implementing support for a new composer or encoder node, this method will
         * need to be modified to query for configuration interface(s) of the new node.
         *
         * @param aNodeContainer Node container of composer node to perform query on
         * @return Completion status
         */
        PVMFStatus QueryNodeConfig(PVAENodeContainer* aNodeContainer, PVInterface*& aInterface);

        /**
         * Configure file format composer
         *
         * When implementing support for a new file format composer node, this method will
         * need to be updated to configure the new composer node.
         *
         * @param aComposerMimeType Composer Mime type
         * @return Completion status
         */
        PVMFStatus ConfigureComposer(const PvmfMimeString& aComposerMimeType);

        /**
         * Configure encoder
         *
         * If the encoder node supports multiple output formats, the engine will need to
         * configure the encoder to encode output to the format type specified by the
         * client in AddMediaTrack.
         *
         * When implementing support for a new encoder node, this method might need to
         * be updated to configure the new node.
         *
         * @param aMimeType Encoder Mime type specified by client
         * @return Completion status
         */
        PVMFStatus ConfigureEncoder(const PvmfMimeString& aMimeType);

        /**
         * Get the PVMFFormatType corresponding to the specified encoder mime type
         *
         * When implementing support for a new encoding type, this method will need
         * to be updated to return the format type mapped to this new encoding type.
         *
         * @param aFormatType Output parameter to store the PVMFFormatType
         * @param aMimeType Mime type to be converted to PVMFFormatType
         * @return PVMFSuccess if a mapping for the Mime type is found, else return PVMFFailure.
         */
        PVMFStatus GetPvmfFormatString(PvmfMimeString& aMimeType, const PvmfMimeString& aNodeMimeType);

        PVAENodeContainer* AllocateNodeContainer(PVMFNodeInterface* aNode);
        PVMFStatus DeallocateNodeContainer(PVAENodeContainerVector& aVector, PVMFNodeInterface* aNode);

        // private functions used internally in cap-config class functions
        //f(n) used in SetParametersSync function of cap-config class
        PVMFStatus DoCapConfigSetParameters(PVEngineCommand& aCmd, bool aSyncCmd = false);	// f(n) used in SetParametersSync function of cap-config class
        //f(n) used in GetParametersSync function of cap-config class
        PVMFStatus DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        //f(n) used in ReleaseParametersSync function of cap-config class
        PVMFStatus DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements);
        //f(n) used in VerifyParametersSync function of cap-config class
        PVMFStatus DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements);
        PVMFStatus DoVerifyAndSetAuthorParameter(PvmiKvp& aParameter, bool aSetParam);
        PVMFStatus DoVerifyAndSetAuthorProductInfoParameter(PvmiKvp& aParameter, bool aSetParam);
        PVMFStatus DoGetAuthorProductInfoParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);
        PVMFStatus DoGetAuthorParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);
    private:

        // Engine commands
        Oscl_Vector<PVEngineCommand, OsclMemAllocator> iPendingCmds;
        Oscl_Vector<PVEngineAsyncEvent, OsclMemAllocator> iPendingEvents;
        PVCommandId iCommandId;

        // Engine observers
        PVCommandStatusObserver* iCmdStatusObserver;
        PVInformationalEventObserver* iInfoEventObserver;
        PVErrorEventObserver* iErrorEventObserver;

        // Node utility and storage
        PVAuthorEngineNodeUtility iNodeUtil;
        PVAENodeContainerVector iDataSourcePool;
        PVAENodeContainerVector iDataSinkPool;
        PVAENodeContainerVector iDataSourceNodes;
        PVAENodeContainerVector iEncoderNodes;
        PVAENodeContainerVector iComposerNodes;
        PVAENodeContainerVector iAllNodes;

        PVMFFormatType iEncodedVideoFormat;
        PVAEState iState;
        PVLogger* iLogger;
        int32 iCapConfigContext;
        PvmiConfigAndCapabilityCmdObserver *iCfgCapCmdObserver;
        int iAsyncNumElements;
        bool iDoResetNodeContainers;
};



#endif // PVAUTHORENGINE_H_INCLUDED


