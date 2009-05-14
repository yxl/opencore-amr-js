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
#ifndef PVMF_JITTER_BUFFER_H_INCLUDED
#include "pvmf_jitter_buffer.h"
#endif

#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

#define PVMF_JITTER_BUFFER_ROLL_OVER_THRESHOLD_16BIT 2000
void PVMFJitterBufferImpl::ResetParams(bool aReleaseMemory)
{
    if (aReleaseMemory)
    {
        DestroyAllocators();
        OSCL_DELETE(iJitterBuffer);
        if (iBufferAlloc)
            iBufferAlloc->removeRef();
    }

    iStreamingState = STREAMINGSTATE_UNKNOWN;
    iSeqNum = 0;
    iReportCanRetrievePacket = false;

    iInPlaceProcessing = true;
    iOnePacketPerFragment = true;
    iOnePacketPerMediaMsg = true;

    iLastPacketOutTs = 0;

    iMediaDataGroupAlloc = NULL;
    iMediaDataImplMemPool = NULL;
    iMediaMsgMemPool = NULL;

    iStartTimeInMS = 0;
    iStopTimeInMS = 0;
    iPlayStopTimeAvailable = 0;
    iBroadCastSession = false;

    iMaxAdjustedRTPTS = 0;

    iSessionDurationExpired = false;
    iDurationInMilliSeconds = 0;
    iRebufferingThresholdInMilliSeconds = 0;

    iMonotonicTimeStamp = 0;
    iFirstSeqNum = 0;
    iJitterBuffer = NULL;
    iEOSSignalled = false;
    iEOSSent = false;
    iStreamID = 0;
    iMaxAdjustedTS = 0;
    iPrevAdjustedTS = 0;
    iHeaderPreParsed = false;

    iRTPTimeScale = 0;
    iPrevTSOut = 0;

    seqNumLock = false;

    iInterArrivalJitter = 0;
    oFirstPacket = false;


    SSRCLock = 0;
    oSSRCFromSetUpResponseAvailable = false;
    SSRCFromSetUpResponse = 0;

    iPrevSeqNumBaseOut = 0;
    seqLockTimeStamp = 0;

    iPrevAdjustedRTPTS = 0;
    iPrevTSIn = 0;
    iPrevSeqNumBaseIn = 0;

    iBufferAlloc = NULL;
    prevMinPercentOccupancy = 100;
    consecutiveLowBufferCount = 0;
    iNumUnderFlow = 0;

    iWaitForOOOPacketCallBkId = 0;
    iMonitorReBufferingCallBkId = 0;
    iJitterBufferDurationCallBkId = 0;

    iMonitorReBufferingCallBkPending = false;
    iWaitForOOOPacketCallBkPending = false;
    iJitterBufferDurationCallBkPending = false;

    ipMediaClockConverter = NULL;
    iTimeScale	= 0;
}

void PVMFJitterBufferImpl::DestroyAllocators()
{
    /* Destroy allocators if present */
    if (iMediaDataGroupAlloc != NULL)
    {
        iMediaDataGroupAlloc->CancelFreeChunkAvailableCallback();
        iMediaDataGroupAlloc->removeRef();
        iMediaDataGroupAlloc = NULL;
    }
    if (iMediaDataImplMemPool != NULL)
    {
        iMediaDataImplMemPool->removeRef();
        iMediaDataImplMemPool = NULL;
    }
    if (iMediaMsgMemPool != NULL)
    {
        iMediaMsgMemPool->removeRef();
        iMediaMsgMemPool = NULL;
    }
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::StreamingSessionStarted()
{
    iStreamingState	= STREAMINGSTATE_STARTED;
    iNumUnderFlow = 0;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::StreamingSessionPaused()
{
    iStreamingState	=	STREAMINGSTATE_PAUSED;
    CancelEventCallBack(JB_NOTIFY_WAIT_FOR_OOO_PACKET_COMPLETE);
    CancelEventCallBack(JB_MONITOR_REBUFFERING);
    CancelEventCallBack(JB_BUFFERING_DURATION_COMPLETE);
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::StreamingSessionStopped()
{
    iStreamingState	=	STREAMINGSTATE_STOPPED;
    CancelEventCallBack(JB_NOTIFY_WAIT_FOR_OOO_PACKET_COMPLETE);
    CancelEventCallBack(JB_MONITOR_REBUFFERING);
    CancelEventCallBack(JB_BUFFERING_DURATION_COMPLETE);
    FlushJitterBuffer();
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetRebufferingThresholdInMilliSeconds(uint32 aRebufferingThresholdInMilliSeconds)
{
    iRebufferingThresholdInMilliSeconds = aRebufferingThresholdInMilliSeconds;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetDurationInMilliSeconds(uint32 aDuration)
{
    iDurationInMilliSeconds = aDuration;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::PrepareForRepositioning()
{
    irDataState = PVMF_JITTER_BUFFER_IN_TRANSITION;
    iEOSSignalled = false;
    iEOSSent = false;
    irDelayEstablished = false;
}

OSCL_EXPORT_REF PVMFJitterBufferDataState PVMFJitterBufferImpl::GetState() const
{
    return irDataState;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetJitterBufferState(PVMFJitterBufferDataState aState)
{
    irDataState = aState;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator)
{
    iBufferAlloc = aDataBufferAllocator;
    iBufferAlloc->addRef();
}

OSCL_EXPORT_REF const char* PVMFJitterBufferImpl::GetMimeType() const
{
    return irMimeType.get_cstr();
}



///////////////////////////////////////////////////////////////////////////////
// Creation and cleanup functions
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFJitterBufferImpl::PVMFJitterBufferImpl(const PVMFJitterBufferConstructParams& aJBConstructParams)
        : irEstimatedServerClock(aJBConstructParams.GetEstimatedServerClock())
        , irClientPlayBackClock(aJBConstructParams.GetClientPlaybackClock())
        , irJBEventNotifier(aJBConstructParams.GetEventNotifier())
        , irMimeType(aJBConstructParams.GetMimeType())
        , irDelayEstablished(aJBConstructParams.GetDelayEstablishStatus())
        , irJitterDelayPercent(aJBConstructParams.GetJBDelayPercent())
        , irDataState(aJBConstructParams.GetJitterBufferState())
        , iObserver(aJBConstructParams.GetJBObserver())
        , iObserverContext(aJBConstructParams.GetContextData())
{
    iMediaMsgMemPool = NULL;
    iMediaDataImplMemPool = NULL;
    iMediaDataGroupAlloc = NULL;

    ipLogger = NULL;
    ipClockLoggerSessionDuration = NULL;
    ipDataPathLogger = NULL;
    ipDataPathLoggerIn = NULL;
    ipDataPathLoggerOut = NULL;
    ipClockLogger = NULL;
    ipRTCPDataPathLoggerIn = NULL;
    ipRTCPDataPathLoggerOut = NULL;
    ipClockLoggerRebuff = NULL;
    ipDataPathLoggerFlowCtrl = NULL;
    ipJBEventsClockLogger = NULL;

    ResetParams(false);
}



OSCL_EXPORT_REF PVMFJitterBufferImpl::~PVMFJitterBufferImpl()
{
    ResetJitterBuffer();
    ResetParams();
    DestroyAllocators();
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::Construct()
{
    ipLogger = PVLogger::GetLoggerObject("JitterBuffer");
    ipClockLoggerSessionDuration = PVLogger::GetLoggerObject("clock.streaming_manager.sessionduration");
    ipDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer");
    ipDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.in");
    ipDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.out");
    ipClockLogger = PVLogger::GetLoggerObject("clock.jitterbuffer");
    ipRTCPDataPathLoggerIn = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp.in");
    ipRTCPDataPathLoggerOut = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.rtcp.out");
    ipMaxRTPTsLogger = PVLogger::GetLoggerObject("MaxRTPTs");
    ipClockLoggerRebuff = PVLogger::GetLoggerObject("clock.jitterbuffernode.rebuffer");
    ipDataPathLoggerFlowCtrl = PVLogger::GetLoggerObject("datapath.sourcenode.jitterbuffer.flowctrl");
    ipJBEventsClockLogger = PVLogger::GetLoggerObject("eventnotifier.PVMFJitterBufferImpl");

    if (!iInPlaceProcessing)
    {
        CreateAllocators();
    }

    uint32 numNodes = 0;

#if (PMVF_JITTER_BUFFER_NODE_USE_NO_RESIZE_ALLOC)
    numNodes = PVMF_JB_NO_RESIZE_ALLOC_NUM_CHUNKS_RTP;
#else
    numNodes = DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER;
#endif
    iJitterBuffer = OSCL_NEW(PVMFDynamicCircularArrayType,
                             (numNodes));


}

///////////////////////////////////////////////////////////////////////////////
// Interface functions
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFJitterBufferRegisterMediaMsgStatus PVMFJitterBufferImpl::RegisterMediaMsg(PVMFSharedMediaMsgPtr& aMsg)
{
    PVMFJitterBufferRegisterMediaMsgStatus retval = PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS;
    PVMFSharedMediaDataPtr dataPacket;

    if (aMsg->getFormatID() > PVMF_MEDIA_CMD_FORMAT_IDS_START)	//IsCmdMsg
    {
        RegisterCmdPacket(aMsg);
    }
    else
    {
        LOGCLIENTANDESTIMATEDSERVCLK_DATAPATH;
        //1. Parse the header, and validate the packet
        //2. Create the packet (if iInPlaceProcessing is false)
        //3. Update statistics
        //4. Register the packet
        //5. Check for flow control [JB specific]
        PVMFSharedMediaDataPtr inputDataPacket;
        convertToPVMFMediaData(inputDataPacket, aMsg);
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataIn;
        if (!inputDataPacket->getMediaDataImpl(mediaDataIn))
        {
            PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::ProcessIncomingMsgRTP: corrupt input media msg"));
            PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::ProcessIncomingMsgRTP: corrupt input media msg"));
            retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_CORRUPT_PACKET;
        }
        else
        {
            if (iOnePacketPerMediaMsg)
            {
                PVMFSharedMediaDataPtr dataPacket;
                PVMFJBPacketParsingAndStatUpdationStatus status = ParsePacketHeaderAndUpdateJBStats(inputDataPacket, dataPacket);
                if (PVMF_JB_PACKET_PARSING_SUCCESS == status)
                {
                    if (CanRegisterMediaMsg())
                    {
                        if (CheckSpaceAvailability())
                        {
                            retval = RegisterDataPacket(inputDataPacket);

                        }
                        else
                        {
                            retval = PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_JB_FULL;
                        }
                    }
                    else
                    {
                        if (dataPacket.GetRep())
                        {
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                            PVMF_JB_LOGDATATRAFFIC_FLOWCTRL_E((0, "PVMFJitterBufferNode::ProcessIncomingMsgRTP: - Ignoring - Wrong State"
                                                               "Size=%d, SSRC=%d", inputDataPacket->getFilledSize(), inputDataPacket->getStreamID()));
                            PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: - Ignoring - Wrong State"
                                                       "Size=%d, SSRC=%d", inputDataPacket->getFilledSize(), inputDataPacket->getStreamID()));
                            PVMF_JB_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: - Ignoring - Wrong State"
                                              "Size=%d, SSRC=%d", inputDataPacket->getFilledSize(), inputDataPacket->getStreamID()));
#endif
                        }
                        PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::RegisterMediaMsg: Invalid State to accept any packet"));
                        PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::RegisterMediaMsg: Invalid State to accept any packet"));
                        retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_CORRUPT_PACKET;
                    }
                }
                else
                {
                    if (PVMF_JB_ERR_INSUFFICIENT_MEM_TO_PACKETIZE == status)
                    {
                        retval = PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_INSUFFICIENT_MEMORY_FOR_PACKETIZATION;
                    }
                    if (PVMF_JB_ERR_INVALID_CONFIGURATION == status)
                    {
                        retval = PVMF_JB_BUFFER_REGISTER_MEDIA_MSG_ERR_CONFIG_NOT_SUPPORTED;
                    }
                    if ((PVMF_JB_ERR_CORRUPT_HDR == status) || (PVMF_JB_ERR_TRUNCATED_HDR == status))
                    {
                        retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_INVALID_HEADER;
                    }
                    if (PVMF_JB_ERR_UNEXPECTED_PKT == status)
                    {
                        retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNEXPECTED_DATA;
                    }
                    if (PVMF_JB_ERR_LATE_PACKET == status)
                    {
                        retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_LATE_MSG;
                    }
                    PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::RegisterMediaMsg: ParsePacketHeader failed retval %d", retval));
                }
            }
            else
            {
                if (iOnePacketPerFragment)
                {
                    const uint32 numOfFragments = inputDataPacket->getNumFragments();
                    for (uint ii = 0; ii < numOfFragments ; ii++)
                    {
                        PVMFSharedMediaDataPtr dataPacket;
                        PVMFJBPacketParsingAndStatUpdationStatus status = ParsePacketHeaderAndUpdateJBStats(inputDataPacket, dataPacket, ii);
                        if (PVMF_JB_PACKET_PARSING_SUCCESS == status)
                        {
                            if (CanRegisterMediaMsg())
                            {
                                if (CheckSpaceAvailability())
                                {
                                    retval = RegisterDataPacket(dataPacket);
                                }
                                else
                                {
                                    retval = PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_JB_FULL;
                                }
                            }
                            else
                            {
                                if (dataPacket.GetRep())
                                {
#if (PVLOGGER_INST_LEVEL > PVLOGMSG_INST_LLDBG)
                                    PVMF_JB_LOGDATATRAFFIC_FLOWCTRL_E((0, "PVMFJitterBufferNode::ProcessIncomingMsgRTP: - Ignoring - Wrong State"
                                                                       "Size=%d, SSRC=%d", inputDataPacket->getFilledSize(), dataPacket->getStreamID()));
                                    PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferNode::ProcessIncomingMsg: - Ignoring - Wrong State"
                                                               "Size=%d, SSRC=%d", inputDataPacket->getFilledSize(), dataPacket->getStreamID()));
                                    PVMF_JB_LOGERROR((0, "PVMFJitterBufferNode::ProcessIncomingMsg: - Ignoring - Wrong State"
                                                      "Size=%d, SSRC=%d", inputDataPacket->getFilledSize(), dataPacket->getStreamID()));
#endif
                                }
                                PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::RegisterMediaMsg: Invalid State to accept any packet"));
                                PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::RegisterMediaMsg: Invalid State to accept any packet"));
                                retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_CORRUPT_PACKET;
                            }
                        }
                        else
                        {
                            if (PVMF_JB_ERR_INSUFFICIENT_MEM_TO_PACKETIZE == status)
                            {
                                retval = PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_INSUFFICIENT_MEMORY_FOR_PACKETIZATION;
                            }
                            if (PVMF_JB_ERR_INVALID_CONFIGURATION == status)
                            {
                                retval = PVMF_JB_BUFFER_REGISTER_MEDIA_MSG_ERR_CONFIG_NOT_SUPPORTED;
                            }
                            if ((PVMF_JB_ERR_CORRUPT_HDR == status) || (PVMF_JB_ERR_TRUNCATED_HDR == status))
                            {
                                retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_INVALID_HEADER;
                            }
                            if (PVMF_JB_ERR_UNEXPECTED_PKT == status)
                            {
                                retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNEXPECTED_DATA;
                            }
                            if (PVMF_JB_ERR_LATE_PACKET == status)
                            {
                                retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_LATE_MSG;
                            }
                            PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::RegisterMediaMsg: ParsePacketHeader failed retval %d", retval));
                        }
                        if (retval == PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_INSUFFICIENT_MEMORY_FOR_PACKETIZATION || retval == PVMF_JB_REGISTER_MEDIA_MSG_FAILURE_JB_FULL)
                        {
                            //Serious err.. drop the packets... May be leave and let the obs trap...?
                            break;
                        }
                    }
                }
                else
                {
                    //invalid use case, not expected, cause exception(leave) here
                    OSCL_LEAVE(JBPacketRegistrationConfigurationCorrupted);
                }
            }
        }
    }
    return retval;
}

OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferImpl::RetrievePacket(PVMFSharedMediaMsgPtr& aMediaMsgPtr, bool& aCmdPacket)
{
    aCmdPacket = false;
    PVMFSharedMediaMsgPtr mediaMsg;
    if (CanRetrievePacket(aMediaMsgPtr, aCmdPacket))
    {
        return PVMFSuccess;
    }
    else
    {
        return PVMFErrNotReady;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferImpl::SetInputPacketHeaderPreparsed(bool aPreParsed)
{
    iHeaderPreParsed = aPreParsed;
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFJitterBufferStats& PVMFJitterBufferImpl::getJitterBufferStats()
{
    OSCL_ASSERT(iJitterBuffer);
    return (iJitterBuffer->getStats());
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::FlushJitterBuffer()
{
    iFirstDataPackets.clear();
    iJitterBuffer->Clear();
    iJitterBuffer->ResetJitterBufferStats();
    iRTPInfoParamsVec.clear();
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::ResetJitterBuffer()
{
    FlushJitterBuffer();
    iJitterBuffer->ResetJitterBufferStats();

    iRTPInfoParamsVec.clear();
    iMonotonicTimeStamp = 0;
    iPrevTSOut = 0;
    iMaxAdjustedRTPTS = 0;
}

OSCL_EXPORT_REF PVMFTimestamp PVMFJitterBufferImpl::peekNextElementTimeStamp()
{
    if (iJitterBuffer->getNumElements() > 0)
    {
        PVMFTimestamp currTS;
        PVMFTimestamp prevTS;
        uint32 aSeqNum;
        iJitterBuffer->peekNextElementTimeStamp(currTS, aSeqNum);
        DeterminePrevTimeStampPeek(aSeqNum, prevTS);
        uint64 ts64 = iMonotonicTimeStamp;
        ts64 += (currTS - prevTS);
        PVMFTimestamp adjTS =
            (PVMFTimestamp)(Oscl_Int64_Utils::get_uint64_lower32(ts64));
        return (adjTS);
    }
    else
    {
        PVMFTimestamp adjTS =
            (PVMFTimestamp)(Oscl_Int64_Utils::get_uint64_lower32(iMonotonicTimeStamp));
        return (adjTS);
    }
}

OSCL_EXPORT_REF PVMFTimestamp PVMFJitterBufferImpl::peekMaxElementTimeStamp()
{
    if (iJitterBuffer->getNumElements() > 0)
    {
        PVMFTimestamp currTS;
        PVMFTimestamp maxTS;
        PVMFTimestamp prevTS;
        uint32 aSeqNum;
        iJitterBuffer->peekNextElementTimeStamp(currTS, aSeqNum);
        DeterminePrevTimeStampPeek(aSeqNum, prevTS);
        iJitterBuffer->peekMaxElementTimeStamp(maxTS, aSeqNum);
        uint64 ts64 = iMonotonicTimeStamp;
        ts64 += (maxTS - prevTS);
        PVMFTimestamp adjTS =
            (PVMFTimestamp)(Oscl_Int64_Utils::get_uint64_lower32(ts64));
        return (adjTS);
    }
    else
    {
        PVMFTimestamp adjTS =
            (PVMFTimestamp)(Oscl_Int64_Utils::get_uint64_lower32(iMonotonicTimeStamp));
        return (adjTS);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFJitterBufferRegisterMediaMsgStatus PVMFJitterBufferImpl::RegisterCmdPacket(PVMFSharedMediaMsgPtr& aMediaCmd)
{
    if (aMediaCmd->getFormatID() == PVMF_MEDIA_CMD_EOS_FORMAT_ID)
    {
        PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::RegisterCmdPacket - EOS Received"));
        PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::RegisterCmdPacket - EOS Received"));
        EOSCmdReceived();
    }
    else /* unknown command */
    {
        PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::ProcessIncomingMsgRTP - Unknown Cmd Recvd"));
        PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::ProcessIncomingMsgRTP - Unknown Cmd Recvd"));
        PVMFJitterBufferStats stats = getJitterBufferStats();
        MediaCommandMsgHolder cmdHolder;
        cmdHolder.iPreceedingMediaMsgSeqNumber = stats.lastRegisteredSeqNum;
        cmdHolder.iCmdMsg = aMediaCmd;
        iMediaCmdVec.push_back(cmdHolder);
    }
    return PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS;
}

PVMFJitterBufferRegisterMediaMsgStatus PVMFJitterBufferImpl::RegisterDataPacket(PVMFSharedMediaDataPtr& aDataPacket)
{
    PVMFJitterBufferRegisterMediaMsgStatus retval = AddPacket(aDataPacket);
    if (retval == PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS)
    {
        PerformFlowControl(true);
    }
    uint32 aClockDiff = 0;
    bool delayEstablished = IsDelayEstablished(aClockDiff);//To update delay percent
    if (iReportCanRetrievePacket && delayEstablished)
    {
        //Check if delay is established
        //There's no hole for for min threshold time
        if (CanRetrievePacket())
        {
            iObserver->PacketReadyToBeRetrieved(iObserverContext);
            iReportCanRetrievePacket = false;
        }
    }
    return retval;
}

PVMFJitterBufferRegisterMediaMsgStatus PVMFJitterBufferImpl::AddPacket(PVMFSharedMediaDataPtr& aDataPacket)
{
    PVMFJitterBufferRegisterMediaMsgStatus retval = PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS;
    if (iEOSSent)
    {
        /* EOS received - do not register packet */
        PVMF_JB_LOGWARNING((0, "0x%x PVMFJitterBufferImpl::addPacket: After EOS Reached!!!", this));
        return PVMF_JB_REGISTER_MEDIA_MSG_ERR_EOS_SIGNALLED;
    }

    if (seqNumLock)
    {
        if (oSSRCFromSetUpResponseAvailable == false)
        {
            oSSRCFromSetUpResponseAvailable = true;
            SSRCLock = aDataPacket->getStreamID();
            PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::addPacket: Ser No SSRC, set to 1st pkt SSRC %d", SSRCLock));
        }
        /* Filter based on SSRC */
        if (aDataPacket->getStreamID() == SSRCLock)
        {
            PVMFJitterBufferAddElemStatus status;

            status = iJitterBuffer->addElement(aDataPacket, iFirstSeqNum);
            if (status == PVMF_JITTER_BUFFER_ADD_ELEM_SUCCESS)
            {
                PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::addPacket: MimeType=%s TS=%u, SEQNUM= %d",
                                           irMimeType.get_cstr(), aDataPacket->getTimestamp(), aDataPacket->getSeqNum()));

                if (iRTPInfoParamsVec.size() > 0)
                {
                    /*
                     * Calculate adjusted RTP TS - Will be used to update
                     * the estimated server clock, if any only if some rtp-info
                     * params have been set
                     */
                    ComputeMaxAdjustedRTPTS();
                }
            }
            else if (status == PVMF_JITTER_BUFFER_ADD_ELEM_PACKET_OVERWRITE)
            {
                PVMF_JB_LOGDATATRAFFIC_IN_E((0, "PVMFJitterBufferImpl::addPacket - OverWrite: MimeType=%s TS=%d, SEQNUM= %d",
                                             irMimeType.get_cstr(), aDataPacket->getTimestamp(), aDataPacket->getSeqNum()));
                retval = PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNKNOWN;
            }
        }
    }
    else
    {
        if (oSSRCFromSetUpResponseAvailable && (aDataPacket->getStreamID() != SSRCFromSetUpResponse))
        {//discard packet
            PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::addPacket: ERROR wrong ssrc %d", aDataPacket->getStreamID()));
            return PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS;
        }
        // Add packet to temporary array
        iFirstDataPackets.push_back(aDataPacket);

        const uint cPktNeededForVote = 5;
        if (iFirstDataPackets.size() < cPktNeededForVote)
            return PVMF_JB_REGISTER_MEDIA_MSG_SUCCESS;

        //Majortiy vote for SSRC first
        //0 count; 1 ssrc; 2 seqbase; 3 timestamp; uint32 PVMFTimestamp;
        uint32 my_ssrc[cPktNeededForVote][4];
        for (uint32 i = 0; i < cPktNeededForVote; i++)
            my_ssrc[i][0] = my_ssrc[i][1] = my_ssrc[i][2] = my_ssrc[i][3] = 0;

        {
            // 1. vote
            for (uint32 i = 0; i < cPktNeededForVote; i++)
            {
                uint32 ssrc = iFirstDataPackets[i]->getStreamID();
                for (uint32 j = 0; j < cPktNeededForVote; j++)
                {
                    if (my_ssrc[j][0] > 0)
                    {
                        if (ssrc ==  my_ssrc[j][1])
                        {
                            my_ssrc[j][0]++;
                            if (iFirstDataPackets[i]->getSeqNum() < my_ssrc[j][2])
                            {
                                my_ssrc[j][2] = iFirstDataPackets[i]->getSeqNum();
                                my_ssrc[j][3] = iFirstDataPackets[i]->getTimestamp();
                            }
                            break;
                        }
                    }
                    else
                    {
                        my_ssrc[j][0]++; //my_ssrc[j][0]=1
                        my_ssrc[j][1] = ssrc;
                        my_ssrc[j][2] = iFirstDataPackets[i]->getSeqNum();
                        my_ssrc[j][3] = iFirstDataPackets[i]->getTimestamp();
                        break;
                    }
                }
            }
        }

        {// 2. poll ssrc
            uint32 first_ssrc_index = 0, second_ssrc_index = 1;
            if (my_ssrc[0][0] < my_ssrc[1][0])
            {
                first_ssrc_index = 1;
                second_ssrc_index = 0;
            }
            for (uint32 i = 2; i < cPktNeededForVote; i++)
            {
                if (my_ssrc[i][0] > first_ssrc_index)
                {
                    second_ssrc_index = first_ssrc_index;
                    first_ssrc_index = i;
                }
                else if (my_ssrc[i][0] > second_ssrc_index)
                {
                    second_ssrc_index =  i;
                }
            }
            if (my_ssrc[first_ssrc_index][0] <= my_ssrc[second_ssrc_index][0])
            {
                //got a tie. should rarely happen
                //for now, just pick the first one
                PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::addPacket: ERROR first %d second %d", first_ssrc_index, second_ssrc_index));
            }
            SSRCLock = my_ssrc[first_ssrc_index][1];
            seqNumLock = true;

            //if we got RTPInfo by now, we should still use it
            if (iRTPInfoParamsVec.size() > 0)
            {
                Oscl_Vector<PVMFRTPInfoParams, OsclMemAllocator>::iterator it;
                it = iRTPInfoParamsVec.begin();
                iFirstSeqNum = (it->seqNumBaseSet) ? it->seqNum : my_ssrc[first_ssrc_index][2];
                seqLockTimeStamp = (it->rtpTimeBaseSet) ? it->rtpTime : my_ssrc[first_ssrc_index][3];
            }
            else
            {
                iFirstSeqNum = my_ssrc[first_ssrc_index][2];
                seqLockTimeStamp = my_ssrc[first_ssrc_index][3];
            }
            // iFirstSeqNum must be initialized when we come here
            iJitterBuffer->setFirstSeqNumAdded(iFirstSeqNum);
            CheckForRTPTimeAndRTPSeqNumberBase();
        }
        // 3.throw away the pkt not belong to current session and register packets

        bool bNoErr = true;
        Oscl_Vector<PVMFSharedMediaDataPtr, OsclMemAllocator>::iterator it;
        for (it = iFirstDataPackets.begin(); it != iFirstDataPackets.end();		it++)
        {
            if ((*it)->getStreamID() == SSRCLock)
            {
                //Validate the packet before trying to push

                PVMFSharedMediaDataPtr& elem = *it;
                uint32 seqNum = elem->getSeqNum();
                PVMFTimestamp packetTs = elem->getTimestamp();

                if (iFirstSeqNum != seqNum)
                {
                    PVMFJitterBufferStats& jbStats = getJitterBufferStats();

                    if (iBroadCastSession == true)
                    {
                        /*
                         * This can happen when using prerecorded transport streams that loop
                         * If this happens, just signal an unexpected data event
                         */
                        if (packetTs < jbStats.maxTimeStampRegistered)
                        {
                            return PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNEXPECTED_DATA;
                        }
                    }

                    if (!IsSeqTsValidForPkt(seqNum, packetTs, jbStats))
                    {
                        return PVMF_JB_REGISTER_MEDIA_MSG_ERR_LATE_MSG;
                    }
                }

                if (! iJitterBuffer->addElement(*it, iFirstSeqNum))
                {
                    PVMF_JB_LOGERROR((0, "PVMFJitterBufferImpl::addPacket: addElement failed"));
                    bNoErr = false;
                }
                else
                {
                    PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::addPacket: MimeType=%s TS=%d, SEQNUM= %d",
                                               irMimeType.get_cstr(), (*it)->getTimestamp(), (*it)->getSeqNum()));
                }
            }
        }

        iFirstDataPackets.clear();
        if (iRTPInfoParamsVec.size() > 0)
        {
            /*
            * Calculate adjusted RTP TS - Will be used to update
            * the estimated server clock, if any only if some rtp-info
            * params have been set
            	*/
            ComputeMaxAdjustedRTPTS();
        }

        if (!bNoErr)
            return PVMF_JB_REGISTER_MEDIA_MSG_ERR_UNKNOWN;

    }

    PVMFJitterBufferStats stats = getJitterBufferStats();
    if (stats.totalPacketsLost > 0)
    {
        PVMF_JB_LOGWARNING((0, "Packet Loss: MimeType=%s, NumPacketsLost=%d", irMimeType.get_cstr(), stats.totalPacketsLost));
    }
    PVMF_JB_LOGINFO((0, "PVMFJitterBufferImpl::addPacket - JB Occup Stats - MimeType=%s, MaxSize=%d, CurrOccupany=%d", irMimeType.get_cstr(), iJitterBuffer->getArraySize(), iJitterBuffer->getNumElements()));
    return retval;
}

///////////////////////////////////////////////////////////////////////////////
//Prtected functions to implement the interface functions
///////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CanRegisterMediaMsg()
{
    if ((STREAMINGSTATE_STARTED == iStreamingState) || (STREAMINGSTATE_PAUSED == iStreamingState))
    {
        return true;
    }
    return false;
}

void PVMFJitterBufferImpl::CreateAllocators()
{
    //Create allocators
    iMediaMsgMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator,
                                (DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER));
    iMediaDataImplMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator,
                                     (DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER));
    iMediaDataGroupAlloc = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemPoolFixedChunkAllocator>,
                                    (DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER,
                                     DEFAULT_NUM_FRAGMENTS_IN_MEDIA_MSG,
                                     iMediaDataImplMemPool));
    iMediaDataGroupAlloc->create();
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CreateMediaData(PVMFSharedMediaDataPtr& dataPacket, OsclSharedPtr<PVMFMediaDataImpl>& mediaDataOut)
{
    int32 err;

    OSCL_TRY(err,
             dataPacket = PVMFMediaData::createMediaData(mediaDataOut,
                          iMediaMsgMemPool););

    if (err != OsclErrNone)
    {
        return false;
    }

    return true;
}
OSCL_EXPORT_REF bool PVMFJitterBufferImpl::Allocate(OsclSharedPtr<PVMFMediaDataImpl>& mediaDataOut)
{
    int32 err;
    OSCL_TRY(err,
             mediaDataOut = iMediaDataGroupAlloc->allocate());

    if (err != OsclErrNone)
    {
        return false;
    }

    // If there is no memory left return false
    if (mediaDataOut.GetRep() == NULL)
    {
        return false;
    }

    return true;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::RequestEventCallBack(JB_NOTIFY_CALLBACK aEventType, uint32 aDelay, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    bool retval = false;
    switch (aEventType)
    {
        case JB_NOTIFY_WAIT_FOR_OOO_PACKET_COMPLETE :
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER, this, NULL);
            retval = irJBEventNotifier.RequestCallBack(eventRequestInfo, aDelay, iWaitForOOOPacketCallBkId);
            if (retval)
            {
                PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::RequestEventCallBack In OOO CallBackId [%d] Mime %s", iWaitForOOOPacketCallBkId, irMimeType.get_cstr()));
                iWaitForOOOPacketCallBkPending = true;
            }
        }
        break;
        case JB_MONITOR_REBUFFERING:
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK, this, NULL);
            retval = irJBEventNotifier.RequestCallBack(eventRequestInfo, aDelay, iMonitorReBufferingCallBkId);
            if (retval)
            {
                PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::RequestEventCallBack In Rebuffering CallBackId [%d] Mime %s", iMonitorReBufferingCallBkId, irMimeType.get_cstr()));
                iMonitorReBufferingCallBkPending = true;
            }
        }
        break;
        case JB_BUFFERING_DURATION_COMPLETE:
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
            retval = irJBEventNotifier.RequestCallBack(eventRequestInfo, iDurationInMilliSeconds, iJitterBufferDurationCallBkId);
            if (retval)
            {
                iJitterBufferDurationCallBkPending = true;
            }
        }
        break;
        default:
        {
            //Log it
        }
    }

    return retval;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::CancelEventCallBack(JB_NOTIFY_CALLBACK aEventType, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    switch (aEventType)
    {
        case JB_NOTIFY_WAIT_FOR_OOO_PACKET_COMPLETE:
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_ESTIMATEDSERVER, this, NULL);
            irJBEventNotifier.CancelCallBack(eventRequestInfo, iWaitForOOOPacketCallBkId);
            iWaitForOOOPacketCallBkPending = false;
        }
        break;
        case JB_MONITOR_REBUFFERING:
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_CLIENTPLAYBACK, this, NULL);
            irJBEventNotifier.CancelCallBack(eventRequestInfo, iMonitorReBufferingCallBkId);
            iMonitorReBufferingCallBkPending = false;
        }
        break;
        case JB_BUFFERING_DURATION_COMPLETE:
        {
            PVMFJBEventNotificationRequestInfo eventRequestInfo(CLOCK_NOTIFICATION_INTF_TYPE_NONDECREASING, this, NULL);
            irJBEventNotifier.CancelCallBack(eventRequestInfo, iJitterBufferDurationCallBkId);
            iJitterBufferDurationCallBkPending = false;
        }
        break;

        default:
        {
            //Log it
        }
    }
    return;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::ProcessCallback(CLOCK_NOTIFICATION_INTF_TYPE aClockNotificationInterfaceType, uint32 aCallBkId, const OsclAny* aContext, PVMFStatus aStatus)
{
    OSCL_UNUSED_ARG(aClockNotificationInterfaceType);
    PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::ProcessCallBack In CallBackId [%d] Mime %s", aCallBkId, irMimeType.get_cstr()));

    if (PVMFSuccess == aStatus)
    {
        if (aCallBkId == iWaitForOOOPacketCallBkId)
        {
            iWaitForOOOPacketCallBkPending = false;
            HandleEvent_NotifyWaitForOOOPacketComplete(aContext);
        }

        if (aCallBkId == iMonitorReBufferingCallBkId)
        {
            iMonitorReBufferingCallBkPending = false;
            HandleEvent_MonitorReBuffering(aContext);
        }
        else if (aCallBkId == iJitterBufferDurationCallBkId)
        {
            iJitterBufferDurationCallBkPending = false;
            HandleEvent_JitterBufferBufferingDurationComplete();
        }
    }
    else
    {
        //Log it
    }
    PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::ProcessCallBack Out"));
}

void PVMFJitterBufferImpl::HandleEvent_MonitorReBuffering(const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    uint32 timebase32 = 0;
    uint32 estServerClock = 0;
    uint32 clientClock = 0;
    bool overflowFlag = false;
    irEstimatedServerClock.GetCurrentTime32(estServerClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    irClientPlayBackClock.GetCurrentTime32(clientClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);


    PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - EstServClock=%d", estServerClock));
    PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - ClientClock=%d", clientClock));

    uint32 clockDiff;
    IsDelayEstablished(clockDiff); //just to evaluate the rebuiffering condition.
}

void PVMFJitterBufferImpl::HandleEvent_NotifyWaitForOOOPacketComplete(const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aContext);
    if (iReportCanRetrievePacket)
    {
        iObserver->PacketReadyToBeRetrieved(iObserverContext);
    }
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::IsCallbackPending(JB_NOTIFY_CALLBACK aEventType, OsclAny* aContext)
{
    bool*  callBackPending = NULL;
    switch (aEventType)
    {
        case JB_MONITOR_REBUFFERING:
        {
            callBackPending = &iMonitorReBufferingCallBkPending;
        }
        break;
        case JB_NOTIFY_WAIT_FOR_OOO_PACKET_COMPLETE:
        {
            callBackPending = &iWaitForOOOPacketCallBkPending;
        }
        break;
        case JB_BUFFERING_DURATION_COMPLETE:
        {
            callBackPending = &iJitterBufferDurationCallBkPending;
        }
        break;
        default:
            OSCL_ASSERT(false);
    }
    // PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::IsCallbackPending - Event Type[%d] CallBackPending [%d] aContext[0x%x]", aEventType, *callBackPending, aContext));
    return *callBackPending;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::QueueBOSCommand(uint32 aStreamId)
{
    iStreamID = aStreamId;
    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
    // Set the formatID, timestamp, sequenceNumber and streamID for the media message
    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);
    uint32 seqNum = 0;
    sharedMediaCmdPtr->setSeqNum(seqNum);
    sharedMediaCmdPtr->setStreamID(iStreamID);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);


    addMediaCommand(mediaMsgOut);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, ipLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFJitterBufferNode::QueueBOSCommand() MIME=%s StreamID=%d", irMimeType.get_cstr(), iStreamID));
    return true;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetInPlaceProcessingMode(bool aInPlaceProcessingMode)
{
    iInPlaceProcessing = aInPlaceProcessingMode;
    if (iInPlaceProcessing)
    {
        iOnePacketPerMediaMsg = true;
    }
    else
    {
        iOnePacketPerMediaMsg = false;
    }

    iOnePacketPerFragment = true;

    if (iInPlaceProcessing == false)
    {
        CreateAllocators();
    }
    else
    {
        DestroyAllocators();
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFJitterBufferImpl::GenerateAndSendEOSCommand(PVMFSharedMediaMsgPtr& aMediaOutMsg, bool& aCmdPacket)
{
    PVMF_JB_LOGINFO((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand"));
    PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "PVMFJitterBufferImpl::GenerateAndSendEOSCommand In Mime[%s]-", irMimeType.get_cstr()));

    if (iEOSSent == false)
    {
        aCmdPacket = true;
        PVMFSharedMediaCmdPtr sharedMediaCmdPtr =
            PVMFMediaCmd::createMediaCmd();

        sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_EOS_FORMAT_ID);

        sharedMediaCmdPtr->setStreamID(iStreamID);

        convertToPVMFMediaCmdMsg(aMediaOutMsg, sharedMediaCmdPtr);

        aMediaOutMsg->setTimestamp(iLastPacketOutTs);

        uint32 timebase32 = 0;
        uint32 clientClock32 = 0;
        bool overflowFlag = false;

        irClientPlayBackClock.GetCurrentTime32(clientClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
        timebase32 = 0;
        uint32 estServClock32 = 0;
        irEstimatedServerClock.GetCurrentTime32(estServClock32, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
        PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand: MimeType=%s, StreamID=%d",
                                    irMimeType.get_cstr(),
                                    aMediaOutMsg->getStreamID()));
        PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand: ClientClock=%d",
                                    clientClock32));
        PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand: EstServClock=%d",
                                    estServClock32));
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand: MimeType=%s, StreamID=%d",
                                           irMimeType.get_cstr(),
                                           aMediaOutMsg->getStreamID()));
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand: ClientClock=%d",
                                           clientClock32));
        PVMF_JB_LOGCLOCK_SESSION_DURATION((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand: EstServClock=%d",
                                           estServClock32));
        iEOSSent = true;
        return PVMFSuccess;
    }
    else
    {
        PVMF_JB_LOGINFO((0, "PVMFJitterBufferNode::GenerateAndSendEOSCommand - EOS already sent..."));
    }

    return PVMFSuccess;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::NotifyCanRetrievePacket()
{
    PVMF_JB_LOGDATATRAFFIC_OUT_E((0, "PVMFJitterBufferImpl::NotifyCanRetrievePacket Mime %s", irMimeType.get_cstr()));
    iReportCanRetrievePacket = true;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::CancelNotifyCanRetrievePacket()
{
    iReportCanRetrievePacket = false;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::IsDelayEstablished(uint32& aClockDiff)
{
    aClockDiff = iDurationInMilliSeconds;
    if (GetState() == PVMF_JITTER_BUFFER_IN_TRANSITION)
    {
        PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Jitter Buffer In Transition - Preparing for Seek"));
        irDelayEstablished = false;
        irJitterDelayPercent = 0;
        return irDelayEstablished;
    }

    uint32 timebase32 = 0;
    uint32 estServerClock = 0;
    uint32 clientClock = 0;
    bool overflowFlag = false;

    irEstimatedServerClock.GetCurrentTime32(estServerClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    irClientPlayBackClock.GetCurrentTime32(clientClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);

    PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - EstServClock=%d", estServerClock));
    PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - ClientClock=%d", clientClock));

    if (iEOSSignalled)
    {
        /*
        * No check needed - We are past the clip time, just play out the last
        * bit in the jitter buffer
        */
        PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Session Duration Expired"));
        if (irDelayEstablished == false)
        {
            /*
            * Coming out of rebuffering in case we had gone into
            * rebuffering just before
            */
            irJitterDelayPercent = 100;
            PVMFAsyncEvent jbEvent(PVMFInfoEvent, PVMFInfoDataReady, NULL, NULL);
            ReportJBInfoEvent(jbEvent);
        }
        irDelayEstablished = true;
    }
    else
    {
        if (estServerClock < clientClock)
        {
            /* Could happen during repositioning */
            if (irDelayEstablished == true)
            {
                aClockDiff = 0;
                irDelayEstablished = false;
                irJitterDelayPercent = 0;
                /* Start timer */
                PVMFAsyncEvent jbEvent(PVMFInfoEvent, PVMFInfoUnderflow, NULL, NULL);
                ReportJBInfoEvent(jbEvent);
                PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - EstServClock=%d",
                                  Oscl_Int64_Utils::get_uint64_lower32(estServerClock)));
                PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - ClientClock=%d",
                                  Oscl_Int64_Utils::get_uint64_lower32(clientClock)));
                PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Estimated Serv Clock Less Than ClientClock!!!!"));
                PVMF_JB_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - EstServClock=%d",
                                         Oscl_Int64_Utils::get_uint64_lower32(estServerClock)));
                PVMF_JB_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Check - ClientClock=%d",
                                         Oscl_Int64_Utils::get_uint64_lower32(clientClock)));
                PVMF_JB_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::IsJitterBufferReady - Estimated Serv Clock Less Than ClientClock!!!!"));
            }
            return irDelayEstablished;
        }
        const uint32 diff32ms = estServerClock - clientClock;
        aClockDiff = diff32ms;
        if (diff32ms >= iDurationInMilliSeconds)
        {
            uint32 jbSize = iBufferAlloc->getBufferSize();
            uint32 largestContiguousFreeBlockSize = iBufferAlloc->getLargestContiguousFreeBlockSize();
            uint32 minPercentOccupancy = 100;
            if ((largestContiguousFreeBlockSize*100 / jbSize) < minPercentOccupancy)
            {
                minPercentOccupancy = (uint32)(largestContiguousFreeBlockSize * 100 / jbSize);
            }

            if ((prevMinPercentOccupancy < MIN_PERCENT_OCCUPANCY_THRESHOLD) && (minPercentOccupancy < MIN_PERCENT_OCCUPANCY_THRESHOLD))
            {
                consecutiveLowBufferCount++;
            }
            else
            {
                consecutiveLowBufferCount = 0;
            }

            prevMinPercentOccupancy = minPercentOccupancy;
            PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - minPercentOccupancy=%d, consecutiveLowBufferCount=%d",
                              minPercentOccupancy,
                              consecutiveLowBufferCount));


            if ((diff32ms > JITTER_BUFFER_DURATION_MULTIPLIER_THRESHOLD*iDurationInMilliSeconds) && !iOverflowFlag && (consecutiveLowBufferCount > CONSECUTIVE_LOW_BUFFER_COUNT_THRESHOLD))
            {
                iOverflowFlag = true;
                PVMFAsyncEvent jbEvent(PVMFInfoEvent, PVMFInfoOverflow, NULL, NULL);
                ReportJBInfoEvent(jbEvent);
                PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady reporting PVMFInfoSourceOverflow"));
            }

            if (irDelayEstablished == false)
            {
                if (CheckNumElements())
                {
                    PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Cancelling Jitter Buffer Duration Timer"));
                    irDelayEstablished = true;
                    irJitterDelayPercent = 100;
                    PVMFAsyncEvent jbEvent(PVMFInfoEvent, PVMFInfoDataReady, NULL, NULL);
                    ReportJBInfoEvent(jbEvent);
                    PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Established - EstServClock=%d", estServerClock));
                    PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Established - ClientClock=%d",  clientClock));
                    PVMF_JB_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Established - EstServClock=%d",
                                             estServerClock));
                    PVMF_JB_LOGCLOCK_REBUFF((0, "PVMFJitterBufferNode::IsJitterBufferReady - Time Delay Established - ClientClock=%d",
                                             clientClock));
                }
                else
                {
                    irJitterDelayPercent = 0;
                }
            }
            else
            {
                irJitterDelayPercent = 100;
            }
        }
        else
        {
            /*
            * Update the buffering percent - to be used while sending buffering
            * status events, in case we go into rebuffering or if we are in buffering
            * state.
            */
            irJitterDelayPercent = ((diff32ms * 100) / iDurationInMilliSeconds);
            if (irDelayEstablished == true)
            {
                if (diff32ms <= iRebufferingThresholdInMilliSeconds)
                {
                    /* Implies that we are going into rebuffering */
                    if (!iEOSSignalled)
                    {
                        irDelayEstablished = false;
                        PVMFAsyncEvent jbEvent(PVMFInfoEvent, PVMFInfoUnderflow, NULL, NULL);
                        ReportJBInfoEvent(jbEvent);
                        LOGCLIENTANDESTIMATEDSERVCLK_REBUFF;
                    }
                    /* we are past the end of the clip, no more rebuffering */
                    irClientPlayBackClock.Pause();
                }
            }
            if (irDelayEstablished == false && CheckNumElements() == false)
            {
                irJitterDelayPercent = 0;
            }
            PVMF_JB_LOGCLOCK((0, "PVMFJitterBufferNode::IsJitterBufferReady: Delay Percent = %d", irJitterDelayPercent));
        }
        /* if we are not rebuffering check for flow control */
        PerformFlowControl(false);
    }
    return (irDelayEstablished);
}

OSCL_EXPORT_REF PVMFSharedMediaDataPtr PVMFJitterBufferImpl::RetrievePacketPayload()
{
    PVMF_JB_LOGINFO((0, "PVMFJitterBufferImpl::retrievePacket - JB Occup Stats - MimeType=%s, MaxSize=%d, CurrOccupany=%d", irMimeType.get_cstr(), iJitterBuffer->getArraySize(), iJitterBuffer->getNumElements()));

    PVMFSharedMediaDataPtr elem = iJitterBuffer->retrieveElement();
    if (elem.GetRep() != NULL)
    {
        /*
         * Adjust TimeStamp - Goal is to provide a monotonically increasing
         * timestamp.
         */
        PVMFTimestamp currTS = elem->getTimestamp();
        DeterminePrevTimeStamp(elem->getSeqNum());
        iMonotonicTimeStamp += (currTS - iPrevTSOut);
        PVMFTimestamp adjustedTS =
            (PVMFTimestamp)(Oscl_Int64_Utils::get_uint64_lower32(iMonotonicTimeStamp));
        elem->setTimestamp(adjustedTS);

        PVMFJitterBufferStats* jbStatsPtr = iJitterBuffer->getStatsPtr();
        jbStatsPtr->maxTimeStampRetrievedWithoutRTPOffset = adjustedTS;

        PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferImpl::retrievePacket: MimeType=%s, TS=%d, SEQNUM= %d",
                                    irMimeType.get_cstr(),
                                    elem->getTimestamp(),
                                    elem->getSeqNum()));
        iPrevTSOut = currTS;
    }
    return elem;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::IsEmpty()
{
    uint32 elems = iJitterBuffer->getNumElements();
    if (elems == 0)
    {
        return true;
    }
    return false;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetEOS(bool aVal)
{
    PVMF_JB_LOG_RTCPDATATRAFFIC_OUT((0, "PVMFJitterBufferImpl::SetEOS Mime[%s]", irMimeType.get_cstr()));
    PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferImpl::SetEOS Mime[%s]", irMimeType.get_cstr()));

    if (aVal && !iEOSSignalled)
    {
        LOGCLIENTANDESTIMATEDSERVCLK_DATAPATH_OUT;
        iEOSSignalled = aVal;
        iObserver->EndOfStreamSignalled(iObserverContext);
    }

    if (iReportCanRetrievePacket)
    {
        if (CanRetrievePacket())
        {
            iObserver->PacketReadyToBeRetrieved(iObserverContext);
        }
    }
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::GetEOS()
{
    return iEOSSignalled;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetPlayRange(int32 aStartTimeInMS, bool aPlayAfterSeek, bool aStopTimeAvailable, int32 aStopTimeInMS)
{
    iStartTimeInMS = aStartTimeInMS;
    if (aStopTimeAvailable)
    {
        iPlayStopTimeAvailable	= aStopTimeAvailable;
        iStopTimeInMS = aStopTimeInMS;
    }

    if (aPlayAfterSeek)
    {
        iEOSSignalled = false;
        iEOSSent = false;
    }
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CheckForHighWaterMark()
{
    PVMFJitterBufferStats stats = iJitterBuffer->getStats();
    uint32 currOccupancy = stats.currentOccupancy;
    uint32 maxOccupancy = stats.maxOccupancy;

    PVMF_JB_LOGINFO((0, "PVMFJitterBufferImpl::CheckForHighWaterMark: CurrOccupancy = %d", currOccupancy));
    if (currOccupancy >=
            maxOccupancy*DEFAULT_JITTER_BUFFER_HIGH_WATER_MARK)
    {
        return true;
    }
    return false;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CheckForLowWaterMark()
{
    PVMFJitterBufferStats stats = iJitterBuffer->getStats();
    uint32 currOccupancy = stats.currentOccupancy;
    uint32 maxOccupancy = stats.maxOccupancy;

    PVMF_JB_LOGINFO((0, "PVMFJitterBufferImpl::CheckForLowWaterMark: CurrOccupancy = %d", currOccupancy));
    if (currOccupancy <=
            maxOccupancy*DEFAULT_JITTER_BUFFER_LOW_WATER_MARK)
    {
        return true;
    }
    return false;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CheckNumElements()
{
    if (iJitterBuffer->getNumElements() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::addMediaCommand(PVMFSharedMediaMsgPtr& aMediaCmd)
{
    MediaCommandMsgHolder cmdHolder;
    cmdHolder.iPreceedingMediaMsgSeqNumber = getJitterBufferStats().lastRegisteredSeqNum;
    cmdHolder.iCmdMsg = aMediaCmd;
    iMediaCmdVec.push_back(cmdHolder);
    return true;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::GetPendingCommand(PVMFSharedMediaMsgPtr& aCmdMsg)
{
    /*
     * Parse the command queue to see if it is time to send out
     * any pending ones. if last retrieved seq num is same as
     * the seq num of media msg that preceeds this cmd, then
     * it is time to send this cmd out
     */
    PVMFJitterBufferStats stats = getJitterBufferStats();
    Oscl_Vector<MediaCommandMsgHolder, OsclMemAllocator>::iterator it;
    for (it = iMediaCmdVec.begin(); it != iMediaCmdVec.end(); it++)
    {
        if (it->iCmdMsg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
        {
            aCmdMsg = it->iCmdMsg;
            iMediaCmdVec.erase(it);
            return true;
        }
        if (it->iPreceedingMediaMsgSeqNumber == stats.lastRetrievedSeqNum)
        {
            aCmdMsg = it->iCmdMsg;
            iMediaCmdVec.erase(it);
            return true;
        }
    }
    return false;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::HasPendingCommand()
{
    /*
     * Parse the command queue to see if it is time to send out
     * any pending ones. if last retrieved seq num is same as
     * the seq num of media msg that preceeds this cmd, then
     * it is time to send this cmd out
     */
    PVMFJitterBufferStats stats = getJitterBufferStats();
    Oscl_Vector<MediaCommandMsgHolder, OsclMemAllocator>::iterator it;
    for (it = iMediaCmdVec.begin(); it != iMediaCmdVec.end(); it++)
    {
        if (it->iCmdMsg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
        {
            return true;
        }
        if (it->iPreceedingMediaMsgSeqNumber == stats.lastRetrievedSeqNum)
        {
            return true;
        }
    }
    return false;
}


OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetAdjustedTSInMS(PVMFTimestamp aAdjustedTSInMS)
{
    // convert adjustedTS to RTP Timescale
    uint32 in_wrap_count = 0;
    MediaClockConverter clockConvertor;
    clockConvertor.set_timescale(1000);
    clockConvertor.set_clock(aAdjustedTSInMS, in_wrap_count);
    uint32 adjustedTSInRTPTS = clockConvertor.get_converted_ts(iRTPTimeScale);
    iMonotonicTimeStamp = adjustedTSInRTPTS;
    PVMF_JB_LOGDATATRAFFIC_OUT((0, "PVMFJitterBufferImpl::SetAdjustedTS(): adjustedTSInRTPTS=%d, iMonotonicTS=%d", adjustedTSInRTPTS, Oscl_Int64_Utils::get_uint64_lower32(iMonotonicTimeStamp)));
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetBroadCastSession()
{
    iBroadCastSession = true;
}

OSCL_EXPORT_REF PVMFRTPInfoParams& PVMFJitterBufferImpl::GetRTPInfoParams()
{
    return *iRTPInfoParamsVec.end();
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::GetRTPTimeStampOffset(uint32& aTimeStampOffset)
{
    if (seqNumLock)
        aTimeStampOffset = seqLockTimeStamp;

    return seqNumLock;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::NotifyFreeSpaceAvailable()
{
    if (iMediaDataGroupAlloc)
    {
        iMediaDataGroupAlloc->notifyfreechunkavailable(*this);
        return true;
    }
    return false;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::freechunkavailable(OsclAny*)
{
    if (iObserver)
    {
        iObserver->JitterBufferFreeSpaceAvailable(iObserverContext);
    }
}



OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CheckSpaceAvailability(PVMFSharedMediaMsgPtr& aMsg)
{
    if (iJitterBuffer)
    {
        return (iJitterBuffer->CheckSpaceAvailability(GetNumOfPackets(aMsg)));
    }
    return false;
}

OSCL_EXPORT_REF bool PVMFJitterBufferImpl::CheckSpaceAvailability()
{
    return (iJitterBuffer->CheckSpaceAvailability());
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::setSSRC(uint32 aSSRC)
{
    PVMF_JB_LOGDATATRAFFIC_IN((0, "PVMFJitterBufferImpl::setSSRC: Setting SSRCFromSetUpResponse - MimeType=%s, SSRC=%d", irMimeType.get_cstr(), aSSRC));
    oSSRCFromSetUpResponseAvailable = true;
    SSRCFromSetUpResponse = aSSRC;
    SSRCLock = aSSRC;
}

OSCL_EXPORT_REF uint32 PVMFJitterBufferImpl::GetSSRC() const
{
    return SSRCLock;
}

/**
*/
//virtual bool CancelServerClockNotificationUpdates();


OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetJitterBufferMemPoolInfo(uint32 aSize,
        uint32 aResizeSize,
        uint32 aMaxNumResizes,
        uint32 aExpectedNumberOfBlocksPerBuffer)
{
    iJitterBufferMemPoolInfo.Init(aSize, aResizeSize, aMaxNumResizes, aExpectedNumberOfBlocksPerBuffer);
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::GetJitterBufferMemPoolInfo(uint32& aSize, uint32& aResizeSize, uint32& aMaxNumResizes, uint32& aExpectedNumberOfBlocksPerBuffer) const
{
    aSize = iJitterBufferMemPoolInfo.iSize;
    aResizeSize = iJitterBufferMemPoolInfo.iResizeSize;
    aMaxNumResizes = iJitterBufferMemPoolInfo.iMaxNumResizes;
    aExpectedNumberOfBlocksPerBuffer = iJitterBufferMemPoolInfo.iExpectedNumberOfBlocksPerBuffer;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetTrackConfig(OsclRefCounterMemFrag& aConfig)
{
    iTrackConfig = aConfig;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetTimeScale(uint32 aTimeScale)
{
    iTimeScale = aTimeScale;
}

OSCL_EXPORT_REF uint32 PVMFJitterBufferImpl::GetTimeScale() const
{
    return iTimeScale;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::SetMediaClockConverter(MediaClockConverter* aConverter)
{
    ipMediaClockConverter = aConverter;
}

OSCL_EXPORT_REF PVMFStatus PVMFJitterBufferImpl::PerformFlowControl(bool aIncomingMedia)
{
    OSCL_UNUSED_ARG(aIncomingMedia);
    return PVMFSuccess;
}


OSCL_EXPORT_REF void PVMFJitterBufferImpl::LogClientAndEstimatedServerClock(PVLogger*& aLogger)
{
    uint32 timebase32 = 0;
    uint32 estServerClock = 0;
    uint32 clientClock = 0;
    bool overflowFlag = false;
    irEstimatedServerClock.GetCurrentTime32(estServerClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    irClientPlayBackClock.GetCurrentTime32(clientClock, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, aLogger, PVLOGMSG_INFO, (0, "The value of estimated server clock is [%d] and client clock is [%d]", estServerClock, clientClock));
}

OSCL_EXPORT_REF uint32 PVMFJitterBufferImpl::GetNumOfPackets(PVMFSharedMediaMsgPtr& aMsg) const
{
    if (iOnePacketPerFragment)
    {
        PVMFSharedMediaDataPtr inputDataPacket;
        convertToPVMFMediaData(inputDataPacket, aMsg);
        return inputDataPacket->getNumFragments();
    }
    else
        return 1;
}

OSCL_EXPORT_REF void PVMFJitterBufferImpl::ReportJBInfoEvent(PVMFAsyncEvent& aEvent)
{
    iObserver->ProcessJBInfoEvent(aEvent);
}

void PVMFJitterBufferImpl::HandleEvent_JitterBufferBufferingDurationComplete()
{
    PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::HandleEvent_JitterBufferBufferingDurationComplete In"));

    if (!irDelayEstablished)
    {
        uint32 aClockDiff = 0;
        bool delayEstablished = IsDelayEstablished(aClockDiff);//To update delay percent
        if (iReportCanRetrievePacket && delayEstablished)
        {
            //Check if delay is established
            //There's no hole for for min threshold time
            if (CanRetrievePacket())
            {
                iObserver->PacketReadyToBeRetrieved(iObserverContext);
                iReportCanRetrievePacket = false;
            }
        }
        else
        {
            RequestEventCallBack(JB_BUFFERING_DURATION_COMPLETE);
        }
    }

    PVMF_JB_LOG_EVENTS_CLOCK((0, "PVMFJitterBufferNode::HandleEvent_JitterBufferBufferingDurationComplete Out"));
}
