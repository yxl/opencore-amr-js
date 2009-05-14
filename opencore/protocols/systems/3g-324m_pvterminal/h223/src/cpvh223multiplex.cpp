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
#include  <stdlib.h>
#include <string.h>
#include "cpvh223multiplex.h"
#include "media_packet.h"
#include "media_fragment.h"
#include "lowerlayer.h"
#include "logicalchannel.h"
#include "adaptationlayer.h"
#include "muxtbl.h"
#include "h223.h"

/* Default sizes for outgoing SDUs */
#define DEF_AL1_SDU_SIZE 1024
#define DEF_AL2_SDU_SIZE 256
#define DEF_AL3_SDU_SIZE 256

/* Default sizes for incoming SDUs */
#define DEF_AL1_SDU_SIZE_R 1024
#define DEF_AL2_SDU_SIZE_R 256
#define DEF_AL3_SDU_SIZE_R 256

#define MIN_SEGMENTABLE_SDU_SIZE 256
#define MAX_NONSEGMENTABLE_SDU_SIZE 128  /* Implementers guide notes that not more than 3 frames of audio per pdu*/
#define DEF_CHANNEL_BITRATE 64000
#define DEF_OUTGOING_MEDIA_QUEUE_SIZE_MS 2000
#define DEF_MIN_SAMPLE_INTERVAL 20

#define GET_SDU_DATA_LIST(segmentable)\
 segmentable?iSegmentableSduDataList:iNonSegmentableSduDataList

#define PV_H223_DEF_SAMPLE_INTERVAL 20
#define H223_LCN_DATA_DETECT_THRESHOLD 1
#define PV_H223_DEF_MULTPLEXING_DELAY_INTERVAL 0
#define PV_H223_INTERLEAVING_FLAG_SEND_FREQUENCY 10

#define H223_LOWERLAYER_PORT_TAG PV_MULTIPLEXED

/** Change this to force the queue size for number of outgoing media data.  Otherwise, it is calculated
    based on bitrate, sample rate and sdu size */
//#define FORCE_NUM_OUTGOING_MEDIA_DATA 150

CPVH223Multiplex::CPVH223Multiplex(TPVLoopbackMode aLoopbackMode)
        : iLowerLayer(NULL),
        iMuxTblMgr(NULL),
        iLoopbackMode(aLoopbackMode),
        iDummyFragment(NULL),
        iMediaDataImplMemAlloc(NULL),
        iMuxPduPacketAlloc(NULL),
        iClock(NULL),
        iInterleavingMultiplexFlagsSize(0),
        iInterleavingMultiplexFlags(NULL)
{
    iMuxTblMgr = OSCL_NEW(MuxTableMgr, ());

    iState = false;
    iLcnsWithData = 0;
    iStuffOnlyFirstSend = false;
    for (int n = 0; n < MAX_LCNS; n++)
    {
        iALIndex[n] = 0;
    }

    Init();

    ResetStats();
}

CPVH223Multiplex::~CPVH223Multiplex()
{
    OSCL_DELETE(iMuxTblMgr);
    iMuxTblMgr = NULL;
}

void CPVH223Multiplex::Init()
{
    iAlSduSize[0] = iMaxAlSduSize[0] = iMaxAlSduSizeR[0] = 0;
    iAlSduSizeR[0] = 0;
    iAlSduSize[1] = iMaxAlSduSize[1] = DEF_AL1_SDU_SIZE;
    iAlSduSize[2] = iMaxAlSduSize[2] = DEF_AL2_SDU_SIZE;
    iAlSduSize[3] = iMaxAlSduSize[3] = DEF_AL3_SDU_SIZE;
    iAlSduSizeR[1] = iMaxAlSduSizeR[1] = DEF_AL1_SDU_SIZE_R;
    iAlSduSizeR[2] = iMaxAlSduSizeR[2] = DEF_AL2_SDU_SIZE_R;
    iAlSduSizeR[3] = iMaxAlSduSizeR[3] = DEF_AL3_SDU_SIZE_R;
    iLcnEnd = NULL;
    iState = false;
    iLcnsWithData = 0;
    ClearSduDataLists();
    for (int n = 0; n < MAX_LCNS; n++)
    {
        iALIndex[n] = 0;
    }
    iALIndex[0] = AdaptationLayerAlloc::GetAlIndex(E_EP_LOW);
    iBitrate = DEFAULT_BITRATE;
    iMinSampleInterval = PV_H223_DEF_SAMPLE_INTERVAL;
    iNumMuxIntervalsPerTimerInterval = 0;
    iNumBytesPerMinSampleInterval = 0;
    iMultiplexingDelayMs = PV_H223_DEF_MULTPLEXING_DELAY_INTERVAL;
    TimeValue time_now;
    iRand.Seed(time_now.to_msec());
    iInLogicalChannelBufferingMs = 0;
    iOutLogicalChannelBufferingMs = 0;
    iFmStuffingRepeatInterval = 0;
    iFmStuffingMode = 0;
    iLevelSetupComplete = false;
    iEnableStuffing = true;
    iInterleavingMultiplexFlagsSize = 0;
}

int CPVH223Multiplex::GetMinTimerRes()
{
    return iLowerLayer->GetMinTimerRes();
}

void CPVH223Multiplex::SetLoopbackMode(TPVLoopbackMode aLoopbackMode)
{
    iLoopbackMode = aLoopbackMode;
    iLowerLayer->SetLoopbackMode(iLoopbackMode);
}

TPVStatusCode CPVH223Multiplex::SetTimerRes(uint32 timer_res)
{
    return iLowerLayer->SetTimerRes(timer_res);
}

TPVStatusCode CPVH223Multiplex::Open()
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::Open"));

    iLcnEnd = NULL;

    for (unsigned out_lcn = 0; out_lcn < iOutgoingChannels.size(); out_lcn++)
    {
        iOutgoingChannels[out_lcn].Unbind();
    }
    for (unsigned in_lcn = 0; in_lcn < iIncomingChannels.size(); in_lcn++)
    {
        iIncomingChannels[in_lcn].Unbind();
    }

    iLowerLayer = OSCL_NEW(H223LowerLayer, (H223_LOWERLAYER_PORT_TAG, iLoopbackMode));
    iLowerLayer->SetObserver(this);
    iLowerLayer->Open();

    UpdateMuxInterval(DEF_MIN_SAMPLE_INTERVAL);

    uint16 pdu_rate = (uint16)((2000.0 / (float)iMinSampleInterval + .5) + 1); /* For audio and video */
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::Open pdu_rate=%d", pdu_rate));
    iMediaDataImplMemAlloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (pdu_rate));
    if (iMediaDataImplMemAlloc == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::Open() Failed to allocate iMediaDataImplMemAlloc"));
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    iMediaDataImplMemAlloc->enablenullpointerreturn();

    iMuxPduPacketAlloc = OSCL_NEW(PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>, (pdu_rate, MAX_FRAGMENTS_PER_H223_PDU_PACKET, iMediaDataImplMemAlloc));
    if (iMuxPduPacketAlloc == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::Open() Failed to allocate iMuxPduPacketAlloc"));
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    iMuxPduPacketAlloc->create();

    iLcnsWithData = 0;
    return EPVT_Success;
}

int CPVH223Multiplex::Start(H223PduParcomSharedPtr parcom)
{
    int ret = 1;
    iState = true;
    iPduNum = 0;
    iLevelSetupComplete = false;
    iEnableStuffing = true;
    OSCL_ASSERT(iOutgoingChannels[0].GetRep());
    OSCL_ASSERT(iIncomingChannels[0].GetRep());
    ret = iLowerLayer->Start(parcom);
    //SetStuffingMsgHeader();
    return ret;
}

TPVStatusCode CPVH223Multiplex::Close()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::Close"));
    if (iInterleavingMultiplexFlags && iInterleavingPacket)
    {
        iInterleavingPacket->clearMediaFragments();
        iInterleavingPacket.Unbind();
        OSCL_DEFAULT_FREE(iInterleavingMultiplexFlags);
        iInterleavingMultiplexFlags = NULL;
    }

    for (unsigned out_lcn = 0;out_lcn < iOutgoingChannels.size(); out_lcn++)
    {
        CloseChannel(OUTGOING, iOutgoingChannels[out_lcn]->GetLogicalChannelNumber());
    }
    for (unsigned in_lcn = 0;in_lcn < iIncomingChannels.size(); in_lcn++)
    {
        CloseChannel(INCOMING, iIncomingChannels[in_lcn]->GetLogicalChannelNumber());
    }
    if (iLowerLayer)
    {
        iLowerLayer->Disconnect();
        iLowerLayer->Close();
        OSCL_DELETE(iLowerLayer);
        iLowerLayer = NULL;
    }

    iMuxPduPacketAlloc->removeRef();
    OSCL_DELETE(iMediaDataImplMemAlloc);
    return EPVT_Success;
}

int CPVH223Multiplex::Stop()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::Stop"));

    iState = false;
    iLcnEnd = NULL;

    iOutgoingChannels[0]->Flush();
    iIncomingChannels[0]->Flush();

    iNonSegmentableSduDataList.clear();
    iSegmentableSduDataList.clear();
    iControlSduDataList.clear();

    iLowerLayer->Stop();

    return 1;
}


void CPVH223Multiplex::DataReceptionStart()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::DataReceptionStart()"));
    iObserver->DataReceptionStart();
}
void CPVH223Multiplex::LevelSetupComplete(PVMFStatus status, TPVH223Level level)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::LevelSetupComplete - status=%d, level=%d", status, level));
    if (iInterleavingMultiplexFlags && iInterleavingPacket)
    {
        iInterleavingPacket->clearMediaFragments();
        iInterleavingPacket.Unbind();
        OSCL_DEFAULT_FREE(iInterleavingMultiplexFlags);
        iInterleavingMultiplexFlags = NULL;
        iInterleavingMultiplexFlagsSize = 0;
    }
    if (status == PVMFPending)
    {
        EnableStuffing(true);
    }
    else
    {
        iLevelSetupComplete = true;
    }
    //SetStuffingMsgHeader();
    iObserver->MuxSetupComplete(status, level);
}

uint32 CPVH223Multiplex::MuxPduIndicate(uint8* pPdu, uint32 sz, int32 fClosing, int32 muxCode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::MuxPduIndicate mux code=%d, closing=%d, size=%d", muxCode, fClosing, sz));
    int ret = MuxToALDispatch(pPdu, sz, fClosing, muxCode);
    return (uint16)ret;
}

void CPVH223Multiplex::MuxPduErrIndicate(EMuxPduError err)
{
    OSCL_UNUSED_ARG(err);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::MuxPduErrIndicate - err(%d)", err));
    iObserver->MuxErrorOccurred(INCOMING, PV_MUX_COMPONENT_MUX, PVMFErrCorrupt);
}

void CPVH223Multiplex::MuxSetupComplete(PVMFStatus status, TPVH223Level level)
{
    OSCL_UNUSED_ARG(status);
    OSCL_UNUSED_ARG(level);
}

unsigned CPVH223Multiplex::DispatchPduPacket(MuxPduPacketList& packets,
        MuxSduData& mux_sdu_data,
        PS_MultiplexEntryDescriptor mux_entry,
        uint32 max_size)
{
    OSCL_ASSERT(mux_entry->size_of_elementList == 1);
    OsclSharedPtr<PVMFMediaDataImpl> packet = InitPduPacket();

    if (!packet.GetRep())
    {
        return 0;
    }

    int pm = false;
    MakePduData(packet,
                max_size - iLowerLayer->GetHeaderSize(),
                mux_entry->elementList,
                mux_sdu_data,
                &pm);
    if (CompletePduPacket(packet, mux_entry->multiplexTableEntryNumber, pm) == PVMFSuccess)
    {
        packets.push_back(packet);
        return packet->getFilledSize();
    }
    return 0;
}

/* Called by lower layer to get the next mux pdu to dispatch. */
PVMFStatus CPVH223Multiplex::GetOutgoingMuxPdus(MuxPduPacketList& packets)
{
    unsigned cur_pdus_size = 0;
    uint16 stuffing_size = (uint16)iLowerLayer->GetStuffingSize();
    OsclSharedPtr<PVMFMediaDataImpl> pkt;

    for (unsigned n = 0;n < iNumMuxIntervalsPerTimerInterval; n++)
    {
        // fill the sdu data lists with lcns that contain data
        UpdateSduDataLists();

        int32 max_pdus_size = iNumBytesPerMinSampleInterval;

        if (iInterleavingPacket &&
                iInterleavingPacket.GetRep() &&
                iInterleavingPacket->getFilledSize() &&
                iInterleavingMultiplexFlags &&
                ((iPduNum % PV_H223_INTERLEAVING_FLAG_SEND_FREQUENCY) == 0))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::GetOutgoingMuxPdus Adding interleaving packet."));
            iInterleavingPacket->clearMediaFragments();
            OsclRefCounterMemFrag frag;
            frag.getMemFrag().len = iInterleavingMultiplexFlagsSize;
            frag.getMemFrag().ptr = iInterleavingMultiplexFlags;
            iInterleavingPacket->appendMediaFragment(frag);
            packets.push_back(iInterleavingPacket);
            max_pdus_size -= iInterleavingPacket->getFilledSize();
        }

        if (iLowerLayer->GetLevel() == H223_LEVEL0)
        {
            if (n == 0)
            {
                pkt = iMuxPduPacketAlloc->allocate(1);
                if (!pkt)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                    (0, "CPVH223Multiplex::GetOutgoingMuxPdus Unable to allocate packet."));
                    return PVMFErrNoMemory;
                }
                iLowerLayer->DoStuffing(iLowerLayer->GetLevel(), pkt, 5);
                packets.push_back(pkt);
                max_pdus_size -= pkt->getFilledSize();
            }
        }
        iPduNum++;

        /* Mux non segmentable data */
        cur_pdus_size = MuxLcnData(GET_SDU_DATA_LIST(false),
                                   packets,
                                   (uint16)OSCL_MIN(max_pdus_size, (int)iLowerLayer->GetPduSize()));
        max_pdus_size -= cur_pdus_size;
        if (max_pdus_size < (int)iLowerLayer->GetHeaderSize())
        {
            return PVMFSuccess;
        }

        /* Mux control data untill current pdu is filled up */
        do
        {
            cur_pdus_size = MuxLcnData(iControlSduDataList, packets, (uint16)iLowerLayer->GetPduSize());
            max_pdus_size -= cur_pdus_size;
        }
        while (max_pdus_size > (int)iLowerLayer->GetHeaderSize() &&
                cur_pdus_size);

        if (max_pdus_size < (int)iLowerLayer->GetHeaderSize())
        {
            return PVMFSuccess;
        }

        /* Mux segmentable data untill current pdu is filled up */
        do
        {
            cur_pdus_size = MuxLcnData(GET_SDU_DATA_LIST(true),
                                       packets,
                                       (uint16)OSCL_MIN(max_pdus_size, (int)iLowerLayer->GetPduSize()));
            max_pdus_size -= cur_pdus_size;
        }
        while (max_pdus_size > (int)iLowerLayer->GetHeaderSize() &&
                cur_pdus_size);

        /* Fill stuffing if space remains in pdu.*/
        if (iEnableStuffing)
        {
            if (!iStuffOnlyFirstSend || (iPduNum == 1))
            {
                stuffing_size = (uint16)((max_pdus_size > stuffing_size) ? max_pdus_size : stuffing_size);
            }
        }
        pkt = iMuxPduPacketAlloc->allocate(1);
        if (!pkt)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "CPVH223Multiplex::GetOutgoingMuxPdus Unable to allocate packet."));
            return PVMFErrNoMemory;
        }
        iLowerLayer->DoStuffing(iLowerLayer->GetLevel(), pkt, stuffing_size);

        if (pkt->getFilledSize())
        {
            packets.push_back(pkt);
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::GetOutgoingMuxPdus WARNING Failed to send single stuffing"));
            return PVMFErrNoMemory;
        }
    }
    return PVMFSuccess;
}



TPVStatusCode CPVH223Multiplex::SetMaxOutgoingPduSize(uint16 Size)
{
    return iLowerLayer->SetMaxOutgoingPduSize(Size);
}

TPVStatusCode CPVH223Multiplex::SetSduSize(TPVDirection direction, uint16 size, ErrorProtectionLevel_t epl)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux: Set sdu size - direction(%d), size(%d), epl(%d)\n", direction, size, epl));
    if (direction == OUTGOING)
    {
        if (iAlSduSize[epl] > size)
        {
            iAlSduSize[epl] = size;
        }
    }
    else
    {
        if (iAlSduSizeR[epl] > size)
        {
            iAlSduSizeR[epl] = size;
        }
    }

    return EPVT_Success;
}


uint16 CPVH223Multiplex::GetSduSize(TPVDirection direction, ErrorProtectionLevel_t epl)
{
    if (direction == OUTGOING)
    {
        return (uint16)iAlSduSize[epl];
    }
    return (uint16)iAlSduSizeR[epl];
}

uint16 CPVH223Multiplex::GetMaxSduSize(TPVDirection direction, ErrorProtectionLevel_t epl)
{
    if (direction == OUTGOING)
    {
        return (uint16)iMaxAlSduSize[epl];
    }
    return (uint16)iMaxAlSduSizeR[epl];
}


TPVStatusCode CPVH223Multiplex::SetMuxLevel(TPVH223Level muxLevel)
{
    TPVStatusCode ret = EPVT_Success;
//	iMutex->Lock();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux: Set Level request (%d)\n", muxLevel));
    ret = iLowerLayer->SetLevel(muxLevel);
//	iMutex->Unlock();
    return ret;
}


TPVH223Level CPVH223Multiplex::GetMuxLevel()
{
    return iLowerLayer->GetLevel();
}

PVMFPortInterface* CPVH223Multiplex::GetLowerLayer()
{
    return iLowerLayer;
}

TPVStatusCode CPVH223Multiplex::SetIncomingMuxTableEntries(CPVMultiplexDescriptor* descriptor)
{
//	iMutex->Lock();
    iMuxTblMgr->SetIncomingDescriptors(descriptor->GetDescriptor());
//	iMutex->Unlock();

    return EPVT_Success;
}

void CPVH223Multiplex::SetIncomingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors, bool replace)
{
    iMuxTblMgr->SetIncomingMuxDescriptors(descriptors, replace);
}

void CPVH223Multiplex::SetOutgoingMuxDescriptors(CPVMultiplexEntryDescriptorVector& descriptors)
{
    iMuxTblMgr->SetOutgoingMuxDescriptors(descriptors);
}

void CPVH223Multiplex::RemoveOutgoingMuxDescriptor(uint8 muxTblNum)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::RemoveOutgoingMuxDescriptor muxTblNum(%d)", muxTblNum));
    iMuxTblMgr->RemoveOutgoingMuxDescriptor(muxTblNum);
}

void CPVH223Multiplex::RemoveIncomingMuxDescriptor(uint8 muxTblNum)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::RemoveIncomingMuxDescriptor muxTblNum(%d)", muxTblNum));
    iMuxTblMgr->RemoveIncomingMuxDescriptor(muxTblNum);
}

PS_MultiplexEntryDescriptor CPVH223Multiplex::GetIncomingMuxDescriptor(uint16 muxTblNum)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::GetIncomingMuxDescriptor muxTblNum(%d)", muxTblNum));
    return iMuxTblMgr->GetIncomingDescriptor((uint8)muxTblNum);
}

PVMFStatus CPVH223Multiplex::GetOutgoingChannel(TPVChannelId id, H223OutgoingChannelPtr& channel)
{
    if (id == CHANNEL_ID_UNKNOWN)
    {
        return PVMFErrArgument;
    }
    for (TPVChannelId index = 0; index < iOutgoingChannels.size(); index++)
    {
        if (iOutgoingChannels[index]->GetLogicalChannelNumber() == id)
        {
            channel = iOutgoingChannels[index];
            return PVMFSuccess;
        }
    }
    return PVMFErrArgument;
}

PVMFStatus CPVH223Multiplex::GetIncomingChannel(TPVChannelId id, H223IncomingChannelPtr& channel)
{
    if (id == CHANNEL_ID_UNKNOWN)
    {
        return PVMFErrArgument;
    }
    for (TPVChannelId index = 0; index < iIncomingChannels.size(); index++)
    {
        if (iIncomingChannels[index]->GetLogicalChannelNumber() == id)
        {
            channel = iIncomingChannels[index];
            return PVMFSuccess;
        }
    }
    return PVMFErrArgument;
}

TPVStatusCode CPVH223Multiplex::GetAdaptationLayer(OsclSharedPtr<AdaptationLayer> &al,
        TPVDirection aDirection,
        PS_H223LogicalChannelParameters aH223lcnParams,
        int32 aSdu_size,
        uint32 aNum_media_data)
{
    al = AdaptationLayerAlloc::GetAdaptationLayer(aDirection,
            aH223lcnParams, (uint16)aSdu_size, aNum_media_data);
    if (!al)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::GetAdaptationLayer - Memory allocation failure on StartAlPdu\n"));
        return EPVT_Failed;
    }
    return EPVT_Success;
}

TPVStatusCode CPVH223Multiplex::OpenChannel(TPVDirection direction,
        TPVChannelId channel_id,
        H223ChannelParam* h223params)
{
    TPVStatusCode ret = EPVT_Success;
    OSCL_ASSERT(h223params);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPVH223Multiplex::OpenChannel direction(%d), id(%d)\n", direction, channel_id));
    PS_H223LogicalChannelParameters h223lcnParams = h223params->GetLcnParams();
    H223LogicalChannel* channel = NULL;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPVH223Multiplex::OpenChannel -  AL index(%d)", h223lcnParams->adaptationLayerType.index));

    /* Set the port property */
    // TODO - Gkl
    /*PVMFPortProperty aProps;
    aProps.iFormatType = PVCodecTypeToPVMFFormatType(GetCodecType(h223params->GetDataType()));
    aProps.iPortType = PORT_TYPE_FOR_DIRECTION(direction);*/
    int* sdu_sizes = (direction == OUTGOING) ? iAlSduSize : iAlSduSizeR;
    int32 sdu_size = 0;
    switch (h223lcnParams->adaptationLayerType.index)
    {
        case 1:
        case 2:
            sdu_size = sdu_sizes[1];
            break;
        case 3:
        case 4:
            sdu_size = sdu_sizes[2];
            break;
        case 5:
            sdu_size = sdu_sizes[3];
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::OpenChannel ERROR - Invalid AL Index.\n"));
            return EPVT_Failed;
    }
    /* Overwrite audio sdu size, otherwise audio memory allocation can become prohibitive*/
    if (!h223lcnParams->segmentableFlag)
    {
        sdu_size = MAX_NONSEGMENTABLE_SDU_SIZE;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::OpenChannel -  SDU Size=%d", sdu_size));

    int bitrate = (h223params->GetBitrate() > 0) ? h223params->GetBitrate() : DEF_CHANNEL_BITRATE;
    uint16 sample_interval = (uint16)h223params->GetSampleInterval();
    uint32 min_sdu_size = (direction == INCOMING && h223lcnParams->segmentableFlag) ? sdu_size = MIN_SEGMENTABLE_SDU_SIZE : sdu_size;
    uint32 num_media_data = (uint32)((double)(bitrate * DEF_OUTGOING_MEDIA_QUEUE_SIZE_MS) / double(1000.0 * 8.0 * min_sdu_size / 2.0) + .99);
    if (h223lcnParams->segmentableFlag && sample_interval)
    {
        num_media_data += (uint32)((double)DEF_OUTGOING_MEDIA_QUEUE_SIZE_MS / (double)sample_interval + 1);
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::OpenChannel bitrate=%d, sample_interval=%d, num_media_data=%d", bitrate, sample_interval, num_media_data));

#if defined(FORCE_NUM_OUTGOING_MEDIA_DATA)
    if (direction == OUTGOING)
    {
        num_media_data = FORCE_NUM_OUTGOING_MEDIA_DATA;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::OpenChannel Forcing outgoing num_media_data=%d", num_media_data));
    }
#endif

    OsclSharedPtr<AdaptationLayer> al;
    if (EPVT_Success != GetAdaptationLayer(al, direction,
                                           h223lcnParams, (uint16)sdu_size, num_media_data))
    {
        return EPVT_Failed;
    }

    if (direction == OUTGOING)
    {
        void* ptr = NULL;
        if (channel_id)
        {
            ptr = OSCL_DEFAULT_MALLOC(sizeof(H223OutgoingChannel));
            channel = new(ptr) H223OutgoingChannel((uint16)channel_id,
                                                   h223lcnParams->segmentableFlag, al, h223params->GetDataType(), this,
                                                   bitrate, sample_interval, num_media_data);
        }
        else
        {
            ptr = OSCL_DEFAULT_MALLOC(sizeof(H223OutgoingControlChannel));
            channel =  new(ptr) H223OutgoingControlChannel(al,
                    h223params->GetDataType(), this, bitrate, sample_interval,
                    num_media_data);
        }

        OsclRefCounterSA<Oscl_TAlloc<H223OutgoingChannel, BasicAlloc> > *channelRefCounter =
            new OsclRefCounterSA<Oscl_TAlloc<H223OutgoingChannel, BasicAlloc> >(ptr);
        OsclSharedPtr<H223OutgoingChannel> channel_ptr((H223OutgoingChannel*)channel,
                channelRefCounter);
        iOutgoingChannels.push_back(channel_ptr);
        UpdateMuxInterval();
        if (channel_id)
            channel_ptr->BufferMedia(iMultiplexingDelayMs);
    }
    else
    {
        void* ptr = OSCL_DEFAULT_MALLOC(sizeof(H223IncomingChannel));
        channel = new(ptr)H223IncomingChannel(channel_id,
                                              h223lcnParams->segmentableFlag, al,
                                              h223params->GetDataType(), this,
                                              bitrate, sample_interval, num_media_data);
        OsclRefCounterSA<Oscl_TAlloc<H223IncomingChannel, BasicAlloc> > *channelRefCounter =
            new OsclRefCounterSA<Oscl_TAlloc<H223IncomingChannel, BasicAlloc> >(ptr);
        OsclSharedPtr<H223IncomingChannel> channel_ptr((H223IncomingChannel*)channel, channelRefCounter);
        iIncomingChannels.push_back(channel_ptr);
    }
    channel->Init();
    // TODO - Gkl
    channel->SetClock(iClock);
    //channel->Configure(aProps);

    uint8* fsi = NULL;
    unsigned fsi_len = 0;
    fsi_len = h223params->GetFormatSpecificInfo(fsi);
    if (fsi_len)
    {
        channel->SetFormatSpecificInfo(fsi, (uint16)fsi_len);
    }
    return ret;
}


TPVStatusCode
CPVH223Multiplex::CloseChannel(TPVDirection direction, TPVChannelId channel_id)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux: Close channel direction(%d), id(%d)\n", direction, channel_id));
    OsclSharedPtr<H223OutgoingChannel> outgoing_channel;
    bool found_channel = false;

    if (direction == OUTGOING)
    {
        Oscl_Vector<OsclSharedPtr<H223OutgoingChannel>, OsclMemAllocator>::iterator iter = iOutgoingChannels.begin();
        while (iter != iOutgoingChannels.end())
        {
            outgoing_channel = *iter;
            if (outgoing_channel->GetLogicalChannelNumber() == channel_id)
            {
                iOutgoingChannels.erase(iter);
                found_channel = true;
                break;
            }
            iter++;
        }
        if (!found_channel)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux: Close channel - Failed to lookup channel\n"));
            return EPVT_Failed;
        }

        /* Release any SDU that the channel has passed to mux */
        MuxSduData* sdu_data_ptr = FindMuxSduData(channel_id, outgoing_channel->IsSegmentable());
        if (sdu_data_ptr)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::CloseChannel - Releasing pending MUX SDU\n"));
            outgoing_channel->ReleasePacket(sdu_data_ptr->sdu);
            RemoveMuxSduData(channel_id, outgoing_channel->IsSegmentable());
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::CloseChannel - Successful\n"));
        return EPVT_Success;
    }
    if (iLcnEnd && (iLcnEnd->GetLogicalChannelNumber() == channel_id))
    {
        iLcnEnd = NULL;
    }
    OsclSharedPtr<H223IncomingChannel> incoming_channel;
    Oscl_Vector<OsclSharedPtr<H223IncomingChannel>, OsclMemAllocator>::iterator iter = iIncomingChannels.begin();

    while (iter != iIncomingChannels.end())
    {
        incoming_channel = *iter;
        if (incoming_channel->GetLogicalChannelNumber() == channel_id)
        {
            iIncomingChannels.erase(iter);
            found_channel = true;
            break;
        }
        iter++;
    }
    if (!found_channel)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Mux: Close channel - Failed to lookup channel\n"));
        return EPVT_Failed;
    }
    UpdateMuxInterval();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::CloseChannel - Successful\n"));
    return EPVT_Success;
}

void CPVH223Multiplex::MakePduData(
    OsclSharedPtr<PVMFMediaDataImpl>& outgoingPkt,
    int32 max_size,
    PS_MultiplexElement pElement,
    MuxSduData& mux_sdu_data,
    int* pm)
{
    *pm = false;

    PVMFSharedMediaDataPtr& sdu_pkt = mux_sdu_data.sdu;
    int bytesToMux = mux_sdu_data.size;
    //OSCL_ASSERT(bytesToMux);

    OsclRefCounterMemFrag frag;
    uint8* pSduFrag = NULL;
    int bytesInCurFragment = 0;

    if (pElement->repeatCount.index == 0)   /* finite */
    {
        bytesToMux = OSCL_MIN(pElement->repeatCount.finite, bytesToMux);
    }

    /* dont exceed avail space on PDU */
    bytesToMux = OSCL_MIN(max_size, bytesToMux);
    if (bytesToMux < 0)
    {
        OSCL_ASSERT(0);
    }
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"MakePduData - lcn(%d), bytesToMux(%d)", pElement->muxType.logicalChannelNumber, bytesToMux));
    while (bytesToMux > 0)
    {
        int n = 0;
        sdu_pkt->getMediaFragment(mux_sdu_data.cur_frag_num, frag);
        bytesInCurFragment = frag.getMemFragSize() - mux_sdu_data.cur_pos;

        pSduFrag = (uint8*)frag.getMemFragPtr() + mux_sdu_data.cur_pos;
        n = min2(bytesToMux, bytesInCurFragment);

        /* create a snapshot of size n */
        frag.getMemFrag().ptr = pSduFrag;
        frag.getMemFrag().len = n;
        /* add it to outgoing pkt */
        outgoingPkt->appendMediaFragment(frag);
        bytesToMux -= n;
        mux_sdu_data.cur_pos = (uint16)(mux_sdu_data.cur_pos + n);
        bytesInCurFragment -= n;
        mux_sdu_data.size = (uint16)(mux_sdu_data.size - n);
        // check for end of fragment
        if (bytesInCurFragment == 0)
        {
            mux_sdu_data.cur_frag_num++;
            mux_sdu_data.cur_pos = 0;
        }
    }
    /* check for end of packet */
    if (mux_sdu_data.size == 0 && mux_sdu_data.lcn->IsSegmentable())
    {
        *pm = true;
    }
    return;
}

int CPVH223Multiplex::MuxToALDispatch(uint8* pdu, int pdu_size, int fClosing, int muxCode)
{
    int ret = 0;

    if (fClosing)
    {
        if (iLcnEnd)
        {
            H223IncomingChannel* lcn = iLcnEnd;
            if (PVMFSuccess != lcn->AlDispatch())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::MuxToALDispatch Memory allocation failure on AlDispatch, channel id=%d", iLcnEnd->GetLogicalChannelNumber()));
                lcn->ResetAlPdu();
            }
            iLcnEnd = NULL;
        }
    }

    PS_MultiplexEntryDescriptor mux_entry = iMuxTblMgr->GetIncomingDescriptor((uint8)muxCode);

    if (mux_entry)
    {
        if (pdu_size)
        {
            ret = BreakPduData(pdu, pdu_size, mux_entry->elementList, mux_entry->size_of_elementList);
        }
    }

    return ret;
}

TPVStatusCode CPVH223Multiplex::AlPduData(H223IncomingChannel* aChannel,
        uint8*& pdu,
        PS_MultiplexElement pElement,
        int n)
{
    OSCL_UNUSED_ARG(pElement);
    if (PVMFSuccess != aChannel->AlPduData(pdu, (uint16)n))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::BreakPduData Memory allocation failure on AlPduData, channel id=%d",
                         pElement->muxType.logicalChannelNumber));
        aChannel->ResetAlPdu();
        return PVMFFailure;
    }
    return PVMFSuccess;
}

TPVStatusCode CPVH223Multiplex::Dispatch(H223IncomingChannel* aChannel,
        PS_MultiplexElement pElement)
{
    OSCL_UNUSED_ARG(pElement);
    if (PVMFSuccess != aChannel->AlDispatch())
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::BreakPduData Memory allocation failure on AlDispatch, channel id=%d",
                         pElement->muxType.logicalChannelNumber));
        aChannel->ResetAlPdu();
        return PVMFErrNoMemory;
    }
    return PVMFSuccess;
}

int CPVH223Multiplex::BreakPduData(uint8*& pdu, int pdu_size, PS_MultiplexElement pElement, int ListSize)
{
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"CPVH223Multiplex::BreakPduData pdu(%x), pdu_size(%d), pElement(%x), ListSize(%d),iObserver(%x)", pdu,pdu_size,pElement,ListSize,iObserver));

    int cnt = 0, cnt2 = 0, LoopCnt = 0, n = 0;
    int demuxed_size = 0;
    H223IncomingChannel* channel = NULL;

    for (cnt = ListSize ; cnt != 0 ; cnt --)
    {
        /* sub-element list search */
        if (pElement->muxType.index != 0)
        {
            LoopCnt = (pElement->repeatCount.index) ?
                      pdu_size /* untill closing flag */ : pElement->repeatCount.finite;

            for (cnt2 = 0 ; cnt2 < LoopCnt ; cnt2 ++)
            {
                demuxed_size += BreakPduData(pdu, pdu_size,
                                             pElement->muxType.subElementList,
                                             pElement->muxType.size);
                pdu += demuxed_size;
                if (demuxed_size >= pdu_size)
                    return demuxed_size; /* Data End */
            }
        }
        /* Element Search */
        else
        {
            for (cnt2 = 0 ; cnt2 < (int)iIncomingChannels.size(); cnt2++)
            {
                if (iIncomingChannels[cnt2].GetRep() == NULL)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::BreakPduData channel(%d) - Does not exist\n", pElement->muxType.logicalChannelNumber));
                }
                if (iIncomingChannels[cnt2].GetRep() &&
                        (pElement->muxType.logicalChannelNumber ==
                         (0xFFFF&iIncomingChannels[cnt2]->GetLogicalChannelNumber())))
                {
                    channel = iIncomingChannels[cnt2];
                    if (pElement->repeatCount.index == 0)
                    {
                        n = min2(pdu_size, pElement->repeatCount.finite);
                    }
                    else
                    {
                        n = pdu_size;
                    }
                    pdu_size -= n;
                    demuxed_size += n;


                    AlPduData(channel, pdu, pElement, n);

                    pdu += n;

                    /* If enough pdus have been received, indicate to observer */
                    if (channel->GetNumSdusIn() == H223_LCN_DATA_DETECT_THRESHOLD)
                    {
                        iObserver->LcnDataDetected(channel->GetLogicalChannelNumber());
                    }

                    if (!channel->IsSegmentable())
                    {
                        Dispatch(channel, pElement);
                    }
                    else
                    {
                        if (pdu_size == 0)
                        {
                            iLcnEnd = channel;
                            return demuxed_size;
                        }
                    }
                    if (pdu_size == 0)
                        return demuxed_size;
                    break;
                }
            }
        }
        pElement++;
    }
    return false;
}


void CPVH223Multiplex::ResetStats()
{
    for (unsigned out_lcn = 0; out_lcn < iOutgoingChannels.size(); out_lcn++)
    {
        iOutgoingChannels[out_lcn]->ResetStats();
    }
    for (unsigned in_lcn = 0; in_lcn < iIncomingChannels.size(); in_lcn++)
    {
        iIncomingChannels[in_lcn]->ResetStats();
    }

    iMuxTblMgr->ResetStats();
    if (iLowerLayer)
    {
        iLowerLayer->ResetStats();
    }
}

void CPVH223Multiplex::LogStats(TPVDirection dir)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "H223 Multiplex Statistics:\n"));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Logical Channel Statistics:\n"));
    if (dir & OUTGOING)
    {
        for (unsigned n = 0; n < iOutgoingChannels.size(); n++)
        {
            iOutgoingChannels[n]->LogStats();
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Outgoing Multiplex Entry Statistics:\n"));
        iMuxTblMgr->LogStats(OUTGOING);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Outgoing Multiplex PDU Statistics:\n"));
        iLowerLayer->LogStats(OUTGOING);
    }

    if (dir & INCOMING)
    {
        for (unsigned n = 0; n < iIncomingChannels.size(); n++)
        {
            iIncomingChannels[n]->LogStats();
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Incoming Multiplex Entry Statistics:\n"));
        iMuxTblMgr->LogStats(INCOMING);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Incoming Multiplex PDU Statistics:\n"));
        iLowerLayer->LogStats(INCOMING);
    }
}

TPVStatusCode CPVH223Multiplex::StopChannel(TPVDirection direction, TPVChannelId channel_id)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::StopChannel direction(%d), channel id(%d)\n", direction, channel_id));
    if (iLcnEnd && (iLcnEnd->GetLogicalChannelNumber() == channel_id))
    {
        iLcnEnd = NULL;
    }
    return FlushChannel(direction, channel_id);
}

TPVStatusCode CPVH223Multiplex::FlushChannel(TPVDirection direction, TPVChannelId channel_id)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::FlushChannel direction(%d), channel id(%d)\n", direction, channel_id));

    if (direction & OUTGOING)
    {
        OsclSharedPtr<H223OutgoingChannel> channel;
        PVMFStatus status = GetOutgoingChannel(channel_id, channel);

        if (status == PVMFSuccess)
        {
            channel->Flush();
            /* flush any partially sent sdu */
            RemoveMuxSduData(channel_id, channel->IsSegmentable());
            return EPVT_Success;
        }
        return EPVT_ErrorInvalidParameter;
    }
    else if (direction & INCOMING)
    {
        OsclSharedPtr<H223IncomingChannel> channel;
        PVMFStatus status = GetIncomingChannel(channel_id, channel);
        if (status == PVMFSuccess)
        {
            channel->Flush();
        }
    }
    return EPVT_Success;
}

void CPVH223Multiplex::LowerLayerError(TPVDirection direction, PVMFStatus error)
{
    OSCL_UNUSED_ARG(direction);
    OSCL_UNUSED_ARG(error);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::LowerLayerError error(%d)\n", error));
    iObserver->MuxErrorOccurred(direction, PV_MUX_COMPONENT_MUX, error);
}

inline int32 CPVH223Multiplex::GetTimestamp()
{
    return iLowerLayer->GetTimestamp();
}

void CPVH223Multiplex::LogicalChannelError(TPVDirection direction, TPVChannelId id, PVMFStatus error)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::LogicalChannelError  error(%d), id(%d)\n", error, id));
    iObserver->MuxErrorOccurred(direction, PV_MUX_COMPONENT_LOGICAL_CHANNEL, error, (uint8*)&id, sizeof(id));
}

void CPVH223Multiplex::SkewDetected(TPVChannelId lcn1, TPVChannelId lcn2, uint32 skew)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SkewDetected lcn1=%d, lcn2=%d, skew=%d", lcn1, lcn2, skew));
    iObserver->SkewDetected(lcn1, lcn2, skew);
}

void CPVH223Multiplex::ReceivedFormatSpecificInfo(TPVChannelId lcn, uint8* fsi, uint32 fsi_len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::ReceivedFormatSpecificInfo lcn=%d, len=%d", lcn, fsi_len));
    iObserver->ReceivedFormatSpecificInfo(lcn, fsi, fsi_len);
}

void CPVH223Multiplex::SetMultiplexLevel(TPVH223Level muxLevel)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetMultiplexLevel  level(%d)\n", muxLevel));
    if (iLowerLayer == NULL)
    {
        OSCL_LEAVE(PVMFErrInvalidState);
    }
    iLowerLayer->SetLevel(muxLevel);
}

unsigned CPVH223Multiplex::GetNumChannels(TPVDirection direction)
{
    if (direction == OUTGOING)
        return iOutgoingChannels.size() - 1;
    return iIncomingChannels.size() - 1;
}

MuxSduData* CPVH223Multiplex::FindMuxSduData(TPVChannelId lcn,
        bool segmentable,
        uint32* p_index)
{
    MuxSduDataList& list = GET_SDU_DATA_LIST(segmentable);
    for (unsigned index = 0; index < list.size();index++)
    {
        if (list[index].lcn->GetLogicalChannelNumber() == lcn)
        {
            if (p_index)
                *p_index = index;
            return &list[index];
        }
    }
    return NULL;
}

void CPVH223Multiplex::AppendMuxSduData(MuxSduData& data)
{
    if (data.lcn->GetLogicalChannelNumber() == 0)
    {
        iControlSduDataList.push_back(data);
        return;
    }
    MuxSduDataList& list = GET_SDU_DATA_LIST(data.lcn->IsSegmentable());
    list.push_back(data);
}

void CPVH223Multiplex::SetMuxSduData(MuxSduData& data)
{
    MuxSduData* mux_sdu_data_ptr = FindMuxSduData(data.lcn->GetLogicalChannelNumber(),
                                   data.lcn->IsSegmentable());
    if (mux_sdu_data_ptr)
    {
        *mux_sdu_data_ptr = data;
    }
}

void CPVH223Multiplex::RemoveMuxSduData(TPVChannelId lcn, bool segmentable)
{
    if (lcn == 0)
    {
        iControlSduDataList.clear();
        return;
    }
    MuxSduDataList& list = GET_SDU_DATA_LIST(segmentable);
    uint32 index = 0;
    while (list.size())
    {
        if (FindMuxSduData(lcn, segmentable, &index))
        {
            list.erase(list.begin() + index);
        }
    }
}

void CPVH223Multiplex::ReleaseMuxSdu(MuxSduData& mux_sdu_data,
                                     MuxSduDataList& list,
                                     unsigned index)
{
    OSCL_UNUSED_ARG(index);
    OSCL_UNUSED_ARG(list);
    mux_sdu_data.lcn->ReleasePacket(mux_sdu_data.sdu);
    if (mux_sdu_data.lcn->GetNextPacket(mux_sdu_data.sdu, PVMFSuccess))
    {
        mux_sdu_data.size = (uint16)mux_sdu_data.sdu->getFilledSize();
        mux_sdu_data.cur_frag_num = 0;
        mux_sdu_data.cur_pos = 0;
    }
    else
    {
        RemoveMuxSduData(mux_sdu_data.lcn->GetLogicalChannelNumber(), mux_sdu_data.lcn->IsSegmentable());
    }
}

unsigned CPVH223Multiplex::UpdateSduDataLists()
{
    unsigned num_lcns = 0;
    for (unsigned lcnindex = 0; lcnindex < iOutgoingChannels.size(); lcnindex++)
    {
        MuxSduData* mux_sdu_data_ptr = NULL;
        if (lcnindex == 0)
        {
            if (iControlSduDataList.size())
                mux_sdu_data_ptr = &iControlSduDataList[0];
        }
        else
        {
            mux_sdu_data_ptr = FindMuxSduData(iOutgoingChannels[lcnindex]->GetLogicalChannelNumber(),
                                              iOutgoingChannels[lcnindex]->IsSegmentable());
        }
        if (mux_sdu_data_ptr)
        {
            num_lcns++;
        }
        else
        {
            // get the next packet and update the sdu size
            PVMFSharedMediaDataPtr sdu;
            if (iOutgoingChannels[lcnindex]->GetNextPacket(sdu, PVMFSuccess))
            {
                MuxSduData mux_sdu_data;
                mux_sdu_data.lcn = iOutgoingChannels[lcnindex];
                mux_sdu_data.sdu = sdu;
                mux_sdu_data.size = (uint16)sdu->getFilledSize();
                mux_sdu_data.cur_frag_num = 0;
                mux_sdu_data.cur_pos = 0;
                AppendMuxSduData(mux_sdu_data);
                num_lcns++;
            }
        }
    }
    return num_lcns;
}

OsclSharedPtr<PVMFMediaDataImpl> CPVH223Multiplex::InitPduPacket()
{
    OsclSharedPtr<PVMFMediaDataImpl> ret = iMuxPduPacketAlloc->allocate(MAX_FRAGMENTS_PER_H223_PDU_PACKET);
    if (!ret)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::InitPduPacket Unable to allocate packet."));
        return ret;
    }
    OsclRefCounterMemFrag hdr;
    iLowerLayer->GetHdrFragment(hdr);
    if (hdr.getMemFragSize())
    {
        ret->appendMediaFragment(hdr);
    }
    else
    {
        ret.Unbind();
    }
    return ret;
}

PVMFStatus CPVH223Multiplex::CompletePduPacket(OsclSharedPtr<PVMFMediaDataImpl>& packet, int mt, int pm)
{
    return iLowerLayer->CompletePacket(packet, mt, pm);
}

void CPVH223Multiplex::UpdateMuxInterval(uint16 aInterval)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::UpdateMuxInterval"));
    iMinSampleInterval = aInterval ? aInterval : (uint16)iLowerLayer->GetMinTimerRes();
    unsigned lcnindex;
    for (lcnindex = 0; lcnindex < iOutgoingChannels.size(); lcnindex++)
    {
        uint16 sample_interval = (uint16)iOutgoingChannels[lcnindex]->GetSampleInterval();
        if (sample_interval && (sample_interval < iMinSampleInterval))
            iMinSampleInterval = sample_interval;
    }

    iNumMuxIntervalsPerTimerInterval = (uint16)(iLowerLayer->GetMinTimerRes() / iMinSampleInterval);
    iNumBytesPerMinSampleInterval = (uint16)((float)(iMinSampleInterval * iBitrate) / 8000.0 + .5);
    iLowerLayer->SetMaxOutgoingPduSize(iNumBytesPerMinSampleInterval);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::UpdateMuxInterval =%d", iMinSampleInterval));
}

void CPVH223Multiplex::ClearSduDataLists()
{
    iControlSduDataList.clear();
    iNonSegmentableSduDataList.clear();
    iSegmentableSduDataList.clear();
}

uint16 CPVH223Multiplex::MuxLcnData(MuxSduDataList& list,
                                    MuxPduPacketList& packets,
                                    uint16 max_size)
{
    if (list.size() == 0)
        return 0;

    int32 pdu_size_left = max_size;
    PS_MultiplexEntryDescriptor mux_entry = NULL;
    for (unsigned n = 0;n < list.size();n++)
    {
        // get descriptor for this logical channel
        mux_entry = iMuxTblMgr->GetOutgoingDescriptor(list[n].lcn,
                    list[n].sdu);
        if (mux_entry == NULL)
        {
            continue;
        }
        uint32 packet_size = DispatchPduPacket(packets, list[n], mux_entry, pdu_size_left);
        if (packet_size == 0)
            break;

        pdu_size_left -= packet_size;
        if (pdu_size_left < 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::MuxSduData Size exceeded, pdu_size_left=%d", pdu_size_left));
        }
        if (list[n].size == 0)
        {
            ReleaseMuxSdu(list[n], list, n);
        }
        if (pdu_size_left <= (int)iLowerLayer->GetHeaderSize())
            break;
    }
    return (uint16)(max_size - pdu_size_left);
}

void CPVH223Multiplex::SetMultiplexingDelayMs(uint16 aDelay)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetMultiplexingDelayMs,aDelay=%d", aDelay));
    iMultiplexingDelayMs = aDelay;
}

void CPVH223Multiplex::SetLogicalChannelBufferingMs(uint32 aInBufferingMs,
        uint32 aOutBufferingMs)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetLogicalChannelBufferingMs,aInBufferingMs=%d,aOutBufferingMs=%d", aInBufferingMs, aOutBufferingMs));
    iInLogicalChannelBufferingMs = aInBufferingMs;
    iOutLogicalChannelBufferingMs = aOutBufferingMs;
}

void CPVH223Multiplex::SetBitrate(uint32 aBitrate)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetLogicalChannelBufferingMs,aBitrate=%d", aBitrate));
    iBitrate = aBitrate;
}

void CPVH223Multiplex::SetStuffingMsgHeader()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetStuffingMsgHeader() mode=%d", iFmStuffingMode));
    const int32 MAX_STUFFING_SEQUENCE_SIZE = 16;
    uint stuffing_size = H223GetMuxStuffingSz(iLowerLayer->GetLevel());
    if (!stuffing_size)
        return;

    uint8 buf[MAX_STUFFING_SEQUENCE_SIZE];
    stuffing_size = H223MuxStuffing(iLowerLayer->GetLevel(), buf, stuffing_size);
    iLowerLayer->SendStuffingMsgHeader(buf, (uint16)stuffing_size);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetStuffingMsgHeader()-done"));
}

void CPVH223Multiplex::SetInterleavingMultiplexFlags(uint16 size, uint8* flags)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPVH223Multiplex::SetInterleavingMultiplexFlags() size=%d, flags=%x", size, flags));

    if (iInterleavingMultiplexFlags && iInterleavingPacket)
    {
        iInterleavingPacket->clearMediaFragments();
        iInterleavingPacket.Unbind();
        OSCL_DEFAULT_FREE(iInterleavingMultiplexFlags);
        iInterleavingMultiplexFlags = NULL;
    }
    iInterleavingMultiplexFlagsSize = size;
    iInterleavingMultiplexFlags = (uint8*)OSCL_DEFAULT_MALLOC(size);
    oscl_memcpy(iInterleavingMultiplexFlags, flags, size);
    iInterleavingPacket = iMuxPduPacketAlloc->allocate(1);
    if (!iInterleavingPacket)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "CPVH223Multiplex::SetInterleavingMultiplexFlags Unable to allocate packet."));
    }
}

void CPVH223Multiplex::EnableStuffing(bool enable)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPVH223Multiplex::EnableStuffing enable=%d\n", enable));
    iEnableStuffing = enable;
}

void CPVH223Multiplex::EnableStuffOnlyFirstSend(bool enable)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPVH223Multiplex::EnableStuffOnlyFirstSend enable=%d\n", enable));
    iStuffOnlyFirstSend = enable;
}

void CPVH223Multiplex::SetMioLatency(int32 aLatency, bool aAudio)
{
    H223IncomingChannel* channel = NULL;
    if (iIncomingChannels.size() != 0)
    {
        for (int32 ii = 0;ii < (int32)iIncomingChannels.size();ii++)
        {
            channel = iIncomingChannels[ii];
            if (aAudio)
            {
                channel->SetAudioLatency(aLatency);
            }
            else
            {
                channel->SetVideoLatency(aLatency);
            }
        }
    }
}


