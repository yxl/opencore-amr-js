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
#ifndef PVMF_JITTER_BUFFER_COMMON_TYPES_H_INCLUDED
#define PVMF_JITTER_BUFFER_COMMON_TYPES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#include "pvmf_sm_tunables.h"
#endif

//Jitter buffer node specific leave codes
#define JBPacketRegistrationConfigurationCorrupted	1000


enum PVMFJitterBufferDataState
{
    PVMF_JITTER_BUFFER_STATE_UNKNOWN,
    PVMF_JITTER_BUFFER_READY,
    PVMF_JITTER_BUFFER_IN_TRANSITION
};

///////////////////////////////////////////////////////////////////////////////
//Specific to RTP streaming session
///////////////////////////////////////////////////////////////////////////////
typedef struct tagPVMFRTPInfoParams
{
    tagPVMFRTPInfoParams()
    {
        seqNumBaseSet = false;
        seqNum = 0;
        rtpTimeBaseSet = false;
        rtpTime = 0;
        nptTimeBaseSet = false;
        nptTimeInMS = 0;
        rtpTimeScale = 0;
        nptTimeInRTPTimeScale = 0;
        isPlayAfterPause = false;
    };

    bool   seqNumBaseSet;
    uint32 seqNum;
    bool   rtpTimeBaseSet;
    uint32 rtpTime;
    uint32 nptTimeInMS;
    bool   nptTimeBaseSet;
    uint32 rtpTimeScale;
    uint32 nptTimeInRTPTimeScale;
    bool   isPlayAfterPause;
} PVMFRTPInfoParams;

///////////////////////////////////////////////////////////////////////////////
//RTSP based streaming specific data structures
///////////////////////////////////////////////////////////////////////////////
enum PVMFJitterBufferFireWallPacketFormat
{
    PVMF_JB_FW_PKT_FORMAT_RTP,
    PVMF_JB_FW_PKT_FORMAT_PV
};

class PVMFJitterBufferFireWallPacketInfo
{
    public:
        PVMFJitterBufferFireWallPacketInfo()
        {
            iServerRoundTripDelayInMS =
                PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_DEFAULT_SERVER_RESPONSE_TIMEOUT_IN_MS;
            iNumAttempts =
                PVMF_JITTER_BUFFER_NODE_DEFAULT_FIREWALL_PKT_ATTEMPTS;
            iFormat = PVMF_JB_FW_PKT_FORMAT_RTP;
        };

        virtual ~PVMFJitterBufferFireWallPacketInfo()
        {
        };

        uint32 iServerRoundTripDelayInMS;
        uint32 iNumAttempts;
        PVMFJitterBufferFireWallPacketFormat iFormat;
};


typedef enum
{
    PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNKNOWN,
    PVMF_JB_REGISTER_MEDIA_MSG_ERR_CORRUPT_PACKET,
    PVMF_JB_REGISTER_MEDIA_MSG_ERR_LATE_MSG,
    PVMF_JB_BUFFER_REGISTER_MEDIA_MSG_ERR_CONFIG_NOT_SUPPORTED,
    PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_JB_FULL,
    PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_INSUFFICIENT_MEMORY_FOR_PACKETIZATION,
    PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNEXPECTED_DATA,
    PVMF_JB_REGISTER_MEDIA_MSG_ERR_EOS_SIGNALLED,
    PVMF_JB_REGISTER_MEDIA_MSG_ERR_INVALID_HEADER,
    PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS
} PVMFJitterBufferRegisterMediaMsgStatus;

class PVMFJBEventNotifier;
class PVMFMediaClock;

enum JB_NOTIFY_CALLBACK
{
    JB_INCOMING_MEDIA_INACTIVITY_DURATION_EXPIRED,
    JB_NOTIFY_REPORT_BUFFERING_STATUS,
    JB_BUFFERING_DURATION_COMPLETE,
    JB_MONITOR_REBUFFERING,
    JB_NOTIFY_SEND_FIREWALL_PACKET,
    JB_NOTIFY_WAIT_FOR_OOO_PACKET_COMPLETE  //OOO->Out Of Order
};

class PVMFJitterBufferObserver;
///////////////////////////////////////////////////////////////////////////////
//PVMFJitterBufferConstructParams
///////////////////////////////////////////////////////////////////////////////
class PVMFJitterBufferConstructParams
{
    public:
        PVMFJitterBufferConstructParams(PVMFMediaClock& aEstimatedServerClock
                                        , PVMFMediaClock& aClientPlaybackClock
                                        , OSCL_HeapString<OsclMemAllocator>& aMimeType
                                        , PVMFJBEventNotifier& aJBEventNotifier
                                        , bool& aDelayEstablished
                                        , uint32& aJitterDelayPercent
                                        , PVMFJitterBufferDataState& aJitterBufferDataState
                                        , PVMFJitterBufferObserver* const aObserver
                                        , OsclAny* const aContext)
                : irEstimatedServerClock(aEstimatedServerClock)
                , irClientPlaybackClock(aClientPlaybackClock)
                , irMimeType(aMimeType)
                , irJBEventNotifier(aJBEventNotifier)
                , irDelayEstablished(aDelayEstablished)
                , irJitterDelayPercent(aJitterDelayPercent)
                , irJitterBufferState(aJitterBufferDataState)
                , ipObserver(aObserver)
                , ipContextData(aContext)
        {}

        PVMFMediaClock&	GetEstimatedServerClock() const
        {
            return irEstimatedServerClock;
        }

        PVMFMediaClock&	GetClientPlaybackClock() const
        {
            return irClientPlaybackClock;
        }

        OSCL_HeapString<OsclMemAllocator>&	GetMimeType() const
        {
            return irMimeType;
        }

        bool& GetDelayEstablishStatus() const
        {
            return irDelayEstablished;
        }

        uint32& GetJBDelayPercent() const
        {
            return irJitterDelayPercent;
        }

        PVMFJBEventNotifier& GetEventNotifier() const
        {
            return irJBEventNotifier;
        }

        PVMFJitterBufferObserver* const GetJBObserver() const
        {
            return ipObserver;
        }

        OsclAny* const	GetContextData() const
        {
            return ipContextData;
        }

        PVMFJitterBufferDataState& GetJitterBufferState() const
        {
            return irJitterBufferState;
        }

    private:
        PVMFMediaClock&	irEstimatedServerClock;
        PVMFMediaClock&	irClientPlaybackClock;
        OSCL_HeapString<OsclMemAllocator>& irMimeType;
        PVMFJBEventNotifier&	irJBEventNotifier;
        bool& irDelayEstablished;
        uint32& irJitterDelayPercent;
        PVMFJitterBufferDataState&	irJitterBufferState;
        PVMFJitterBufferObserver *const	ipObserver;
        OsclAny* const	ipContextData;
};

///////////////////////////////////////////////////////////////////////////////
//RTPSessionInfoForFirewallExchange
///////////////////////////////////////////////////////////////////////////////
class PVMFPortInterface;
class RTPSessionInfoForFirewallExchange
{
    public:
        RTPSessionInfoForFirewallExchange(): ipRTPDataJitterBufferPort(NULL), iSSRC(0) {}
        RTPSessionInfoForFirewallExchange(const RTPSessionInfoForFirewallExchange& aRTPSessionInfo)
        {
            ipRTPDataJitterBufferPort = aRTPSessionInfo.ipRTPDataJitterBufferPort;
            iSSRC = aRTPSessionInfo.iSSRC;
        }
        RTPSessionInfoForFirewallExchange(PVMFPortInterface* aInputPort, uint32 aSSRC): ipRTPDataJitterBufferPort(aInputPort)
                , iSSRC(aSSRC) {}

        PVMFPortInterface* ipRTPDataJitterBufferPort;
        uint32 iSSRC;
};

/**
 * An enumeration of info codes from PVMFJitterBufferNode
 **/
typedef enum
{
    PVMFJitterBufferNodeInfoEventStart = 8192,
    PVMFJitterBufferNodeJitterBufferFull,
    PVMFJitterBufferNodeTrackEOSReached,
    PVMFJitterBufferNodeExcercisingPortFlowControl,
    PVMFJitterBufferNodeRTCPBYERecvd,
    PVMFJitterBufferNodeJitterBufferLowWaterMarkReached,
    PVMFJitterBufferNodeJitterBufferHighWaterMarkReached,
    PVMFJitterBufferNodeStreamThinningRecommended,
    PVMFJitterBufferNodeRTCPDataProcessed,
    PVMFJitterBufferNodeInfoEventEnd
} PVMFJitterBufferNodeInfoEventType;

#endif
