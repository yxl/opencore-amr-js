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
#include "pvmf_jitter_buffer_node.h"
#endif
#ifndef PVMF_JITTER_BUFFER_INTERNAL_H_INCLUDED
#include "pvmf_jitter_buffer_internal.h"
#endif
#ifndef PV_LOGGGER_H_INCLUDED
#include "pvlogger.h"
#endif

PVMFJitterBufferExtensionInterfaceImpl::PVMFJitterBufferExtensionInterfaceImpl(PVMFJitterBufferNode*c)
        : PVInterfaceImpl<OsclMemAllocator>(PVUuid(PVMF_JITTERBUFFERNODE_EXTENSIONINTERFACE_UUID))
        , iContainer(c)
{}

PVMFJitterBufferExtensionInterfaceImpl::~PVMFJitterBufferExtensionInterfaceImpl()
{}

OSCL_EXPORT_REF
bool PVMFJitterBufferExtensionInterfaceImpl::setPortParams(PVMFPortInterface* aPort,
        uint32 aTimeScale,
        uint32 aBitRate,
        OsclRefCounterMemFrag& aConfig,
        bool aRateAdaptation,
        uint32 aRateAdaptationFeedBackFrequency)
{
    return (iContainer->SetPortParams(aPort,
                                      aTimeScale,
                                      aBitRate,
                                      aConfig,
                                      aRateAdaptation,
                                      aRateAdaptationFeedBackFrequency));
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setPlayBackThresholdInMilliSeconds(uint32 threshold)
{
    iContainer->SetPlayBackThresholdInMilliSeconds(threshold);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setJitterBufferRebufferingThresholdInMilliSeconds(uint32 aThreshold)
{
    iContainer->SetJitterBufferRebufferingThresholdInMilliSeconds(aThreshold);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::getJitterBufferRebufferingThresholdInMilliSeconds(uint32& aThreshold)
{
    iContainer->GetJitterBufferRebufferingThresholdInMilliSeconds(aThreshold);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setJitterBufferDurationInMilliSeconds(uint32 duration)
{
    iContainer->SetJitterBufferDurationInMilliSeconds(duration);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::getJitterBufferDurationInMilliSeconds(uint32& duration)
{
    iContainer->GetJitterBufferDurationInMilliSeconds(duration);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setEarlyDecodingTimeInMilliSeconds(uint32 duration)
{
    iContainer->SetEarlyDecodingTimeInMilliSeconds(duration);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setBurstThreshold(float burstThreshold)
{
    iContainer->SetBurstThreshold(burstThreshold);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setClientPlayBackClock(PVMFMediaClock* clientClock)
{
    iContainer->SetClientPlayBackClock(clientClock);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setMaxInactivityDurationForMediaInMs(uint32 duration)
{
    iContainer->SetMaxInactivityDurationForMediaInMs(duration);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::getMaxInactivityDurationForMediaInMs(uint32& duration)
{
    iContainer->GetMaxInactivityDurationForMediaInMs(duration);
}

OSCL_EXPORT_REF
void PVMFJitterBufferExtensionInterfaceImpl::setRTCPIntervalInMicroSecs(uint32 aRTCPInterval)
{
    iContainer->SetRTCPIntervalInMicroSecs(aRTCPInterval);
}

OSCL_EXPORT_REF
bool PVMFJitterBufferExtensionInterfaceImpl::setPlayRange(int32 aStartTimeInMS,
        int32 aStopTimeInMS,
        bool oPlayAfterASeek,
        bool aStopTimeAvailable)
{
    return (iContainer->SetPlayRange(aStartTimeInMS,
                                     aStopTimeInMS,
                                     oPlayAfterASeek,
                                     aStopTimeAvailable));
}

OSCL_EXPORT_REF
bool PVMFJitterBufferExtensionInterfaceImpl::PrepareForRepositioning(bool oUseExpectedClientClockVal,
        uint32 aExpectedClientClockVal)
{
    return (iContainer->PrepareForRepositioning(oUseExpectedClientClockVal, aExpectedClientClockVal));
}

OSCL_EXPORT_REF
bool PVMFJitterBufferExtensionInterfaceImpl::setPortSSRC(PVMFPortInterface* aPort, uint32 aSSRC)
{
    return (iContainer->SetPortSSRC(aPort, aSSRC));
}


OSCL_EXPORT_REF
bool PVMFJitterBufferExtensionInterfaceImpl::setPortRTPParams(PVMFPortInterface* aPort,
        bool   aSeqNumBasePresent,
        uint32 aSeqNumBase,
        bool   aRTPTimeBasePresent,
        uint32 aRTPTimeBase,
        bool   aNPTTimeBasePresent,
        uint32 aNPTInMS,
        bool oPlayAfterASeek)
{
    return (iContainer->SetPortRTPParams(aPort,
                                         aSeqNumBasePresent,
                                         aSeqNumBase,
                                         aRTPTimeBasePresent,
                                         aRTPTimeBase,
                                         aNPTTimeBasePresent,
                                         aNPTInMS,
                                         oPlayAfterASeek));
}

OSCL_EXPORT_REF
bool PVMFJitterBufferExtensionInterfaceImpl::setPortRTCPParams(PVMFPortInterface* aPort,
        int aNumSenders,
        uint32 aRR,
        uint32 aRS)
{
    return iContainer->SetPortRTCPParams(aPort, aNumSenders, aRR, aRS);
}

OSCL_EXPORT_REF
PVMFTimestamp PVMFJitterBufferExtensionInterfaceImpl::getActualMediaDataTSAfterSeek()
{
    return (iContainer->GetActualMediaDataTSAfterSeek());
}

OSCL_EXPORT_REF
PVMFTimestamp PVMFJitterBufferExtensionInterfaceImpl::getMaxMediaDataTS()
{
    return (iContainer->GetMaxMediaDataTS());
}

OSCL_EXPORT_REF PVMFStatus
PVMFJitterBufferExtensionInterfaceImpl::setServerInfo(PVMFJitterBufferFireWallPacketInfo& aServerInfo)
{
    return (iContainer->SetServerInfo(aServerInfo));
}

OSCL_EXPORT_REF PVMFStatus
PVMFJitterBufferExtensionInterfaceImpl::NotifyOutOfBandEOS()
{
    return (iContainer->NotifyOutOfBandEOS());
}

OSCL_EXPORT_REF PVMFStatus
PVMFJitterBufferExtensionInterfaceImpl::SendBOSMessage(uint32 aStreamID)
{
    return (iContainer->SendBOSMessage(aStreamID));
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::SetSharedBufferResizeParams(uint32 maxNumResizes, uint32 resizeSize)
{
    iContainer->SetSharedBufferResizeParams(maxNumResizes, resizeSize);
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::GetSharedBufferResizeParams(uint32& maxNumResizes, uint32& resizeSize)
{
    iContainer->GetSharedBufferResizeParams(maxNumResizes, resizeSize);
}

OSCL_EXPORT_REF bool
PVMFJitterBufferExtensionInterfaceImpl::ClearJitterBuffer(PVMFPortInterface* aPort, uint32 aSeqNum)
{
    return (iContainer->ClearJitterBuffer(aPort, aSeqNum));
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::FlushJitterBuffer()
{
    iContainer->FlushJitterBuffer();
}

OSCL_EXPORT_REF bool
PVMFJitterBufferExtensionInterfaceImpl::NotifyAutoPauseComplete()
{
    return (iContainer->NotifyAutoPauseComplete());
}

OSCL_EXPORT_REF bool
PVMFJitterBufferExtensionInterfaceImpl::NotifyAutoResumeComplete()
{
    return (iContainer->NotifyAutoResumeComplete());
}

OSCL_EXPORT_REF PVMFStatus
PVMFJitterBufferExtensionInterfaceImpl::SetInputMediaHeaderPreParsed(PVMFPortInterface* aPort,
        bool aHeaderPreParsed)
{
    return (iContainer->SetInputMediaHeaderPreParsed(aPort, aHeaderPreParsed));
}

OSCL_EXPORT_REF PVMFStatus
PVMFJitterBufferExtensionInterfaceImpl::HasSessionDurationExpired(bool& aExpired)
{
    return (iContainer->HasSessionDurationExpired(aExpired));
}
OSCL_EXPORT_REF bool
PVMFJitterBufferExtensionInterfaceImpl::PurgeElementsWithNPTLessThan(NptTimeFormat &aNPTTime)
{
    return (iContainer->PurgeElementsWithNPTLessThan(aNPTTime));
}
OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::SetBroadCastSession()
{
    iContainer->SetBroadCastSession();
}
OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::DisableFireWallPackets()
{
    iContainer->DisableFireWallPackets();
}
OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::StartOutputPorts()
{
    iContainer->StartOutputPorts();
}
OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::StopOutputPorts()
{
    iContainer->StopOutputPorts();
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::UpdateJitterBufferState()
{
    iContainer->UpdateJitterBufferState();
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::SetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort, uint32 aSize, uint32 aResizeSize, uint32 aMaxNumResizes, uint32 aExpectedNumberOfBlocksPerBuffer)
{
    iContainer->SetJitterBufferMemPoolInfo(aPort, aSize, aResizeSize, aMaxNumResizes, aExpectedNumberOfBlocksPerBuffer);
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::GetJitterBufferMemPoolInfo(const PvmfPortBaseImpl* aPort, uint32& aSize, uint32& aResizeSize, uint32& aMaxNumResizes, uint32& aExpectedNumberOfBlocksPerBuffer) const
{
    iContainer->GetJitterBufferMemPoolInfo(aPort, aSize, aResizeSize, aMaxNumResizes, aExpectedNumberOfBlocksPerBuffer);
}

OSCL_EXPORT_REF void
PVMFJitterBufferExtensionInterfaceImpl::SetJitterBufferChunkAllocator(OsclMemPoolResizableAllocator* aDataBufferAllocator, const PVMFPortInterface* aPort)
{
    iContainer->SetJitterBufferChunkAllocator(aDataBufferAllocator, aPort);
}

OSCL_EXPORT_REF bool
PVMFJitterBufferExtensionInterfaceImpl::PrepareForPlaylistSwitch()
{
    return (iContainer->PrepareForPlaylistSwitch());
}
