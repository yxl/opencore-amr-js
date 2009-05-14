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
#ifndef PVMF_JB_JITTERBUFFERMISC_H_INCLUDED
#include "pvmf_jb_jitterbuffermisc.h"
#endif

#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif

#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif

OSCL_EXPORT_REF PVMFJitterBufferMisc* PVMFJitterBufferMisc::New(PVMFJitterBufferMiscObserver* aObserver, PVMFMediaClock& aClientPlaybackClock, Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>& aPortParamsQueue)
{
    int32 err = OsclErrNone;
    PVMFJitterBufferMisc* ptr = NULL;
    OSCL_TRY(err, ptr = OSCL_NEW(PVMFJitterBufferMisc, (aObserver, aClientPlaybackClock, aPortParamsQueue));
             ptr->Construct());
    if (err != OsclErrNone)
    {
        ptr = NULL;
    }
    return ptr;
}

void PVMFJitterBufferMisc::Construct()
{
    ipJBEventsClockLogger = PVLogger::GetLoggerObject("jitterbuffernode.eventsclock");
    ipRTCPDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp.in");
    ipClockLoggerSessionDuration = PVLogger::GetLoggerObject("clock.streaming_manager.sessionduration");
    ipClockLogger = PVLogger::GetLoggerObject("clock.jitterbuffernode");
    ipDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.in");
    ipDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.out");
    ipDataPathLoggerRTCP = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp");
    ipLogger = PVLogger::GetLoggerObject("PVMFJitterBufferMisc");
    ipClockLoggerRebuff = PVLogger::GetLoggerObject("sourcenode.clock.rebuff");

    CreateProtocolObjects();

    ResetParams(false);

    //Look for the input ports in the port vect
    //Look for the corresponding input port and the jitter buffer associated with it
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter_begin = irPortParamsQueue.begin();
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter_end = irPortParamsQueue.end();
    while (iter_begin != iter_end)
    {
        PVMFJitterBufferPortParams* portParams = *iter_begin;
        if (PVMF_JITTER_BUFFER_PORT_TYPE_INPUT == portParams->iTag)//input port
        {
            PVMFPortInterface* feedbackPort = NULL;
            PVMFJitterBuffer* jitterBuffer = NULL;
            PVRTCPChannelController* rtcpChannelController = NULL;
            if (LookupRTCPChannelParams(&portParams->irPort, feedbackPort, jitterBuffer))
            {
                rtcpChannelController = PVRTCPChannelController::New(ipRTCPProtoImplementator, *jitterBuffer, feedbackPort, irClientPlaybackClock, *ipWallClock);
                ipRTCPProtoImplementator->AddPVRTCPChannelController(rtcpChannelController);
            }
        }
        iter_begin++;
    }
}

bool PVMFJitterBufferMisc::CreateProtocolObjects()
{
    uint32 start = 0;
    bool overflowFlag = false;

    ipEstimatedServerClock = OSCL_NEW(PVMFMediaClock, ());
    ipEstimatedServerClock->SetClockTimebase(iEstimatedServerClockTimeBase);
    if (ipEstimatedServerClock)
    {
        ipEstimatedServerClock->Stop();
        ipEstimatedServerClock->SetStartTime32(start, PVMF_MEDIA_CLOCK_MSEC, overflowFlag);
    }

    ipWallClock = OSCL_NEW(PVMFMediaClock, ());
    ipWallClock->SetClockTimebase(iWallClockTimeBase);
    if (ipWallClock)
    {
        ipWallClock->SetStartTime32(start, PVMF_MEDIA_CLOCK_MSEC, overflowFlag);
    }

    ipNonDecreasingClock = OSCL_NEW(PVMFMediaClock, ());
    ipNonDecreasingClock->SetClockTimebase(iNonDecreasingTimeBase);
    if (ipNonDecreasingClock)
    {
        ipNonDecreasingClock->SetStartTime32(start, PVMF_MEDIA_CLOCK_MSEC, overflowFlag);
    }

    ipEventNotifier = PVMFJBEventNotifier::New(*ipNonDecreasingClock, irClientPlaybackClock, *ipEstimatedServerClock);

    return true;
}

void PVMFJitterBufferMisc::ResetParams(bool aReleaseMemory)
{
    if (ipFireWallPacketExchangerImpl && aReleaseMemory)
    {
        OSCL_DELETE(ipFireWallPacketExchangerImpl);
    }

    ipFireWallPacketExchangerImpl = NULL;

    iSessionDuration = 0;
    iStreamingSessionExpired = false;
    iPlayDurationAvailable = false;
    iBroadcastSession = false;

    iPlayStartTimeInMS = 0;
    iPlayStopTimeInMS = 0;
    iPlayStopTimeAvailable = false;


    iFireWallPacketsExchangeEnabled = true;
    iEstimatedServerClockUpdateCallbackId = 0;
    iEstimatedServerClockUpdateCallbackPending = false;


}

OSCL_EXPORT_REF PVMFJitterBufferMisc::~PVMFJitterBufferMisc()
{
    ResetParams(true);
    if (ipRTCPProtoImplementator)
    {
        ipRTCPProtoImplementator->RemoveAllRTCPChannelControllers();
        OSCL_DELETE(ipRTCPProtoImplementator);
    }

    if (ipEventNotifier)
    {
        OSCL_DELETE(ipEventNotifier);
    }

    if (ipSessionDurationTimer)
    {
        OSCL_DELETE(ipSessionDurationTimer);
    }

    if (ipEstimatedServerClock)
    {
        OSCL_DELETE(ipEstimatedServerClock);
    }

    if (ipWallClock)
    {
        OSCL_DELETE(ipWallClock);
    }

    if (ipNonDecreasingClock)
    {
        OSCL_DELETE(ipNonDecreasingClock);
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::StreamingSessionStarted()
{
    //if not already started...
    if (ipNonDecreasingClock)
    {
        ipNonDecreasingClock->Start();
    }

    if (ipWallClock)
    {
        //Starts if not already running, check for state
        ipWallClock->Start();
    }

    //Estimated server is to be updated only by the jitter buffer.
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
    {
        PVMFJitterBufferPortParams* pPortParam = *iter;
        if (pPortParam)
        {
            PVMFJitterBuffer* jitterBuffer = pPortParam->ipJitterBuffer;
            if (jitterBuffer)
            {
                jitterBuffer->StreamingSessionStarted();
            }
        }
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::StreamingSessionStopped()
{
    Reset();
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
    {
        PVMFJitterBufferPortParams* pPortParam = *iter;
        if (pPortParam)
        {
            PVMFJitterBuffer* jitterBuffer = pPortParam->ipJitterBuffer;
            if (jitterBuffer)
            {
                jitterBuffer->StreamingSessionStopped();
            }
        }
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::StreamingSessionPaused()
{
    ipNonDecreasingClock->Pause();
    ipEstimatedServerClock->Pause();
    if (ipSessionDurationTimer)
        ipSessionDurationTimer->Cancel();

    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
    {
        PVMFJitterBufferPortParams* pPortParam = *iter;
        if (pPortParam)
        {
            PVMFJitterBuffer* jitterBuffer = pPortParam->ipJitterBuffer;
            if (jitterBuffer)
            {
                jitterBuffer->StreamingSessionPaused();
            }
        }
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::StreamingSessionBufferingStart()
{
    if (ipSessionDurationTimer)
        ipSessionDurationTimer->Cancel();
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::StreamingSessionBufferingEnd()
{
    if (ipSessionDurationTimer)
    {
        ComputeCurrentSessionDurationMonitoringInterval();
        ipSessionDurationTimer->Start();
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::SetBroadcastSession()
{
    iBroadcastSession = true;
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::ResetEstimatedServerClock()
{
    if (ipEstimatedServerClock)
    {
        uint32 start = 0;
        ipEstimatedServerClock->Stop();
        bool overflowFlag = false;
        ipEstimatedServerClock->SetStartTime32(start, PVMF_MEDIA_CLOCK_MSEC, overflowFlag);
    }
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::PrepareForRepositioning(bool oUseExpectedClientClockVal, uint32 aExpectedClientClockVal)
{
    bool overflowFlag = false;

    //A session will have three things
    //Media channel		:	Valid for any type of streaming
    //Feedback Channel	:	Valid for RTSP based streaming
    //Session Info	:	Valid for any type of streaming

    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = irPortParamsQueue.begin();
            it != irPortParamsQueue.end();
            it++)
    {
        PVMFJitterBufferPortParams* pPortParam = *it;
        if (pPortParam->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            pPortParam->ipJitterBuffer->PrepareForRepositioning();
        }
    }

    PVMFTimestamp ts = 0;
    if (oUseExpectedClientClockVal)
    {
        ts = aExpectedClientClockVal;
    }
    else
    {
        //reset player clock
        ts = GetActualMediaDataTSAfterSeek();
    }

    irClientPlaybackClock.Stop();
    irClientPlaybackClock.SetStartTime32(ts,
                                         PVMF_MEDIA_CLOCK_MSEC, overflowFlag);

    LOGCLIENTANDESTIMATEDSERVCLK_DATAPATH;
    LOGCLIENTANDESTIMATEDSERVCLK_DATAPATH_OUT;
    LOGCLIENTANDESTIMATEDSERVCLK_REBUFF;

    if (ipRTCPProtoImplementator)
        ipRTCPProtoImplementator->Prepare(true);
    iStreamingSessionExpired = false;

    return true;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::PurgeElementsWithNPTLessThan(NptTimeFormat &aNPTTime)
{
    if (aNPTTime.npt_format != NptTimeFormat::NPT_SEC)
    {
        return false;
    }

    uint32 i;
    for (i = 0; i < irPortParamsQueue.size(); i++)
    {
        PVMFJitterBufferPortParams* portParams = irPortParamsQueue[i];
        portParams->irPort.ClearMsgQueues();
    }

    for (i = 0; i < irPortParamsQueue.size(); i++)
    {
        PVMFJitterBufferPortParams* portParams = irPortParamsQueue[i];
        if (portParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (portParams->ipJitterBuffer != NULL)
            {
                //portParams.iJitterBuffer->FlushJitterBuffer();
                PVMFTimestamp baseTS = 1000 * aNPTTime.npt_sec.sec + aNPTTime.npt_sec.milli_sec;

                portParams->iMediaClockConverter.set_clock_other_timescale(baseTS, 1000);
                baseTS = portParams->iMediaClockConverter.get_current_timestamp();
                portParams->ipJitterBuffer->PurgeElementsWithTimestampLessThan(baseTS);
            }
        }
    }

    // Update client clock here to avoid premature buffer fullness
    PVMFTimestamp ts = 1000 * aNPTTime.npt_sec.sec + aNPTTime.npt_sec.milli_sec;
    bool overflowFlag = false;
    irClientPlaybackClock.Stop();
    irClientPlaybackClock.SetStartTime32(ts, PVMF_MEDIA_CLOCK_MSEC, overflowFlag);
    return true;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::IsSessionExpired()
{
    return iStreamingSessionExpired;
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::Prepare()
{
    if (ipNonDecreasingClock)
    {
        ipNonDecreasingClock->Start();
    }

    if (ipWallClock)
    {
        //Starts if not already running, check for state
        ipWallClock->Start();
    }

    if (UseSessionDurationTimerForEOS())
    {
        ipSessionDurationTimer = OSCL_NEW(PvmfJBSessionDurationTimer, (this));
        if (ipSessionDurationTimer && ipEstimatedServerClock)
        {
            ipSessionDurationTimer->SetEstimatedServerClock(ipEstimatedServerClock);
        }
    }
    if (RTCPProtocolImplementorRequired())
    {
        ipRTCPProtoImplementator = PVRTCPProtoImplementor::New(irClientPlaybackClock, *ipWallClock, this, iBroadcastSession);

        //Set the rate adaptaton parmas if not already set
        PVMFPortInterface* feedbackPort = NULL;
        PVMFJitterBuffer* jitterBuffer = NULL;
        Oscl_Vector<RateAdapatationInfo, OsclMemAllocator>::iterator iter;
        for (iter = iRateAdaptationInfos.begin(); iter < iRateAdaptationInfos.end(); iter++)
        {
            RateAdapatationInfo rateAdaptationInfo = *iter;
            feedbackPort = NULL;
            jitterBuffer = NULL;
            if (LookupRTCPChannelParams(rateAdaptationInfo.iPort, feedbackPort, jitterBuffer))
            {
                PVRTCPChannelController* rtcpChannelController = ipRTCPProtoImplementator->GetRTCPChannelController(feedbackPort);
                if (rtcpChannelController)
                {
                    rtcpChannelController->SetRateAdaptation(rateAdaptationInfo.iRateAdapatationInfo.iRateAdaptation, rateAdaptationInfo.iRateAdapatationInfo.iRateAdaptationFeedBackFrequency, rateAdaptationInfo.iRateAdapatationInfo.iRateAdaptationFreeBufferSpaceInBytes);

                }
                else
                {
                    PVMFJitterBufferPort* rtpDataPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, rateAdaptationInfo.iPort);
                    jitterBuffer = rtpDataPort->GetPortParams()->ipJitterBuffer;
                    PVMFJitterBufferPortParams* rtpPortParams = rtpDataPort->GetPortParams();
                    PVMFJitterBufferPortParams* feedbackPortParams = NULL;
                    LocateFeedBackPort(rtpPortParams, feedbackPortParams);
                    if (feedbackPortParams)
                    {
                        feedbackPort = &feedbackPortParams->irPort;
                    }
                    rtcpChannelController = PVRTCPChannelController::New(ipRTCPProtoImplementator, *jitterBuffer, feedbackPort, irClientPlaybackClock, *ipWallClock);
                    rtcpChannelController->SetRateAdaptation(rateAdaptationInfo.iRateAdapatationInfo.iRateAdaptation, rateAdaptationInfo.iRateAdapatationInfo.iRateAdaptationFeedBackFrequency, rateAdaptationInfo.iRateAdapatationInfo.iRateAdaptationFreeBufferSpaceInBytes);
                    ipRTCPProtoImplementator->AddPVRTCPChannelController(rtcpChannelController);
                }
            }
        }

        Oscl_Vector<RTCPParams, OsclMemAllocator>::iterator rtcpParamIter;
        for (rtcpParamIter = iRTCPParamsVect.begin(); rtcpParamIter < iRTCPParamsVect.end(); rtcpParamIter++)
        {
            RTCPParams rtcpParams = *rtcpParamIter;
            ipRTCPProtoImplementator->SetPortRTCPParams(rtcpParams.iFeedbackPort, rtcpParams.iNumSenders, rtcpParams.iRR, rtcpParams.iRS);
        }

        {
            //Provide media clcok converter
            Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
            for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
            {
                PVMFJitterBufferPortParams* pPortParams = *iter;
                if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
                {

                    SetMediaClockConverter(&pPortParams->irPort, &pPortParams->iMediaClockConverter);
                }
            }
        }
    }

    iRateAdaptationInfos.clear();
    iRTCPParamsVect.clear();
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::Reset()
{
    if (ipEventNotifier)
    {
        ipEventNotifier->CancelAllPendingCallbacks();
    }
    if (ipSessionDurationTimer)
    {
        ipSessionDurationTimer->Stop();
    }
    if (ipRTCPProtoImplementator)
    {
        ipRTCPProtoImplementator->Reset();
    }
    if (ipFireWallPacketExchangerImpl)
    {
        ipFireWallPacketExchangerImpl->CancelFirewallPacketExchange();
    }
    if (ipEstimatedServerClock)
    {
        ipEstimatedServerClock->Stop();
    }
    if (ipWallClock)
    {
        ipWallClock->Stop();
    }
    if (ipNonDecreasingClock)
    {
        ipNonDecreasingClock->Stop();
    }

    iSessionDuration = 0;
    iStreamingSessionExpired = true;
    iPlayDurationAvailable = false;
    iBroadcastSession = false;
    iFireWallPacketsExchangeEnabled = true;
}

OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferMisc::PrepareMediaReceivingChannel()
{
    if (ipFireWallPacketExchangerImpl && iFireWallPacketsExchangeEnabled)
    {
        ipFireWallPacketExchangerImpl->InitiateFirewallPacketExchange();
        return PVMFPending;
    }
    else
    {
        if (!iFireWallPacketsExchangeEnabled)
        {
            ipObserver->MediaReceivingChannelPrepared(true);
            return PVMFSuccess;
        }
        return PVMFPending;	//Wait for the SetServerInfo call
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferMisc::CancelMediaReceivingChannelPreparation()
{
    if (ipFireWallPacketExchangerImpl)
        ipFireWallPacketExchangerImpl->CancelFirewallPacketExchange();
    return PVMFSuccess;
}

bool PVMFJitterBufferMisc::FirewallPacketExchangerRequired() const
{
    if (iFireWallPacketsExchangeEnabled)
    {
        char mimeRequiredForFirewallPacketExchange[] = "rtp";
        char mediaChannelMimeType[255] = {'0'};
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
        for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end(); iter++)
        {
            PVMFJitterBufferPortParams* pPortParams = *iter;
            if (pPortParams && (pPortParams->ipJitterBuffer))
            {
                oscl_memset(mediaChannelMimeType, 0, sizeof(mediaChannelMimeType));
                const char* tmpMediaChannelMimeType = pPortParams->ipJitterBuffer->GetMimeType();
                const int32 mediaChannelMimeLen =  oscl_strlen(tmpMediaChannelMimeType);
                if (tmpMediaChannelMimeType)
                {
                    for (int ii = 0; ii < mediaChannelMimeLen; ii++)
                    {
                        mediaChannelMimeType[ii] = oscl_tolower(tmpMediaChannelMimeType[ii]);
                    }
                    mediaChannelMimeType[mediaChannelMimeLen] = '\0';
                }

                if (oscl_strstr(mediaChannelMimeType, mimeRequiredForFirewallPacketExchange))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool PVMFJitterBufferMisc::RTCPProtocolImplementorRequired() const
{
    char mimeRequiredForRTCPSupport[] = "rtp";
    char mediaChannelMimeType[255] = {'0'};
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end(); iter++)
    {
        PVMFJitterBufferPortParams* pPortParams = *iter;
        if (pPortParams && (pPortParams->ipJitterBuffer))
        {
            oscl_memset(mediaChannelMimeType, 0, sizeof(mediaChannelMimeType));
            const char* tmpMediaChannelMimeType = pPortParams->ipJitterBuffer->GetMimeType();
            const int32 mediaChannelMimeLen =  oscl_strlen(tmpMediaChannelMimeType);
            if (tmpMediaChannelMimeType)
            {
                for (int ii = 0; ii < mediaChannelMimeLen; ii++)
                {
                    mediaChannelMimeType[ii] = oscl_tolower(tmpMediaChannelMimeType[ii]);
                }
                mediaChannelMimeType[mediaChannelMimeLen] = '\0';
            }

            if (oscl_strstr(mediaChannelMimeType, mimeRequiredForRTCPSupport))
            {
                return true;
            }
        }
    }
    return false;//based on mime type
}

OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferMisc::ProcessFeedbackMessage(PVMFJitterBufferPortParams& aParam, PVMFSharedMediaMsgPtr aMsg)
{
    PVMFStatus status = PVMFSuccess;
    if (ipRTCPProtoImplementator)
    {
        status = ipRTCPProtoImplementator->ProcessRTCPReport(&aParam.irPort, aMsg);
    }
    else
    {
        status = PVMFFailure;
    }
    return status;
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::SetRateAdaptationInfo(PVMFPortInterface* aPort, bool aRateAdaptation, uint32 aRateAdaptationFeedBackFrequency, uint32 aRateAdaptationFreeBufferSpaceInBytes)
{
    //Persist it: We'll update it when RTCP controller will be prepared
    RateAdapatationInfo rateAdadpatatinInfo;
    rateAdadpatatinInfo.iPort = aPort;
    rateAdadpatatinInfo.iRateAdapatationInfo.iRateAdaptation = aRateAdaptation;
    rateAdadpatatinInfo.iRateAdapatationInfo.iRateAdaptationFeedBackFrequency = aRateAdaptationFeedBackFrequency;
    rateAdadpatatinInfo.iRateAdapatationInfo.iRateAdaptationFreeBufferSpaceInBytes = aRateAdaptationFreeBufferSpaceInBytes;
    iRateAdaptationInfos.push_back(rateAdadpatatinInfo);
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::SetRTCPIntervalInMicroSecs(uint32 aRTCPInterval)
{
    if (ipRTCPProtoImplementator)
        ipRTCPProtoImplementator->SetRTCPIntervalInMicroSecs(aRTCPInterval);
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::SetPortRTCPParams(PVMFPortInterface* aPort, int aNumSenders, uint32 aRR, uint32 aRS)
{
    bool retval = false;
    if (ipRTCPProtoImplementator)
        retval = ipRTCPProtoImplementator->SetPortRTCPParams(aPort, aNumSenders, aRR, aRS);
    //retval eq to false implies rtcp controller doesnt exist as fo now for the port "aPort"
    if (!retval)
    {
        RTCPParams rtcpParams;
        rtcpParams.iFeedbackPort = aPort;
        rtcpParams.iNumSenders = aNumSenders;
        rtcpParams.iRR = aRR;
        rtcpParams.iRS = aRS;
        iRTCPParamsVect.push_back(rtcpParams);
        retval = true;
    }
    return retval;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::ResetSession()
{
    iStreamingSessionExpired = false;
    if (ipSessionDurationTimer)
    {
        ipSessionDurationTimer->Cancel();
        ipSessionDurationTimer->Stop();
    }
    return true;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::SetSessionDurationExpired()
{
    iStreamingSessionExpired = true;
    if (ipSessionDurationTimer)
    {
        ipSessionDurationTimer->Cancel();
        ipSessionDurationTimer->Stop();
    }

    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
    for (it = irPortParamsQueue.begin(); it != irPortParamsQueue.end(); it++)
    {
        PVMFJitterBufferPortParams* pPortParams = *it;

        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            pPortParams->ipJitterBuffer->SetEOS(true);
        }
    }


    uint32 timebase32 = 0;
    uint32 clientClock32 = 0;
    bool overflowFlag = false;
    irClientPlaybackClock.GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    timebase32 = 0;
    uint32 estServClock32 = 0;
    ipEstimatedServerClock->GetCurrentTime32(estServClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);

    PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferMisc::SetSessionDurationExpired- Estimated Server Clock [%d] Client Clock[%d]", estServClock32, clientClock32));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferMisc::SetSessionDurationExpired- Estimated Server Clock [%d] Client Clock[%d]", estServClock32, clientClock32));

    return true;
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::MediaReceivingChannelPreparationRequired(bool aRequired)
{
    iFireWallPacketsExchangeEnabled = aRequired;
}

OSCL_EXPORT_REF PVMFMediaClock& PVMFJitterBufferMisc::GetEstimatedServerClock()
{
    return *ipEstimatedServerClock;
}

OSCL_EXPORT_REF PVMFJBEventNotifier* PVMFJitterBufferMisc::GetEventNotifier()
{
    return ipEventNotifier;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::SetPlayRange(int32 aStartTimeInMS, int32 aStopTimeInMS, bool aPlayAfterASeek, bool aStopTimeAvailable)
{
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
    {
        PVMFJitterBufferPortParams* pPortParam = *iter;
        if (pPortParam && (PVMF_JITTER_BUFFER_PORT_TYPE_INPUT == pPortParam->iTag))
        {
            PVMFJitterBuffer* jitterBuffer = pPortParam->ipJitterBuffer;
            if (jitterBuffer)
            {
                jitterBuffer->SetPlayRange(aStartTimeInMS, aPlayAfterASeek, aStopTimeAvailable, aStopTimeInMS);
            }
        }
    }

    //Configure the RTCP timer stuff
    if (ipRTCPProtoImplementator)
    {
        ipRTCPProtoImplementator->Prepare(aPlayAfterASeek);
        ipRTCPProtoImplementator->StartRTCPMessageExchange();
    }

    iPlayStartTimeInMS = aStartTimeInMS;
    iPlayStopTimeInMS = aStopTimeInMS;
    iPlayStopTimeAvailable = aStopTimeAvailable;

    if (iPlayStopTimeAvailable == true)
    {
        /* Start Session Duration Timer only if stop duration is set */
        if ((ipSessionDurationTimer) && (!iStreamingSessionExpired || (aPlayAfterASeek)))
        {
            ipSessionDurationTimer->Stop();
            iStreamingSessionExpired = false;
            ipSessionDurationTimer->setSessionDurationInMS(((iPlayStopTimeInMS - iPlayStartTimeInMS) + PVMF_EOS_TIMER_GAURD_BAND_IN_MS));
            ComputeCurrentSessionDurationMonitoringInterval();
            ipSessionDurationTimer->Start();
        }
    }

    if (aPlayAfterASeek)
    {
        //Will eventually update the estimated server clock
        //This call to "GetActualMediaDataTSAfterSeek" will normalize the ts's across the jitter buffers
        //associated with the streaming session
        GetActualMediaDataTSAfterSeek();
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
        for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
        {
            PVMFJitterBufferPortParams* pPortParams = *iter;
            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                if (pPortParams->ipJitterBuffer != NULL)
                {
                    pPortParams->ipJitterBuffer->AdjustRTPTimeStamp();
                }
            }
        }
    }
    return true;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::SetPortSSRC(PVMFPortInterface* aPort, uint32 aSSRC)
{
    //Update with the JB
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
    {
        PVMFJitterBufferPortParams* pPortParam = *iter;
        if (pPortParam && ((&pPortParam->irPort) == aPort))
        {
            PVMFJitterBuffer* jitterBuffer = pPortParam->ipJitterBuffer;
            if (jitterBuffer)
            {
                jitterBuffer->setSSRC(aSSRC);
            }
            break;
        }
    }
    //update port's ssrc with the Firewall controller
    RTPSessionInfoForFirewallExchange rtpSessioninfo(aPort, aSSRC);
    iRTPExchangeInfosForFirewallExchange.push_back(rtpSessioninfo);
    return true;
}

OSCL_EXPORT_REF uint32 PVMFJitterBufferMisc::GetEstimatedServerClockValue()
{
    uint32 serverClock32 = 0;
    bool overflowFlag = false;
    ipEstimatedServerClock->GetCurrentTime32(serverClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC);
    return serverClock32;
}

OSCL_EXPORT_REF bool PVMFJitterBufferMisc::PlayStopTimeAvailable() const
{
    return iPlayStopTimeAvailable;
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::SetServerInfo(PVMFJitterBufferFireWallPacketInfo& aServerInfo)
{
    if (iFireWallPacketsExchangeEnabled)
    {
        if (!ipFireWallPacketExchangerImpl)
        {
            //create it and start firewall packet exchange
            ipFireWallPacketExchangerImpl = PVFirewallPacketExchangeImpl::New(aServerInfo, *ipEventNotifier, ipObserver);
            Oscl_Vector<RTPSessionInfoForFirewallExchange, OsclMemAllocator>::iterator iter;
            for (iter = iRTPExchangeInfosForFirewallExchange.begin(); iter != iRTPExchangeInfosForFirewallExchange.end(); iter++)
            {
                ipFireWallPacketExchangerImpl->SetRTPSessionInfoForFirewallExchange(*iter);
            }
            ipFireWallPacketExchangerImpl->InitiateFirewallPacketExchange();
        }
        else
        {
            OSCL_ASSERT(false);
        }
    }
}

OSCL_EXPORT_REF PVMFTimestamp PVMFJitterBufferMisc::GetMaxMediaDataTS()
{
    PVMFTimestamp mediaTS = 0;
    uint32 in_wrap_count = 0;
    uint32 i;

    uint32 numOfJitterBuffers = 0;
    for (i = 0; i < irPortParamsQueue.size(); i++)
    {
        PVMFJitterBufferPortParams* pPortParams = irPortParamsQueue[i];

        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (pPortParams->ipJitterBuffer != NULL)
            {
                PVMFTimestamp ts =
                    pPortParams->ipJitterBuffer->peekNextElementTimeStamp();
                ++numOfJitterBuffers;
                /*
                 * Convert Time stamp to milliseconds
                 */
                pPortParams->iMediaClockConverter.set_clock(ts, in_wrap_count);
                PVMFTimestamp converted_ts =
                    pPortParams->iMediaClockConverter.get_converted_ts(1000);
                if (converted_ts > mediaTS)
                {
                    mediaTS = converted_ts;
                }
            }
        }
    }

    if (numOfJitterBuffers > 1)	//Need to normalize ts across jb's with the session (E.g. RTSP based streaming).
    {
        for (i = 0; i < irPortParamsQueue.size(); i++)
        {
            PVMFJitterBufferPortParams* pPortParams = irPortParamsQueue[i];

            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                if (pPortParams->ipJitterBuffer != NULL)
                {
                    pPortParams->ipJitterBuffer->SetAdjustedTSInMS(mediaTS);
                }
            }
        }
    }

    return mediaTS;
}

/* computes the max next ts of all tracks */
OSCL_EXPORT_REF PVMFTimestamp PVMFJitterBufferMisc::GetActualMediaDataTSAfterSeek()
{
    PVMFTimestamp mediaTS = 0;
    uint32 in_wrap_count = 0;
    uint32 i;

    uint32 numOfJitterBuffers = 0;
    for (i = 0; i < irPortParamsQueue.size(); i++)
    {
        PVMFJitterBufferPortParams* pPortParams = irPortParamsQueue[i];

        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (pPortParams->ipJitterBuffer != NULL)
            {
                PVMFTimestamp ts =
                    pPortParams->ipJitterBuffer->peekNextElementTimeStamp();
                ++numOfJitterBuffers;
                /*
                 * Convert Time stamp to milliseconds
                 */
                pPortParams->iMediaClockConverter.set_clock(ts, in_wrap_count);
                PVMFTimestamp converted_ts =
                    pPortParams->iMediaClockConverter.get_converted_ts(1000);
                if (converted_ts > mediaTS)
                {
                    mediaTS = converted_ts;
                }
            }
        }
    }

    if (numOfJitterBuffers > 1)	//Need to normalize ts across jb's with the session (E.g. RTSP based streaming).
    {
        for (i = 0; i < irPortParamsQueue.size(); i++)
        {
            PVMFJitterBufferPortParams* pPortParams = irPortParamsQueue[i];

            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                if (pPortParams->ipJitterBuffer != NULL)
                {
                    pPortParams->ipJitterBuffer->SetAdjustedTSInMS(mediaTS);
                }
            }
        }
    }

    return mediaTS;
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::SetMediaClockConverter(PVMFPortInterface* apPort, MediaClockConverter* apMediaClockConverter)
{
    PVMFPortInterface* feedbackPort = NULL;
    PVMFJitterBuffer* jitterBuffer = NULL;
    if (LookupRTCPChannelParams(apPort, feedbackPort, jitterBuffer))
    {
        PVRTCPChannelController* rtcpChannelController = ipRTCPProtoImplementator->GetRTCPChannelController(feedbackPort);
        if (rtcpChannelController)
        {
            rtcpChannelController->SetMediaClockConverter(apMediaClockConverter);
        }
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferMisc::ProcessFirstPacketAfterSeek()
{
    //This call to "GetActualMediaDataTSAfterSeek" will normalize the ts's across the jitter buffers
    //associated with the streaming session
    GetActualMediaDataTSAfterSeek();
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end() ; iter++)
    {
        PVMFJitterBufferPortParams* pPortParams = *iter;
        if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
        {
            if (pPortParams->ipJitterBuffer != NULL)
            {
                pPortParams->ipJitterBuffer->AdjustRTPTimeStamp();
            }
        }
    }
}

PVMFStatus PVMFJitterBufferMisc::RTCPPacketReceived(RTCPPacketType aPacketType, PVRTCPChannelController* aController)
{
    PVMF_JB_LOG_RTCPDATATRAFFIC_IN_E((0, "PVMFJitterBufferMisc::RTCPPacketReceived -- iPlayStopTimeAvailable[%d]", iPlayStopTimeAvailable));
    if (BYE_RTCP_PACKET == aPacketType && aController)
    {
        //for live streams, treat RTCP BYE as EOS
        if (iPlayStopTimeAvailable == false)
        {
            PVMF_JB_LOGDATATRAFFIC_IN((0, "USING RTCP_BYE TO TRIGGER EOS: Mime=%s", aController->GetJitterBuffer().GetMimeType()));
            PVMF_JB_LOGDATATRAFFIC_OUT((0, "USING RTCP_BYE TO TRIGGER EOS: Mime=%s", aController->GetJitterBuffer().GetMimeType()));
            PVMF_JB_LOG_RTCPDATATRAFFIC_IN((0, "USING RTCP_BYE TO TRIGGER EOS: Mime=%s", aController->GetJitterBuffer().GetMimeType()));
            SetSessionDurationExpired();
            ipEstimatedServerClock->Pause();
            ipWallClock->Pause();
        }

        if (ipRTCPProtoImplementator->RTCPByeReceivedOnAllControllers())
        {
            SetSessionDurationExpired();
            ipEstimatedServerClock->Pause();
            ipWallClock->Pause();
        }

        PVUuid eventuuid = PVMFJitterBufferNodeEventTypeUUID;
        int32 infocode = PVMFJitterBufferNodeRTCPBYERecvd;

        PVMFBasicErrorInfoMessage* eventmsg;
        eventmsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (infocode, eventuuid, NULL));
        PVMFErrorInfoMessageInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFErrorInfoMessageInterface*, eventmsg);
        PVMFAsyncEvent asyncevent(PVMFInfoEvent,
                                  PVMFInfoRemoteSourceNotification,
                                  NULL,
                                  OSCL_STATIC_CAST(PVInterface*, interimPtr),
                                  (OsclAny*)(aController->GetJitterBuffer().GetMimeType()),
                                  NULL,
                                  0);
        ipObserver->ProcessRTCPControllerEvent(asyncevent);
        eventmsg->removeRef();
    }
    return PVMFSuccess;
}

PVMFStatus PVMFJitterBufferMisc::RTCPReportReadyToSend(PVMFPortInterface*& aPort, PVMFSharedMediaMsgPtr& aMessage)
{
    ipObserver->MessageReadyToSend(aPort, aMessage);
    return PVMFSuccess;
}

PVMFStatus PVMFJitterBufferMisc::ProcessInfoEvent(PVMFAsyncEvent& aEvent)
{
    ipObserver->ProcessRTCPControllerEvent(aEvent);
    return PVMFSuccess;
}

void PVMFJitterBufferMisc::PVMFJBSessionDurationTimerEvent()
{
    PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - Session Duration Timer Expired"));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - Session Duration Timer Expired"));
    /* Check if the estimated server clock is past the expected value */
    uint32 expectedEstServClockVal =
        ipSessionDurationTimer->GetExpectedEstimatedServClockValAtSessionEnd();
    uint32 timebase32 = 0;
    uint32 estServClock = 0;
    bool overflowFlag = false;


    ipEstimatedServerClock->GetCurrentTime32(estServClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - CurrEstServClock = %2d", estServClock));
    PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - ExpectedEstServClock = %2d", expectedEstServClockVal));
    if (estServClock >= expectedEstServClockVal)
    {
        PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent estServClock[%d] expectedEstServClockVal[%d]", estServClock, expectedEstServClockVal));
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - Session Duration Has Elapsed"));
        iStreamingSessionExpired = true;
        ipObserver->SessionSessionExpired();
        /* Cancel clock update notifications */
        Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;
        for (it = irPortParamsQueue.begin(); it != irPortParamsQueue.end(); it++)
        {
            PVMFJitterBufferPortParams* pPortParams = *it;

            if (pPortParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_INPUT)
            {
                pPortParams->ipJitterBuffer->SetEOS(true);
            }
        }
        /* Pause Estimated server clock & RTCP Clock */
        ipEstimatedServerClock->Pause();
        ipWallClock->Pause();
    }
    else
    {
        /*
         * Since we monitor the session duration in intervals, it is possible that this call back
         * happens when one such interval expires
         */
        uint64 elapsedTime = ipSessionDurationTimer->GetMonitoringIntervalElapsed();
        uint32 elapsedTime32 = Oscl_Int64_Utils::get_uint64_lower32(elapsedTime);
        ipSessionDurationTimer->UpdateElapsedSessionDuration(elapsedTime32);
        uint32 totalSessionDuration = ipSessionDurationTimer->getSessionDurationInMS();
        uint32 elapsedSessionDurationInMS = ipSessionDurationTimer->GetElapsedSessionDurationInMS();
        if (elapsedSessionDurationInMS < totalSessionDuration)
        {
            uint32 interval = (totalSessionDuration - elapsedSessionDurationInMS);
            if (interval > PVMF_JITTER_BUFFER_NODE_SESSION_DURATION_MONITORING_INTERVAL_MAX_IN_MS)
            {
                interval = PVMF_JITTER_BUFFER_NODE_SESSION_DURATION_MONITORING_INTERVAL_MAX_IN_MS;
            }
            ipSessionDurationTimer->setCurrentMonitoringIntervalInMS(interval);
            ipSessionDurationTimer->ResetEstimatedServClockValAtLastCancel();
            PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - TotalDuration=%d, ElapsedDuration=%d, CurrMonitoringInterval=%d", totalSessionDuration, elapsedSessionDurationInMS, interval));
        }
        else
        {
            /*
             * 1) Register for est serv clock update notifications on all jitter buffers
             * 2) Reschedule the session duration timer
             */
            PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - Past Session End Time - Starting to monitor Estimated Server Clock expectedEstServClockVal%d estServClock %d", expectedEstServClockVal, estServClock));

            uint64 diff = (expectedEstServClockVal - estServClock);
            uint32 diff32 = Oscl_Int64_Utils::get_uint64_lower32(diff);

            PVMFJBEventNotificationRequestInfo requestInfo(CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER, this, NULL);

            if (iEstimatedServerClockUpdateCallbackPending)
            {
                ipEventNotifier->CancelCallBack(requestInfo, iEstimatedServerClockUpdateCallbackId);
                iEstimatedServerClockUpdateCallbackPending = false;
            }

            ipEventNotifier->RequestAbsoluteTimeCallBack(requestInfo, expectedEstServClockVal, iEstimatedServerClockUpdateCallbackId);
            iEstimatedServerClockUpdateCallbackPending = true;

            /*
             * This is intentional. We do not expect the session duration and monitoring
             * intervals to exceed the max timer limit of 32 mins
             */
            ipSessionDurationTimer->setSessionDurationInMS(diff32);
            ipSessionDurationTimer->setCurrentMonitoringIntervalInMS(diff32);
            ipSessionDurationTimer->ResetEstimatedServClockValAtLastCancel();
            PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - ExpectedEstServClock=%d EstServClock=%d", Oscl_Int64_Utils::get_uint64_lower32(expectedEstServClockVal), Oscl_Int64_Utils::get_uint64_lower32(estServClock)));
            PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::PVMFJBSessionDurationTimerEvent - TotalDuration=%d, Interval=%d", diff32, diff32));
        }
        ipSessionDurationTimer->Start();
    }
    return;
}

void PVMFJitterBufferMisc::ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aClockNotificationInterfaceType);
    OSCL_UNUSED_ARG(aContext);
    if (aCallBkId == iEstimatedServerClockUpdateCallbackId && (PVMFSuccess == aStatus))
    {
        ipSessionDurationTimer->EstimatedServerClockUpdated();
    }
}

bool PVMFJitterBufferMisc::UseSessionDurationTimerForEOS()
{
    char mimeForSessioDurationTmrNecessity[] = "rtp";
    char mediaChannelMimeType[255] = {'0'};
    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator iter;
    for (iter = irPortParamsQueue.begin(); iter != irPortParamsQueue.end(); iter++)
    {
        PVMFJitterBufferPortParams* pPortParams = *iter;
        if (pPortParams && (pPortParams->ipJitterBuffer))
        {
            oscl_memset(mediaChannelMimeType, 0, sizeof(mediaChannelMimeType));
            const char* tmpMediaChannelMimeType = pPortParams->ipJitterBuffer->GetMimeType();
            const int32 mediaChannelMimeLen =  oscl_strlen(tmpMediaChannelMimeType);
            if (tmpMediaChannelMimeType)
            {
                for (int ii = 0; ii < mediaChannelMimeLen; ii++)
                {
                    mediaChannelMimeType[ii] = oscl_tolower(tmpMediaChannelMimeType[ii]);
                }
                mediaChannelMimeType[mediaChannelMimeLen] = '\0';
            }

            if (oscl_strstr(mediaChannelMimeType, mimeForSessioDurationTmrNecessity))
            {
                return true;
            }
        }
    }
    return false;//based on mime type
}

void PVMFJitterBufferMisc::LogClientAndEstimatedServerClock(PVLogger* aLogger)
{
    uint32 timebase32 = 0;
    uint32 clientClock32 = 0;
    uint32 serverClock32 = 0;
    bool overflowFlag = false;
    irClientPlaybackClock.GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);

    if (ipEstimatedServerClock)
        ipEstimatedServerClock->GetCurrentTime32(serverClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, aLogger, PVLOGMSG_INFO, (0, "Value of Client Clock %d and value of Estimated Sever Clock %d", clientClock32, serverClock32));
}

bool PVMFJitterBufferMisc::LookupRTCPChannelParams(PVMFPortInterface* rtpPort, PVMFPortInterface*& rtcpPort, PVMFJitterBuffer*& rtpPktJitterBuffer)
{
    bool retval = false;
    PVMFJitterBufferPort* jbInputPort = OSCL_STATIC_CAST(PVMFJitterBufferPort*, rtpPort);
    PVMFJitterBufferPortParams* inputPortParams = jbInputPort->GetPortParams();
    PVMFJitterBufferPortParams* feedbackPortParams = NULL;

    if (LocateFeedBackPort(inputPortParams, feedbackPortParams))
    {
        rtcpPort = 	&feedbackPortParams->irPort;
        rtpPktJitterBuffer = inputPortParams->ipJitterBuffer;
        retval = true;
    }
    return retval;
}

bool PVMFJitterBufferMisc::LocateFeedBackPort(PVMFJitterBufferPortParams*& aInputPortParamsPtr, PVMFJitterBufferPortParams*& aFeedBackPortParamsPtr)
{
    uint32 inputPortId = aInputPortParamsPtr->iId;

    /* Feedback port id must be inputPortId + 2 */

    Oscl_Vector<PVMFJitterBufferPortParams*, OsclMemAllocator>::iterator it;

    for (it = irPortParamsQueue.begin(); it != irPortParamsQueue.end(); it++)
    {
        PVMFJitterBufferPortParams* portParams = *it;
        if ((portParams->iTag == PVMF_JITTER_BUFFER_PORT_TYPE_FEEDBACK) &&
                ((int32)portParams->iId == (int32)inputPortId + 2))
        {
            aFeedBackPortParamsPtr = portParams;
            return true;
        }
    }
    return false;
}
