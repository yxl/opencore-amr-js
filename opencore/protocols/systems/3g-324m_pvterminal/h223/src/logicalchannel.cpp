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
#include "oscl_rand.h"
#include "logicalchannel.h"
#include "pvmf_simple_media_buffer.h"
#include "pvmf_media_data.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"

#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#ifndef PER_COPIER
#include "h245_copier.h"
#endif

#ifndef PER_DELETER
#include "h245_deleter.h"
#endif
// decreasing DEF_CHANNEL_BUFFER_SIZE_MS can result out of memory errors,
// please change also PV_MAX_VIDEO_FRAME_SIZE to lower value.
#define DEF_CHANNEL_BITRATE 64000
#define DEF_INCOMING_CHANNEL_BUFFER_SIZE_MS 2800
#define DEF_OUTGOING_CHANNEL_BUFFER_SIZE_MS 2800
#define TIMESTAMP_MAX INT_MAX
#define DEF_SEGMENTABLE_CHANNEL_OH_BPS 2000
#define H223_INCOMING_CHANNEL_NUM_MEDIA_MSG 300
#define H223_OUTGOING_CHANNEL_NUM_MEDIA_MSG 300
#define H223_INCOMING_CHANNEL_FRAGMENT_SIZE 128
#define H223_INCOMING_CHANNEL_NUM_MEDIA_DATA 300
#define H223_INCOMING_CHANNEL_NUM_FRAGS_IN_MEDIA_DATA (3*1024/128)
#define PV2WAY_BPS_TO_BYTES_PER_MSEC_RIGHT_SHIFT 13


#define H223_LCN_IN_TIMESTAMP_BASE 40


H223LogicalChannel::H223LogicalChannel(TPVChannelId num,
                                       bool segmentable,
                                       OsclSharedPtr<AdaptationLayer>& al,
                                       PS_DataType data_type,
                                       LogicalChannelObserver* observer,
                                       uint32 bitrate,
                                       uint32 sample_interval,
                                       uint32 num_media_data)
        : PvmfPortBaseImpl(num, this),
        lcn(num),
        next(NULL),
        iAl(al),
        iBitrate(0),
        iSampleInterval(0),
        iObserver(observer),
        iFormatSpecificInfo(NULL),
        iFormatSpecificInfoLen(0),
        iLogger(NULL),
        iDataType(NULL),
        iAudioLatency(0),
        iVideoLatency(0)
{
    iSegmentable = segmentable;
    iIncomingSkew = 0;
    iLastSduTimestamp = 0;
    iSampleInterval = sample_interval;
    uint32 bitrate_overhead = IsSegmentable() ? DEF_SEGMENTABLE_CHANNEL_OH_BPS :
                              (uint32)((2000 / sample_interval + 1) >> 1) * (al->GetHdrSz() + al->GetTrlrSz());
    iBitrate = bitrate + bitrate_overhead;
    iNumMediaData = num_media_data;
    iMaxSduSize = (uint16)(iAl->GetSduSize() - iAl->GetHdrSz() - iAl->GetTrlrSz());
    if (data_type)
    {
        iDataType = Copy_DataType(data_type);
    }
    iMediaType = GetFormatType();
    iPaused = false;
    iClock = NULL;
}

H223LogicalChannel::~H223LogicalChannel()
{
    if (iDataType)
    {
        Delete_DataType(iDataType);
        OSCL_DEFAULT_FREE(iDataType);
        iDataType = NULL;
    }
    if (iFormatSpecificInfo)
    {
        oscl_free(iFormatSpecificInfo);
        iFormatSpecificInfo = NULL;
        iFormatSpecificInfoLen = 0;
    }

}

void H223LogicalChannel::Init()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LogicalChannel::Init"));
    iSendFormatSpecificInfo = false;
}

PVMFStatus H223LogicalChannel::SetFormatSpecificInfo(uint8* info, uint16 info_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LogicalChannel::SetFormatSpecificInfo lcn=%d, info_len=%d, info=%x", lcn, info_len, info));
    iSendFormatSpecificInfo = false;
    if (iFormatSpecificInfo)
    {
        oscl_free(iFormatSpecificInfo);
        iFormatSpecificInfo = NULL;
        iFormatSpecificInfoLen = 0;
    }

    if (info == NULL || info_len == 0)
        return PVMFSuccess;

    iFormatSpecificInfo = (uint8*)oscl_malloc(info_len);
    oscl_memcpy(iFormatSpecificInfo, info, info_len);
    iFormatSpecificInfoLen = info_len;
    iSendFormatSpecificInfo = true;
    return PVMFSuccess;
}

const uint8* H223LogicalChannel::GetFormatSpecificInfo(uint32* info_len)
{
    if (info_len == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LogicalChannel::GetFormatSpecificInfo ERROR info_len==NULL"));
        return NULL;
    }
    *info_len = iFormatSpecificInfoLen;
    return iFormatSpecificInfo;
}


OSCL_EXPORT_REF void H223LogicalChannel::QueryInterface(const PVUuid& aUuid, OsclAny*& aPtr)
{
    aPtr = NULL;
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        aPtr = (PvmiCapabilityAndConfig*)this;
    }
    else if (aUuid == PVH324MLogicalChannelInfoUuid)
    {
        aPtr = (LogicalChannelInfo*)this;
    }
    else
    {
        OSCL_LEAVE(OsclErrNotSupported);
    }
}

void H223LogicalChannel::Pause()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Pause lcn=%d", lcn));
    iPaused = true;
    // flush any pending media data
    Flush();
}

void H223LogicalChannel::Resume()
{
    iPaused = false;
}

H223OutgoingChannel::H223OutgoingChannel(TPVChannelId num,
        bool segmentable,
        OsclSharedPtr<AdaptationLayer>& al,
        PS_DataType data_type,
        LogicalChannelObserver* observer,
        uint32 bitrate,
        uint32 sample_interval,
        uint32 num_media_data)
        : H223LogicalChannel(num, segmentable, al, data_type, observer, bitrate, sample_interval, num_media_data),
        iMediaMsgMemoryPool(NULL),
        iMediaDataEntryAlloc(NULL),
        iMediaFragGroupAlloc(NULL),
        iPduPktMemPool(NULL)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.H223OutgoingChannel");
    iOutgoingVideoLogger = PVLogger::GetLoggerObject("datapath.outgoing.video.h223.lcn");
    iOutgoingAudioLogger = PVLogger::GetLoggerObject("datapath.outgoing.audio.h223.lcn");

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::H223OutgoingChannel - num(%d),segmentable(%d)", num, segmentable));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::H223OutgoingChannel - AL SDU size(%d), hdr(%d), trlr(%d)", iAl->GetSduSize(), iAl->GetHdrSz(), iAl->GetTrlrSz()));
    ResetStats();
    lastMediaData = NULL;

    iSetBufferMediaMs = 0;
    iSetBufferMediaBytes = 0;
    iBufferMediaMs = 0;
    iBufferMediaBytes = 0;
    iCurPduTimestamp = 0;
    iNumPendingPdus = 0;
    iMuxingStarted = false;
    iWaitForRandomAccessPoint = false;
    iBufferSizeMs = DEF_OUTGOING_CHANNEL_BUFFER_SIZE_MS;

}

H223OutgoingChannel::~H223OutgoingChannel()
{
    if (iDataType)
    {
        Delete_DataType(iDataType);
        OSCL_DEFAULT_FREE(iDataType);
        iDataType = NULL;
    }
    Flush();
    iMediaFragGroupAlloc->removeRef();
    OSCL_DELETE(iPduPktMemPool);
    OSCL_DELETE(iMediaDataEntryAlloc);
    OSCL_DELETE(iMediaMsgMemoryPool);

}

void H223OutgoingChannel::Init()
{
    H223LogicalChannel::Init();
    iMediaMsgMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (H223_OUTGOING_CHANNEL_NUM_MEDIA_MSG));
    iMediaMsgMemoryPool->enablenullpointerreturn();
    iMediaDataEntryAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (iNumMediaData, sizeof(LCMediaDataEntry)));
    iMediaDataEntryAlloc->enablenullpointerreturn();
    iPduPktMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (iNumMediaData));
    iPduPktMemPool->enablenullpointerreturn();
    iMediaFragGroupAlloc = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>, (iNumMediaData, 10, iPduPktMemPool));
    iMediaFragGroupAlloc->create();
}


void H223OutgoingChannel::BufferMedia(uint16 aMs)
{
    if (iBufferSizeMs && aMs > iBufferSizeMs)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::BufferMedia ERROR buffer interval=%d > buffer size=%d", aMs, iBufferSizeMs));
    }
    iSetBufferMediaMs = iBufferMediaMs = aMs;
    iSetBufferMediaBytes = iBufferMediaBytes = ((iBufferSizeMs * iBitrate + 4000) / 8000);
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"H223OutgoingChannel::BufferMedia ms=%d,bytes=%d", iBufferMediaMs,iBufferMediaBytes));
}

void H223OutgoingChannel::Resume()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Resume lcn=%d", lcn));
    H223LogicalChannel::Resume();
    iPaused = false;
    // start muxing on a random access point
    //iWaitForRandomAccessPoint=true;
}

void H223OutgoingChannel::SetBufferSizeMs(uint32 buffer_size_ms)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingLogicalChannel::SetBufferSizeMs buffer_size_ms=%d", buffer_size_ms));
    iBufferSizeMs = buffer_size_ms;
}



PVMFStatus H223OutgoingChannel::PutData(PVMFSharedMediaMsgPtr media_msg)
{
    PVMFStatus ret = PVMFSuccess;
    PVMFSharedMediaDataPtr mediaData;
    convertToPVMFMediaData(mediaData, media_msg);

    PV_STAT_SET_TIME(iStartTime, iNumPacketsIn)
    PV_STAT_INCR(iNumPacketsIn, 1)

    /* zero through 255 is reserved for media data */
    if (media_msg->getFormatID() >= PVMF_MEDIA_CMD_FORMAT_IDS_START)
    {
        return PVMFSuccess;
    }

    PV_STAT_INCR(iNumBytesIn, (mediaData->getFilledSize()))
    TimeValue timenow;
    if (iMediaType.isCompressed() && iMediaType.isAudio())
    {
        PVMF_OUTGOING_AUDIO_LOGDATATRAFFIC((0, "Outgoing audio frames received. Stats: Entry time=%ud, lcn=%d, size=%d", timenow.to_msec(), lcn, mediaData->getFilledSize()));
    }
    else if (iMediaType.isCompressed() && iMediaType.isVideo())
    {
        PVMF_OUTGOING_VIDEO_LOGDATATRAFFIC((0, "Outgoing video frames received.Stats: Entry time=%ud, lcn=%d, size=%d", timenow.to_msec(), lcn, mediaData->getFilledSize()));
    }

    if (iNumPacketsIn % 20 == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData lcn=%d, size=%d, ts=%d", lcn, mediaData->getFilledSize(), mediaData->getTimestamp()));
    }

    // Check for FormatSpecificInfo.  Sending FSI with data is being obsoleted, but there is no harm in leaving this in for now.
    if (mediaData->getFormatSpecificInfo(iFsiFrag) && iFsiFrag.getMemFragPtr() && iFsiFrag.getMemFragSize())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData Received Format Specific Info, len=%d", iFsiFrag.getMemFragSize()));
        iObserver->ReceivedFormatSpecificInfo(lcn, (uint8*)iFsiFrag.getMemFragPtr(), iFsiFrag.getMemFragSize());
    }

    if (IsSegmentable() && iWaitForRandomAccessPoint)
    {
        if ((mediaData->getMarkerInfo()&PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT) == 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData Not random access point.  Dropping media data."));
            return PVMFErrInvalidState;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData Found random access point."));
        iWaitForRandomAccessPoint = false;
    }
    else if (iNumPendingPdus == (iNumMediaData - 1))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData - ERROR Overflow, iNumPendingPdus=%d", iNumPendingPdus));
        return PVMFErrOverflow;
    }

    uint32 num_frags_required = 0;
    uint32 frag_num = 0;
    for (frag_num = 0; frag_num < mediaData->getNumFragments(); frag_num++)
    {
        OsclRefCounterMemFrag memfrag;
        mediaData->getMediaFragment(frag_num, memfrag);
        if (memfrag.getMemFragSize() > iMaxSduSize)
        {
            num_frags_required++;
        }
    }

    if ((mediaData->getNumFragments() + num_frags_required + iNumPendingPdus) >= (iNumMediaData - 1))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData - ERROR Overflow, iNumPendingPdus=%d, num_frags_required=%d,iNumMediaData=%d", iNumPendingPdus, num_frags_required, iNumMediaData));
        Flush();
        /* Start re-buffering */
        BufferMedia((uint16)iSetBufferMediaMs);
        return PVMFErrOverflow;
    }

    /* Fragment the sdu if needed */
    PVMFSharedMediaDataPtr& fragmentedMediaData = mediaData;
    if (num_frags_required)
    {
        if (true != FragmentPacket(mediaData, fragmentedMediaData))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData - Memory allocation failure on Fragment\n"));
            return PVMFErrOverflow;
        }
    }

    uint32 sdu_size = 0;
    if (iCurPdu.GetRep())
    {
        sdu_size = iCurPdu->getFilledSize() - iAl->GetHdrSz();
        /* Is the timestamp different ? */
        if (iCurPduTimestamp != fragmentedMediaData->getTimestamp() || !IsSegmentable())
        {
            if (PVMFSuccess != CompletePdu())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData - Memory allocation failure on CompletePdu\n"));
                return PVMFErrOverflow;
            }
            sdu_size = 0;
        }
    }
    if (sdu_size == 0)
    {
        //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"H223OutgoingChannel::PutData Sdu size == 0"));
        iCurPdu = StartAlPdu();
        if (!iCurPdu)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "H223OutgoingChannel::PutData - Memory allocation failure on StartAlPdu\n"));
            return PVMFErrOverflow;
        }

        if (iFsiFrag.getMemFragSize())
        {
            iCurPdu->appendMediaFragment(iFsiFrag);
            // reset the FSI frag
            OsclRefCounterMemFrag frag;
            iFsiFrag = frag;
        }
    }

    for (frag_num = 0; frag_num < fragmentedMediaData->getNumFragments(); frag_num++)
    {
        OsclRefCounterMemFrag frag;
        fragmentedMediaData->getMediaFragment(frag_num, frag);
        OSCL_ASSERT(frag.getMemFragSize() <= iMaxSduSize);

        if (sdu_size &&
                ((sdu_size + frag.getMemFragSize() > iMaxSduSize) || !IsSegmentable()))
        {
            if (PVMFSuccess != CompletePdu())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PutData - Memory allocation failure on CompletePdu\n"));
                return PVMFErrOverflow;
            }
            sdu_size = 0;

            iCurPdu = StartAlPdu();

            if (!iCurPdu)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "H223OutgoingChannel::PutData - Memory allocation failure on StartAlPdu\n"));
                return PVMFErrOverflow;
            }
        }

        iCurPdu->appendMediaFragment(frag);
        sdu_size += frag.getMemFragSize();
        iCurPduTimestamp = fragmentedMediaData->getTimestamp();

    }

    if (iMediaType.isCompressed() && iMediaType.isAudio())
    {
        PVMF_OUTGOING_AUDIO_LOGDATATRAFFIC((0, "Stats of the outgoing audio SDU are: timestamp=%d, size=%d", iCurPduTimestamp, sdu_size));
    }
    else if (iMediaType.isCompressed() && iMediaType.isVideo())
    {
        PVMF_OUTGOING_VIDEO_LOGDATATRAFFIC((0, "Stats of the outgoing video SDU are: timestamp=%d, size=%d", iCurPduTimestamp, sdu_size));
    }
    return ret;
}

bool H223OutgoingChannel::FragmentPacket(PVMFSharedMediaDataPtr& aMediaData, PVMFSharedMediaDataPtr& aFragmentedMediaData)
{
    OsclRefCounterMemFrag memfrag;
    OsclSharedPtr<PVMFMediaDataImpl> newpack;
    newpack = iMediaFragGroupAlloc->allocate();
    if (!newpack.GetRep())
    {
        return false;
    }

    int32 pkt_size = 0;
    PVMFTimestamp timestamp = aMediaData->getTimestamp();
    for (uint32 frag_num = 0; frag_num < aMediaData->getNumFragments(); frag_num++)
    {
        aMediaData->getMediaFragment(frag_num, memfrag);
        pkt_size = memfrag.getMemFragSize();
        if ((unsigned)pkt_size <= iMaxSduSize)
        {
            newpack->appendMediaFragment(memfrag);
        }
        else  /* Need to fragment it */
        {
            uint8* pos = (uint8*)memfrag.getMemFragPtr();
            int32 trim_frag_sz = iMaxSduSize;
            while (pkt_size)
            {
                trim_frag_sz = ((unsigned)pkt_size > iMaxSduSize) ? iMaxSduSize : pkt_size;
                pkt_size -= trim_frag_sz;
                OsclRefCounterMemFrag trim_frag(memfrag);
                trim_frag.getMemFrag().ptr = pos;
                trim_frag.getMemFrag().len = trim_frag_sz;
                newpack->appendMediaFragment(trim_frag);
                pos += trim_frag_sz;
            }
        }
    }
    aFragmentedMediaData = PVMFMediaData::createMediaData(newpack, iMediaMsgMemoryPool);
    if (aFragmentedMediaData.GetRep())
    {
        aFragmentedMediaData->setTimestamp(timestamp);
        return true;
    }
    return false;
}

OsclSharedPtr<PVMFMediaDataImpl> H223OutgoingChannel::StartAlPdu()
{
    PV_STAT_INCR(iNumSdusIn, 1)

    // allocate packet
    OsclSharedPtr<PVMFMediaDataImpl> pdu = iMediaFragGroupAlloc->allocate();
    if (pdu)
    {
        // Add header
        PVMFStatus status = iAl->StartPacket(pdu);
        if (status != PVMFSuccess)
        {
            pdu.Unbind();
            return pdu;
        }
        iNumPendingPdus++;
    }

    return pdu;
}

PVMFStatus H223OutgoingChannel::CompletePdu()
{
    PVMFStatus status = iAl->CompletePacket(iCurPdu);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::CompletePdu Memory allocation failedlcn=%d, CompletePacket status=%d", lcn, status));
        return status;
    }
    // Add it to the outgoing queue
    status = AppendOutgoingPkt(iCurPdu, iCurPduTimestamp);
    if (status != PVMFSuccess)
    {
        return status;
    }
    iCurPdu.Unbind();
    iCurPduTimestamp = 0;
    return PVMFSuccess;
}

PVMFStatus H223OutgoingChannel::AppendOutgoingPkt(OsclSharedPtr<PVMFMediaDataImpl>& pdu,
        PVMFTimestamp timestamp,
        OsclRefCounterMemFrag* fsi)
{
    PVMFSharedMediaDataPtr mediaData = PVMFMediaData::createMediaData(pdu, iMediaMsgMemoryPool);
    if (mediaData.GetRep() == NULL)
    {
        return PVMFErrNoMemory;
    }

    mediaData->setTimestamp(timestamp);
    if (fsi)
    {
        mediaData->setFormatSpecificInfo(*fsi);
    }
    void* memory_for_entry = iMediaDataEntryAlloc->allocate(sizeof(LCMediaDataEntry));
    if (!memory_for_entry)
    {
        // if couldn't allocate memory - leave
        return PVMFErrNoMemory;
    }
    LCMediaDataEntry* entry = new(memory_for_entry) LCMediaDataEntry();
    entry->mediaData = mediaData;

    LCMediaDataEntry* first = entry;
    PVMFTimestamp lastTS = timestamp;
    if (lastMediaData)
    {
        first = lastMediaData->next;
        lastMediaData->next = entry;
        lastTS = lastMediaData->mediaData->getTimestamp();
    }
    lastMediaData = entry;
    entry->next = first;

    /* Adjust buffering parameters */
    if (iBufferMediaMs)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::AppendOutgoingPkt lcn=%d, last ts=%d,cur ts=%d", lcn, lastTS, timestamp));
        /* Compute delta_t from last media data */
        int32 delta_t = timestamp - lastTS;
        if (delta_t < 0)
            delta_t += TIMESTAMP_MAX;
        iBufferMediaMs -= delta_t;
        iBufferMediaBytes -= mediaData->getFilledSize();
        if (iBufferMediaMs <= 0 || iBufferMediaBytes <= 0)
        {
            iBufferMediaMs = 0;
            iBufferMediaBytes = 0;
        }
    }
    return PVMFSuccess;
}

bool H223OutgoingChannel::GetNextPacket(PVMFSharedMediaDataPtr& aMediaData, PVMFStatus aStatus)
{
    if (!iMuxingStarted && aStatus == PVMFSuccess)
        iMuxingStarted = true;

    if (lastMediaData == NULL)
    {
        return false;
    }
    if ((aStatus == PVMFSuccess) && iPaused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::GetNextPacket Logical channel %d paused.", lcn));
        return false;

    }
    if ((aStatus == PVMFSuccess) && iBufferMediaMs && iBufferMediaBytes)
    {
        /* Still buffering */
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::GetNextPacket Buffering lcn=%d, ms left=%d", lcn, iBufferMediaMs));
        return false;
    }

    LCMediaDataEntry* first = lastMediaData->next;
    aMediaData = first->mediaData;

    if (first == lastMediaData)
    {
        lastMediaData = NULL;
    }
    else
    {
        lastMediaData->next = first->next;
    }
    first->~LCMediaDataEntry();
    iMediaDataEntryAlloc->deallocate(first);

    iNumPendingPdus--;
    return true;
}

OsclAny H223OutgoingChannel::ReleasePacket(PVMFSharedMediaDataPtr& aMediaData)
{
    OsclSharedPtr<PVMFMediaDataImpl> aMediaDataImpl;
    aMediaData->getMediaDataImpl(aMediaDataImpl);
    aMediaDataImpl->clearMediaFragments();
}

OsclAny H223OutgoingChannel::Flush()
{
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"H223OutgoingChannel::Flush\n"));

    PVMFSharedMediaDataPtr aMediaData;
    // clear messages in input queue
    ClearMsgQueues();
    // go through pending queue
    while (GetNextPacket(aMediaData, PVMFErrCancelled))
    {
        PV_STAT_INCR(iNumBytesFlushed, aMediaData->getFilledSize())
        OsclSharedPtr<PVMFMediaDataImpl> aMediaDataImpl;
        aMediaData->getMediaDataImpl(aMediaDataImpl);
        aMediaDataImpl->clearMediaFragments();
    }
    if (iCurPdu.GetRep())
    {
        iCurPdu->clearMediaFragments();
        iCurPdu.Unbind();
    }
    iCurPduTimestamp = 0;
    iNumPendingPdus = 0;
}
OsclAny H223OutgoingChannel::ResetStats()
{
    iNumPacketsIn = 0;
    iNumSdusIn = 0;
    iNumBytesIn = 0;
    iNumSdusDropped = 0;
    iNumSdusOut = 0;
    iNumBytesOut = 0;
    iMaxPacketMuxTime = 0;
    iMaxSduMuxTime = 0;
    iNumFlush = 0;
    iNumBytesFlushed = 0;
}

OsclAny H223OutgoingChannel::LogStats()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Outgoing Logical Channel %d Statistics:\n", lcn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Adaptation layer header bytes -  %d\n", iAl->GetHdrSz()));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Adaptation layer trailer bytes -  %d\n", iAl->GetTrlrSz()));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num packets received - %d\n", iNumPacketsIn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus received - %d\n", iNumSdusIn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes received - %d\n", iNumBytesIn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus dropped - %d\n", iNumSdusDropped));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus output - %d\n", iNumSdusOut));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes output - %d\n", iNumBytesOut));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Max packet mux time - %d\n", iMaxPacketMuxTime));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Max sdu mux time - %d\n", iMaxSduMuxTime));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num flush - %d\n", iNumFlush));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes flushed - %d\n", iNumBytesFlushed));
}

OSCL_EXPORT_REF PVMFStatus H223OutgoingChannel::Connect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Connect, aPort=%x", aPort));

    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Connect Error: Already connected"));
        return PVMFFailure;
    }

    PvmiCapabilityAndConfig* config = NULL;
    OsclAny* tempInterface = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, tempInterface);
    config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, tempInterface);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Connect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }

    PVMFStatus status = NegotiateInputSettings(config);

    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Connect: Error - Settings negotiation failed. status=%d", status));
        return status;
    }

    //Automatically connect the peer.
    if ((status = aPort->PeerConnect(this)) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::Connect: Error - Peer Connect failed. status=%d", status));
        return status;
    }

    iConnectedPort = aPort;

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}
OSCL_EXPORT_REF PVMFStatus H223OutgoingChannel::PeerConnect(PVMFPortInterface* aPort)
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::PeerConnect aPort=0x%x", this, aPort));
    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "0x%x H223OutgoingChannel::PeerConnect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }
    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "0x%x H223OutgoingChannel::PeerConnect: Error - Already connected", this));
        return PVMFFailure;
    }

    OsclAny* config = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, config);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::PeerConnect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }


    PVMFStatus status = PVMFSuccess;

    status = NegotiateInputSettings((PvmiCapabilityAndConfig*)config);

    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::PeerConnect: Error - Settings negotiation failed. status=%d", status));
        return status;
    }


    iConnectedPort = aPort;
    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);

    return status;
}

PVMFStatus H223OutgoingChannel::NegotiateFSISettings(PvmiCapabilityAndConfig* aConfig)
{
    PvmiKvp* kvp = NULL;
    int numParams = 0;

    // Preconfigured FSI
    uint8* pc_fsi = NULL;
    unsigned pc_fsilen = ::GetFormatSpecificInfo(iDataType, pc_fsi);
    if (pc_fsilen && pc_fsi)
    {
        /*
         * Create PvmiKvp for capability settings
         */
        OsclMemAllocator alloc;
        PvmiKvp kvp;
        kvp.key = NULL;
        kvp.length = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY) + 1; // +1 for \0
        kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
        if (kvp.key == NULL)
        {
            return PVMFFailure;
        }
        oscl_strncpy(kvp.key, PVMF_FORMAT_SPECIFIC_INFO_KEY, kvp.length);

        kvp.value.key_specific_value = (OsclAny*)pc_fsi;
        kvp.capacity = pc_fsilen;
        kvp.length = pc_fsilen;

        PvmiKvp* retKvp = NULL; // for return value
        int32 err;
        OSCL_TRY(err, aConfig->setParametersSync(NULL, &kvp, 1, retKvp););
        alloc.deallocate((OsclAny*)(kvp.key));
        if (err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::NegotiateFSISettings, Failed to set FSI on peer, err=%d", err));
        }
        return err;
    }


    // No preconfigured FSI.  In this case try to get the FSI from the peer.
    PVMFStatus status = aConfig->getParametersSync(NULL, (PvmiKeyType)PVMF_FORMAT_SPECIFIC_INFO_KEY, kvp, numParams, NULL);
    if (status != PVMFSuccess || numParams != 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateFSISettings: Failed to get FSI from peer"));
        return PVMFSuccess;
    }
    else
    {
        ReceivedFSIFromPeer(kvp);
        aConfig->releaseParameters(NULL, kvp, numParams);
    }

    kvp = NULL;
    numParams = 0;
    return PVMFSuccess;

}

PVMFStatus H223OutgoingChannel::ReceivedFSIFromPeer(PvmiKvp* kvp)
{
    // Create mem frag for VOL header
    OsclRefCounter* my_refcnt;
    OsclMemAllocDestructDealloc<uint8> my_alloc;
    uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >));
    uint8* my_ptr = (uint8*) my_alloc.allocate(aligned_refcnt_size + kvp->length);
    my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterSA< OsclMemAllocDestructDealloc<uint8> >(my_ptr));
    my_ptr += aligned_refcnt_size;

    oscl_memcpy(my_ptr, kvp->value.key_specific_value, kvp->length);

    OsclMemoryFragment memfrag;
    memfrag.len = kvp->length;
    memfrag.ptr = my_ptr;

    OsclRefCounterMemFrag configinfo(memfrag, my_refcnt, kvp->length);
    iFsiFrag = configinfo;

    SetFormatSpecificInfo((uint8*)kvp->value.key_specific_value, kvp->length);

    iObserver->ReceivedFormatSpecificInfo(lcn, (uint8*)kvp->value.key_specific_value, kvp->length);
    return PVMFSuccess;
}


PVMFStatus H223OutgoingChannel::NegotiateInputSettings(PvmiCapabilityAndConfig* aConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::NegotiateInputSettings, aConfig=%x", aConfig));

    PvmiKvp* kvp = NULL;
    int numParams = 0;
    int32 i = 0;

    // Get supported output formats from peer
    PVMFStatus status = aConfig->getParametersSync(NULL,
                        OSCL_CONST_CAST(char*, OUTPUT_FORMATS_CAP_QUERY),
                        kvp, numParams, NULL);
    if (status != PVMFSuccess || numParams == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings, Error:  getParametersSync failed.  status=%d", status));
        return status;
    }

    PvmiKvp* selectedKvp = NULL;
    PVCodecType_t codec_type = GetCodecType(iDataType);
    PVMFFormatType lcn_format_type = PVCodecTypeToPVMFFormatType(codec_type);

    for (i = 0; i < numParams && !selectedKvp; i++)
    {
        if (lcn_format_type == kvp[i].value.pChar_value)
            selectedKvp = &(kvp[i]);
    }

    if (!selectedKvp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings, Error:  Input format not supported by peer"));
        return PVMFFailure;
    }
    if (PVMFSuccess != setConfigParametersSync(selectedKvp, aConfig))
    {
        return PVMFFailure;
    }

    aConfig->releaseParameters(NULL, kvp, numParams);
    kvp = NULL;
    numParams = 0;


    if (iMediaType.isVideo())
    {
        // frame width negotiations
        uint32 width = GetVideoFrameSize(iDataType, true);
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_WIDTH_CAP_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings: Error - config->getParametersSync(cap width) failed"));
            // do not report error for now as enc nodes dont implemlement some parameters

        }
        else
        {
            if (kvp[0].value.uint32_value > width)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(VIDEO_OUTPUT_WIDTH_CUR_QUERY) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings: Error - alloc failed for VIDEO_OUTPUT_WIDTH_CUR_QUERY kvp "));
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, VIDEO_OUTPUT_WIDTH_CUR_QUERY, kvp.length);
                kvp.value.uint32_value = width;

                if (PVMFSuccess != setConfigParametersSync(&kvp, aConfig))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings, Error:  setConfigParametersSync width failed"));
                    //dont return PVMFFailure for now ;
                }


                alloc.deallocate((OsclAny*)(kvp.key));

            }
            aConfig->releaseParameters(NULL, kvp, numParams);
        }

        kvp = NULL;
        numParams = 0;



        // frame height negotiations
        uint32 height = GetVideoFrameSize(iDataType, false);
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_HEIGHT_CAP_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings: Error - config->getParametersSync(cap height) failed"));
            // do not report error for now as enc nodes dont implemlement some parameters

        }
        else
        {
            if (kvp[0].value.uint32_value > height)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(VIDEO_OUTPUT_HEIGHT_CUR_QUERY) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings: Error - alloc failed for VIDEO_OUTPUT_HEIGHT_CUR_QUERY kvp "));
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, VIDEO_OUTPUT_HEIGHT_CUR_QUERY, kvp.length);
                kvp.value.uint32_value = height;

                if (PVMFSuccess != setConfigParametersSync(&kvp, aConfig))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings, Error:  setConfigParametersSync height failed"));
                    //dont return PVMFFailure for now;
                }
                alloc.deallocate((OsclAny*)(kvp.key));

            }
            aConfig->releaseParameters(NULL, kvp, numParams);
        }

        kvp = NULL;
        numParams = 0;

        // frame rate negotiations
        uint32 framerate = GetMaxFrameRate(iDataType);
        // VIDEO_OUTPUT_FRAME_RATE_CAP_QUERY not available
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings: Error - config->getParametersSync(cap frame rate failed"));
            // do not report error for now as enc nodes dont implemlement some parameters

        }
        else
        {
            if (kvp[0].value.float_value > framerate)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings: Error - alloc failed for VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY kvp "));
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY, kvp.length);
                kvp.value.float_value = (float)framerate;

                if (PVMFSuccess != setConfigParametersSync(&kvp, aConfig))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "H223OutgoingChannel::NegotiateInputSettings, Error:  setConfigParametersSync frame rate failed"));
                    //dont return PVMFFailure for now ;
                }
                alloc.deallocate((OsclAny*)(kvp.key));

            }
            aConfig->releaseParameters(NULL, kvp, numParams);
        }

        kvp = NULL;
        numParams = 0;
    }

    return NegotiateFSISettings(aConfig);
}
////////////////////////////////////////////////////////////////////////////
//                  PvmiCapabilityAndConfig
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223OutgoingChannel::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    // Not supported
    OSCL_UNUSED_ARG(aObserver);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus H223OutgoingChannel::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::getParametersSync"));
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);

    parameters = NULL;
    num_parameter_elements = 0;

    if (pv_mime_strcmp(identifier, INPUT_FORMATS_CAP_QUERY) == 0)
    {
        num_parameter_elements = 1;
        PVMFStatus status = AllocateKvp(iKvpMemAlloc, parameters,
                                        OSCL_CONST_CAST(char*, INPUT_FORMATS_VALTYPE),
                                        num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::getParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        PVCodecType_t codec_type = GetCodecType(iDataType);

        PVMFFormatType format = PVCodecTypeToPVMFFormatType(codec_type).getMIMEStrPtr();
        if (format == PVMF_MIME_AMR_IF2)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_AMR_IF2;
        else if (format == PVMF_MIME_PCM16)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_PCM16;
        else if (format ==  PVMF_MIME_YUV420)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_YUV420;
        else if (format ==  PVMF_MIME_M4V)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_M4V;
        else if (format ==  PVMF_MIME_H2632000)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_H2632000;
        else if (format ==  PVMF_MIME_H2631998)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_H2631998;
        else
            parameters[0].value.pChar_value = (char*) PVMF_MIME_FORMAT_UNKNOWN;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus H223OutgoingChannel::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(num_elements);

    if (parameters)
    {
        iKvpMemAlloc.deallocate((OsclAny*)parameters);
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223OutgoingChannel::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223OutgoingChannel::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223OutgoingChannel::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223OutgoingChannel::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    OSCL_UNUSED_ARG(session);
    PVMFStatus status = PVMFSuccess;
    ret_kvp = NULL;

    for (int32 i = 0; i < num_elements; i++)
    {
        status = VerifyAndSetParameter(&(parameters[i]), true);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::setParametersSync: Error - VerifiyAndSetParameter failed on parameter #%d", i));
            ret_kvp = &(parameters[i]);
            /* Silently ignore unrecognized codecs untill CapEx is supported by peer */
            //OSCL_LEAVE(OsclErrArgument);
        }
    }
}

PVMFStatus H223OutgoingChannel::VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam)
{
    OSCL_UNUSED_ARG(aSetParam);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::VerifyAndSetParameter: aKvp=0x%x, aSetParam=%d", aKvp, aSetParam));

    if (!aKvp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::VerifyAndSetParameter: Error - Invalid key-value pair"));
        return PVMFFailure;
    }

    if (iDataType == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::VerifyAndSetParameter: Error - DataType == NULL"));
        return PVMFErrNotSupported;
    }

    if (pv_mime_strcmp(aKvp->key, INPUT_FORMATS_VALTYPE) == 0)
    {
        PVCodecType_t codec_type = GetCodecType(iDataType);
        PVMFFormatType lcn_format_type = PVCodecTypeToPVMFFormatType(codec_type);
        if (pv_mime_strcmp(lcn_format_type.getMIMEStrPtr(), aKvp->value.pChar_value) != 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::VerifyAndSetParameter: Error - Input format %s not supported", aKvp->value.pChar_value));
            return PVMFErrNotSupported;
        }
    }
    else if (pv_mime_strcmp(aKvp->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        ReceivedFSIFromPeer(aKvp);
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId H223OutgoingChannel::setParametersAsync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp,
        OsclAny* context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(ret_kvp);
    OSCL_UNUSED_ARG(context);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 H223OutgoingChannel::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus H223OutgoingChannel::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    OSCL_UNUSED_ARG(session);

    PVMFStatus status = PVMFSuccess;
    for (int32 i = 0; (i < num_elements) && (status == PVMFSuccess); i++)
        status = VerifyAndSetParameter(&(parameters[i]), true);

    return status;
}

void H223OutgoingChannel::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    if (aActivity.iType != PVMF_PORT_ACTIVITY_INCOMING_MSG &&
            aActivity.iType != PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::HandlePortActivity Unhandled port activity: %d", aActivity.iType));
        return;
    }
    PVMFStatus aStatus;
    PVMFSharedMediaMsgPtr aMsg;
    while (IncomingMsgQueueSize())
    {
        aStatus = DequeueIncomingMsg(aMsg);
        if (aStatus == PVMFSuccess)
        {
            PutData(aMsg);
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223OutgoingChannel::HandlePortActivity Failed to DeQueue incoming message: %d", aStatus));
            break;
        }
    }
}

OSCL_EXPORT_REF PVMFStatus H223OutgoingControlChannel::PeerConnect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingControlChannel::PeerConnect aPort=0x%x", this, aPort));
    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "0x%x H223OutgoingControlChannel::PeerConnect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }
    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "0x%x H223OutgoingControlChannel::PeerConnect: Error - Already connected", this));
        return PVMFFailure;
    }

    iConnectedPort = aPort;
    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);

    return PVMFSuccess;

}
PVMFStatus H223OutgoingControlChannel::PutData(PVMFSharedMediaMsgPtr aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingControlChannel::PutData - iNumPendingPdus=%d,iNumMediaData=%d", iNumPendingPdus, iNumMediaData));

    PVMFSharedMediaDataPtr mediaData;
    convertToPVMFMediaData(mediaData, aMsg);

    PV_STAT_SET_TIME(iStartTime, iNumPacketsIn)
    PV_STAT_INCR(iNumPacketsIn, 1)
    PV_STAT_INCR(iNumSdusIn, 1)
    PV_STAT_INCR(iNumBytesIn, (mediaData->getFilledSize()))

    OsclSharedPtr<PVMFMediaDataImpl> pdu;
    pdu = iMediaFragGroupAlloc->allocate();

    if (!pdu)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingControlChannel::PutData - Memory allocation failure on iMediaFragGroupAlloc->allocate\n"));
        return PVMFErrNoMemory;
    }

    TimeValue timenow;
    OsclRefCounterMemFrag frag;
    for (uint frag_num = 0; frag_num < mediaData->getNumFragments(); frag_num++)
    {
        mediaData->getMediaFragment(frag_num, frag);
        // Add data fragments
        pdu->appendMediaFragment(frag);
    }

    PVMFStatus status = iAl->CompletePacket(pdu);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingControlChannel::PutData - Memory allocation failure on iAl->CompletePacket()"));
        return status;
    }

    OsclRefCounterMemFrag fsi;
    bool fsi_available = mediaData->getFormatSpecificInfo(fsi);
    // Add it to the outgoing queue
    if (PVMFSuccess != AppendOutgoingPkt(pdu, timenow.to_msec(), (fsi_available ? &fsi : NULL)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingControlChannel::PutData - Memory allocation failure on AppendOutgoingPkt()"));
        return PVMFErrNoMemory;
    }
    return PVMFSuccess;
}

void H223LogicalChannel::SetDatapathLatency(uint32 aLatency)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LogicalChannel::SetDatapathLatency lcn=%d, aLatency=%d", lcn, aLatency));
    iDatapathLatency = aLatency;
}

PVMFStatus H223LogicalChannel::setConfigParametersSync(PvmiKvp* selectedKvp,
        PvmiCapabilityAndConfig* aConfig,
        PVMFFormatType lcn_format_type,
        bool aTryTwice)
{
    int32 err = 0;
    PvmiKvp* retKvp = NULL;
    if (!aTryTwice)
    {
        OSCL_TRY(err, aConfig->setParametersSync(NULL, selectedKvp, 1, retKvp););
        OSCL_FIRST_CATCH_ANY(err,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::setConfigParametersSync, Error:  setParametersSync failed, err=%d", err));
                             return PVMFFailure;
                            );
    }
    else
    {
        int32 err = 0;
        OSCL_TRY(err, aConfig->setParametersSync(NULL, selectedKvp, 1, retKvp););
        if (err)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::setConfigParametersSync, Error:  setParametersSync failed for pChar value, trying uint32, err=%d", err));
            selectedKvp->value.pChar_value = OSCL_STATIC_CAST(mbchar*, lcn_format_type.getMIMEStrPtr());
            err = 0;
            OSCL_TRY(err, aConfig->setParametersSync(NULL, selectedKvp, 1, retKvp););
            if (err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::setConfigParametersSync, Error:  setParametersSync failed, err=%d", err));
                return PVMFFailure;
            }
        }
    }
    return PVMFSuccess;
}
H223IncomingChannel::H223IncomingChannel(TPVChannelId num,
        bool segmentable,
        OsclSharedPtr<AdaptationLayer>& al,
        PS_DataType data_type,
        LogicalChannelObserver* observer,
        uint32 bitrate,
        uint32 sample_interval,
        uint32 num_media_data)
        : H223LogicalChannel(num, segmentable, al, data_type, observer, bitrate, sample_interval, num_media_data),
        iMediaMsgMemoryPool(NULL),
        iMediaFragGroupAlloc(NULL),
        iPduPktMemPool(NULL),
        iMediaDataAlloc(&iMemAlloc),
        iPduSize(al->GetPduSize()),
        iCurPduSize(0),
        iRenderingSkew(0)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.H223IncomingChannel");
    iIncomingAudioLogger = PVLogger::GetLoggerObject("datapath.incoming.audio.h223.lcn");
    iIncomingVideoLogger = PVLogger::GetLoggerObject("datapath.incoming.video.h223.lcn");
    iAlPduFragPos = NULL;
    ResetStats();
}

H223IncomingChannel::~H223IncomingChannel()
{
    Flush();
    OsclRefCounterMemFrag frag;
    iAlPduFrag = frag;
    if (iMediaFragGroupAlloc)
    {
        iMediaFragGroupAlloc->removeRef();
    }
    if (iPduPktMemPool)
    {
        OSCL_DELETE(iPduPktMemPool);
    }
    if (iMediaMsgMemoryPool)
    {
        OSCL_DELETE(iMediaMsgMemoryPool);
    }
}

void H223IncomingChannel::Init()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Init"));
    H223LogicalChannel::Init();

    int bitrate = (GetBitrate() > 0) ? GetBitrate() : DEF_CHANNEL_BITRATE;
    int mem_size = (DEF_INCOMING_CHANNEL_BUFFER_SIZE_MS * bitrate) >> PV2WAY_BPS_TO_BYTES_PER_MSEC_RIGHT_SHIFT;
    int num_fragments = (mem_size / H223_INCOMING_CHANNEL_FRAGMENT_SIZE) + 1;
    iMediaMsgMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (H223_INCOMING_CHANNEL_NUM_MEDIA_MSG));
    if (iMediaMsgMemoryPool == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    iMediaMsgMemoryPool->enablenullpointerreturn();

    iPduPktMemPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (H223_INCOMING_CHANNEL_NUM_MEDIA_DATA));
    if (iPduPktMemPool == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    iPduPktMemPool->enablenullpointerreturn();

    iMediaFragGroupAlloc = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>, (H223_INCOMING_CHANNEL_NUM_MEDIA_DATA, H223_INCOMING_CHANNEL_NUM_FRAGS_IN_MEDIA_DATA, iPduPktMemPool));
    if (iMediaFragGroupAlloc == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    iMediaFragGroupAlloc->create();

    iMemFragmentAlloc.SetLeaveOnAllocFailure(false);
    iMemFragmentAlloc.size((uint16)num_fragments, (uint16)H223_INCOMING_CHANNEL_FRAGMENT_SIZE);

    ResetAlPdu();
    AllocateAlPdu();
    iCurTimestamp = 0;
    iNumSdusIn = 0;
    iNumPdusIn = 0;
}

OsclAny H223IncomingChannel::ResetAlPdu()
{
    iAlPduFragPos = NULL;
    iCurPduSize = 0;
    OsclRefCounterMemFrag frag;
    iAlPduFrag = frag;
    iAlPduMediaData.Unbind();
}

OsclAny H223IncomingChannel::AllocateAlPdu()
{
    iAlPduMediaData = iMediaFragGroupAlloc->allocate();
    if (iAlPduMediaData.GetRep() == NULL)
    {
        return;
    }
    AppendAlPduFrag();
}

OsclAny H223IncomingChannel::AppendAlPduFrag()
{
    OsclRefCounterMemFrag ref_counter_mem_frag = iMemFragmentAlloc.get();
    if (ref_counter_mem_frag.getMemFragPtr() == NULL)
    {
        return;
    }
    ref_counter_mem_frag.getMemFrag().len = 0;
    iAlPduFrag = ref_counter_mem_frag;
    iAlPduFragPos = (uint8*)iAlPduFrag.getMemFragPtr();
}

uint32 H223IncomingChannel::CopyAlPduData(uint8* buf, uint16 len)
{
    int32 remaining = len;
    uint32 copied = 0;
    do
    {
        copied = CopyToCurrentFrag(buf, (uint16)remaining);
        buf += copied;
        remaining -= copied;
    }
    while (remaining && copied);
    return (uint32)(len - remaining);
}

uint32 H223IncomingChannel::CopyToCurrentFrag(uint8* buf, uint16 len)
{
    if (iAlPduMediaData.GetRep() == NULL)
    {
        AllocateAlPdu();
    }
    else if (iAlPduFragPos == NULL)
    {
        AppendAlPduFrag();
    }
    if (iAlPduFragPos == NULL)
    {
        return 0;
    }

    uint32 space_in_current_frag = ((uint8*)iAlPduFrag.getMemFragPtr() + iAlPduFrag.getCapacity() - iAlPduFragPos);
    OSCL_ASSERT(space_in_current_frag > 0);
    uint32 num_bytes_copied = (len > space_in_current_frag) ? space_in_current_frag : len;
    oscl_memcpy(iAlPduFragPos, buf, num_bytes_copied);
    iAlPduFrag.getMemFrag().len += num_bytes_copied;
    iAlPduFragPos += num_bytes_copied;
    space_in_current_frag -= num_bytes_copied;
    if (space_in_current_frag == 0)
    {
        iAlPduMediaData->appendMediaFragment(iAlPduFrag);
        iAlPduFragPos = NULL;
    }
    PVLOGGER_LOG_USE_ONLY(
        if (iAlPduMediaData->getFilledSize() > iPduSize)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::CopyToCurrentFrag WARNING current pdu size=%d > iPduSize=%d", iAlPduMediaData->getFilledSize(), iPduSize));
    }
    );
    return num_bytes_copied;
}

void H223IncomingChannel::PreAlPduData()

{
    if (iSendFormatSpecificInfo)
    {
        SendFormatSpecificInfo();
    }

    if (iPaused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::PreAlPduData Logical channel paused.  Dropping media data."));
        return;
    }
}

PVMFStatus H223IncomingChannel::AlPduData(uint8* buf, uint16 len)
{
    PV_STAT_INCR(iNumBytesIn, len)
    PV_STAT_SET_TIME(iStartTime, iNumBytesIn)

    PreAlPduData();
    if (PVMFSuccess != DispatchPendingSdus())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::AlPduData lcn=%d, Failed to dispatch pending sdus", lcn));
    }
    if (iAlPduMediaData.GetRep() == NULL || iAlPduMediaData->getFilledSize() == 0)
    {
        bool overflow = false;
        iClock->GetCurrentTime32(iCurTimestamp, overflow, PVMF_MEDIA_CLOCK_MSEC);
    }
    uint32 copied = CopyAlPduData(buf, len);
    return (copied == len) ? PVMFSuccess : PVMFFailure;
}

PVMFStatus H223IncomingChannel::AlDispatch()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::AlDispatch lcn=%d, iCurPduSize=%d, sn=%d", lcn, iCurPduSize, iNumSdusIn));
    IncomingALPduInfo info;

    if (iPaused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::AlDispatch Logical channel paused."));
        return PVMFFailure;
    }

    /*  Nothing to dispatch */
    if (!iAlPduMediaData.GetRep())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::AlDispatch Nothing to dispatch."));
        ResetAlPdu();
        return PVMFSuccess;
    }

    /*  Add pending fragment to the media message */
    if (iAlPduFragPos)
    {
        iAlPduMediaData->appendMediaFragment(iAlPduFrag);
    }

    /* Parse AL headers etc */
    iAl->ParsePacket(iAlPduMediaData, info);
    int32 len = info.sdu_size;
    if (len <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223OutgoingChannel::AlDispatch Empty SDU lcn=%d, len=%d", lcn, len));
        ResetAlPdu();
        return PVMFErrCorrupt;
    }

    PV_STAT_INCR_COND(iNumCrcErrors, 1, info.crc_error)
    PV_STAT_INCR_COND(iNumSeqNumErrors, 1, info.seq_num_error)
    PVMFStatus status = PVMFSuccess;

    // set the errors flag
    uint32 errorsFlag = 0;
    if (info.crc_error)
    {
        errorsFlag |= PVMF_MEDIA_DATA_BIT_ERRORS;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::AlDispatch CRC error lcn=%d, size=%d", lcn, len));
        status =  PVMFErrCorrupt;
    }
    else
    {
        PV_STAT_INCR(iNumSdusIn, 1)
    }
    if (info.seq_num_error)
    {
        errorsFlag |= PVMF_MEDIA_DATA_PACKET_LOSS;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::AlDispatch Sequence number error lcn=%d, size=%d", lcn, len));
        status = PVMFErrCorrupt;
    }

    OsclSharedPtr<PVMFMediaData> aMediaData = PVMFMediaData::createMediaData(iAlPduMediaData, iMediaMsgMemoryPool);
    if (aMediaData.GetRep() == NULL)
    {
        return PVMFErrNoMemory;
    }

    PVMFTimestamp baseTimestamp = 0;
    SetSampleTimestamps(baseTimestamp);
    aMediaData->setTimestamp(baseTimestamp);

    iAlPduMediaData->setErrorsFlag(errorsFlag);

    PVMFSharedMediaMsgPtr aMediaMsg;
    convertToPVMFMediaMsg(aMediaMsg, aMediaData);

    PVMFFormatType mediaType = GetFormatType();
    if (mediaType.isCompressed() && mediaType.isAudio())
    {
        // we are using only full audio frames
        aMediaData->setMarkerInfo(PVMF_MEDIA_DATA_MARKER_INFO_M_BIT);
    }

    if (IsConnected())
    {
        if (mediaType.isCompressed() && mediaType.isAudio())
        {
            PVMF_INCOMING_AUDIO_LOGDATATRAFFIC((0, "Incoming audio SDU received. Stats: Entry time=%d, lcn=%d, size=%d,FmtType=%s", iCurTimestamp, lcn, aMediaData->getFilledSize(), mediaType.getMIMEStrPtr()));
        }
        else if (mediaType.isCompressed() && mediaType.isVideo())
        {
            PVMF_INCOMING_VIDEO_LOGDATATRAFFIC((0, "Incoming video SDU received.Stats: Entry time=%d, lcn=%d, size=%d,FmtType=%s", iCurTimestamp, lcn, aMediaData->getFilledSize(), mediaType.getMIMEStrPtr()));
        }
        PVMFStatus dispatch_status = QueueOutgoingMsg(aMediaMsg);
        if (dispatch_status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::AlDispatch Failed to queue outgoing media message lcn=%d, size=%d, status=%d", lcn, len, dispatch_status));
            status = dispatch_status;
        }
    }
    else if (IsSegmentable())
    {
        iPendingSdus.push_back(aMediaMsg);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::AlDispatchL Dropping pdu lcn=%d, iCurPduSize=%d", lcn, iCurPduSize));
        status = PVMFErrNotReady;
    }
    ResetAlPdu();
    AllocateAlPdu();
    return status;
}

OsclAny H223IncomingChannel::Flush()
{
    PV_STAT_INCR(iNumBytesFlushed, (iAlPduFragPos - (uint8*)iAlPduFrag.getMemFragPtr()))
    PV_STAT_INCR_COND(iNumAbort, 1, (iAlPduFragPos - (uint8*)iAlPduFrag.getMemFragPtr()))
    iPendingSdus.clear();
    ResetAlPdu();

}

PVMFStatus H223IncomingChannel::Connect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect lcn(%d)\n", lcn));
    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect Error: Already connected"));
        return PVMFFailure;
    }

    PvmiCapabilityAndConfig* config = NULL;
    OsclAny * tempInterface = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, tempInterface);
    config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, tempInterface);
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }

    PVMFStatus status = NegotiateOutputSettings(config);

    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect: Error - Settings negotiation failed. status=%d", status));
        return status;
    }

    //Automatically connect the peer.
    if ((status = aPort->PeerConnect(this)) != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect: Error - Peer Connect failed. status=%d", status));
        return status;
    }

    iConnectedPort = aPort;

    //Check the BOS command status
    status = SendBeginOfStreamMediaCommand();

    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect: Failed to send BOS message status=%d", status));
        return status;
    }

    /* Send any format specific info if available */
    if (iSendFormatSpecificInfo)
        SendFormatSpecificInfo();

    DispatchPendingSdus();

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);

    return PVMFSuccess;
}

OsclAny H223IncomingChannel::ResetStats()
{
    iNumSdusIn = 0;
    iNumBytesIn = 0;
    iSduSizeExceededCnt = 0;
    iNumCrcErrors = 0;
    iNumSeqNumErrors = 0;
    iNumBytesFlushed = 0;
    iNumAbort = 0;
}

OsclAny H223IncomingChannel::LogStats()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Incoming Logical Channel %d Statistics:\n", lcn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus received - %d\n", iNumSdusIn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes received - %d\n", iNumBytesIn));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num times sdu size exceeded - %d\n", iSduSizeExceededCnt));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus with CRC errors - %d\n", iNumCrcErrors));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus with sequence number errors - %d\n", iNumSeqNumErrors));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num sdus aborted - %d\n", iNumAbort));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes aborted - %d\n", iNumBytesFlushed));
}

PVMFStatus H223IncomingChannel::DispatchPendingSdus()
{
    if (!iConnectedPort)
        return PVMFFailure;
    if (iPendingSdus.size())
    {
        /* Dispatch any pending sdus */
        for (unsigned i = 0; i < iPendingSdus.size(); i++)
        {
            PVMFStatus status = QueueOutgoingMsg(iPendingSdus[i]);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect Error: PutData failed for buffered sdus lcn=%d, status=%d, i=%d", lcn, status, i));
                iObserver->LogicalChannelError(INCOMING, lcn, status);
                return PVMFFailure;
            }
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::Connect lcn=%d,num sdus=%d", lcn, iPendingSdus.size()));
        iPendingSdus.clear();
    }
    return PVMFSuccess;
}

OsclAny H223IncomingChannel::SendFormatSpecificInfo()
{
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"H223IncomingChannel::SendFormatSpecificInfo lcn=%d, iFormatSpecificInfoLen=%d, iFormatSpecificInfo=%x", lcn,iFormatSpecificInfoLen,iFormatSpecificInfo));
    //printBuffer(iLogger, iFormatSpecificInfo, (uint16)iFormatSpecificInfoLen);
    if (!IsConnected())
    {
        //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"H223IncomingChannel::SendFormatSpecificInfo ERROR Not connected."));
        //OSCL_LEAVE(PVMFErrNotReady);
        return;
    }

    // Create mem frag for VOL header
    // Create new media data buffer for header fragment
    OsclSharedPtr<PVMFMediaDataImpl> hdrImpl = iMediaDataAlloc.allocate(iFormatSpecificInfoLen);
    if (!hdrImpl)
    {
        return;
    }
    PVMFSharedMediaDataPtr hdrMediaData = PVMFMediaData::createMediaData(hdrImpl);
    OsclRefCounterMemFrag myVolHeader;
    hdrMediaData->getMediaFragment(0, myVolHeader);
    oscl_memcpy(myVolHeader.getMemFragPtr(), iFormatSpecificInfo, iFormatSpecificInfoLen);
    myVolHeader.getMemFrag().len = iFormatSpecificInfoLen;

    // Create new media data buffer for the message
    OsclSharedPtr<PVMFMediaDataImpl> emptyImpl = iMediaDataAlloc.allocate(0);
    if (!emptyImpl)
    {
        return;
    }
    PVMFSharedMediaDataPtr volData = PVMFMediaData::createMediaData(emptyImpl);

    // Set format specific info in media data message
    volData->setFormatSpecificInfo(myVolHeader);

    // Send VOL header to downstream node
    PVMFSharedMediaMsgPtr volMsg;
    convertToPVMFMediaMsg(volMsg, volData);
    PVMFStatus status = QueueOutgoingMsg(volMsg);
    if (status != PVMFSuccess)
    {
        OSCL_LEAVE(status);
    }
    iSendFormatSpecificInfo = false;
}

MuxSduData::MuxSduData()
{
    size = 0;
    cur_frag_num = 0;
    cur_pos = 0;
}
PVMFStatus H223IncomingChannel::NegotiateOutputSettings(PvmiCapabilityAndConfig* aConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::NegotiateInputSettings, aConfig=%x", aConfig));

    PvmiKvp* kvp = NULL;
    int numParams = 0;
    int32 i = 0;

    // Get supported input formats from peer
    PVMFStatus status = aConfig->getParametersSync(NULL,
                        OSCL_CONST_CAST(char*, INPUT_FORMATS_CAP_QUERY),
                        kvp, numParams, NULL);

    if (status != PVMFSuccess)
    {
        status = aConfig->getParametersSync(NULL,
                                            OSCL_CONST_CAST(char*, "x-pvmf/video/decode/input_formats"),
                                            kvp, numParams, NULL);
    }

    if (status != PVMFSuccess || numParams == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::NegotiateInputSettings, Error:  getParametersSync failed.  status=%d", status));

        return PVMFSuccess;
    }

    PvmiKvp* selectedKvp = NULL;
    PVCodecType_t codec_type = GetCodecType(iDataType);
    PVMFFormatType lcn_format_type = PVCodecTypeToPVMFFormatType(codec_type);

    for (i = 0; i < numParams && !selectedKvp; i++)
    {
        if (lcn_format_type == kvp[i].value.pChar_value)
            selectedKvp = &(kvp[i]);
    }

    if (!selectedKvp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::NegotiateInputSettings, Error:  Input format not supported by peer"));
        return PVMFFailure;
    }

    /* This is for the silly problem of MIO components having the convention
       of returning uint32 for a query and requiring pChar for a setting
       we don't know if we are talking to an MIO or a decoder node
       (which will want a uint32), so we try both.  Try the pchar
       first, because if its expecting pchar and gets uint32, it will
       crash.
    */
    if (PVMFSuccess != setConfigParametersSync(selectedKvp, aConfig, lcn_format_type, true))
        return PVMFFailure;

    aConfig->releaseParameters(NULL, kvp, numParams);
    kvp = NULL;
    numParams = 0;

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
//                  PvmiCapabilityAndConfig
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223IncomingChannel::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    // Not supported
    OSCL_UNUSED_ARG(aObserver);
    OSCL_LEAVE(OsclErrNotSupported);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus H223IncomingChannel::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::getParametersSync"));
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);

    parameters = NULL;
    num_parameter_elements = 0;

    if (pv_mime_strcmp(identifier, OUTPUT_FORMATS_CAP_QUERY) == 0)
    {
        num_parameter_elements = 1;
        PVMFStatus status = AllocateKvp(iKvpMemAlloc, parameters,
                                        OSCL_CONST_CAST(char*, OUTPUT_FORMATS_VALTYPE),
                                        num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::getParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        PVCodecType_t codec_type = GetCodecType(iDataType);
        PVMFFormatType format = PVCodecTypeToPVMFFormatType(codec_type).getMIMEStrPtr();
        if (format == PVMF_MIME_AMR_IF2)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_AMR_IF2;
        else if (format == PVMF_MIME_PCM16)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_PCM16;
        else if (format ==  PVMF_MIME_YUV420)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_YUV420;
        else if (format ==  PVMF_MIME_M4V)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_M4V;
        else if (format ==  PVMF_MIME_H2632000)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_H2632000;
        else if (format ==  PVMF_MIME_H2631998)
            parameters[0].value.pChar_value = (char*) PVMF_MIME_H2631998;
        else
            parameters[0].value.pChar_value = (char*) PVMF_MIME_FORMAT_UNKNOWN;

    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus H223IncomingChannel::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(num_elements);

    if (parameters)
    {
        iKvpMemAlloc.deallocate((OsclAny*)parameters);
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223IncomingChannel::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223IncomingChannel::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223IncomingChannel::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void H223IncomingChannel::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    OSCL_UNUSED_ARG(session);
    PVMFStatus status = PVMFSuccess;
    ret_kvp = NULL;

    for (int32 i = 0; i < num_elements; i++)
    {
        status = VerifyAndSetParameter(&(parameters[i]), true);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::setParametersSync: Error - VerifiyAndSetParameter failed on parameter #%d", i));
            ret_kvp = &(parameters[i]);
            /* Silently ignore unrecognized codecs untill CapEx is supported by peer */
            //OSCL_LEAVE(OsclErrArgument);
        }
    }
}

PVMFStatus H223IncomingChannel::VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam)
{
    OSCL_UNUSED_ARG(aSetParam);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::VerifyAndSetParameter: aKvp=0x%x, aSetParam=%d", aKvp, aSetParam));

    if (!aKvp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::VerifyAndSetParameter: Error - Invalid key-value pair"));
        return PVMFFailure;
    }

    if (iDataType == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::VerifyAndSetParameter: Error - DataType == NULL"));
        return PVMFErrNotSupported;
    }

    if (pv_mime_strcmp(aKvp->key, OUTPUT_FORMATS_VALTYPE) == 0)
    {
        PVCodecType_t codec_type = GetCodecType(iDataType);
        PVMFFormatType lcn_format_type = PVCodecTypeToPVMFFormatType(codec_type);
        if (lcn_format_type != aKvp->value.pChar_value)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::VerifyAndSetParameter: Error - Output format %s not supported", aKvp->value.pChar_value));
            return PVMFErrNotSupported;
        }
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId H223IncomingChannel::setParametersAsync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp,
        OsclAny* context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(ret_kvp);
    OSCL_UNUSED_ARG(context);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 H223IncomingChannel::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus H223IncomingChannel::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    OSCL_UNUSED_ARG(session);

    PVMFStatus status = PVMFSuccess;
    for (int32 i = 0; (i < num_elements) && (status == PVMFSuccess); i++)
        status = VerifyAndSetParameter(&(parameters[i]), true);

    return status;
}

void H223IncomingChannel::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    if (aActivity.iType != PVMF_PORT_ACTIVITY_OUTGOING_MSG &&
            aActivity.iType != PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::HandlePortActivity Unhandled port activity: %d", aActivity.iType));
        return;
    }
    PVMFStatus aStatus;
    PVMFSharedMediaMsgPtr aMsg;
    while (OutgoingMsgQueueSize())
    {
        aStatus = Send();
        if (aStatus != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223IncomingChannel::HandlePortActivity Failed to DeQueue incoming message: %d", aStatus));
            break;
        }
    }
}

PVMFStatus H223IncomingChannel::SendBeginOfStreamMediaCommand()
{
    PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
    sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);

    sharedMediaCmdPtr->setTimestamp(iCurTimestamp);

    uint32 seqNum = 0;
    uint32 streamID = 0;
    sharedMediaCmdPtr->setSeqNum(seqNum);

    PVMFSharedMediaMsgPtr mediaMsgOut;
    convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
    mediaMsgOut->setStreamID(streamID);

    PVMFStatus status = QueueOutgoingMsg(mediaMsgOut);
    if (status != PVMFSuccess)
    {
        // Output queue is busy, so wait for the output queue being ready
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING,
                        (0, "H223IncomingChannel::SendBeginOfMediaStreamCommand: Outgoing queue busy. "));
        return status;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223IncomingChannel::SendBeginOfMediaStreamCommand() BOS Sent StreamId %d ", streamID));
    return status;
}

void H223IncomingChannel::SetSampleTimestamps(PVMFTimestamp& aTSOffset)
{
    iRenderingSkew = iAudioLatency - iVideoLatency;
    uint32 skewDelta = 0;
    if (iRenderingSkew >= (int32)iIncomingSkew)
    {
        skewDelta = iRenderingSkew - iIncomingSkew;
        if (iMediaType.isCompressed() && iMediaType.isAudio())
        {
            aTSOffset = (PVMFTimestamp)(iAudioLatency + PARSING_JITTER_DURATION);
        }
        else if (iMediaType.isCompressed() && iMediaType.isVideo())
        {
            aTSOffset = (PVMFTimestamp)(iVideoLatency + skewDelta + PARSING_JITTER_DURATION);
        }
    }
    else if (iRenderingSkew < (int32)iIncomingSkew)
    {
        skewDelta = iIncomingSkew - iRenderingSkew;
        if (iMediaType.isCompressed() && iMediaType.isAudio())
        {
            aTSOffset = (PVMFTimestamp)(iAudioLatency + PARSING_JITTER_DURATION + skewDelta);
        }
        else if (iMediaType.isCompressed() && iMediaType.isVideo())
        {
            aTSOffset = (PVMFTimestamp)(iVideoLatency + PARSING_JITTER_DURATION);
        }
    }
    aTSOffset += iCurTimestamp;
}


