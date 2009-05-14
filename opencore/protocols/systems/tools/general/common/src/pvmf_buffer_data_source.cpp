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
#include "pvmf_buffer_data_source.h"

OSCL_EXPORT_REF PVMFBufferDataSource::PVMFBufferDataSource(int32 aPortTag,
        unsigned bitrate,
        unsigned min_sample_sz,
        unsigned max_sample_sz,
        uint8*   fsi,
        unsigned fsi_len):
        PvmfPortBaseImpl(aPortTag, this),
        iTimer("PVMFBufferDataSource"),
        iMediaDataAlloc(NULL),
        iFsi(NULL),
        iFsiLen(0)
{
    iBitrate = bitrate;
    iMinSampleSz = min_sample_sz;
    iMaxSampleSz = max_sample_sz;
    iTimestamp = 0;
    iSampleInterval = 0;
    if (fsi && fsi_len)
    {
        iFsi = (uint8*)OSCL_DEFAULT_MALLOC(fsi_len);
        oscl_memcpy(iFsi, fsi, fsi_len);
        iFsiLen = fsi_len;
    }
}

OSCL_EXPORT_REF PVMFBufferDataSource::~PVMFBufferDataSource()
{
    Stop();
    if (iMediaDataAlloc)
    {
        OSCL_DELETE(iMediaDataAlloc);
        iMediaDataAlloc = NULL;
    }
    if (iFsi)
    {
        OSCL_DEFAULT_FREE(iFsi);
        iFsi = NULL;
    }
}

void PVMFBufferDataSource::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    if (aActivity.iType != PVMF_PORT_ACTIVITY_OUTGOING_MSG)
        return;
    PVMFSharedMediaMsgPtr aMsg;
    while (OutgoingMsgQueueSize())
    {
        Send();
    }
}

OSCL_EXPORT_REF void PVMFBufferDataSource::Start()
{
    iMediaDataAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (&iMemAlloc));
    unsigned ave_sample_sz = (iMinSampleSz + iMaxSampleSz) / 2;
    unsigned frequency = (iBitrate >> 3) / ave_sample_sz;
    iSampleInterval = 1000 / frequency;
    iTimer .SetFrequency(frequency);
    iTimer.SetObserver(this);
    iTimer.Request(1/*timer id*/, ave_sample_sz/*timer info*/ , 1/*num ticks*/, this, 1/*recurring*/);
}

OSCL_EXPORT_REF void PVMFBufferDataSource::Stop()
{
    iTimer.Clear();
    Disconnect();
}

void PVMFBufferDataSource::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timerID);
    unsigned bytesToSend = timeoutInfo;
    if (bytesToSend <= 0)
        return;

    if (!IsConnected())
        return;

    // Create new media data buffer
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl = iMediaDataAlloc->allocate(bytesToSend);
    PVMFSharedMediaDataPtr mediaData;
    int leavecode = 0;
    OSCL_TRY(leavecode, mediaData = PVMFMediaData::createMediaData(mediaDataImpl));
    OSCL_FIRST_CATCH_ANY(leavecode, return);

    // Send FSI if available
    if (iFsi)
    {
        OsclSharedPtr<PVMFMediaDataImpl> fsiMediaDataImpl = iMediaDataAlloc->allocate(iFsiLen);
        PVMFSharedMediaDataPtr fsiMediaData;
        OSCL_TRY(leavecode, fsiMediaData = PVMFMediaData::createMediaData(fsiMediaDataImpl));
        OSCL_FIRST_CATCH_ANY(leavecode, return);
        OsclRefCounterMemFrag fsi_frag;
        fsiMediaData->getMediaFragment(0, fsi_frag);
        oscl_memcpy((uint8*)fsi_frag.getMemFragPtr(), iFsi, iFsiLen);
        fsi_frag.getMemFrag().len = iFsiLen;
        mediaData->setFormatSpecificInfo(fsi_frag);
        OSCL_DEFAULT_FREE(iFsi);
        iFsi = NULL;
        iFsiLen = 0;
    }

    // Retrieve memory fragment to write to
    OsclRefCounterMemFrag refCtrMemFrag;
    mediaData->getMediaFragment(0, refCtrMemFrag);
    if (refCtrMemFrag.getCapacity() < bytesToSend)
        return;

    oscl_memset((uint8*)refCtrMemFrag.getMemFragPtr(), 7, bytesToSend);
    mediaDataImpl->setMediaFragFilledLen(0, bytesToSend);
    mediaData->setTimestamp(iTimestamp);
    iTimestamp += iSampleInterval;

    // Send frame to downstream node
    PVMFSharedMediaMsgPtr mediaMsg;
    convertToPVMFMediaMsg(mediaMsg, mediaData);
    QueueOutgoingMsg(mediaMsg);
}

// PVMFPortInterface virtuals

PVMFStatus PVMFBufferDataSource::PutData(PVMFSharedMediaMsgPtr aMsg)
{
    OSCL_UNUSED_ARG(aMsg);
    return PVMFSuccess;
}

PVMFStatus PVMFBufferDataSource::GetData(PVMFSharedMediaMsgPtr aMsg)
{
    OSCL_UNUSED_ARG(aMsg);
    return PVMFSuccess;
}

OSCL_EXPORT_REF void PVMFBufferDataSource::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    OSCL_UNUSED_ARG(aObserver);
}

OSCL_EXPORT_REF PVMFStatus PVMFBufferDataSource::getParametersSync(PvmiMIOSession aSession, PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters, int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
    OSCL_UNUSED_ARG(aContext);

    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus PVMFBufferDataSource::releaseParameters(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    return PVMFSuccess;
}

OSCL_EXPORT_REF void PVMFBufferDataSource::createContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
}

OSCL_EXPORT_REF void PVMFBufferDataSource::setContextParameters(PvmiMIOSession aSession, PvmiCapabilityContext& aContext,
        PvmiKvp* aParameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

OSCL_EXPORT_REF void PVMFBufferDataSource::DeleteContext(PvmiMIOSession aSession, PvmiCapabilityContext& aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aContext);
}

OSCL_EXPORT_REF void PVMFBufferDataSource::setParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp * & aRet_kvp)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
}


OSCL_EXPORT_REF PVMFCommandId PVMFBufferDataSource::setParametersAsync(PvmiMIOSession aSession, PvmiKvp* aParameters,
        int num_elements, PvmiKvp*& aRet_kvp, OsclAny* context)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(aRet_kvp);
    OSCL_UNUSED_ARG(context);

    OSCL_LEAVE(OsclErrNotSupported);
    return -1;
}

OSCL_EXPORT_REF uint32 PVMFBufferDataSource::getCapabilityMetric(PvmiMIOSession aSession)
{
    OSCL_UNUSED_ARG(aSession);

    return 1;
}

OSCL_EXPORT_REF PVMFStatus PVMFBufferDataSource::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_elements);

    OSCL_LEAVE(OsclErrNotSupported);
    return PVMFFailure;
}

OSCL_EXPORT_REF void PVMFBufferDataSource::QueryInterface(const PVUuid& aUuid, OsclAny*& aPtr)
{
    aPtr = NULL;
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        aPtr = (PvmiCapabilityAndConfig*)this;
    }
    else
    {
        OSCL_LEAVE(OsclErrNotSupported);
    }
}
