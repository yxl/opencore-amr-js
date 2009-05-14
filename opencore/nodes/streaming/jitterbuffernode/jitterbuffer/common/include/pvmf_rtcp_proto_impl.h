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
#ifndef PVMF_RTCP_PROTO_IMPL_H_INCLUDED
#define PVMF_RTCP_PROTO_IMPL_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_RETURN_CODES_H_INCLUDED
#include "pvmf_return_codes.h"
#endif
#ifndef PVMF_MEDIA_MSG_H_INCLUDED
#include "pvmf_media_msg.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif

#ifndef PVMF_JITTER_BUFFER_H_INCLUDED
#include "pvmf_jitter_buffer.h"
#endif

#ifndef PVMF_RTCP_TIMER_H_INCLUDED
#include "pvmf_rtcp_timer.h"
#endif

#ifndef PVMF_JB_EVENT_NOTIFIER_H
#include "pvmf_jb_event_notifier.h"
#endif

#ifndef OSCL_RAND_H_INCLUDED
#include "oscl_rand.h"
#endif

#ifndef RTCP_H
#include "rtcp.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#include "pvmf_resizable_simple_mediamsg.h"
#endif

#ifndef PVMF_PORT_INTERFACE_H_INCLUDED
#include "pvmf_port_interface.h"
#endif

//May be can add a separate config file
#define MAX_RTCP_SOURCES 31
#define MAX_RTCP_BLOCK_SIZE 512
#define DEFAULT_RTCP_MEM_POOL_BUFFERS 4
#define RTCP_RAND_SEED 666

#define MIN_RTSP_SERVER_INACTIVITY_TIMEOUT_IN_SEC	1
#define MAX_RTSP_SERVER_INACTIVITY_TIMEOUT_IN_SEC	300

// RTCP INTERVAL related
#define DEFAULT_RTCP_INTERVAL_USEC 1000000
#define DEFAULT_RTCP_INTERVAL_SEC  1
#define RTCP_FIXED_INTERVAL_MODE   1
#define PVMF_MEDIA_DATA_CLASS_SIZE 128

///////////////////////////////////////////////////////////////////////////////
//PVMFRTCPStats
///////////////////////////////////////////////////////////////////////////////
class PVMFRTCPStats
{
    public:
        PVMFRTCPStats()
        {
            Reset();
        }

        PVMFRTCPStats(const PVMFRTCPStats& aSrc)
        {
            lastSenderReportNTP = aSrc.lastSenderReportNTP;
            lastSenderReportRTP = aSrc.lastSenderReportRTP;
            lastSenderReportTS = aSrc.lastSenderReportTS;
            lastSenderReportRecvTime = aSrc.lastSenderReportRecvTime;
            packetLossUptoThisRR = aSrc.packetLossUptoThisRR;
            maxSeqNumRecievedUptoThisRR = aSrc.maxSeqNumRecievedUptoThisRR;
            lastRRGenTime = aSrc.lastRRGenTime;
            iLastSenderReportSSRC = aSrc.iLastSenderReportSSRC;
            avg_rtcp_compound_pkt_size = aSrc.avg_rtcp_compound_pkt_size;
            oSRRecvd = aSrc.oSRRecvd;
            oRTCPByeRecvd = aSrc.oRTCPByeRecvd;
        };

        PVMFRTCPStats& operator=(const PVMFRTCPStats& a)
        {
            if (&a != this)
            {
                lastSenderReportNTP = a.lastSenderReportNTP;
                lastSenderReportRTP = a.lastSenderReportRTP;
                lastSenderReportTS = a.lastSenderReportTS;
                lastSenderReportRecvTime = a.lastSenderReportRecvTime;
                packetLossUptoThisRR = a.packetLossUptoThisRR;
                maxSeqNumRecievedUptoThisRR = a.maxSeqNumRecievedUptoThisRR;
                lastRRGenTime = a.lastRRGenTime;
                iLastSenderReportSSRC = a.iLastSenderReportSSRC;
                avg_rtcp_compound_pkt_size = a.avg_rtcp_compound_pkt_size;
                oSRRecvd = a.oSRRecvd;
                oRTCPByeRecvd = a.oRTCPByeRecvd;
            }
            return *this;
        }

        void Reset()
        {
            lastSenderReportNTP = 0;
            lastSenderReportRTP = 0;
            lastSenderReportTS = 0;
            lastSenderReportRecvTime = 0;
            packetLossUptoThisRR = 0;
            maxSeqNumRecievedUptoThisRR = 0;
            lastRRGenTime = 0;
            iLastSenderReportSSRC = 0;
            avg_rtcp_compound_pkt_size = 0;
            oSRRecvd = false;
            oRTCPByeRecvd = false;
        }



        uint64	lastSenderReportNTP;
        uint32	lastSenderReportRTP;
        uint32	lastSenderReportTS;
        uint64	lastSenderReportRecvTime;
        int32	packetLossUptoThisRR;
        int32   maxSeqNumRecievedUptoThisRR;
        uint64	lastRRGenTime;
        uint32  iLastSenderReportSSRC;
        float	avg_rtcp_compound_pkt_size;
        bool    oSRRecvd;
        bool    oRTCPByeRecvd;
};

class RTPRateAdaptationInfo
{
    public:
        RTPRateAdaptationInfo(): iRateAdaptation(false)
                , iRateAdaptationFeedBackFrequency(0)
                , iRateAdaptationRTCPRRCount(0)
                , iRateAdaptationFreeBufferSpaceInBytes(0)
        {
        }
        bool	iRateAdaptation;
        uint32	iRateAdaptationFeedBackFrequency;
        uint32	iRateAdaptationRTCPRRCount;
        uint32	iRateAdaptationFreeBufferSpaceInBytes;
};

class PVRTCPChannelController;
///////////////////////////////////////////////////////////////////////////////
//PVRTCPChannelControllerObserver
///////////////////////////////////////////////////////////////////////////////
class PVRTCPChannelControllerObserver
{
    public:
        virtual PVMFStatus RTCPSRReveived(PVRTCPChannelController* channelController) = 0;
        virtual PVMFStatus RTCPByeReceived(PVRTCPChannelController* channelController) = 0;
        virtual PVMFStatus RTCPRRReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//PVRTCPChannelController
///////////////////////////////////////////////////////////////////////////////
class PVRTCPChannelController: public PvmfRtcpTimerObserver
{
    public:
        static PVRTCPChannelController* New(PVRTCPChannelControllerObserver* aObserver, PVMFJitterBuffer& aRTPJitterBuffer, PVMFPortInterface* aFeedbackPort, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aRTCPClock);
        ~PVRTCPChannelController();

        void Reset();

        PVMFJitterBuffer& GetJitterBuffer()
        {
            return irRTPDataJitterBuffer;
        }

        void StartRTCPMessageExchange();
        void PauseRTCPMessageExchange();
        void StopRTCPMessageExchange();

        void SetRateAdaptation(bool aRateAdaptation, uint32 aRateAdaptationFeedBackFrequency, uint32 aRateAdaptationFreeBufferSpaceInBytes);
        void SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval);
        bool SetPortRTCPParams(int aNumSenders, uint32 aRR, uint32 aRS)
        {
            iNumSenders = aNumSenders;
            iRR = aRR;
            iRS = aRS;
            iRtcpBwConfigured = true;
            return true;
        }
        PVMFStatus ProcessRTCPReport(PVMFSharedMediaMsgPtr& aMsg);
        bool CheckAssociatedFeedbackPort(const PVMFPortInterface* aFeedbackPort)const
        {
            return (ipFeedbackPort == aFeedbackPort);
        }

        void SetMediaClockConverter(MediaClockConverter* aClockConverter)
        {
            ipMediaClockConverter = aClockConverter;
        }

        void Prepare(bool aAfterSeek)
        {
            if (aAfterSeek)
            {
                iRTCPStats.oRTCPByeRecvd = false;
            }
            if (iRtcpBwConfigured && (iRR == 0))
            {
                ipRTCPTimer->setRTCPInterval(0);
                return;
            }
        }

        const PVMFRTCPStats& GetRTCPStats()
        {
            return iRTCPStats;
        }

        virtual void RtcpTimerEvent();
    private:
        void Construct();
        PVRTCPChannelController(PVRTCPChannelControllerObserver* aObserver, PVMFJitterBuffer& aRTPJitterBuffer, PVMFPortInterface* aFeedbackPort, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aRTCPClock);
        PVRTCPChannelController& operator =(const PVRTCPChannelController&);
        PVRTCPChannelController(const PVRTCPChannelController&);
        void ResetParams(bool aMemoryCleanUp = false);
        PVMFStatus ComposeFeedBackPacket();
        uint32 CalcRtcpInterval();
        PVMFStatus GenerateRTCPRR();
        PVMFResizableSimpleMediaMsgAlloc*	CreateRTCPRRBufAllocReSize();

        PVRTCPChannelControllerObserver*	ipObserver;
        PVMFJitterBuffer&					irRTPDataJitterBuffer;
        PVMFPortInterface*					ipFeedbackPort;
        PVMFMediaClock&						irClientPlayBackClock;
        PVMFMediaClock&						irRTCPClock;

        RTPRateAdaptationInfo				iRTPChannelRateAdaptationInfo;
        PVMFRTCPStats						iRTCPStats;
        int									iNumSenders;
        uint32								iRR;
        uint32								iRS;
        bool								iRtcpBwConfigured;
        bool								iInitialRTCPIntervalComputation;
        uint32								iRTCPIntervalInMicroSeconds;
        bool								iInitialRTCPPacket;
        PvmfRtcpTimer*						ipRTCPTimer;
        OsclRand							iRandGen;
        MediaClockConverter*				ipMediaClockConverter;

        PVLogger*							ipRTCPDataPathLoggerIn;
        PVLogger*							ipRTCPDataPathLoggerOut;
};

///////////////////////////////////////////////////////////////////////////////
//PVRTCPProtoImplementorObserver
///////////////////////////////////////////////////////////////////////////////
class PVRTCPProtoImplementorObserver
{
    public:
        virtual PVMFStatus RTCPPacketReceived(RTCPPacketType aPacketType, PVRTCPChannelController* aController) = 0;
        virtual PVMFStatus RTCPReportReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage) = 0;
        virtual PVMFStatus ProcessInfoEvent(PVMFAsyncEvent& aEvent) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//PVRTCPProtoImplementor
///////////////////////////////////////////////////////////////////////////////
class PVRTCPProtoImplementor: public PVRTCPChannelControllerObserver
{
    public:
        OSCL_IMPORT_REF static PVRTCPProtoImplementor* New(PVMFMediaClock& aClientPlayBackClock, PVMFMediaClock& aRTCPClock, PVRTCPProtoImplementorObserver* aObs, bool aBroadcastSession);
        OSCL_IMPORT_REF ~PVRTCPProtoImplementor();

        OSCL_EXPORT_REF void AddPVRTCPChannelController(PVRTCPChannelController* aPVRTCPChannelController);
        OSCL_EXPORT_REF void RemoveAllRTCPChannelControllers();
        OSCL_EXPORT_REF PVRTCPChannelController* GetRTCPChannelController(const PVMFPortInterface* aPort) const;

        OSCL_IMPORT_REF void StartRTCPMessageExchange();
        OSCL_IMPORT_REF void PauseRTCPMessageExchange();
        OSCL_IMPORT_REF void StopRTCPMessageExchange();

        OSCL_IMPORT_REF void Reset();

        OSCL_IMPORT_REF PVMFStatus ProcessRTCPReport(PVMFPortInterface* aFeedbackPort, PVMFSharedMediaMsgPtr& aMsg);
        OSCL_IMPORT_REF void SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval);
        OSCL_IMPORT_REF bool SetPortRTCPParams(PVMFPortInterface* aPort, int aNumSenders, uint32 aRR, uint32 aRS);

        OSCL_IMPORT_REF void Prepare(bool aPlayAfterSeek);


        bool RTCPByeReceivedOnAllControllers()
        {
            PVRTCPChannelController* rtcpChannelController = NULL;
            Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
            for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
            {
                rtcpChannelController = *iter;
                if (!(rtcpChannelController->GetRTCPStats().oRTCPByeRecvd))
                {

                    return false;
                }
            }
            return true;
        }

        //Implementation of PVRTCPChannelControllerObserver
        PVMFStatus RTCPSRReveived(PVRTCPChannelController* aChannelController);
        PVMFStatus RTCPByeReceived(PVRTCPChannelController* aChannelController);
        PVMFStatus RTCPRRReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage);
    private:
        PVRTCPProtoImplementor(PVMFMediaClock& aClientPlayBackClock, PVMFMediaClock& aRTCPClock, PVRTCPProtoImplementorObserver* aObs, bool aBroadcastSession);
        void ResetParams(bool aMemoryCleanUp = false);
        bool ProcessRTCPSRforAVSync();
        void Construct();

        const bool						iBroadcastSession;
        PVMFMediaClock&					irClientPlayBackClock;
        PVMFMediaClock&					irRTCPClock;
        PVRTCPProtoImplementorObserver*	ipObserver;

        bool							iPerformRTCPBasedAVSync;
        bool							iRTCPAVSyncProcessed;
        bool							iPlayStopTimeAvailable;
        Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>	iPVRTCPChannelController;

        PVLogger*	ipRTCPDataPathLoggerIn;
        PVLogger*	ipRTCPAVSyncLogger;
};

#endif
