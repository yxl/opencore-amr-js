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
#include "oscl_time.h"
#include "lowerlayer.h"
#include "media_fragment.h"
#include "h223.h"
#include "pvlogger.h"
#include "level0.h"
#include "level1.h"
#include "level2.h"

#define PV_H223_MAIN_TIMER_ID 0
#define H223_MIN_SAMPLE_INTERVAL 20
#if defined(__WINS__) || defined(WIN32)
#define TIMER_RES 100
#else
#define TIMER_RES 40
#endif
#define H223_MIN_TIMER_RESOLUTION 20
#define H223_LEVEL_DEFAULT H223_LEVEL2

#define H223_INITIAL_STUFFING_SEND_CNT 3
#define H223_INITIAL_STUFFING_SEND_CNT_LEVEL0 25
#define H223_SEND_PDU_SIZE_EXTRA 128

#define CHECKSIZE 21

#define H223_DEMUX_BUFFER_SIZE 2048

H223LowerLayer::H223LowerLayer(int32 aPortTag, TPVLoopbackMode aLoopbackMode)
        :   PvmfPortBaseImpl(aPortTag, this),
        OsclTimerObject(OsclActiveObject::EPriorityHigh, "H223AO"),
        iTimerIntervalNum(0),
        iMinTimerResolution(TIMER_RES),
        iObserver(NULL),
        iMemFragmentAlloc(NULL),
        iDispatchPacketAlloc(NULL),
        iMediaDataImplMemAlloc(NULL),
        iMediaMsgPoolAlloc(NULL),
        iSendPduSz(0),
        iPduSize(H223_DEFAULT_PDU_SIZE),
        iStuffingSize(0),
        iTimer("H223LL"),
        iLoopbackMode(aLoopbackMode),
        iTimerCnt(0),
        iBytesSent(0),
        iMediaDataAlloc(&iMemAlloc),
        iLogger(NULL),
        iDemuxBuffer(NULL),
        iDemuxBufferPos(NULL),
        iIdleSyncCheckBuffer(NULL)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.lowerlayer");
    AddToScheduler();
    InitParams();
    ResetStats();
}

H223LowerLayer::~H223LowerLayer()
{
    Cancel();
    RemoveFromScheduler();
}

void H223LowerLayer::InitParams()
{
    iMinTimerResolution = TIMER_RES;
    iState = 0;
    iStateVal = 0;

    iBitrate = DEFAULT_BITRATE;

    iUseAnnexA = true;
    iUseAnnexADoubleFlag = true;
    iUseAnnexB = true;
    iUseAnnexBwithHeader = true;

    for (uint16 level = H223_LEVEL0; level < H223_LEVEL_UNKNOWN; level++)
    {
        iStuffingSz[level] = (uint16)H223GetMuxStuffingSz((TPVH223Level)level);
    }

    UpdatePduSize();

    InitRuntimeParams();
    SetLevel(H223_LEVEL_DEFAULT);

    iDemuxBufferSize = 0;
    iSyncCheckCount = CHECKSIZE;
    SetLoopbackMode(iLoopbackMode);
    iSkipLevelCheck = false;
}

void H223LowerLayer::InitRuntimeParams()
{
    iState = iStateVal = 0;
    iTimerCnt = 0;
    iBytesSent = 0;
    iLevelSetupComplete = false;
    iSyncDetected = false;
    iDataReceptionStart = false;
}

TPVStatusCode H223LowerLayer::Open()
{
    unsigned pdu_rate = (unsigned)(1000.0 / (float)H223_MIN_SAMPLE_INTERVAL + .5) + 1;
    iMemFragmentAlloc = OSCL_NEW(PVMFBufferPoolAllocator, ());
    iMemFragmentAlloc->SetLeaveOnAllocFailure(false);
    iMemFragmentAlloc->size((uint16)(pdu_rate*2), (uint16)(iSendPduSz + H223_SEND_PDU_SIZE_EXTRA));

    iMediaMsgPoolAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (pdu_rate));
    iMediaMsgPoolAlloc->enablenullpointerreturn();

    uint media_data_imp_size = oscl_mem_aligned_size(sizeof(PVMFMediaFragGroup<OsclMemAllocator>)) +
                               oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    iMediaDataImplMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (pdu_rate, media_data_imp_size));
    iMediaDataImplMemAlloc->enablenullpointerreturn();
    iDispatchPacketAlloc = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>, (pdu_rate, 30, iMediaDataImplMemAlloc));
    iDispatchPacketAlloc->create();

    iDemuxBufferSize = (uint32)((float)(H223_DEMUX_BUFFER_INTERVAL_MS * iBitrate) / 8000.0);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Open iBitrate=%d, iDemuxBufferSize=%d", iBitrate, iDemuxBufferSize));
    iDemuxBufferPos = iDemuxBuffer = (uint8*)OSCL_DEFAULT_MALLOC(H223_DEMUX_BUFFER_SIZE);
    iIdleSyncCheckBuffer = (uint8*)OSCL_DEFAULT_MALLOC(H223_DEMUX_BUFFER_SIZE);
    return EPVT_Success;
}

TPVStatusCode H223LowerLayer::Close()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Close\n"));

    OSCL_DEFAULT_FREE(iIdleSyncCheckBuffer);
    iIdleSyncCheckBuffer = NULL;

    OSCL_DEFAULT_FREE(iDemuxBuffer);
    iDemuxBuffer = iDemuxBufferPos = NULL;

    iDispatchPacketAlloc->removeRef();
    iDispatchPacketAlloc = NULL;

    OSCL_DELETE(iMediaMsgPoolAlloc);
    iMediaMsgPoolAlloc = NULL;

    OSCL_DELETE(iMediaDataImplMemAlloc);
    iMediaDataImplMemAlloc = NULL;

    iMemFragmentAlloc->clear();
    OSCL_DELETE(iMemFragmentAlloc);
    return EPVT_Success;
}

TPVStatusCode H223LowerLayer::Start(H223PduParcomSharedPtr aParcom)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Start(), parcom=%x", aParcom.GetRep()));
    InitRuntimeParams();

    if (aParcom.GetRep())
    {
        iParcom = aParcom;
        iParcom->SetObserver(this);
    }

    /* Start timer */
    iTimer.SetFrequency(1000 / iMinTimerResolution);
    iTimer.SetObserver(this);
    iTimer.Request(PV_H223_MAIN_TIMER_ID, PV_H223_MAIN_TIMER_ID , 1, this, 1);

    iState = 1;
    iSkipLevelCheck = false;
    Mux();
    return 1;
}

TPVStatusCode H223LowerLayer::Stop()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Stop"));

    iTimer.Clear();

    iState = 0;

    iParcom.Unbind();

    iDemuxBufferPos = iDemuxBuffer;

    InitParams();
    iState = 0;

    return EPVT_Success;
}

int H223LowerLayer::Abort()
{
    iState = 0;

    Disconnect();

    //Flush remaining data.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "LL:Abort Complete\n"));

    return 1;
}

TPVStatusCode H223LowerLayer::SetLevel(TPVH223Level muxLevel)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::SetLevel muxLevel=%d", muxLevel));

    if (muxLevel == H223_LEVEL_UNKNOWN || muxLevel > H223_LEVEL2_OH)
    {
        OSCL_LEAVE(PVMFErrNotSupported);
    }

    CreateParcom(muxLevel);

    SetStuffingSize(muxLevel);
    return EPVT_Success;
}

TPVStatusCode H223LowerLayer::SetBitrate(uint32 bitRate)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux[LL]: SetBitrate Request(%d)\n", bitRate));
    iBitrate = bitRate;
    UpdatePduSize();
    return EPVT_Success;
}

TPVStatusCode H223LowerLayer::SetMaxOutgoingPduSize(uint16 Size)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::SetMaxOutgoingPduSize(%d)\n", Size));

    if (Size < H223_MIN_PDU_SIZE)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::SetMaxOutgoingPduSize -  size < min(%d)!\n", H223_MIN_PDU_SIZE));
        Size = H223_MIN_PDU_SIZE;
    }
    uint16 max_pdu_size = H223_MAX_MUX_PDU_SIZE;
    if (iParcom->GetLevel() == H223_LEVEL2)
    {
        max_pdu_size = H223_MAX_MUX_PDU_SIZE_LEVEL2;
    }
    if (Size >= max_pdu_size)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::SetMaxOutgoingPduSize - Size >= max_pdu_size(%d)\n", max_pdu_size));
        return EPVT_Success;
    }

    SetPduSize(Size);
    return EPVT_Success;
}

unsigned H223LowerLayer::SetPduSize(unsigned pdu_size)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "SetPduSize - current(%d), new(%d)!\n", iPduSize, pdu_size));
    if (pdu_size < iPduSize)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "SetPduSize - setting pdu size to (%d)", pdu_size));
        iPduSize = pdu_size;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "SetPduSize - not setting pdu size "));
    }
    return iPduSize;
}

void H223LowerLayer::GetHdrFragment(OsclRefCounterMemFrag& hdr_frag)
{
    iParcom->GetHdrFragment(hdr_frag);
}

PVMFStatus H223LowerLayer::CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& packet, int mt_entry, int pm)
{
    PV_STAT_INCR(iNumPdusTx, 1)
    PV_STAT_INCR(iNumMuxSduBytesTx, packet->getFilledSize() - iParcom->GetHeaderSz())
    PV_STAT_INCR_COND(iNumPmTx, 1, pm)
    return iParcom->CompletePdu(packet, (int8)mt_entry, (uint8)pm);
}

void H223LowerLayer::TimeoutOccurred(int32 timerID, int32 timeoutInfo)
{
    OSCL_UNUSED_ARG(timerID);
    OSCL_UNUSED_ARG(timeoutInfo);
    if (PVMFSuccess != Mux())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::TimeoutOccurred - Memory allocation failed on Mux()\n"));
    }
}

/* H223PduParcomObserver virtuals */
uint32 H223LowerLayer::MuxPduIndicate(uint8* pPdu, uint32 pduSz, int32 fClosing, int32 muxCode)
{
    return iObserver->GetParcomObserver()->MuxPduIndicate(pPdu, pduSz, fClosing, muxCode);
}

void H223LowerLayer::MuxPduErrIndicate(EMuxPduError err)
{
    iObserver->GetParcomObserver()->MuxPduErrIndicate(err);
}

void H223LowerLayer::MuxSetupComplete(PVMFStatus status, TPVH223Level level)
{
    OSCL_UNUSED_ARG(level);
    if (status == PVMFPending)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::MuxSetupComplete Pending level setup for level=%d", level));
        iSkipLevelCheck = true;
        return;
    }
    if (status == PVMFSuccess)
    {
        iLevelSetupComplete = true;
    }
    SignalLevelSetupComplete(status);
}

void H223LowerLayer::Run()
{
}

void H223LowerLayer::DoCancel()
{
    iTimer.Clear();
    Cancel();
}

void H223LowerLayer::SendStuffingMsgHeader(uint8* stuffing, uint16 stuffing_len)
{
    OSCL_UNUSED_ARG(stuffing);
    OSCL_UNUSED_ARG(stuffing_len);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::SendStuffingMsgHeader stuffing(%x), len(%d)\n", stuffing, stuffing_len));
}

PVMFStatus H223LowerLayer::Mux()
{

    if (!IsConnected())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "H223LowerLayer::Mux - Error Port not connected"));
        return PVMFFailure;
    }
    TimeValue timenow;
    OsclSharedPtr<PVMFMediaDataImpl> dispatchPacket = iDispatchPacketAlloc->allocate(30);
    if (!dispatchPacket)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "H223LowerLayer::Mux - Unable to allocate dispatchPacket"));
        return PVMFFailure;
    }

    dispatchPacket->clearMediaFragments();

    if (iSendTimes == 0)
    {
        iStartMuxTime = timenow;
    }
    iLastMuxTime = timenow;
    PV_STAT_SET_TIME(iStartTimeTx, iNumBytesTx)
    uint32 max_cnt = (iParcom->GetLevel() == H223_LEVEL0) ? H223_INITIAL_STUFFING_SEND_CNT_LEVEL0 : H223_INITIAL_STUFFING_SEND_CNT;

    if (iSendTimes < max_cnt)
    {
        DoStuffing(iParcom->GetLevel(),
                   dispatchPacket,
                   iPduSize,
                   0);
        if (dispatchPacket->getFilledSize() == 0)
        {
            return PVMFFailure;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Mux sendtimes %d, sending %d bytes of stuffing", iSendTimes, dispatchPacket->getFilledSize()));
    }
    else
    {
        if (iObserver)
        {
            PVMFStatus status = iObserver->GetOutgoingMuxPdus(iSendPacketList);
            if (status != PVMFSuccess)
            {
                iSendPacketList.clear();
                return status;
            }
        }

        OsclRefCounterMemFrag dispatch_frag = iMemFragmentAlloc->get();
        if (dispatch_frag.getMemFragPtr() == NULL)
        {
            iSendPacketList.clear();
            return PVMFFailure;
        }

        uint8 *start_pos = (uint8*)dispatch_frag.getMemFragPtr();
        uint8 *pos = start_pos;
        uint16 max_size = (uint16)(iSendPduSz + H223_SEND_PDU_SIZE_EXTRA);
        for (unsigned pktnum = 0; pktnum < iSendPacketList.size(); pktnum++)
        {
            for (unsigned frag_num = 0;frag_num < iSendPacketList[pktnum]->getNumFragments();frag_num++)
            {
                OsclRefCounterMemFrag frag;
                iSendPacketList[pktnum]->getMediaFragment(frag_num, frag);
                if (pos - start_pos + frag.getMemFragSize() > max_size)
                    break;
                oscl_memcpy(pos, frag.getMemFragPtr(), frag.getMemFragSize());
                pos += frag.getMemFragSize();
            }
            iSendPacketList[pktnum]->clearMediaFragments();
        }
        iSendPacketList.clear();
        dispatch_frag.getMemFrag().len = (uint16)(pos - start_pos);
        if (dispatch_frag.getMemFrag().len)
            dispatchPacket->appendMediaFragment(dispatch_frag);
    }

    DispatchPacket(dispatchPacket);
    return PVMFSuccess;
}

void H223LowerLayer::MuxException()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::MuxException"));
}

void H223LowerLayer::Parse(uint8* buf, uint16 size)
{
    uint16 used_bytes = 0;
    if (!size)
        return;


    if (!iSkipLevelCheck && !iLevelSetupComplete)
    {
        if (buf[0] != 0x7e && buf[0] != 0xe1)
        {
            oscl_memset(iIdleSyncCheckBuffer, buf[0], size);
            if (oscl_memcmp(iIdleSyncCheckBuffer, buf, size) == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Parse Detected idle sync=%x, dropping %d bytes", buf[0], size));
                return;
            }
        }

        // check for start of data reception
        if (!iDataReceptionStart)
        {
            for (uint16 i = 0; i < size; i++)
            {
                if (buf[i] == 0xE1  || buf[i] == 0xA3 || buf[i] == 0x7E)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Parse Data reception started, byte=%x", buf[i]));
                    iDataReceptionStart = true;
                    iObserver->DataReceptionStart();
                    break;
                }
            }
            if (!iDataReceptionStart)
            {
                return;
            }
        }
        if (CheckLevelWithSync(buf, size, &used_bytes))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux[LL]: Level Setup Complete - %d\n", iParcom->GetLevel()));
            SignalLevelSetupComplete(PVMFSuccess);
        }
    }
    if (iDataReceptionStart)
    {
        iParcom->Parse(buf, size);
    }
}

void H223LowerLayer::DispatchPacket(OsclSharedPtr<PVMFMediaDataImpl>& pack)
{
    unsigned pkt_size = pack->getFilledSize();
    PVMFStatus status = PVMFSuccess;

    PV_STAT_INCR(iNumBytesTx, pkt_size)

    OSCL_ASSERT(pkt_size > 0);

    OsclSharedPtr<PVMFMediaData> aMediaData = PVMFMediaData::createMediaData(pack, iMediaMsgPoolAlloc);
    if (aMediaData.GetRep() == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::DispatchPacket Failed to allocate media data"));
        return;
    }

    if (iLoopbackMode == PV_LOOPBACK_MUX)
    {
        PacketIn(aMediaData);
        iSendTimes++;
    }
    else
    {
        PVMFSharedMediaMsgPtr aMediaMsg;
        convertToPVMFMediaMsg(aMediaMsg, aMediaData);
        aMediaMsg->setSeqNum(iSendTimes++);
        status = QueueOutgoingMsg(aMediaMsg);
        if (status != PVMFSuccess && status != PVMFErrInvalidState)
        {
            // happens when we start sending data too soon
            iObserver->LowerLayerError(OUTGOING, PV2WayDispatchError);
        }
    }

    if ((iSendTimes % 100) == 0)
    {
        PVLOGGER_LOG_USE_ONLY(TimeValue timenow);
        PVLOGGER_LOG_USE_ONLY(TimeValue delta_t = timenow - iStartTimeTx);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux::DispatchPacket - Sent %d bytes in time(%d ms)\n", iNumBytesTx, delta_t.to_msec()));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux::DispatchPacket - Received %d bytes in time(%d ms)\n", iNumBytesRx, delta_t.to_msec()));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux::DispatchPacket - Delta = %d\n", iNumBytesTx - iNumBytesRx));
    }
}

/* Incoming packet from TSAP to be demuxed */
void H223LowerLayer::PacketIn(PVMFSharedMediaDataPtr aMediaData)
{
    PV_STAT_SET_TIME(iStartTimeRx, iNumBytesRx)
    OsclRefCounterMemFrag refCtrMemFrag;
    unsigned cur_size = iDemuxBufferPos - iDemuxBuffer;
    for (uint16 frag_num = 0; frag_num < aMediaData->getNumFragments(); frag_num++)
    {
        aMediaData->getMediaFragment(frag_num, refCtrMemFrag);
        PV_STAT_INCR(iNumBytesRx, (refCtrMemFrag.getMemFragSize()))
        if (refCtrMemFrag.getMemFragSize() > iDemuxBufferSize)
        {
            if (cur_size)
            {
                Parse(iDemuxBuffer, (uint16)cur_size);
                iDemuxBufferPos = iDemuxBuffer;
                cur_size = 0;
            }
            Parse((uint8*)refCtrMemFrag.getMemFragPtr(), (uint16)refCtrMemFrag.getMemFragSize());
            iDemuxBufferSize = 0;
            continue;
        }
        else
        {
            oscl_memcpy(iDemuxBufferPos, refCtrMemFrag.getMemFragPtr(), refCtrMemFrag.getMemFragSize());
            iDemuxBufferPos += refCtrMemFrag.getMemFragSize();
            cur_size += refCtrMemFrag.getMemFragSize();
            iDemuxBufferSize -= refCtrMemFrag.getMemFragSize();
        }
    }
}


void H223LowerLayer::SignalLevelSetupComplete(PVMFStatus status)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::SignalLevelSetupComplete"));
    TimeValue timenow;
    iLevelSetupTime = timenow - iStartTimeTx;
    iObserver->LevelSetupComplete(status, iParcom->GetLevel());
}

void H223LowerLayer::DoStuffing(TPVH223Level level,
                                OsclSharedPtr<PVMFMediaDataImpl>& pkt,
                                uint32 stuffing_sz,
                                uint8 mux_code)
{
    OSCL_UNUSED_ARG(level);
    if (!iParcom.GetRep())
        return;

    OsclRefCounterMemFrag stuffing_frag = iMemFragmentAlloc->get();
    if (stuffing_frag.getMemFragPtr() == NULL)
    {
        return;
    }

    int bytes_to_stuff = stuffing_sz > stuffing_frag.getCapacity() ? stuffing_frag.getCapacity() : stuffing_sz;

    uint32 sz = iParcom->GetStuffing((uint8*)stuffing_frag.getMemFragPtr(), bytes_to_stuff, mux_code);
    if (sz)
    {
        stuffing_frag.getMemFrag().len = sz;
        pkt->appendMediaFragment(stuffing_frag);
        PV_STAT_INCR(iNumStuffingBytesTx, sz)
    }

    return;
}

/***********************************************************************
  function name	       :CheckLevel
  function outline     :
  function discription :CheckLevel( pData, size )
  input data		   :pData Pointer to data to use
  output data		   :size  Size of data
  draw time		       :'96.07.09
*************************************************************************/
#define STUFFING_BUF_SZ 8
#define SYNC_DETECT_TOLERANCE_BITS 2
bool H223LowerLayer::CheckLevelWithSync(uint8* pData, uint16 size, uint16* used_bytes)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "level check with sync, size(%d)", size));
    uint8* pos = pData;

    int sync_detected = false;
    if (CheckLevel(pos, size, used_bytes, &sync_detected))
    {
        return true;
    }
    if (sync_detected)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevelWithSync - Sync detected, but level mismatch"));
        // Reset iSendTimes to force the mux to send out stuffing H223_INITIAL_STUFFING_SEND_CNT times
        iSendTimes = 0;
        return false;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "level check with sync - done"));
    return false;
}

bool H223LowerLayer::CheckLevel(uint8* pData, uint16 size, uint16* used_bytes, int* sync_detected)
{
    int j = 0;
    int level = H223_LEVEL_UNKNOWN;
    uint stuffing_sz =  0;
    uint check_sz = 0;
    uint8 stuffing_buf[STUFFING_BUF_SZ];
    uint8* pos = pData;
    uint32 check_cnt = 0;
    *sync_detected  = false;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel size=%d, current level=%d, iSyncCheckCount=%d", size, iParcom->GetLevel(), iSyncCheckCount));

    if (size < iSyncCheckCount*H223GetMuxStuffingSz(H223_LEVEL_DEFAULT))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel Size inadequate to complete sync check reqd=%d", H223GetMuxStuffingSz(H223_LEVEL_DEFAULT)));
        return false;
    }
    level = (iParcom->GetLevel() == H223_LEVEL_UNKNOWN) ? H223_LEVEL_DEFAULT : iParcom->GetLevel();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel starting level=%d", level));

    for (; level >= H223_LEVEL0; level--)
    {
        stuffing_sz =  H223GetMuxStuffingSz((TPVH223Level)level);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel checking level=%d, stuffing sz=%d", level, stuffing_sz));
        H223MuxStuffing((TPVH223Level)level, stuffing_buf, STUFFING_BUF_SZ);
        uint tolerance = (level == H223_LEVEL0) ? 0 : SYNC_DETECT_TOLERANCE_BITS;
        if (level == H223_LEVEL1_DF)
        {
            continue;
        }
        check_sz = stuffing_sz * iSyncCheckCount;
        for (j = 0; j < (int)(size - check_sz + 1); j++)
        {
            pos = pData + j;
            check_cnt = 0;
            for (unsigned checknum = 0; checknum < iSyncCheckCount; checknum++)
            {
                if (H223CheckSync((TPVH223Level)level, pos, stuffing_sz, tolerance))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel Found a match, check_cnt=%d", check_cnt));
                    /* found a match */
                    pos += stuffing_sz;
                    *used_bytes = (uint16)(pos - pData);
                    check_cnt++;
                }
                else
                {
                    check_cnt = 0;
                    break;
                }
            }
            if (check_cnt == iSyncCheckCount)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel Sync Detected for level=%d", level));
                *sync_detected = true;
                if (iParcom->GetLevel() == level)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel Level setup complete"));
                    iLevelSetupComplete = true;
                    return true;
                }
                if (!iSyncDetected)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::CheckLevel sync detected for level for first time(%d)", level));
                    iSyncDetected = true;
                    int32 err;
                    OSCL_TRY(err, CreateParcom((TPVH223Level)level))
                    OSCL_FIRST_CATCH_ANY(err, SignalLevelSetupComplete(PVMFErrNoMemory));
                    iObserver->LevelSetupComplete(PVMFPending, (TPVH223Level)level);
                    return false;
                }
            }
        }
    }

    *used_bytes = (uint16)(pos - pData);
    return false;
}

void H223LowerLayer::ResetStats()
{
    // Outgoing
    iSendTimes = 0;
    iNumBytesTx = 0;
    iNumMuxSduBytesTx = 0;
    iNumStuffingBytesTx = 0;
    iNumPdusTx = 0;
    iNumPmTx = 0;

    // Incoming
    iNumBytesRx = 0;

    if (iParcom)
    {
        iParcom->ResetStats();
    }
}

void H223LowerLayer::LogStats(TPVDirection dir)
{
    if (dir & OUTGOING)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux-pdu(O) Statistics:\n"));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes tx - %d\n", iNumBytesTx));

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num mux-sdu bytes tx - %d\n", iNumMuxSduBytesTx));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num stuffing bytes tx during level setup - %d\n", iNumStuffingBytesTx));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num pdus tx - %d\n", iNumPdusTx));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num pm tx - %d\n", iNumPmTx));
        if (iParcom)
        {
            iParcom->LogStats(OUTGOING);
        }
    }

    if (dir & INCOMING)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux-pdu(I) Statistics:\n"));
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Num bytes rx - %d\n", iNumBytesRx));
        if (iParcom)
        {
            iParcom->LogStats(INCOMING);
        }
    }

}

unsigned H223LowerLayer::SetStuffingSize(TPVH223Level level)
{
    iStuffingSize = iStuffingSz[level];
    return iStuffingSize;
}


/* PVMFPortInterface virtuals */
// All requests are synchronous


/* Incoming data from the network interface */
PVMFStatus H223LowerLayer::PutData(PVMFSharedMediaMsgPtr aMsg)
{
    /* In mux loopback mode, discard data from connected port */
    if (iLoopbackMode == PV_LOOPBACK_MUX)
        return PVMFSuccess;
    /* Dont processs data in invalid state */
    if (iState <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::PutData Error - Invalid state(%d)", iState));
        return PVMFErrInvalidState;
    }

    PVMFStatus pvmfStatus = PVMFSuccess;
    /* Convert to media data */
    PVMFSharedMediaDataPtr mediaData;
    convertToPVMFMediaData(mediaData, aMsg);

    PacketIn(mediaData);

    return pvmfStatus;
}

int32 H223LowerLayer::GetTimestamp()
{
    return (iLastMuxTime -iStartMuxTime).to_msec();
}

void H223LowerLayer::UpdatePduSize()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::UpdatePduSize"));
    iSendPduSz = (uint16)(iMinTimerResolution * iBitrate / (1000 * 8));
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer: Send pdu size(%d), pdu size(%d)\n", iSendPduSz, iPduSize));
}

unsigned H223LowerLayer::GetSendSize()
{
    return iSendPduSz;
}

PVMFStatus H223LowerLayer::Connect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Connect"));
    if (IsConnected())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::Connect - Already Connected !!"));
        return PVMFErrAlreadyExists;
    }

    PvmfPortBaseImpl::Connect(aPort);
    return PVMFSuccess;
}


void H223LowerLayer::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVMFStatus aStatus;
    if (aActivity.iType == PVMF_PORT_ACTIVITY_INCOMING_MSG)
    {
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223LowerLayer::HandlePortActivity Failed to DeQueue incoming message: %d", aStatus));
                break;
            }
        }
    }
    else if (aActivity.iType == PVMF_PORT_ACTIVITY_OUTGOING_MSG)
    {
        while (OutgoingMsgQueueSize())
        {
            aStatus = Send();
            if (aStatus != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "H223LowerLayer::HandlePortActivity Failed to Send outgoing message: %d", aStatus));
                break;
            }
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223LowerLayer::HandlePortActivity Unhandled port activity: %d", aActivity.iType));
    }
}
void H223LowerLayer::SetLoopbackMode(TPVLoopbackMode aLoopbackMode)
{
    iLoopbackMode = aLoopbackMode;
}


void H223LowerLayer::CreateParcom(TPVH223Level level)
{
    iParcom.Unbind();


    H223PduParcom* parcom = NULL;

    switch (level)
    {
        case H223_LEVEL0:
            parcom = OSCL_NEW(Level0PduParcom, ());
            break;
        case H223_LEVEL1:
        case H223_LEVEL1_DF:
            parcom = OSCL_NEW(Level1PduParcom, ());
            if (level == H223_LEVEL1)
                break;
            ((Level1PduParcom*)parcom)->UseDf(true);
            break;
        case H223_LEVEL2:
        case H223_LEVEL2_OH:
            parcom = OSCL_NEW(Level2PduParcom, ());

            if (level == H223_LEVEL2)
                break;
            ((Level2PduParcom*)parcom)->UseOh(true);
            break;
        default:
            OSCL_LEAVE(PVMFErrArgument);
            ;
    }
    PduParcomRefCounter *refCounter = OSCL_NEW(PduParcomRefCounter, (parcom));
    H223PduParcomSharedPtr sharedPtr(parcom, refCounter);
    iParcom = sharedPtr;

    uint16 pdu_rate = (uint16)((1000.0 / (float)H223_MIN_TIMER_RESOLUTION + .5) + 1);
    pdu_rate <<= 1; /* Double it since it will be used for video and control data also */
    iParcom->Construct(pdu_rate);
    iParcom->SetObserver(this);
    SetStuffingSize(level);
}
