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
#ifndef PVMF_JITTER_BUFFER_NODE_H_INCLUDED
#define PVMF_JITTER_BUFFER_NODE_H_INCLUDED
#ifndef OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifdef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMF_JITTER_BUFFER_PORT_H_INCLUDED
#include "pvmf_jitter_buffer_port.h"
#endif
#ifndef PVMF_JITTER_BUFFER_EXT_INTERFACE_H_INCLUDED
#include "pvmf_jitter_buffer_ext_interface.h"
#endif
#ifndef PVMF_JITTER_BUFFER_INTERNAL_H_INCLUDED
#include "pvmf_jitter_buffer_internal.h"
#endif
#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif

#ifndef PVMF_JB_JITTERBUFFERMISC_H_INCLUDED
#include "pvmf_jb_jitterbuffermisc.h"
#endif

#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif

///////////////////////////////////////////////
//IDs for all of the asynchronous node commands.
///////////////////////////////////////////////
enum TPVMFJitterBufferNodeCommand
{
    PVMF_JITTER_BUFFER_NODE_QUERYUUID,
    PVMF_JITTER_BUFFER_NODE_QUERYINTERFACE,
    PVMF_JITTER_BUFFER_NODE_REQUESTPORT,
    PVMF_JITTER_BUFFER_NODE_RELEASEPORT,
    PVMF_JITTER_BUFFER_NODE_INIT,
    PVMF_JITTER_BUFFER_NODE_PREPARE,
    PVMF_JITTER_BUFFER_NODE_START,
    PVMF_JITTER_BUFFER_NODE_STOP,
    PVMF_JITTER_BUFFER_NODE_FLUSH,
    PVMF_JITTER_BUFFER_NODE_PAUSE,
    PVMF_JITTER_BUFFER_NODE_RESET,
    PVMF_JITTER_BUFFER_NODE_CANCELALLCOMMANDS,
    PVMF_JITTER_BUFFER_NODE_CANCELCOMMAND,
    /* add jitter buffer node specific commands here */
    PVMF_JITTER_BUFFER_NODE_COMMAND_LAST
};

enum PVMFStreamType
{
    PVMF_STREAM_TYPE_UNKNOWN = 0,
    PVMF_STREAM_TYPE_AUDIO = 1,
    PVMF_STREAM_TYPE_VIDEO = 2,

    //NUM_PVMF_STREAM_TYPES must always be at the end.
    NUM_PVMF_STREAM_TYPES
};

class PVLogger;
class PVMFJitterBufferExtensionInterfaceImpl;
class PVMFJitterBufferConstructParams;
class JitterBufferFactory;

class PVMFJitterBufferNode : public PVInterface,
            public PVMFNodeInterface,
            public OsclActiveObject,
            public PVMFJitterBufferObserver,
            public PVMFJitterBufferMiscObserver,
            public PVMFJBEventNotifierObserver,
            public PVMFMediaClockStateObserver
{
    public:
        OSCL_IMPORT_REF PVMFJitterBufferNode(int32 aPriority, JitterBufferFactory* aJBFactory);			//may be provide static func for creation and make the ctor private
        OSCL_IMPORT_REF virtual ~PVMFJitterBufferNode();

        //Overrides from PVInterface
        virtual void addRef()
        {
        }
        virtual void removeRef()
        {
        }
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        //Overrides from PVMFNodeInterface
        OSCL_IMPORT_REF PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF PVMFStatus ThreadLogoff();
        OSCL_IMPORT_REF PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);
        OSCL_IMPORT_REF PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryUUID(PVMFSessionId,
                                                const PvmfMimeString& aMimeType,
                                                Oscl_Vector< PVUuid, OsclMemAllocator >& aUuids,
                                                bool aExactUuidsOnly = false,
                                                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId QueryInterface(PVMFSessionId,
                const PVUuid& aUuid,
                PVInterface*& aInterfacePtr,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId RequestPort(PVMFSessionId,
                int32 aPortTag,
                const PvmfMimeString* aPortConfig = NULL,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId ReleasePort(PVMFSessionId,
                PVMFPortInterface& aPort,
                const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Init(PVMFSessionId,
                                           const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Prepare(PVMFSessionId aSession,
                                              const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Start(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Stop(PVMFSessionId,
                                           const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Flush(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Pause(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId Reset(PVMFSessionId,
                                            const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelAllCommands(PVMFSessionId,
                const OsclAny* aContextData = NULL);
        OSCL_IMPORT_REF PVMFCommandId CancelCommand(PVMFSessionId,
                PVMFCommandId aCmdId,
                const OsclAny* aContextData = NULL);
        void HandlePortActivity(const PVMFPortActivity& aActivity);   //from PVMFPortActivityHandler

        //Overrides from PVMFJitterBufferObserver
        virtual void JitterBufferFreeSpaceAvailable(OsclAny* aContext);
        virtual void ProcessJBInfoEvent(PVMFAsyncEvent& aEvent);
        virtual void PacketReadyToBeRetrieved(OsclAny* aContext);
        virtual void EndOfStreamSignalled(OsclAny* aContext);

        //PVMFJitterBufferMiscObserver
        virtual void MessageReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage);
        virtual void MediaReceivingChannelPrepared(bool aStatus);
        virtual void ProcessRTCPControllerEvent(PVMFAsyncEvent& aEvent);
        virtual void SessionSessionExpired();


        //Overrides from PVMFJBEventNotifierObserver
        virtual void ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus);

        //Override from PVMFMediaClockStateObserver
        virtual void ClockStateUpdated();
        virtual void NotificationsInterfaceDestroyed();

    private:
        //Overrides from OsclActiveObject
        void Run();
        void DoCancel();

        //Extension interface functions implemented in Node
        virtual void SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval);	//Not used as of now
        virtual bool SetPortParams(PVMFPortInterface* aPort,
                                   uint32 aTimeScale,
                                   uint32 aBitRate,
                                   OsclRefCounterMemFrag& aConfig,
                                   bool aRateAdaptation = false,
                                   uint32 aRateAdaptationFeedBackFrequency = 0);
        virtual bool SetPlayRange(int32 aStartTimeInMS,
                                  int32 aStopTimeInMS,
                                  bool oPlayAfterASeek,
                                  bool aStopTimeAvailable = true);
        virtual void SetPlayBackThresholdInMilliSeconds(uint32 threshold);	//Not used as of now [replace with wait for OOO timeout]
        virtual void SetJitterBufferRebufferingThresholdInMilliSeconds(uint32 aThreshold);
        virtual void GetJitterBufferRebufferingThresholdInMilliSeconds(uint32& aThreshold);
        virtual void SetJitterBufferDurationInMilliSeconds(uint32 duration);
        virtual void GetJitterBufferDurationInMilliSeconds(uint32& duration);
        virtual void SetEarlyDecodingTimeInMilliSeconds(uint32 duration);
        virtual void SetBurstThreshold(float burstThreshold);
        //While in buffering/start state, Jitter Buffer node expects its upstream peer node to send media msg at its input port in duration < inactivity duration
        virtual void SetMaxInactivityDurationForMediaInMs(uint32 duration);
        virtual void GetMaxInactivityDurationForMediaInMs(uint32& duration);

        virtual void SetClientPlayBackClock(PVMFMediaClock* clientClock);
        virtual bool PrepareForRepositioning(bool oUseExpectedClientClockVal = false,
                                             uint32 aExpectedClientClockVal = 0);	//called for RTSP based streaming only
        virtual bool SetPortSSRC(PVMFPortInterface* aPort, uint32 aSSRC);
        virtual bool SetPortRTPParams(PVMFPortInterface* aPort,
                                      bool   aSeqNumBasePresent,
                                      uint32 aSeqNumBase,
                                      bool   aRTPTimeBasePresent,
                                      uint32 aRTPTimeBase,
                                      bool   aNPTTimeBasePresent,
                                      uint32 aNPTInMS,
                                      bool oPlayAfterASeek = false);
        virtual bool SetPortRTCPParams(PVMFPortInterface* aPort,
                                       int aNumSenders,
                                       uint32 aRR,
                                       uint32 aRS);
        virtual PVMFTimestamp GetActualMediaDataTSAfterSeek();
        virtual PVMFTimestamp GetMaxMediaDataTS();

        virtual PVMFStatus SetServerInfo(PVMFJitterBufferFireWallPacketInfo& aServerInfo);
        virtual PVMFStatus NotifyOutOfBandEOS();
        virtual PVMFStatus SendBOSMessage(uint32 aStramID);

        virtual void SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator, const PVMFPortInterface* aPort);

        virtual void SetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort, uint32 aSize, uint32 aResizeSize, uint32 aMaxNumResizes, uint32 aExpectedNumberOfBlocksPerBuffer);
        virtual void GetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort, uint32& aSize, uint32& aResizeSize, uint32& aMaxNumResizes, uint32& aExpectedNumberOfBlocksPerBuffer) const ;

        virtual void SetSharedBufferResizeParams(uint32 maxNumResizes, uint32 resizeSize);
        virtual void GetSharedBufferResizeParams(uint32& maxNumResizes, uint32& resizeSize);

        virtual bool ClearJitterBuffer(PVMFPortInterface* aPort,
                                       uint32 aSeqNum);
        virtual void FlushJitterBuffer();


        virtual PVMFStatus SetInputMediaHeaderPreParsed(PVMFPortInterface* aPort,
                bool aHeaderPreParsed);

        virtual PVMFStatus HasSessionDurationExpired(bool& aExpired);
        virtual bool PurgeElementsWithNPTLessThan(NptTimeFormat& aNPTTime);

        virtual void SetBroadCastSession();
        virtual void DisableFireWallPackets();
        virtual void UpdateJitterBufferState();
        virtual void StartOutputPorts();
        virtual void StopOutputPorts();

        bool SetPortParams(PVMFPortInterface* aPort,
                           uint32 aTimeScale,
                           uint32 aBitRate,
                           OsclRefCounterMemFrag& aConfig,
                           bool aRateAdaptation,
                           uint32 aRateAdaptationFeedBackFrequency,
                           uint aMaxNumBuffResizes, uint aBuffResizeSize);
        bool SetPortParams(PVMFPortInterface* aPort,
                           uint32 aTimeScale,
                           uint32 aBitRate,
                           OsclRefCounterMemFrag& aConfig,
                           bool aRateAdaptation,
                           uint32 aRateAdaptationFeedBackFrequency,
                           bool aUserSpecifiedBuffParams,
                           uint aMaxNumBuffResizes = 0, uint aBuffResizeSize = 0);
        bool PrepareForPlaylistSwitch();

        //Async command handling functions
        void DoQueryUuid(PVMFJitterBufferNodeCommand&);
        void DoQueryInterface(PVMFJitterBufferNodeCommand&);
        void DoRequestPort(PVMFJitterBufferNodeCommand&);
        void DoReleasePort(PVMFJitterBufferNodeCommand&);
        void DoInit(PVMFJitterBufferNodeCommand&);
        void DoPrepare(PVMFJitterBufferNodeCommand& aCmd);
        void CompletePrepare();
        void CancelPrepare();
        void DoStart(PVMFJitterBufferNodeCommand&);
        void CompleteStart();
        void CancelStart();

        //Utility functions
        void Construct();
        void ResetNodeParams(bool aReleaseMemory = true);
        bool ProcessPortActivity(PVMFJitterBufferPortParams*);
        PVMFStatus ProcessIncomingMsg(PVMFJitterBufferPortParams*);
        PVMFStatus ProcessOutgoingMsg(PVMFJitterBufferPortParams*);
        void QueuePortActivity(PVMFJitterBufferPortParams*,
                               const PVMFPortActivity&);
        bool CheckForPortRescheduling();
        bool CheckForPortActivityQueues();

        bool getPortContainer(PVMFPortInterface* aPort,
                              PVMFJitterBufferPortParams*& aPortParams);


        PVMFCommandId QueueCommandL(PVMFJitterBufferNodeCommand&);
        void MoveCmdToCurrentQueue(PVMFJitterBufferNodeCommand& aCmd);
        bool ProcessCommand(PVMFJitterBufferNodeCommand&);
        void CommandComplete(PVMFJitterBufferNodeCmdQ&,
                             PVMFJitterBufferNodeCommand&,
                             PVMFStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);
        void CommandComplete(PVMFJitterBufferNodeCommand&,
                             PVMFStatus,
                             OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);
        void InternalCommandComplete(PVMFJitterBufferNodeCmdQ&,
                                     PVMFJitterBufferNodeCommand&,
                                     PVMFStatus aStatus,
                                     OsclAny* aEventData = NULL);
        PVMFJBCommandContext* RequestNewInternalCmd();

        void DoStop(PVMFJitterBufferNodeCommand&);
        void DoFlush(PVMFJitterBufferNodeCommand&);
        bool FlushPending();

        void DoPause(PVMFJitterBufferNodeCommand&);
        void DoReset(PVMFJitterBufferNodeCommand&);
        void DoCancelAllCommands(PVMFJitterBufferNodeCommand&);
        void DoCancelCommand(PVMFJitterBufferNodeCommand&);

        void ReportErrorEvent(PVMFEventType aEventType,
                              OsclAny* aEventData = NULL,
                              PVUuid* aEventUUID = NULL,
                              int32* aEventCode = NULL);
        void ReportInfoEvent(PVMFEventType aEventType,
                             OsclAny* aEventData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);
        void SetState(TPVMFNodeInterfaceState);

        void CleanUp();



        friend class PVMFJitterBufferPort;
        friend class PVMFJitterBufferExtensionInterfaceImpl;



        bool CheckStateForRegisteringRTPPackets();

        bool NotifyAutoPauseComplete()
        {

            return true;
        }
        bool NotifyAutoResumeComplete()
        {

            return true;
        }

        PVMFPortInterface* getPortCounterpart(PVMFPortInterface* aPort);
        bool CheckForSpaceInJitterBuffer(PVMFPortInterface* aPort);
        PVMFStatus CheckForHighWaterMark(PVMFPortInterface* aPort, bool& aHighWaterMarkReached);
        PVMFStatus CheckForLowWaterMark(PVMFPortInterface* aPort, bool& aLowWaterMarkReached);
        PVMFStatus RequestMemCallBackForAutoResume(PVMFPortInterface* aPort);
        void freeblockavailable(OsclAny* aContextData);
        void freememoryavailable(OsclAny* aContextData);
        void chunkdeallocated(OsclAny* aContextData);
        bool RegisterDataPacket(PVMFPortInterface* aPort,
                                PVMFJitterBuffer* aJitterBuffer,
                                PVMFSharedMediaDataPtr& aDataPacket);
        PVMFJitterBuffer* findJitterBuffer(PVMFPortInterface* aPort);
        PVMFPortInterface* findPortForJitterBuffer(PVMFJitterBuffer*);
        bool IsJitterBufferReady(PVMFJitterBufferPortParams*, uint32& aClockDiff);
        PVMFStatus SendData(PVMFPortInterface*);
        PVMFStatus CheckJitterBufferEmpty(bool& oEmpty);


        void LogSessionDiagnostics();
        void LogPortDiagnostics();

        PVMFStatus CheckForEOS();
        PVMFStatus GenerateAndSendEOSCommand(PVMFPortInterface* aPort);


        bool QueueBOSCommand(PVMFPortInterface* aPort);

        PVMFStatus SetInPlaceProcessingMode(PVMFPortInterface* aPort,
                                            bool aInPlaceProcessing)
        {
            PVMFJitterBufferPortParams* portParamsPtr = NULL;
            if (!getPortContainer(aPort, portParamsPtr))
            {
                return PVMFFailure;
            }

            if (portParamsPtr->ipJitterBuffer != NULL)
            {
                portParamsPtr->ipJitterBuffer->SetInPlaceProcessingMode(aInPlaceProcessing);
            }
            return PVMFSuccess;
        }


        OsclAny* AllocatePort();
        bool PushPortToVect(PVMFJitterBufferPort*& aPort);
        bool PushPortParamsToQ(PVMFJitterBufferPortParams*& aPortParams);

        // 	PvmfRtcpTimer* iRTCPTimer;
        void RtcpTimerEvent(PvmfRtcpTimer* pTimer);
        uint32 CalcRtcpInterval(PVMFJitterBufferPortParams* pFeedbackPort);
        PVMFStatus ProcessIncomingRTCPReport(PVMFSharedMediaMsgPtr&,
                                             PVMFJitterBufferPortParams*);

        PVMFStatus GenerateRTCPRR(PVMFJitterBufferPortParams* pPort);

        bool LocateFeedBackPort(PVMFJitterBufferPortParams*&,
                                PVMFJitterBufferPortParams*&);

        bool LocateInputPortForFeedBackPort(PVMFJitterBufferPortParams*&,
                                            PVMFJitterBufferPortParams*&);

        PVMFStatus ComposeAndSendFeedBackPacket(PVMFJitterBufferPortParams*&,
                                                PVMFJitterBufferPortParams*&);

        PVMFStatus ComposeAndSendRateAdaptationFeedBackPacket(PVMFJitterBufferPortParams*&,
                PVMFJitterBufferPortParams*&);




        void PVMFJBSessionDurationTimerEvent();
        void ComputeCurrentSessionDurationMonitoringInterval();


        PVMFStatus CreateFireWallPacketMemAllocators(PVMFJitterBufferPortParams*);
        PVMFStatus DestroyFireWallPacketMemAllocators(PVMFJitterBufferPortParams*);

        PVMFStatus SendFireWallPackets();

        bool Allocate(PVMFJitterBufferPortParams* it, PVMFSharedMediaDataPtr& fireWallPkt, OsclSharedPtr<PVMFMediaDataImpl>& mediaDataImpl, const int size);

        PVMFStatus ResetFireWallPacketInfoAndResend();
        void CheckForFireWallRecv(bool &aComplete);
        void CheckForFireWallPacketAttempts(bool &aComplete);
        PVMFStatus DecodeFireWallPackets(PVMFSharedMediaDataPtr aPacket,
                                         PVMFJitterBufferPortParams* aPortParamsPtr);
        void SetJitterBufferSize(uint32 aBufferSz);
        void GetJitterBufferSize(uint32& aBufferSz) const;
        void SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator);
        bool RequestEventCallBack(JB_NOTIFY_CALLBACK aEventType, uint32 aDelay = 0, OsclAny* aContext = NULL);
        void CancelEventCallBack(JB_NOTIFY_CALLBACK aEventType, OsclAny* aContext = NULL);
        void HandleEvent_IncomingMediaInactivityDurationExpired();
        void HandleEvent_NotifyReportBufferingStatus();
        void HandleEvent_JitterBufferBufferingDurationComplete();

        /* Bitstream thinning releated */
        void UpdateRebufferingStats(PVMFEventType aEventType);
        PVMFNodeCapability iCapability;
        PVMFJitterBufferNodeCmdQ iInputCommands;
        PVMFJitterBufferNodeCmdQ iCurrentCommand;
        PVMFPortVector<PVMFJitterBufferPort, OsclMemAllocator> iPortVector;
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator> iPortParamsQueue;
        Oscl_Vector<PVMFPortActivity, OsclMemAllocator> iPortActivityQueue;

        bool	oStartPending;
        bool	oStopOutputPorts;
        uint32	iPauseTime;

        PVMFMediaClock*	ipClientPlayBackClock;

        bool	iMediaReceiveingChannelPrepared;

        PVMFJitterBufferMisc*	ipJitterBufferMisc;

        bool iBroadCastSession;

        JitterBufferFactory* ipJitterBufferFactory;
        /////////////////////////////////////////////////////////////////////////////
        //Following variables are shared with the jitter buffers associated with the session and are passed by refernce
        //Node should only observe these and initialize it, and modification to these are supposed to be done by the
        //jitter buffer only
        bool    iDelayEstablished;
        uint32	iJitterDelayPercent;
        PVMFJitterBufferDataState iJitterBufferState;

        /////////////////////////////////////////////////////////////////////////////
        //Extention interface
        PVMFJitterBufferExtensionInterfaceImpl* ipExtensionInterface;
        //Variables to persist info passed on by the extension interface
        uint32 iRebufferingThreshold;
        uint32 iJitterBufferDurationInMilliSeconds;
        uint32 iMaxInactivityDurationForMediaInMs;
        uint32 iEstimatedServerKeepAheadInMilliSeconds;
        /* resizable reallocator configuration */
        uint32 iJitterBufferSz;
        uint iMaxNumBufferResizes;
        uint iBufferResizeSize;
        uint32 iBufferingStatusIntervalInMs;

        PVMFJitterBufferFireWallPacketInfo iFireWallPacketInfo;
        bool iDisableFireWallPackets;
        bool iPlayingAfterSeek;
        ///////////////////////////////////////////////////////////////////////
        //EventNotification tracking vars
        PVMFJBEventNotifier*	ipEventNotifier;
        uint32 iIncomingMediaInactivityDurationCallBkId;
        bool   iIncomingMediaInactivityDurationCallBkPending;
        uint32 iNotifyBufferingStatusCallBkId;
        bool   iNotifyBufferingStatusCallBkPending;
        uint32 iJitterBufferDurationCallBkId;
        bool   iJitterBufferDurationCallBkPending;

        //////////////////////// Diagnostic log related  //////////////////////
        bool iDiagnosticsLogged;
        uint32 iNumRunL;
        ///////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////
        //Loggers
        PVLogger* ipLogger;
        PVLogger* ipDataPathLogger;
        PVLogger* ipDataPathLoggerIn;
        PVLogger* ipDataPathLoggerOut;
        PVLogger* ipDataPathLoggerFlowCtrl;
        PVLogger* ipClockLogger;
        PVLogger* ipClockLoggerSessionDuration;
        PVLogger* ipClockLoggerRebuff;
        PVLogger* ipDiagnosticsLogger;
        PVLogger* ipJBEventsClockLogger;
        ///////////////////////////////////////////////////////////////////////
};
#endif


