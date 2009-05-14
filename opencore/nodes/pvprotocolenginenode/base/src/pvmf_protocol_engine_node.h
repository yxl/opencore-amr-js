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

#ifndef PVMF_PROTOCOLENGINE_NODE_H_INCLUDED
#define PVMF_PROTOCOLENGINE_NODE_H_INCLUDED

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#include "pvmf_pool_buffer_allocator.h"
#endif

#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif

#ifndef PVMF_MEMPOOL_H_INCLUDED
#include "pvmf_mempool.h"
#endif

#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUED
#include "pvmi_data_stream_interface.h"
#endif

#ifndef PVMF_PROTOCOLENGINE_PORT_H_INCLUDED
#include "pvmf_protocol_engine_port.h"
#endif

#ifndef PVMF_PROTOCOLENGINE_DEFS_H_INCLUDED
#include "pvmf_protocol_engine_defs.h"
#endif

#ifndef PVMF_DATA_SOURCE_INIT_EXTENSION_H_INCLUDED
#include "pvmf_data_source_init_extension.h"
#endif

#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif

#ifndef PVMF_PROTOCOLENGINE_NODE_EXTENSION_H_INCLUDED
#include "pvmf_protocol_engine_node_extension.h"
#endif

#ifndef PVMF_PROTOCOLENGINE_NODE_EVENTS_H_INCLUDED
#include "pvmf_protocol_engine_node_events.h"
#endif

#ifndef PVMF_DOWNLOAD_PROGRESS_EXTENSION_H
#include "pvmf_download_progress_interface.h"
#endif

#ifndef PVMF_TRACK_SELECTION_EXTENSION_H_INCLUDED
#include "pvmf_track_selection_extension.h"
#endif

#ifndef PVMF_PROTOCOLENGINE_NODE_COMMON_H_INCLUDED
#include "pvmf_protocol_engine_node_common.h"
#endif

#ifndef PVMF_PROTOCOLENGINE_NODE_REGISTRY_H_INCLUDED
#include "pvmf_protocol_engine_node_registry.h"
#endif



///////////////////////////////////////////////
// The PVMFProtocolEngineNode Node Implementation Class
///////////////////////////////////////////////

class PVLogger;
class PVMFProtocolEnginePort;
class ProtocolContainer;
class ProtocolContainerFactory;
class PVMFProtocolEngineNodeOutput;
class PVDlCfgFileContainer;
class PVMFDownloadDataSourceContainer;
class SDPInfoContainer;
class AutoCleanup;

class PVMFProtocolEngineNode :  public PVMFNodeInterface,
            public OsclTimerObject,
            public PVMFDataSourceInitializationExtensionInterface,
            public PVMIDatastreamuserInterface,
            public PVMFProtocolEngineNodeExtensionInterface,
            public PVMFDownloadProgressInterface,
            public PVMFTrackSelectionExtensionInterface,
            public PVMFProtocolEngineNodeMSHTTPStreamingExtensionInterface,
            public ProtocolObserver,
            public PVMFProtocolEngineNodeOutputObserver,
            public OsclTimerObserver,
            public PvmiDataStreamRequestObserver,
            public EventReporterObserver,
            public ProtocolContainerObserver

{
    public:
        PVMFProtocolEngineNode(int32 aPriority = OsclActiveObject::EPriorityNominal);
        virtual ~PVMFProtocolEngineNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();
        PVMFStatus ThreadLogoff();
        PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        PVMFCommandId QueryUUID(PVMFSessionId, const PvmfMimeString& aMimeType,
                                Oscl_Vector<PVUuid, PVMFProtocolEngineNodeAllocator>& aUuids,
                                bool aExactUuidsOnly = false,
                                const OsclAny* aContext = NULL);
        PVMFCommandId QueryInterface(PVMFSessionId, const PVUuid& aUuid,
                                     PVInterface*& aInterfacePtr,
                                     const OsclAny* aContext = NULL);
        PVMFCommandId RequestPort(PVMFSessionId, int32 aPortTag, const PvmfMimeString* aPortConfig = NULL, const OsclAny* aContext = NULL);
        PVMFCommandId ReleasePort(PVMFSessionId, PVMFPortInterface& aPort, const OsclAny* aContext = NULL);
        PVMFCommandId Init(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Prepare(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Start(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Stop(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Flush(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Pause(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId Reset(PVMFSessionId, const OsclAny* aContext = NULL);
        PVMFCommandId CancelAllCommands(PVMFSessionId, const OsclAny* aContextData = NULL);
        PVMFCommandId CancelCommand(PVMFSessionId, PVMFCommandId aCmdId, const OsclAny* aContextData = NULL);

        //From PVMFDataSourceInitializationExtensionInterface
        void addRef();
        void removeRef();
        PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                                               PVMFFormatType& aSourceFormat,
                                               OsclAny* aSourceData);
        PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);

        // From PVMFProtocolEngineNodeExtensionInterface and
        // From PVMFProtocolEngineNodeMSHTTPStreamingExtensionInterface
        PVMFStatus GetHTTPHeader(uint8*& aHeader, uint32& aHeaderLen);
        void GetFileSize(uint32& aFileSize)
        {
            aFileSize = (iInterfacingObjectContainer == NULL ? 0 : iInterfacingObjectContainer->getFileSize());
        }
        bool GetSocketConfig(OSCL_String &aPortConfig);
        bool GetSocketConfigForLogging(OSCL_String &aPortConfig);
        bool SetUserAgent(OSCL_wString &aUserAgent, const bool isOverwritable = false);
        void SetHttpVersion(const uint32 aHttpVersion = HTTP_V11);
        void SetNetworkTimeout(const uint32 aTimeout);
        void SetNetworkLoggingTimeout(const uint32 aTimeout);
        void SetKeepAliveTimeout(const uint32 aTimeout)
        {
            uint32 timeout = aTimeout;
            if ((int32)timeout < 0) timeout = 0x7fffffff;
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setKeepAliveTimeout(timeout);
        }
        void SetNumRedirectTrials(const uint32 aNumTrials)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setNumRedirectTrials(aNumTrials);
        }
        void SetHttpExtensionHeaderField(OSCL_String &aFieldKey, OSCL_String &aFieldValue, const HttpMethod aMethod = HTTP_GET, const bool aPurgeOnRedirect = false);
        void SetLoggingURL(OSCL_wString& aSourceURL);
        void SetDownloadProgressMode(const DownloadProgressMode aMode = DownloadProgressMode_TimeBased)
        {
            if (iDownloadProgess) iDownloadProgess->setDownloadProgressMode(aMode);
        }
        bool SetStreamingProxy(OSCL_wString& aProxyURL, const uint32 aProxyPort)
        {
            return (iInterfacingObjectContainer == NULL ? false : iInterfacingObjectContainer->setStreamingProxy(aProxyURL, aProxyPort));
        }
        void DisableHttpHeadRequest(const bool aDisableHeadRequest = true)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setHttpHeadRequestDisabled(aDisableHeadRequest);
        }
        bool GetASFHeader(Oscl_Vector<OsclRefCounterMemFrag, OsclMemAllocator> &aHeader)
        {
            return iProtocol->getHeader(aHeader);
        }
        bool SetStreamParams(const PVMFProtocolEngineNodeMSHTTPStreamingParams &aParams)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setStreamParams((PVMFProtocolEngineNodeMSHTTPStreamingParams &)aParams);
            return true;
        }
        PVMFCommandId Seek(PVMFSessionId aSessionId, uint64 aNPTInMS, uint32& aFirstSeqNumAfterSeek, OsclAny* aContext);
        PVMFCommandId BitstreamSwitch(PVMFSessionId aSessionId, uint64 aNPTInMS, uint32& aFirstSeqNumAfterSwitch, OsclAny* aContext);

        PVMFStatus SetMediaMsgAllocatorNumBuffers(PVMFPortInterface* aPort, uint32 aNumBuffersInAllocator)
        {
            OSCL_UNUSED_ARG(aPort);
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setMediaMsgAllocatorNumBuffers(aNumBuffersInAllocator);
            return PVMFSuccess;
        }
        void SetMaxASFHeaderSize(const uint32 aMaxASFHeaderSize = PVPROTOCOLENGINE_DEFAULT_MAXIMUM_ASF_HEADER_SIZE)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setMaxASFHeaderSize(aMaxASFHeaderSize);
        }
        void SetUserAuthInfo(OSCL_String &aUserID, OSCL_String &aPasswd)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->setUserAuthInfo(aUserID, aPasswd);
        }
        bool IsWMServerVersion4();
        void SetAccelBitrate(const uint32 aAccelBitrate)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->SetAccelBitrate(aAccelBitrate);
        }
        void SetAccelDuration(const uint32 aAccelDuration)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->SetAccelDuration(aAccelDuration);
        }

        void SetNumBuffersInMediaDataPoolSMCalc(uint32 aVal)
        {
            iInterfacingObjectContainer->setNumBuffersInMediaDataPoolSMCalc(aVal);
        }

        void SetMaxHttpStreamingSize(const uint32 aMaxHttpStreamingSize)
        {
            if (iInterfacingObjectContainer) iInterfacingObjectContainer->SetMaxHttpStreamingSize(aMaxHttpStreamingSize);
        }

        // From PVMIDatastreamuserInterface
        void PassDatastreamFactory(PVMFDataStreamFactory& aFactory, int32 aFactoryTag, const PvmfMimeString* aFactoryConfig = NULL);

        // From PVMFDownloadProgressInterface
        void setFormatDownloadSupportInterface(PVMFFormatProgDownloadSupportInterface* download_support_interface);
        void setClipDuration(const uint32 aClipDurationMsec);
        OsclSharedPtr<PVMFMediaClock> getDownloadProgressClock();
        void requestResumeNotification(const uint32 currentNPTReadPosition,
                                       bool& aDownloadComplete);
        void cancelResumeNotification();
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        //From PVMFTrackSelectionExtensionInterface
        OSCL_IMPORT_REF PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

        // From PvmiDataStreamRequestObserver
        PvmiDataStreamCommandId DataStreamRequest(PvmiDataStreamSession aSessionID, PvmiDataStreamRequest aRequestID, OsclAny* aRequestData, OsclAny* aContextData);
        PvmiDataStreamStatus DataStreamRequestSync(PvmiDataStreamSession aSessionID, PvmiDataStreamRequest aRequestID, OsclAny* aRequestData);

        // Currently all data stream requests are repositioning requests
        // otherwise, we'll need to peek into the queue
        bool IsRepositioningRequestPending();

    private:
        //from OsclActiveObject
        void Run();

        //from PVMFPortActivityHandler
        void HandlePortActivity(const PVMFPortActivity& aActivity);

        //From PVMFDataSourceInitializationExtensionInterface, the following two APIs are not used in this node
        PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock)
        {
            OSCL_UNUSED_ARG(aClientClock);
            OSCL_LEAVE(OsclErrNotSupported);
            return PVMFFailure;
        }

        // From PVMFDownloadProgressInterface
        void setMP4DownloadSupportInterface(PVMFMP4ProgDownloadSupportInterface* download_support_interface)
        {
            OSCL_UNUSED_ARG(download_support_interface);
        }

        // Port processing
        bool ProcessPortActivity();
        void QueuePortActivity(const PVMFPortActivity& aActivity);
        void QueueActivityIncomingMessage(const PVMFStatus aStatus, const PVMFPortActivity &aActivity); // called by ProcessPortActivity()
        void QueueActivityOutgoingMessage(const PVMFStatus aStatus, const PVMFPortActivity &aActivity); // called by ProcessPortActivity()
        PVMFStatus ProcessIncomingMsg(PVMFPortInterface* aPort);
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);
        PVMFStatus PostProcessForMsgSentSuccess(PVMFPortInterface* aPort, PVMFSharedMediaMsgPtr &aMsg);
        void SendOutgoingQueueReadyEvent(PVMFPortInterface* aPort);
        bool SearchPortActivityInQueue(const PVMFPortActivityType aType);
        void ProcessOutgoingQueueReady();
        void UpdateTimersInProcessIncomingMsg(const bool aEOSMsg, PVMFPortInterface* aPort);
        void UpdateTimersInProcessOutgoingMsg(const bool isMediaData, PVMFPortInterface* aPort);

        //Command processing
        PVMFCommandId QueueCommandL(PVMFProtocolEngineNodeCommand&);
        bool ProcessCommand(PVMFProtocolEngineNodeCommand&);
        void CommandComplete(PVMFProtocolEngineNodeCmdQ&,
                             PVMFProtocolEngineNodeCommand&,
                             PVMFStatus, OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL,
                             int32 aEventDataLen = 0);
        int32 HandleCommandComplete(PVMFProtocolEngineNodeCmdQ& aCmdQ,
                                    PVMFProtocolEngineNodeCommand& aCmd,
                                    int32 aStatus);
        PVMFProtocolEngineNodeCommand* FindCmd(PVMFProtocolEngineNodeCmdQ &aCmdQueue, int32 aCmdId);
        bool FlushPending();

        //Command handlers.
        PVMFStatus DoReset(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoRequestPort(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoReleasePort(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoQueryUuid(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoQueryInterface(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoInit(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoPrepare(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoStart(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoStop(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoFlush(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoPause(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoCancelAllCommands(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoCancelCommand(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoSeek(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoBitsteamSwitch(PVMFProtocolEngineNodeCommand&);
        PVMFStatus DoReposition(PVMFProtocolEngineNodeCommand&);
        bool CheckAvailabilityOfDoStart(PVMFProtocolEngineNodeCommand& aCmd); // called in DoStart() only
        inline bool IsDataFlowEventAlreadyInQueue();
        void PassInObjects(); // called by DoInit()

        // Run decomposition
        bool HandleRunPortActivityProcessing();
        void HandleRunFlush();

        // Event reporting
        void ReportInfoEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, const int32 aEventCode = 0, OsclAny* aEventLocalBuffer = NULL, const uint32 aEventLocalBufferSize = 0);
        void ReportErrorEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, const int32 aEventCode = 0, int32 aEventDataLen = 0);
        void SetState(TPVMFNodeInterfaceState);

        // From OsclTimerObserver
        void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        // From ProtocolObserver
        void ProtocolStateComplete(const ProtocolStateCompleteInfo &aInfo);
        void OutputDataAvailable(OUTPUT_DATA_QUEUE &aOutputQueue, ProtocolEngineOutputDataSideInfo &aSideInfo);
        void ProtocolStateError(int32 aErrorCode); // server response error or other internal fatal error
        bool GetBufferForRequest(PVMFSharedMediaDataPtr &aMediaData);	// to contruct HTTP request
        void ProtocolRequestAvailable(uint32 aRequestType = ProtocolRequestType_Normaldata); // need to send to port

        // From PVMFProtocolEngineNodeOutputObserver
        void OutputBufferAvailable();
        void OutputBufferPoolFull();
        void ReadyToUpdateDownloadControl();
        bool QueueOutgoingMsgSentComplete(PVMFProtocolEnginePort *aPort, PVMFSharedMediaMsgPtr &aMsg, const PVMFStatus aStatus);

        // From EventReporterObserver
        void ReportEvent(PVMFEventType aEventType, OsclAny* aEventData = NULL, const int32 aEventCode = 0, OsclAny* aEventLocalBuffer = NULL, const uint32 aEventLocalBufferSize = 0);
        void NotifyContentTooLarge();

        // From ProtocolContainerObserver
        uint32 GetObserverState();
        void SetObserverState(const uint32 aState);
        bool DispatchEvent(PVProtocolEngineNodeInternalEvent *aEvent);
        bool SendMediaCommand(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId, const bool isForLogging = false);
        void ClearRest(const bool aNeedDelete = false);
        void RecheduleDataFlow();
        void SendManualResumeNotificationEvent();
        bool IsRepositionCmdPending();
        PVMFProtocolEngineNodeCommand* FindPendingCmd(int32 aCmdId);
        void CompletePendingCmd(int32 status);
        void CompleteInputCmd(PVMFProtocolEngineNodeCommand& aCmd, int32 status);
        void ErasePendingCmd(PVMFProtocolEngineNodeCommand *aCmd);

        // Internal methods
        bool HandleProcessingState();
        void SetProcessingState(PVProtocolEngineNodePrcoessingState aState);
        bool DispatchInternalEvent(PVProtocolEngineNodeInternalEvent *aLatestEvent = NULL);
        void LogIncomingMessage(PVMFSharedMediaMsgPtr &aMsg, bool isEOS, PVMFPortInterface* aPort);
        bool IgnoreCurrentInputData(PVMFPortInterface* aPort, const bool isEOS, PVMFSharedMediaMsgPtr &aMsg);
        bool CheckEndOfProcessingInIgoreData(const bool isEOS, const bool isDataPort = true);
        // support GetSocketConfig and GetSocketConfigForLogging
        bool GetSocketConfigImp(const INetURI &aURI, OSCL_String &aPortConfig);
        bool ComposeSocketConfig(OSCL_String &aServerAddr, const uint32 aPortNum, OSCL_String &aPortConfig);
        bool CheckUsingProxy(OSCL_String &aPortConfig);
        bool getProxy(OSCL_String& aProxyName, uint32 &aProxyPort); // called by CheckUsingProxy() only

        uint32 GetProtocolType(PVMFFormatType& aSourceFormat, OsclAny* aSourceData);
        bool CreateProtocolObjects(OsclAny* &aProtocolInfo);
        bool CreateRestObjects();
        bool CreateEventHandlers();
        void DeleteProtocolObjects();
        void DeleteRestObjects();
        bool RecheckProtocolObjects(OsclAny* aSourceData, OsclAny* aPluginInfo);
        ProtocolContainer* CreateProtocolContainer(const uint32 aProtocolType);
        bool SendPortMediaCommand(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId, const bool isForLogging = false);
        void RerunForPostProcessAfterOutgoingMsgSent(PVMFProtocolEnginePort *aPort, PVMFSharedMediaMsgPtr &aMsg);
        void LogPortMediaCmdQueued(PVMFProtocolEnginePort *aPort, PVUid32 aCmdId);
        bool CheckFormatSpecificInfoForMediaCommand(PVMFSharedMediaCmdPtr &aCmdPtr, PVUid32 aCmdId, const bool isForLogging = false);
        void ClearPorts(const bool aNeedDelete = false);
        void Clear(const bool aNeedDelete = false);
        void ResetClear(const bool aNeedDelete = false);
        void StopClear();
        void CancelClear();
        void GetObjects();

    private:
        int32 iStatusCode; // work as a global variable
        PVProtocolEngineNodePrcoessingState iProcessingState;

        // this object container contains the multiple input data from node user and output data to node user
        InterfacingObjectContainer *iInterfacingObjectContainer;

        friend class AutoCleanup;
        friend class PVProtocolEngineNodeInternalEventHandler;
        friend class ProtocolStateErrorHandler;
        friend class HttpHeaderAvailableHandler;
        friend class FirstPacketAvailableHandler;
        friend class NormalDataAvailableHandler;
        friend class ProtocolStateCompleteHandler;
        friend class NormalDataFlowHandler;
        friend class EndOfDataProcessingHandler;
        friend class ServerResponseErrorBypassingHandler;
        friend class CheckResumeNotificationHandler;
        friend class OutgoingMsgSentSuccessHandler;

        PVMFProtocolEngineNodeRegistry iRegistry;
        HttpBasedProtocol *iProtocol;
        ProtocolContainer *iProtocolContainer;
        ProtocolContainerFactory *iProtocolContainerFactory;
        PVMFProtocolEngineNodeOutput *iNodeOutput;
        PVProtocolEngineNodeInternalEventHandler *iEventHandlers[EVENT_HANDLER_TOTAL];
        PVProtocolEngineNodeInternalEventHandler *iCurrEventHandler;
        PVUuid iUuidForProtocolContainer;

        // PVDlCfgFileContainer
        PVDlCfgFileContainer *iCfgFileContainer;
        PVMFDownloadDataSourceContainer *iDownloadSource;

        // download control, auto-resume control
        DownloadControlInterface *iDownloadControl;

        // download progress
        DownloadProgressInterface *iDownloadProgess;

        // fasttrack only
        SDPInfoContainer *iSDPInfo;

        // user agent field
        UserAgentField *iUserAgentField;

        // report event
        EventReporter *iEventReport;

        // Fragment pool for port config format specific info for redirect
        OsclRefCounterMemFragAlloc *iPortConfigFSInfoAlloc;
        OsclMemPoolFixedChunkAllocator iPortConfigMemPool;

        // network timer for timeout
        PVMFProtocolEngineNodeTimer *iNodeTimer;

        Oscl_Vector<PVProtocolEngineNodeInternalEvent, PVMFProtocolEngineNodeAllocator> iInternalEventQueue;


        // Queue of incoming data
        // Input ports media msg queue should be used, but difficult without disturbing input port activity handling
        INPUT_DATA_QUEUE iDataInQueue; // INPUT_DATA_QUEUE is type defined in pvmf_protocol_engine_node_internal.h

        // Vector of ports contained in this node
        PVMFPortVector<PVMFProtocolEnginePort, PVMFProtocolEngineNodeAllocator> iPortVector;
        PVMFProtocolEnginePort *iPortInForData, *iPortInForLogging, *iPortOut;
        friend class PVMFProtocolEnginePort;
        Oscl_Vector<PVMFPortActivity, PVMFProtocolEngineNodeAllocator> iPortActivityQueue;

        PVMFProtocolEngineNodeCmdQ iInputCommands;
        PVMFProtocolEngineNodeCmdQ iCurrentCommand;
        PVMFNodeCapability iCapability;
        PVMFCommandId iCurrentCmdId;
        PVMFPortInterface* iCmdRespPort;

        PVLogger* iLogger;
        PVLogger* iDataPathLogger;
        PVLogger* iClockLogger;

        int32 iExtensionRefCount;
        PvmiDataStreamCommandId iCurrentDataStreamCmdId;
};

class AutoCleanup
{
    public:
        AutoCleanup(PVMFProtocolEngineNode *aNode) : iNode(aNode), iNeedDeleteObjects(true)
        {
            ;
        }
        ~AutoCleanup()
        {
            if (iNode && iNeedDeleteObjects) iNode->DeleteProtocolObjects();
        }

        void cancel()
        {
            iNeedDeleteObjects = false;
        }

    private:
        PVMFProtocolEngineNode *iNode;
        bool iNeedDeleteObjects;
};

#endif // PVMF_PROTOCOLENGINE_NODE_H_INCLUDED


