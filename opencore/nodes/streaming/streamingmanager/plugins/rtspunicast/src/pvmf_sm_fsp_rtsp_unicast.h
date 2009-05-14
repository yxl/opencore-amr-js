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
#ifndef PVMF_SM_FSP_RTSP_UNICAST_H
#define PVMF_SM_FSP_RTSP_UNICAST_H

#ifndef PVMF_SM_FSP_BASE_IMPL_H
#include "pvmf_sm_fsp_base_impl.h"
#endif

#ifndef __SDP_INFO_H__
#include "sdp_info.h"
#endif

#ifndef PVMF_STREAMING_REAL_INTERFACES_INCLUDED
#include "pvmf_streaming_real_interfaces.h"
#endif

#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif

#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif

#ifndef PVMF_JITTER_BUFFER_PORT_H_INCLUDED
#include "pvmf_jitter_buffer_port.h"
#endif
/**
 * Macros for calling PVLogger
 */
#define PVMF_SM_RTSP_LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define PVMF_SM_RTSP_LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_NOTICE,m);
#define PVMF_SM_RTSP_LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define PVMF_SM_RTSP_LOGSTACKTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_STACK_TRACE,m);
#define PVMF_SM_RTSP_LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);
#define PVMF_SM_RTSP_LOGINFO(m) PVMF_SM_RTSP_LOGINFOMED(m)
#define PVMF_SM_RTSP_LOG_COMMAND_SEQ(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iCommandSeqLogger,PVLOGMSG_DEBUG,m);
#define PVMF_SM_RTSP_LOG_COMMAND_REPOS(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);

/*
 * Track related structures
 */
class PVMFRTSPTrackInfo
{
    public:
        PVMFRTSPTrackInfo()
        {
            trackID = 0;
            rdtStreamID = 0;
            portTag = 0;
            bitRate = 0;
            trackTimeScale = 1;
            iNetworkNodePort = NULL;
            iJitterBufferInputPort = NULL;
            iJitterBufferOutputPort = NULL;
            iMediaLayerInputPort = NULL;
            iMediaLayerOutputPort = NULL;
            iJitterBufferRTCPPort = NULL;
            iNetworkNodeRTCPPort = NULL;
            iSessionControllerOutputPort = NULL;
            iSessionControllerFeedbackPort = NULL;
            iRTPSocketID = 0;
            iRTCPSocketID = 0;
            iRateAdaptation = false;
            iRateAdaptationFeedBackFrequency = 0;
            iFormatType = PVMF_MIME_FORMAT_UNKNOWN;
            iRTCPBwSpecified = false;
            iTrackDisable = false;
        };

        PVMFRTSPTrackInfo(const PVMFRTSPTrackInfo& a)
        {
            trackID = a.trackID;
            rdtStreamID = a.rdtStreamID;
            portTag = a.portTag;
            bitRate = a.bitRate;
            trackTimeScale = a.trackTimeScale;
            iTrackConfig = a.iTrackConfig;
            iTransportType = a.iTransportType;
            iFormatType = a.iFormatType;
            iMimeType = a.iMimeType;
            iNetworkNodePort = a.iNetworkNodePort;
            iJitterBufferInputPort = a.iJitterBufferInputPort;
            iJitterBufferOutputPort = a.iJitterBufferOutputPort;
            iMediaLayerInputPort = a.iMediaLayerInputPort;
            iMediaLayerOutputPort = a.iMediaLayerOutputPort;
            iJitterBufferRTCPPort = a.iJitterBufferRTCPPort;
            iNetworkNodeRTCPPort = a.iNetworkNodeRTCPPort;
            iSessionControllerOutputPort = a.iSessionControllerOutputPort;
            iSessionControllerFeedbackPort = a.iSessionControllerFeedbackPort;
            iRTPSocketID = a.iRTPSocketID;
            iRTCPSocketID = a.iRTCPSocketID;
            iRateAdaptation = a.iRateAdaptation;
            iRateAdaptationFeedBackFrequency = a.iRateAdaptationFeedBackFrequency;
            iRTCPBwSpecified = a.iRTCPBwSpecified;
            iTrackDisable = a.iTrackDisable;
            iRR = a.iRR;
            iRS = a.iRS;

        };

        PVMFRTSPTrackInfo& operator=(const PVMFRTSPTrackInfo& a)
        {
            if (&a != this)
            {
                trackID = a.trackID;
                rdtStreamID = a.rdtStreamID;
                portTag = a.portTag;
                bitRate = a.bitRate;
                trackTimeScale = a.trackTimeScale;
                iTrackConfig = a.iTrackConfig;
                iTransportType = a.iTransportType;
                iFormatType = a.iFormatType;
                iMimeType = a.iMimeType;
                iNetworkNodePort = a.iNetworkNodePort;
                iJitterBufferInputPort = a.iJitterBufferInputPort;
                iJitterBufferOutputPort = a.iJitterBufferOutputPort;
                iMediaLayerInputPort = a.iMediaLayerInputPort;
                iMediaLayerOutputPort = a.iMediaLayerOutputPort;
                iJitterBufferRTCPPort = a.iJitterBufferRTCPPort;
                iNetworkNodeRTCPPort = a.iNetworkNodeRTCPPort;
                iSessionControllerOutputPort = a.iSessionControllerOutputPort;
                iSessionControllerFeedbackPort = a.iSessionControllerFeedbackPort;
                iRTPSocketID = a.iRTPSocketID;
                iRTCPSocketID = a.iRTCPSocketID;
                iRateAdaptation = a.iRateAdaptation;
                iRateAdaptationFeedBackFrequency = a.iRateAdaptationFeedBackFrequency;
                iRTCPBwSpecified = a.iRTCPBwSpecified;
                iTrackDisable = a.iTrackDisable;
                iRR = a.iRR;
                iRS = a.iRS;

            }
            return *this;
        };

        virtual ~PVMFRTSPTrackInfo() {};

        uint32 trackID;
        uint32 rdtStreamID;
        uint32 portTag;
        uint32 bitRate;
        uint32 trackTimeScale;
        OsclRefCounterMemFrag iTrackConfig;
        OSCL_HeapString<OsclMemAllocator> iTransportType;
        PVMFFormatType iFormatType;
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        PVMFPortInterface* iNetworkNodePort;
        PVMFPortInterface* iJitterBufferInputPort;
        PVMFPortInterface* iJitterBufferOutputPort;
        PVMFPortInterface* iMediaLayerInputPort;
        PVMFPortInterface* iMediaLayerOutputPort;
        PVMFPortInterface* iJitterBufferRTCPPort;
        PVMFPortInterface* iNetworkNodeRTCPPort;
        PVMFPortInterface* iSessionControllerOutputPort;
        PVMFPortInterface* iSessionControllerFeedbackPort;

        uint32 iRTPSocketID;
        uint32 iRTCPSocketID;
        bool   iRateAdaptation;
        uint32 iRateAdaptationFeedBackFrequency;

        // RTCP bandwidth related
        bool   iRTCPBwSpecified;
        uint32 iRR;
        uint32 iRS;

        //Check track disable or not
        bool iTrackDisable;


};
typedef Oscl_Vector<PVMFRTSPTrackInfo, OsclMemAllocator> PVMFRTSPTrackInfoVector;

class PVMFSMRTSPUnicastNode: public PVMFSMFSPBaseNode
{
    public:
        static PVMFSMRTSPUnicastNode* New(int32 aPriority);
        virtual ~PVMFSMRTSPUnicastNode();


        //Function to handle command completion from child nodes
        /* From PVMFNodeCmdStatusObserver */
        virtual void NodeCommandCompleted(const PVMFCmdResp& aResponse);

        /* From PVMFDataSourceInitializationExtensionInterface */
        virtual PVMFStatus SetSourceInitializationData(OSCL_wString& aSourceURL,
                PVMFFormatType& aSourceFormat,
                OsclAny* aSourceData);
        virtual PVMFStatus SetClientPlayBackClock(PVMFMediaClock* aClientClock);
        virtual PVMFStatus SetEstimatedServerClock(PVMFMediaClock* aClientClock);

        /* From PVMFTrackSelectionExtensionInterface */
        virtual PVMFStatus GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo);
        virtual PVMFStatus SelectTracks(PVMFMediaPresentationInfo& aInfo);

        /* From PVMFMetadataExtensionInterface */
        virtual uint32 GetNumMetadataKeys(char* aQueryKeyString = NULL);
        virtual uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        PVMFStatus DoGetMetadataKeys(PVMFSMFSPBaseNodeCommand& aCmd);
        PVMFStatus DoGetMetadataValues(PVMFSMFSPBaseNodeCommand& aCmd);
        PVMFStatus GetRTSPPluginSpecificValues(PVMFSMFSPBaseNodeCommand& aCmd);
        virtual PVMFStatus ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                uint32 aStartingKeyIndex,
                uint32 aEndKeyIndex);
        virtual PVMFStatus ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                uint32 aStartingValueIndex,
                uint32 aEndValueIndex);

        virtual void setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver);
        virtual PVMFStatus getParametersSync(PvmiMIOSession aSession,
                                             PvmiKeyType aIdentifier,
                                             PvmiKvp*& aParameters,
                                             int& aNumParamElements,
                                             PvmiCapabilityContext aContext);
        virtual PVMFStatus releaseParameters(PvmiMIOSession aSession,
                                             PvmiKvp* aParameters,
                                             int num_elements);

        virtual void createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext);
        virtual void setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
                                          PvmiKvp* aParameters, int num_parameter_elements);
        virtual void DeleteContext(PvmiMIOSession aSession,
                                   PvmiCapabilityContext& aContext);
        virtual void setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
                                       int num_elements, PvmiKvp * & aRet_kvp);
        virtual uint32 getCapabilityMetric(PvmiMIOSession aSession);
        virtual PVMFStatus verifyParametersSync(PvmiMIOSession aSession,
                                                PvmiKvp* aParameters,
                                                int num_elements);
        /**/
        virtual void HandleNodeInformationalEvent(const PVMFAsyncEvent& aEvent);
        PVMFStatus ComputeSkipTimeStamp(PVMFTimestamp aTargetNPT,
                                        PVMFTimestamp aActualNPT,
                                        PVMFTimestamp aActualMediaDataTS,
                                        PVMFTimestamp& aSkipTimeStamp,
                                        PVMFTimestamp& aStartNPT);
    private:
        PVMFSMRTSPUnicastNode(int32 aPriority);
        void Construct();
        void CreateCommandQueues();
        void CreateChildNodes();
        void DestroyChildNodes();
        void CleanUp();

        virtual bool ProcessCommand(PVMFSMFSPBaseNodeCommand&);

        //Functions to service commands queued up in input command Q by base class
        //node commands
        void DoQueryInterface(PVMFSMFSPBaseNodeCommand&);
        void DoRequestPort(PVMFSMFSPBaseNodeCommand&);
        void DoReleasePort(PVMFSMFSPBaseNodeCommand&);
        void DoInit(PVMFSMFSPBaseNodeCommand&);
        void DoPrepare(PVMFSMFSPBaseNodeCommand&);
        void DoStart(PVMFSMFSPBaseNodeCommand&);
        void DoStop(PVMFSMFSPBaseNodeCommand&);
        void DoPause(PVMFSMFSPBaseNodeCommand&);


        void DoSetDataSourcePosition(PVMFSMFSPBaseNodeCommand&);
        void DoSetDataSourcePositionPlayList(PVMFSMFSPBaseNodeCommand&);
        void DoQueryDataSourcePosition(PVMFSMFSPBaseNodeCommand&);
        void DoSetDataSourcePositionOverflow(PVMFSMFSPBaseNodeCommand&);

        bool CompleteFeedBackPortsSetup();
        bool SendSessionControlPrepareCompleteParams();
        bool SendSessionControlStartCompleteParams();

        void GetActualMediaTSAfterSeek();
        void GetMaxMediaTS();

        void ResetStopCompleteParams();

        void QueryChildNodesExtentionInterface();

        //RequestPort
        PVMFRTSPTrackInfo* FindTrackInfo(uint32 tag);

        //Init
        PVMFStatus DoPreInit(PVMFSMFSPBaseNodeCommand& aCmd);
        PVMFStatus ProcessSDP();
        PVMFStatus PopulateAvailableMetadataKeys();
        virtual bool RequestUsageComplete()
        {
            return true;
        }

        //Compute Jitter buffer mem pool size
        uint32 GetJitterBufferMemPoolSize(PVMFJitterBufferNodePortTag aJBNodePortTag, PVMFRTSPTrackInfo& aRTSPTrackInfo);

        //Graph Construction
        PVMFStatus DoGraphConstruct();
        bool GraphConnect();
        bool ReserveSockets();
        bool RequestNetworkNodePorts(int32 portTag, uint32& numPortsRequested);
        bool RequestJitterBufferPorts(int32 portType, uint32 &numPortsRequested);
        bool RequestMediaLayerPorts(int32 portType, uint32& numPortsRequested);
        bool SendSessionSourceInfoToSessionController();
        bool PopulateTrackInfoVec();
        PVMFStatus ConnectPortPairs(PVMFPortInterface* aPort1, PVMFPortInterface* aPort2);
        bool ConstructGraphFor3GPPUDPStreaming();
        void CompleteGraphConstruct();
        PVMFStatus  InitMetaData();
        virtual void PopulateDRMInfo();

        void DoSetDataSourcePosition();
        bool DoRepositioningPause3GPPStreaming();
        PVMFStatus DoRepositioningStart3GPPStreaming();
        bool DoRepositioningStart3GPPPlayListStreamingDuringPlay();
        bool DoRepositioningStart3GPPPlayListStreaming();


        PVMFStatus SetRTSPPlaybackRange();
        bool CanPerformRepositioning(bool aRandAccessDenied);

        void HandleChildNodeCommandCompletion(const PVMFCmdResp& , bool&);

        void CompleteInit();
        bool CheckChildrenNodesInit();

        void CompletePrepare();
        bool CheckChildrenNodesPrepare();

        void CompleteStart();
        bool CheckChildrenNodesStart();

        void CompleteStop();
        bool CheckChildrenNodesStop();

        void CompletePause();
        bool CheckChildrenNodesPause();

        //For parsing the possible payloads received by streaming protocols
        void PopulatePayloadParserRegistry();
        void DestroyPayloadParserRegistry();

        void HandleSocketNodeCommandCompleted(const PVMFCmdResp&, bool& aPerformErrHandling);
        void HandleRTSPSessionControllerCommandCompleted(const PVMFCmdResp&, bool& aPerformErrHandling);
        void HandleJitterBufferCommandCompleted(const PVMFCmdResp&, bool& aPerformErrHandling);
        void HandleMediaLayerCommandCompleted(const PVMFCmdResp&, bool& aPerformErrHandling);
        PVMFStatus GetConfigParameter(PvmiKvp*& aParameters, int& aNumParamElements,
                                      int32 aIndex, PvmiKvpAttr reqattr);
        PVMFStatus VerifyAndSetConfigParameter(int index, PvmiKvp& aParameter, bool set);
        void setJitterBufferDurationInMilliSeconds(uint32 duration);
        bool IsFSPInternalCmd(PVMFCommandId aId);
        void addRef();
        void removeRef();
        void ResetNodeParams(bool aReleaseMemmory = true);


        uint32 iJitterBufferDurationInMilliSeconds;

        PVMFMediaPresentationInfo iCompleteMediaPresetationInfo;
        PVMFMediaPresentationInfo iSelectedMediaPresetationInfo;

        PVMFRTSPTrackInfoVector	iTrackInfoVec;
        OsclSharedPtr<SDPInfo> iSdpInfo;
        bool oAutoReposition;
        bool iPauseDenied;	//For live streaming sessions, pause is denied.
};
#endif
