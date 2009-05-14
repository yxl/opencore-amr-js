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
#include "pvmf_rtcp_proto_impl.h"
#endif

#ifndef RTCP_DECODER_H
#include "rtcp_decoder.h"
#endif
#ifndef RTCP_ENCODER_H
#include "rtcp_encoder.h"
#endif

#ifndef OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif

#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//PVRTCPChannelController Implementation
///////////////////////////////////////////////////////////////////////////////
PVRTCPChannelController* PVRTCPChannelController::New(PVRTCPChannelControllerObserver* aObserver, PVMFJitterBuffer& aRTPJitterBuffer, PVMFPortInterface* aFeedbackPort, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aRTCPClock)
{
    PVRTCPChannelController* rtcpChannelController = NULL;
    int32 err = OsclErrNone;
    OSCL_TRY(err, rtcpChannelController = OSCL_NEW(PVRTCPChannelController, (aObserver, aRTPJitterBuffer, aFeedbackPort, aClientPlaybackClock, aRTCPClock));
             rtcpChannelController->Construct(););
    if (err != OsclErrNone && rtcpChannelController)
    {
        OSCL_DELETE(rtcpChannelController);
        rtcpChannelController = NULL;
    }
    return rtcpChannelController;
}

PVRTCPChannelController::PVRTCPChannelController(PVRTCPChannelControllerObserver* aObserver, PVMFJitterBuffer& aRTPJitterBuffer, PVMFPortInterface* aFeedbackPort, PVMFMediaClock& aClientPlaybackClock, PVMFMediaClock& aRTCPClock)
        : ipObserver(aObserver)
        , irRTPDataJitterBuffer(aRTPJitterBuffer)
        , ipFeedbackPort(aFeedbackPort)
        , irClientPlayBackClock(aClientPlaybackClock)
        , irRTCPClock(aRTCPClock)
{
    ipRTCPDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp.in");
    ipRTCPDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp.out");

    ipRTCPTimer	=	NULL;
    iRandGen.Seed(RTCP_RAND_SEED);
    ResetParams();
}

void PVRTCPChannelController::ResetParams(bool aMemoryCleanUp)
{
    OSCL_UNUSED_ARG(aMemoryCleanUp);
    iRTPChannelRateAdaptationInfo.iRateAdaptation = false;
    iRTPChannelRateAdaptationInfo.iRateAdaptationFeedBackFrequency = 0;
    iRTPChannelRateAdaptationInfo.iRateAdaptationRTCPRRCount = 0;
    iRTPChannelRateAdaptationInfo.iRateAdaptationFreeBufferSpaceInBytes = 0;
    iRTCPStats.Reset();
    iNumSenders = 0;
    iRR = 0;
    iRS = 0;
    iRtcpBwConfigured = false;
    iInitialRTCPIntervalComputation = false;
    iRTCPIntervalInMicroSeconds = DEFAULT_RTCP_INTERVAL_USEC;
    iInitialRTCPPacket = true;
    ipMediaClockConverter	=	NULL;
}

void PVRTCPChannelController::Construct()
{
    int32 err = OsclErrNone;
    OSCL_TRY(err, ipRTCPTimer = OSCL_NEW(PvmfRtcpTimer, (this)););
    if (err != OsclErrNone || !ipRTCPTimer)
    {
        OSCL_LEAVE(PVMFErrNoResources);
    }

}

PVRTCPChannelController::~PVRTCPChannelController()
{
    Reset();
    if (ipRTCPTimer)
        OSCL_DELETE(ipRTCPTimer);
}
void PVRTCPChannelController::StartRTCPMessageExchange()
{
    //noop
    iRTCPIntervalInMicroSeconds = CalcRtcpInterval();
    ipRTCPTimer->setRTCPInterval(iRTCPIntervalInMicroSeconds);
    ipRTCPTimer->Start();
}

void PVRTCPChannelController::PauseRTCPMessageExchange()
{
    if (ipRTCPTimer)
        ipRTCPTimer->Cancel();
}

void PVRTCPChannelController::StopRTCPMessageExchange()
{
    if (ipRTCPTimer)
        ipRTCPTimer->Cancel();
}

void PVRTCPChannelController::Reset()
{
    if (ipRTCPTimer)
        ipRTCPTimer->Stop();
    ResetParams(true);
}

void PVRTCPChannelController::SetRateAdaptation(bool aRateAdaptation, uint32 aRateAdaptationFeedBackFrequency, uint32 aRateAdaptationFreeBufferSpaceInBytes)
{
    iRTPChannelRateAdaptationInfo.iRateAdaptation = aRateAdaptation;
    iRTPChannelRateAdaptationInfo.iRateAdaptationFeedBackFrequency = aRateAdaptationFeedBackFrequency;
    iRTPChannelRateAdaptationInfo.iRateAdaptationFreeBufferSpaceInBytes = aRateAdaptationFreeBufferSpaceInBytes;
}

void PVRTCPChannelController::SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval)
{
    iRTCPIntervalInMicroSeconds = aRTCPInterval;
}

PVMFStatus PVRTCPChannelController::ProcessRTCPReport(PVMFSharedMediaMsgPtr& aMsg)
{
    RTCP_Decoder rtcpDec;
    RTCP_SR rtcpSR;
    RTCP_BYE rtcpBye;
    RTCP_Decoder::Error_t retval;
    int32 max_array_size = MAX_RTCP_SOURCES;
    RTCPPacketType array_of_packet_types[MAX_RTCP_SOURCES];
    OsclMemoryFragment array_of_packets[MAX_RTCP_SOURCES];
    int32 filled_size = 0;

    PVMFSharedMediaDataPtr rtcpDataIn;
    convertToPVMFMediaData(rtcpDataIn, aMsg);

    const uint32 numFrags = rtcpDataIn->getNumFragments();

    //Process each RTCP packet.
    //Typically, only one is received at a time.
    for (uint32 i = 0; i < numFrags; i++)
    {
        OsclRefCounterMemFrag memfrag;

        //Get the next memory fragment from the media message.
        if (rtcpDataIn->getMediaFragment(i, memfrag) == false)
        {
            return PVMFFailure;
        }

        //Get the pointer to the packet.
        OsclMemoryFragment receivedMsg = memfrag.getMemFrag();

        /* Find out what type of RTCP packet we have */
        //This populates the variables "filled_size", "array_of_packet_types", and "array_of_packets"
        //by breaking up compound RTCP packets into individual reports.
        rtcpDec.scan_compound_packet(receivedMsg,
                                     max_array_size,
                                     filled_size,
                                     array_of_packet_types,
                                     array_of_packets);

        // update packet size averages - we treat the compound packet
        // as a single packet
        iRTCPStats.avg_rtcp_compound_pkt_size = OSCL_STATIC_CAST(float, (receivedMsg.len + 15.0 * iRTCPStats.avg_rtcp_compound_pkt_size) / 16.0);

        //Process each individual report.
        for (int32 ii = 0; ii < filled_size; ii++)
        {
            /* Use the appropriate decoder */

            //If the RTCP type indicates a Sender Report...
            if (SR_RTCP_PACKET == array_of_packet_types[ii])
            {
                PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingRTCPReport - Sender Report"));
                //Decode the Sender Report.
                retval = rtcpDec.DecodeSR(array_of_packets[ii], rtcpSR);
                if (RTCP_Decoder::FAIL == retval)
                {
                    return PVMFFailure;
                }

                uint32 tsOffset = 0;
                if (irRTPDataJitterBuffer.GetRTPTimeStampOffset(tsOffset))
                {
                    if (rtcpSR.senderSSRC !=  irRTPDataJitterBuffer.GetSSRC())
                    {
                        PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "PVRTCPChannelController::ProcessRTCPReport - RTCP ssrc %u JB SSRC %u RTCP report is invalid (from some other session), ignoring it", rtcpSR.senderSSRC, irRTPDataJitterBuffer.GetSSRC()));
                        return PVMFSuccess;
                    }
                }

                iRTCPStats.iLastSenderReportSSRC = rtcpSR.senderSSRC;

                iRTCPStats.lastSenderReportTS =
                    (rtcpSR.NTP_timestamp_high << 16) |
                    ((rtcpSR.NTP_timestamp_low >> 16) & 0x0000ffff);

                /*
                 * Get RTCP Recv Time in milliseconds
                 */
                uint32 srRecvTime;
                bool overflowFlag = false;
                irRTCPClock.GetCurrentTime32(srRecvTime, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);

                iRTCPStats.lastSenderReportRecvTime = srRecvTime;

                //Save the NTP and RTP timestamps for later calculations...
                iRTCPStats.lastSenderReportNTP =
                    (((uint64)rtcpSR.NTP_timestamp_high) << 32) + (uint64)rtcpSR.NTP_timestamp_low;
                iRTCPStats.lastSenderReportRTP = rtcpSR.RTP_timestamp;
                PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingRTCPReport - Sender Report TS iRTCPStats.lastSenderReportRTP %u, iRTCPStats.iLastSenderReportSSRC %u ", rtcpSR.RTP_timestamp, iRTCPStats.iLastSenderReportSSRC));
                PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingRTCPReport - Sender Report NPT rtcpSR.NTP_timestamp_high %u rtcpSR.NTP_timestamp_low %u SR Ts %u ", rtcpSR.NTP_timestamp_high, rtcpSR.NTP_timestamp_low , iRTCPStats.lastSenderReportTS));

                ipObserver->RTCPSRReveived(this);
            }

            //If the RTCP type is BYE, set the end-of-stream flag.
            if (BYE_RTCP_PACKET == array_of_packet_types[ii])
            {
                PVMF_JB_LOG_RTCPDATATRAFFIC_IN_E((0, "PVMFJitterBufferNode::ProcessIncomingRTCPReport-  RTCP_BYE_RECVD: Mime=%s", irRTPDataJitterBuffer.GetMimeType()));
                retval = rtcpDec.DecodeBYE(array_of_packets[ii], rtcpBye);
                if (RTCP_Decoder::FAIL == retval)
                {
                    return PVMFFailure;
                }
                /* The packet is a RTCP BYE, set the end of stream flag */
                else if (retval == RTCP_Decoder::RTCP_SUCCESS)
                {
                    iRTCPStats.oRTCPByeRecvd = true;
                    ipObserver->RTCPByeReceived(this);
                }
            }

            //All other RTCP types (Receiver Reports) are ignored.
            iRTCPStats.oSRRecvd = true;
        }
    }
    return PVMFSuccess;
}

uint32 PVRTCPChannelController::CalcRtcpInterval()
{
    float interval;

#if RTCP_FIXED_INTERVAL_MODE
    interval = DEFAULT_RTCP_INTERVAL_SEC;
#else
    float rtcp_min_time = iInitialRTCPIntervalComputation ? (float)DEFAULT_RTCP_INTERVAL_SEC / 2.0 :
                          (float)DEFAULT_RTCP_INTERVAL_SEC;

    if (iRtcpBwConfigured && (iRR > 0))
    {
        float divisor = (float)iRR;
        if (iRR > iRS)
        {
            divisor = (float)iRS;
        }
        interval = iRTCPStats.avg_rtcp_compound_pkt_size * 8 *
                   ((float)iNumSenders + 1) / divisor;

        if (interval < rtcp_min_time)
        {
            interval = rtcp_min_time;
        }
    }
    else
    {
        interval = rtcp_min_time;
    }
#endif

    // generate a random number on [0, 1000)
    uint32 n = iRandGen.Rand() % 1000;

    // map the number onto the range [0.5, 1.5)
    float window = OSCL_STATIC_CAST(float, (0.5 + ((float)n) / 1000.0));

    // generate the actual interval, in seconds
    float interval_scaled = OSCL_STATIC_CAST(float, interval * window / 1.21828);

    // return the interval in microseconds
    return (uint32)(interval_scaled * 1000000);
}

void PVRTCPChannelController::RtcpTimerEvent()
{
    if (iRTCPStats.oRTCPByeRecvd == false)
    {
        // timer reconsideration
        uint32 timer = CalcRtcpInterval();
        PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "%x timer %d iRTCPIntervalInMicroSeconds %d", this, timer, iRTCPIntervalInMicroSeconds));
        if (timer > iRTCPIntervalInMicroSeconds)
        {
            ipRTCPTimer->setRTCPInterval(timer - iRTCPIntervalInMicroSeconds);
            ipRTCPTimer->Start();
            iRTCPIntervalInMicroSeconds = timer;
            return;
        }
        else
        {
            GenerateRTCPRR();
        }
    }
}

PVMFStatus PVRTCPChannelController::GenerateRTCPRR()
{
    if (ipFeedbackPort->IsOutgoingQueueBusy() == false)
    {
        ComposeFeedBackPacket();
    }

    /* Reschedule the RTCP timer for the next interval */
    iRTCPIntervalInMicroSeconds = CalcRtcpInterval();
    PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "%x GeneratedRTCPRR", this));
    ipRTCPTimer->setRTCPInterval(iRTCPIntervalInMicroSeconds);
    ipRTCPTimer->Start();
    return PVMFSuccess;
}

PVMFStatus PVRTCPChannelController::ComposeFeedBackPacket()
{
    uint32 senderSSRC;
    RTCP_Encoder rtcpEncode;
    RTCP_ReportBlock *reportBlock = NULL;
    RTCP_RR *pRR = NULL;
    OsclExclusivePtr<RTCP_RR> rtcpRRAutoPtr;

    pRR = OSCL_NEW(RTCP_RR, (1));

    if (NULL == pRR)
    {
        return PVMFErrNoMemory;
    }
    rtcpRRAutoPtr.set(pRR);

    reportBlock = pRR->get_report_block(0);
    if (NULL == reportBlock)
    {
        return PVMFErrNoMemory;
    }

    /* Get Jitter Buffer Stats from RTP port */
    PVMFJitterBufferStats jbStats =
        irRTPDataJitterBuffer.getJitterBufferStats();

    /* Get InterArrivalJitter from RTP port */
    uint32 interArrivalJitter =
        irRTPDataJitterBuffer.getInterArrivalJitter();

    uint32 sourceSSRC32 = jbStats.ssrc;
    senderSSRC = (sourceSSRC32 >> 10) | (sourceSSRC32 << 22);

    pRR->senderSSRC = senderSSRC;
    rtcpEncode.SetSSRC(senderSSRC);
    reportBlock->sourceSSRC = sourceSSRC32;

    /* Compute packet loss fraction */
    if (iRTCPStats.maxSeqNumRecievedUptoThisRR == 0)
    {
        iRTCPStats.maxSeqNumRecievedUptoThisRR =
            jbStats.seqNumBase;
    }
    if (jbStats.maxSeqNumReceived -
            iRTCPStats.maxSeqNumRecievedUptoThisRR)
    {
        reportBlock->fractionLost =
            (int8)(((jbStats.totalPacketsLost - iRTCPStats.packetLossUptoThisRR) * 256) /
                   (jbStats.maxSeqNumReceived - iRTCPStats.maxSeqNumRecievedUptoThisRR));
    }
    else
    {
        reportBlock->fractionLost = 0;
    }

    reportBlock->cumulativeNumberOfPacketsLost = jbStats.totalPacketsLost;
    reportBlock->highestSequenceNumberReceived = jbStats.maxSeqNumReceived;
    reportBlock->interarrivalJitter = interArrivalJitter;
    reportBlock->lastSR = iRTCPStats.lastSenderReportTS;

    if (iRTCPStats.oSRRecvd)
    {
        uint32 currRRGenTime;
        bool overflowFlag = false;

        irRTCPClock.GetCurrentTime32(currRRGenTime, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);

        uint32 lastSenderReportRecvTime = (uint32)
                                          iRTCPStats.lastSenderReportRecvTime;

        uint32 delaySinceLastSR64 =
            (currRRGenTime - lastSenderReportRecvTime);

        uint32 delaySinceLastSR32 = delaySinceLastSR64;

        reportBlock->delaySinceLastSR = (delaySinceLastSR32 << 16) / 1000;

        iRTCPStats.lastRRGenTime = currRRGenTime;
    }

    /* Update variables for the next RR cycle */
    iRTCPStats.maxSeqNumRecievedUptoThisRR =
        jbStats.maxSeqNumReceived;
    iRTCPStats.packetLossUptoThisRR =
        jbStats.totalPacketsLost;


    PVMFSharedMediaDataPtr rtcpOut;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
    PVMFRTCPMemPool* rtcpBufAlloc = ipRTCPTimer->getRTCPBuffAlloc();
    if (!rtcpBufAlloc->ipMediaDataMemPool)
    {
        return PVMFErrNoMemory;
    }
    int32 err;

    OSCL_TRY(err,
             mediaDataImpl = rtcpBufAlloc->GetMediaDataImpl(MAX_RTCP_BLOCK_SIZE);
             rtcpOut = PVMFMediaData::createMediaData(mediaDataImpl,
                       (rtcpBufAlloc->ipMediaDataMemPool));
            );

    if (err != OsclErrNone)
    {
        return PVMFErrNoMemory;
    }

    /* Retrieve memory fragment to write to */
    OsclRefCounterMemFrag refCtrMemFragOut;
    rtcpOut->getMediaFragment(0, refCtrMemFragOut);

    OsclMemoryFragment memFrag = refCtrMemFragOut.getMemFrag();
    memFrag.len = MAX_RTCP_BLOCK_SIZE;

    RTCP_APP *appPtr = NULL;
    RTCP_APP App;

    PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "RTCP_PKT: Mime=%s, MaxSNRecvd=%d, MaxTSRecvd=%d, MaxSNRet=%d, MaxTSRet=%d", irRTPDataJitterBuffer.GetMimeType(), jbStats.maxSeqNumReceived, jbStats.maxTimeStampRegistered, jbStats.lastRetrievedSeqNum, jbStats.maxTimeStampRetrieved));
    /*
     * If Rate Adaptation is enabled and we have received some RTP packets, then send NADU APP packet,
     * if frequency criteria is met
     */
    if (iRTPChannelRateAdaptationInfo.iRateAdaptation && (jbStats.totalNumPacketsReceived > 0))
    {
        iRTPChannelRateAdaptationInfo.iRateAdaptationRTCPRRCount++;
        if (iRTPChannelRateAdaptationInfo.iRateAdaptationRTCPRRCount ==
                iRTPChannelRateAdaptationInfo.iRateAdaptationFeedBackFrequency)
        {
            oscl_memcpy(App.type, PSS0_APP_RTCP_NAME, oscl_strlen(PSS0_APP_RTCP_NAME));
            App.ssrc = senderSSRC;
            App.subtype = RTCP_NADU_APP_SUBTYPE;
            App.pss0_app_data.sourcessrc = sourceSSRC32;
            PVMFTimestamp converted_ts = 0;
            //set playoutdelay to 0xffff by default, if JB is empty we will use this
            uint32 diff32 = RTCP_NADU_APP_DEFAULT_PLAYOUT_DELAY;
            uint32 clientClock32 = 0;
            uint32 timebase32 = 0;
            bool overflowFlag = false;

            irClientPlayBackClock.GetCurrentTime32(clientClock32, overflowFlag,
                                                   PVMF_MEDIA_CLOCK_MSEC,
                                                   timebase32);
            PVMFTimestamp tsOfNextPacketToBeDecoded = jbStats.maxTimeStampRetrievedWithoutRTPOffset;
            if (jbStats.currentOccupancy > 0)
            {
                tsOfNextPacketToBeDecoded =
                    irRTPDataJitterBuffer.peekNextElementTimeStamp();

                uint32 in_wrap_count = 0;
                /*
                 * Convert Time stamp to milliseconds
                 */
                ipMediaClockConverter->set_clock(tsOfNextPacketToBeDecoded, in_wrap_count);
                converted_ts =
                    ipMediaClockConverter->get_converted_ts(1000);

                //ts should be ahead of clock
                //if not we are falling behind on one track, so set playout delay to zero
                diff32 = 0;
                bool clkEarly =
                    PVTimeComparisonUtils::IsEarlier(clientClock32,
                                                     converted_ts,
                                                     diff32);
                if (clkEarly == false)
                {
                    diff32 = 0;
                }
            }
            else	//If Jb is empty we have some data in the pipeline that is sent out to the peer node.
            {
                diff32 = tsOfNextPacketToBeDecoded - clientClock32;
            }

            PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "RTCP_PKT: Mime=%s, RTP_TS=%d, C_CLOCK=%d, DIFF=%d", irRTPDataJitterBuffer.GetMimeType(), converted_ts, clientClock32, diff32));
            App.pss0_app_data.playoutdelayinms = (uint16)diff32;
            App.pss0_app_data.nsn = OSCL_STATIC_CAST(uint16, (jbStats.lastRetrievedSeqNum + 1));
            if (0 == jbStats.lastRetrievedSeqNum)
            {
                App.pss0_app_data.nsn = OSCL_STATIC_CAST(uint16, jbStats.seqNumBase);
            }
            App.pss0_app_data.nun = RTCP_NADU_APP_DEFAULT_NUN;

            uint32 fbsInBytes = 0;
            if (jbStats.packetSizeInBytesLeftInBuffer < iRTPChannelRateAdaptationInfo.iRateAdaptationFreeBufferSpaceInBytes)
            {
                fbsInBytes =
                    (iRTPChannelRateAdaptationInfo.iRateAdaptationFreeBufferSpaceInBytes - jbStats.packetSizeInBytesLeftInBuffer);
            }
            PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::ComposeAndSendFeedBackPacket: Total=%d, Occ=%d, freebufferspace = %d",
                                             iRTPChannelRateAdaptationInfo.iRateAdaptationFreeBufferSpaceInBytes,
                                             jbStats.packetSizeInBytesLeftInBuffer,
                                             fbsInBytes));
            App.pss0_app_data.freebufferspace = OSCL_STATIC_CAST(uint16, (fbsInBytes) / 64);
            iRTPChannelRateAdaptationInfo.iRateAdaptationRTCPRRCount = 0;
            appPtr = &App;
            PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "NADU_PKT: Mime=%s, PDelay=%d, FBS_BYTES=%d, FBS=%d, NSN=%d",
                                             irRTPDataJitterBuffer.GetMimeType(),
                                             App.pss0_app_data.playoutdelayinms,
                                             fbsInBytes,
                                             App.pss0_app_data.freebufferspace,
                                             App.pss0_app_data.nsn));
        }
    }
    if (rtcpEncode.EncodeCompoundRR(*pRR,
                                    memFrag,
                                    appPtr) != RTCP_Encoder::RTCP_SUCCESS)
    {
        PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "0x%x PVMFJitterBufferNode::ComposeAndSendFeedBackPacket: EncodeCompoundRR failed", this));
        return PVMFFailure;
    }
    rtcpOut->setMediaFragFilledLen(0, memFrag.len);


    // update average packet length - treat compound packets as single
    iRTCPStats.avg_rtcp_compound_pkt_size = OSCL_STATIC_CAST(float, (memFrag.len + 15.0 * iRTCPStats.avg_rtcp_compound_pkt_size) / 16.0);



    PVMFSharedMediaMsgPtr rtcpMsg;
    convertToPVMFMediaMsg(rtcpMsg, rtcpOut);

    ipObserver->RTCPRRReadyToSend(ipFeedbackPort, rtcpMsg);

    iInitialRTCPPacket = false;
    return PVMFSuccess;
}

///////////////////////////////////////////////////////////////////////////////
//PVRTCPProtoImplementor Implementation
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVRTCPProtoImplementor* PVRTCPProtoImplementor::New(PVMFMediaClock& aClientPlayBackClock, PVMFMediaClock& aRTCPClock, PVRTCPProtoImplementorObserver* aEventNotifier, bool aBroadcastSession)
{
    int32 err = OsclErrNone;
    PVRTCPProtoImplementor*	pRtcpProtoImplementor = NULL;
    OSCL_TRY(err, pRtcpProtoImplementor = OSCL_NEW(PVRTCPProtoImplementor, (aClientPlayBackClock, aRTCPClock, aEventNotifier, aBroadcastSession));
             if (pRtcpProtoImplementor)
{
    pRtcpProtoImplementor->Construct();
    }
            );

    if (OsclErrNone != err && pRtcpProtoImplementor)
    {
        if (pRtcpProtoImplementor)
            OSCL_DELETE(pRtcpProtoImplementor);

        pRtcpProtoImplementor = NULL;
    }

    return pRtcpProtoImplementor;
}

PVRTCPProtoImplementor::PVRTCPProtoImplementor(PVMFMediaClock& aClientPlayBackClock, PVMFMediaClock& aRTCPClock, PVRTCPProtoImplementorObserver*	aObserver, bool aBroadcastSession)
        : iBroadcastSession(aBroadcastSession)
        , irClientPlayBackClock(aClientPlayBackClock)
        , irRTCPClock(aRTCPClock)
        , ipObserver(aObserver)
{
    ipRTCPDataPathLoggerIn = NULL;
    ipRTCPAVSyncLogger = NULL;
    ResetParams();
}

void PVRTCPProtoImplementor::ResetParams(bool aMemoryCleanUp)
{
    OSCL_UNUSED_ARG(aMemoryCleanUp);
    iPerformRTCPBasedAVSync = iBroadcastSession ? true : false;
    iRTCPAVSyncProcessed =	false;
    iPlayStopTimeAvailable = false;
}

void PVRTCPProtoImplementor::Construct()
{
    ipRTCPDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp.in");
    ipRTCPAVSyncLogger = PVLogger::GetLoggerObject("PVRTCPProtoImplementor.rtcp.avsync");
}

OSCL_EXPORT_REF PVRTCPProtoImplementor::~PVRTCPProtoImplementor()
{
    Reset();
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::AddPVRTCPChannelController(PVRTCPChannelController* aPVRTCPChannelController)
{
    iPVRTCPChannelController.push_back(aPVRTCPChannelController);
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::RemoveAllRTCPChannelControllers()
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.end() - 1; iter >= iPVRTCPChannelController.begin(); iter--)
    {
        OSCL_DELETE(*iter);
    }
    iPVRTCPChannelController.clear();
}

OSCL_EXPORT_REF PVRTCPChannelController* PVRTCPProtoImplementor::GetRTCPChannelController(const PVMFPortInterface* aPort) const
{
    PVRTCPChannelController* rtcpChannelController = NULL;
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* currRtcpChannelController = *iter;
        if (currRtcpChannelController->CheckAssociatedFeedbackPort(aPort))
        {
            rtcpChannelController = currRtcpChannelController;
            break;
        }
    }
    return rtcpChannelController;
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::StartRTCPMessageExchange()
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* rtcpChannelController = *iter;
        rtcpChannelController->StartRTCPMessageExchange();
    }
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::PauseRTCPMessageExchange()
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* rtcpChannelController = *iter;
        rtcpChannelController->PauseRTCPMessageExchange();
    }
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::StopRTCPMessageExchange()
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* rtcpChannelController = *iter;
        rtcpChannelController->StopRTCPMessageExchange();
    }
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::Reset()
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* rtcpChannelController = *iter;
        rtcpChannelController->Reset();
    }
    ResetParams(true);
}

OSCL_EXPORT_REF PVMFStatus PVRTCPProtoImplementor::ProcessRTCPReport(PVMFPortInterface* aFeedbackPort, PVMFSharedMediaMsgPtr& aMsg)
{
    PVMFStatus status = PVMFFailure;
    PVRTCPChannelController* rtcpChannelController = GetRTCPChannelController(aFeedbackPort);
    if (rtcpChannelController)
    {
        status = rtcpChannelController->ProcessRTCPReport(aMsg);
    }
    return status;
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval)
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter !=  iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* rtcpChannelController = *iter;
        rtcpChannelController->SetRTCPIntervalInMicroSecs(aRTCPInterval);
    }
}

OSCL_EXPORT_REF bool PVRTCPProtoImplementor::SetPortRTCPParams(PVMFPortInterface* aPort, int aNumSenders, uint32 aRR, uint32 aRS)
{
    bool retval = false;
    PVRTCPChannelController* rtcpChannelController = GetRTCPChannelController(aPort);
    if (rtcpChannelController)
    {
        rtcpChannelController->SetPortRTCPParams(aNumSenders, aRR, aRS);
        retval = true;
    }
    return retval;
}

OSCL_EXPORT_REF void PVRTCPProtoImplementor::Prepare(bool aPlayAfterSeek)
{
    Oscl_Vector<PVRTCPChannelController*, OsclMemAllocator>::iterator iter;
    for (iter = iPVRTCPChannelController.begin(); iter != iPVRTCPChannelController.end(); iter++)
    {
        PVRTCPChannelController* pRTCPChannelController = *iter;
        if (pRTCPChannelController)
        {
            pRTCPChannelController->Prepare(aPlayAfterSeek);
        }
    }
}

PVMFStatus PVRTCPProtoImplementor::RTCPSRReveived(PVRTCPChannelController* aChannelController)
{
    OSCL_UNUSED_ARG(aChannelController);
    PVMFStatus status = PVMFSuccess;
    if (iPerformRTCPBasedAVSync && !iRTCPAVSyncProcessed)
    {
        bool ret = ProcessRTCPSRforAVSync();
        if (ret == false)
        {
            // No need to return error as perhaps there's not enough information yet
            // to attempt a/v sync
            return PVMFSuccess;
        }
    }
    return status;
}

bool PVRTCPProtoImplementor::ProcessRTCPSRforAVSync()
{
    // The following criteria must hold before the RTCP SRs can be processed for a/v sync
    // a) The Jitter Buffers of all tracks have received at least one packet
    // b) At least one RTCP report has been received for each track
    // c) The wall clock value of the RTCP SRs is not zero

    // Check the criteria
    for (uint32 ii = 0; ii < iPVRTCPChannelController.size(); ii++)
    {
        PVRTCPChannelController* rtcpChannelController = iPVRTCPChannelController[ii];
        uint32 tsOffset = 0;
        if (rtcpChannelController->GetJitterBuffer().GetRTPTimeStampOffset(tsOffset) == false)
        {
            return false;
        }

        if (rtcpChannelController->GetRTCPStats().lastSenderReportRecvTime == OSCL_STATIC_CAST(uint64, 0))
        {
            return false;
        }

        if (rtcpChannelController->GetRTCPStats().lastSenderReportNTP == OSCL_STATIC_CAST(uint64, 0))
        {
            return false;
        }
    }

    // temporary vector to save the calculated init ntp for each track
    Oscl_Vector<uint64, OsclMemAllocator> initNtpTracks;

    // temporary vector to save the calculated rtp timebase of each track
    Oscl_Vector<uint32, OsclMemAllocator> RTPTBArray;

    // Initialize temporary vectors
    int32 err = OsclErrNone;
    OSCL_TRY(err, initNtpTracks.reserve(iPVRTCPChannelController.size()));
    if (err != OsclErrNone)
        return false;

    OSCL_TRY(err, RTPTBArray.push_back(iPVRTCPChannelController.size()));
    if (err != OsclErrNone)
        return false;

    for (uint32 tt = 0; tt < iPVRTCPChannelController.size(); tt++)
    {
        initNtpTracks.push_back(0);
        RTPTBArray.push_back(0);
    }


    // Find the track whose first rtp packet correspond to the smallest NTP
    uint32 lowestNTPIndex = 0;
    uint64 lowestNTP = 0;
    for (uint32 jj = 0; jj < iPVRTCPChannelController.size(); jj++)
    {
        uint32 firstRTP;
        iPVRTCPChannelController[jj]->GetJitterBuffer().GetRTPTimeStampOffset(firstRTP);
        uint32 timescale = iPVRTCPChannelController[jj]->GetJitterBuffer().GetTimeScale();
        uint32 srRTP = iPVRTCPChannelController[jj]->GetRTCPStats().lastSenderReportRTP;
        uint64 srNTP = iPVRTCPChannelController[jj]->GetRTCPStats().lastSenderReportNTP;

        uint32 deltaRTP = 0;
        if (srRTP >= firstRTP)
        {
            deltaRTP = srRTP - firstRTP;
        }
        else
        {
            deltaRTP = firstRTP - srRTP;
        }

        uint64 deltaRTPInNTPFormat = ((uint64) deltaRTP / (uint64)timescale) << 32;
        deltaRTPInNTPFormat += ((uint64) deltaRTP % (uint64)timescale) * (uint64)0xFFFFFFFF / (uint64)timescale;

        uint64 initNTP = 0;
        if (srRTP >= firstRTP)
        {
            initNTP = srNTP - deltaRTPInNTPFormat;
        }
        else
        {
            initNTP = srNTP + deltaRTPInNTPFormat;
        }


        if (jj == 0)
        {
            lowestNTPIndex = jj;
            lowestNTP = initNTP;
        }
        else
            if (initNTP < lowestNTP)
            {
                lowestNTPIndex = jj;
                lowestNTP = initNTP;
            }

        // Save the reference ntp value
        initNtpTracks[jj] = initNTP;

        PVMF_JB_LOG_RTCP_AVSYNC((0,
                                 "PVMFJitterBufferNode::ProcessRTCPSRforAVSync(): srRTP=%d, firstRTP=%d, timescale=%d srNTPHigh=0x%x, srNTPLow=0x%x initNTPHigh=0x%x initNTPLow=0x%x deltaRTPHigh=0x%x deltaRTPLow=0x%x",
                                 srRTP, firstRTP, timescale, Oscl_Int64_Utils::get_uint64_upper32(srNTP), Oscl_Int64_Utils::get_uint64_lower32(srNTP),
                                 Oscl_Int64_Utils::get_uint64_upper32(initNTP), Oscl_Int64_Utils::get_uint64_lower32(initNTP),
                                 Oscl_Int64_Utils::get_uint64_upper32(deltaRTPInNTPFormat), Oscl_Int64_Utils::get_uint64_lower32(deltaRTPInNTPFormat)));

    }


    // Calculate the new timebase for all tracks
    for (uint32 kk = 0; kk < iPVRTCPChannelController.size(); kk++)
    {

        uint32 firstRTP;
        iPVRTCPChannelController[kk]->GetJitterBuffer().GetRTPTimeStampOffset(firstRTP);

        if (kk == lowestNTPIndex)
        {
            // Just set the RTP TB to the first rtp packet
            RTPTBArray[kk] = firstRTP;
        }
        else
        {
            uint64 initNTP = initNtpTracks[kk];
            uint32 timescale = iPVRTCPChannelController[kk]->GetJitterBuffer().GetTimeScale();

            OSCL_ASSERT(lowestNTP <= initNTP);

            uint64 deltaNTP = initNTP - lowestNTP;
            uint32 deltaNTPInRTPUnits = OSCL_STATIC_CAST(uint32, ((deltaNTP * (uint64)timescale) + (uint64)0x80000000) >> 32);
            uint32 rtpTimeBase = firstRTP - deltaNTPInRTPUnits;
            RTPTBArray[kk] = rtpTimeBase;
        }
    }

#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
    // Log parameters
    for (uint32 mm = 0; mm < iPVRTCPChannelController.size(); mm++)
    {
        uint32 firstRTP;
        iPVRTCPChannelController[mm]->GetJitterBuffer().GetRTPTimeStampOffset(firstRTP);
        uint32 timescale = iPVRTCPChannelController[mm]->GetJitterBuffer().GetTimeScale();
        uint32 srRTP = iPVRTCPChannelController[mm]->GetRTCPStats().lastSenderReportRTP;
        uint64 srNTP = iPVRTCPChannelController[mm]->GetRTCPStats().lastSenderReportNTP;
        int32 delta = ((firstRTP - RTPTBArray[mm]) * 1000) / timescale;
        uint32 srNTPHigh = Oscl_Int64_Utils::get_uint64_upper32(srNTP);
        srNTP = srNTP & uint64(0xffffffff);
        srNTP *= uint64(1000000);
        srNTP += uint64(500000);
        srNTP = srNTP / uint64(0xffffffff);
        uint32 srNTPLow = Oscl_Int64_Utils::get_uint64_lower32(srNTP);

        PVMF_JB_LOG_RTCP_AVSYNC((0,
                                 "Stream %d: mime=%s timeScale=%uHz firstTS=%u RTCP.RTP=%u RTCP.NTP=%u.%06u newTB=%u delta=%dms\n",
                                 mm,
                                 iPVRTCPChannelController[mm]->GetJitterBuffer().GetMimeType(),
                                 timescale,
                                 firstRTP,
                                 srRTP,
                                 srNTPHigh,
                                 srNTPLow,
                                 RTPTBArray[mm],
                                 delta
                                )
                               );

    }
#endif

    // Adjust the RTP TB
    for (uint32 ll = 0; ll < iPVRTCPChannelController.size(); ll++)
    {
        iPVRTCPChannelController[ll]->GetJitterBuffer().SetRTPTimeStampOffset(RTPTBArray[ll]);
    }

    //Notify SM plugin that RTP TB data is available for PVR purposes
    // No need to create a public class to publish the format of the information sent in this event
    // Just define this structure internally. The only client of this event is the SM broadcast
    // plugin, so it's the only component that needs to be aware of this format
    struct RTPTBInfoEventData
    {
        const PvmfMimeString* mimeType;
        uint32 rtpTB;
    };

    for (uint32 nn = 0; nn < iPVRTCPChannelController.size(); nn++)
    {
        OSCL_HeapString<OsclMemAllocator> mimeStr(iPVRTCPChannelController[nn]->GetJitterBuffer().GetMimeType());

        RTPTBInfoEventData infoData;
        infoData.mimeType = &mimeStr;
        infoData.rtpTB = RTPTBArray[nn];
        PVMFAsyncEvent infoEvent(PVMFInfoEvent, PVMFJitterBufferNodeRTCPDataProcessed, NULL, (OsclAny*)(&infoData));
        ipObserver->ProcessInfoEvent(infoEvent);
    }

    iRTCPAVSyncProcessed = true;
    return true;
}

PVMFStatus PVRTCPProtoImplementor::RTCPByeReceived(PVRTCPChannelController* aChannelController)
{
    PVMF_JB_LOG_RTCPDATATRAFFIC_IN_E((0, "PVRTCPProtoImplementor::RTCPByeReceived for mime %s", aChannelController->GetJitterBuffer().GetMimeType()));
    ipObserver->RTCPPacketReceived(BYE_RTCP_PACKET, aChannelController);
    aChannelController->GetJitterBuffer().SetEOS(true);
    return PVMFSuccess;
}

PVMFStatus PVRTCPProtoImplementor::RTCPRRReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage)
{
    ipObserver->RTCPReportReadyToSend(aPort, aMessage);
    return PVMFSuccess;
}
