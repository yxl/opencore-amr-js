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
#include "adaptationlayer.h"
#define SREJ	    0
#define	DRTX	    255
#define PV2WAY_H223_AL2_SN_WRAPAROUND 256
#define PV2WAY_H223_AL2_CRC_SIZE 1
#define PV2WAY_H223_AL3_CRC_SIZE 2
#define PV2WAY_MAX_PACKET_MEM_FRAG 32
#define PV2WAY_H223_AL2_MAX_HDR_TRLR_FRAG_SIZE 4
#define PV2WAY_H223_AL3_SNPOS1_BITLEN 7
#define PV2WAY_H223_AL3_SNPOS1_VRMAX 0x7F
#define PV2WAY_H223_AL3_SNPOS2_VRMAX 0x7FFF
#define PV2WAY_H223_AL3_SNPOS2_PT_LEN 1


void AdaptationLayer1::Construct()
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.AdaptationLayer1");
}

void AdaptationLayer1::ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer1::ParsePacket, pdu_size(%d)", pkt->getFilledSize()));
    oscl_memset(&info, 0, sizeof(IncomingALPduInfo));
    info.sdu_size = (int16)pkt->getFilledSize();

}


PVMFStatus AdaptationLayer1::CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt)
{
    OSCL_UNUSED_ARG(pkt);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer1::CompletePacket packet size(%d)", pkt->getFilledSize()));
    return PVMFSuccess;
}

void AdaptationLayer2::Construct()
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.AdaptationLayer2");
    // Do not Leave on allocation failure
    iMemFragmentAlloc.SetLeaveOnAllocFailure(false);
    iMemFragmentAlloc.size((uint16)iMaxNumSdus, PV2WAY_H223_AL2_MAX_HDR_TRLR_FRAG_SIZE);
}

void AdaptationLayer2::SetSeqnum(bool on_off)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer2::SetSeqnum(%d)", on_off));
    if (on_off)
    {
        iSNPos = 1;
        iHdrSz = 1;
    }
    else
    {
        iSNPos = 0;
        iHdrSz = 0;
    }
}

PVMFStatus AdaptationLayer2::StartPacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt)
{
    if (iSNPos)
    {
        OsclRefCounterMemFrag hdr_frag = iMemFragmentAlloc.get();
        if (hdr_frag.getMemFragPtr() == NULL)
        {
            return PVMFErrNoMemory;
        }
        hdr_frag.getMemFrag().len = 1;
        pkt->appendMediaFragment(hdr_frag);
    }
    return PVMFSuccess;
}



PVMFStatus AdaptationLayer2::CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt)
{
    int Size = pkt->getFilledSize();
    uint8 Crc;
    uint8* pos = NULL;
    OsclRefCounterMemFrag hdr_frag;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer2::CompletePacket packet, size(%d)", Size));

    OsclRefCounterMemFrag trlr_frag = iMemFragmentAlloc.get();
    if (trlr_frag.getMemFragPtr() == NULL)
    {
        return PVMFErrNoMemory;
    }
    trlr_frag.getMemFrag().len = 1;

    if (iSNPos)
    {
        pkt->getMediaFragment(0, hdr_frag);
        Size += iSNPos;
        pos = (uint8*)hdr_frag.getMemFragPtr();
        *pos = (uint8)(iSeqNum);
        if (iSeqNum != 0xff)
            iSeqNum ++;
        else
            iSeqNum = 0;
    }

    Crc = crc.Crc8Check(pkt);
    pkt->appendMediaFragment(trlr_frag);
    pos = (uint8*)trlr_frag.getMemFragPtr();

    *pos = Crc;
    Size ++;

    return PVMFSuccess;
}

#define WINSIZE	10

void AdaptationLayer2::ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info)
{
    OsclRefCounterMemFrag frag;
    uint8 Crc = 0;
    info.crc_error = false; // No CRC error.
    info.seq_num_error = 0; // No sequence number error.
    uint8 SeqNum = 0;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer2::ParsePacket pdu size(%d)", pkt->getFilledSize()));
    info.sdu_size = (uint16)(pkt->getFilledSize() - iSNPos - PV2WAY_H223_AL2_CRC_SIZE);
    if (info.sdu_size  <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "AdaptationLayer2::ParsePacket sdu size(%d) < 0", info.sdu_size));
        return;
    }
    OsclRefCounterMemFrag last_frag;
    pkt->getMediaFragment(pkt->getNumFragments() - 1, last_frag);
    Crc = *((uint8*)last_frag.getMemFragPtr() + last_frag.getMemFragSize() - 1);
    pkt->setMediaFragFilledLen(pkt->getNumFragments() - 1, last_frag.getMemFrag().len - PV2WAY_H223_AL2_CRC_SIZE);

    OsclRefCounterMemFrag first_frag;
    pkt->getMediaFragment(0, first_frag);

    uint16 CrcCheck = crc.Crc8Check(pkt, false);
    if (Crc != CrcCheck)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "AdaptationLayer2::ParsePacket CRC error sn(%d)", iSeqNum));
        info.crc_error = true;
        //Update expected sequence number.
        iSeqNum = (iSeqNum + 1) % PV2WAY_H223_AL2_SN_WRAPAROUND;
    }
    else
    {
        if (iSNPos == 1)
        {
            SeqNum = *((uint8*)first_frag.getMemFragPtr());
            first_frag.getMemFrag().len -= iSNPos;
            first_frag.getMemFrag().ptr = (uint8*)first_frag.getMemFrag().ptr + iSNPos;

            //If sequence number is good.
            if (iSeqNum == SeqNum)
            {
                iSeqNum = (iSeqNum + 1) % PV2WAY_H223_AL2_SN_WRAPAROUND;
            }
            //Else sequence number is not good.
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "AdaptationLayer2::ParsePacket Sequence number error expected(%d), received(%d)", iSeqNum, SeqNum));

                //Check the difference between expected seq number and actual seq number.

                //Check if sequence number wrapped.
                if (iSeqNum > SeqNum)
                {
                    info.seq_num_error = (PV2WAY_H223_AL2_SN_WRAPAROUND - iSeqNum) + SeqNum;
                }
                //Else no wrap.
                {
                    info.seq_num_error = SeqNum - iSeqNum;
                }

                //Update expected seq number based on actual seq number received.
                iSeqNum = (SeqNum + 1) % PV2WAY_H223_AL2_SN_WRAPAROUND;
            }
        }
    }
    if (iSNPos)
    {
        OsclRefCounterMemFrag frags[PV2WAY_MAX_PACKET_MEM_FRAG];
        unsigned int num_frags = pkt->getNumFragments();
        if (num_frags <= PV2WAY_MAX_PACKET_MEM_FRAG)
        {
            unsigned int fragnum;
            frags[0] = first_frag;
            for (fragnum = 1;fragnum < num_frags;fragnum++)
            {
                pkt->getMediaFragment(fragnum, frags[fragnum]);
            }
            pkt->clearMediaFragments();
            for (fragnum = 0;fragnum < num_frags;fragnum++)
            {
                pkt->appendMediaFragment(frags[fragnum]);
            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer2::ParsePacket pkt->getNumFragments() is greater then PV2WAY_MAX_PACKET_MEM_FRAG"));
        }
    }
}

void AdaptationLayer3::Construct()
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.AdaptationLayer3");
    // Do not Leave on allocation failure
    iMemFragmentAlloc.SetLeaveOnAllocFailure(false);
    iMemFragmentAlloc.size((uint16)(iMaxNumSdus*2), 4);
}

void AdaptationLayer3::SetSeqnumSz(uint16 sz)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer3::SetSeqnumSz(%d)", sz));
    iSNPos = sz;
    iHdrSz = sz;
}

PVMFStatus AdaptationLayer3::StartPacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt)
{
    if (iSNPos)
    {
        OsclRefCounterMemFrag hdr_frag = iMemFragmentAlloc.get();
        if (hdr_frag.getMemFragPtr() == NULL)
        {
            return PVMFErrNoMemory;
        }
        hdr_frag.getMemFrag().len = iSNPos;
        pkt->appendMediaFragment(hdr_frag);
    }
    return PVMFSuccess;
}

PVMFStatus AdaptationLayer3::CompletePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt)
{
    int Size = pkt->getFilledSize();
    unsigned Crc = 0, usTmp = 0;
    uint8* pos = NULL;
    OsclRefCounterMemFrag hdr_frag;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer3::CompletePacket(%d)", pkt->getFilledSize()));
    OsclRefCounterMemFrag trlr_frag = iMemFragmentAlloc.get();
    if (trlr_frag.getMemFragPtr() == NULL)
    {
        return PVMFErrNoMemory;
    }

    trlr_frag.getMemFrag().len = 2;
    Size += iSNPos;

    switch (iSNPos)
    {
        case 0:
            break;
        case 1:
            usTmp = (uint16)iSeqNum;
            pkt->getMediaFragment(0, hdr_frag);
            pos = (uint8*)hdr_frag.getMemFragPtr();

            *pos = (uint8)((usTmp << 1) | 1);
            if (usTmp != PV2WAY_H223_AL3_SNPOS1_VRMAX)
                iSeqNum ++;
            else
                iSeqNum = 0;
            break;        /* SN( 7bit ) */
        case 2:
            usTmp = iSeqNum;
            pkt->getMediaFragment(0, hdr_frag);
            pos = (uint8*)hdr_frag.getMemFragPtr();

            *pos = (uint8)((usTmp >> 7) | 1);
            *(pos + 1) = (uint8)(usTmp & 0xff);
            if (usTmp != PV2WAY_H223_AL3_SNPOS2_VRMAX)
                iSeqNum ++;
            else
                iSeqNum = 0;
            break;			/* SN( 15bit )*/
    }

    Crc = crc.Crc16Check(pkt);
    pkt->appendMediaFragment(trlr_frag);
    pos = (uint8*)trlr_frag.getMemFragPtr();

    *(pos) = (uint8)(Crc & 0xff);
    *(pos + 1) = (uint8)(Crc >> 8);
    Size += 2;
    return PVMFSuccess;
}

#define WINSIZE	10

void AdaptationLayer3::ParsePacket(OsclSharedPtr<PVMFMediaDataImpl>& pkt, IncomingALPduInfo& info)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "AdaptationLayer3::ParsePacket pdu_size(%d)", pkt->getFilledSize()));
    OsclRefCounterMemFrag frag;
    uint16 SeqNum = 0;
    uint16 Crc = 0, VrMax = 0;
    iPktNum++;
    info.crc_error = false;
    info.seq_num_error = 0;
    info.sdu_size = (uint16)(pkt->getFilledSize() - iSNPos - PV2WAY_H223_AL3_CRC_SIZE);
    if (info.sdu_size <= 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "AdaptationLayer3::ParsePacket sdu size(%d) < 0", info.sdu_size));
        return;
    }

    OsclRefCounterMemFrag first_frag, last_frag;
    pkt->getMediaFragment(0, first_frag);
    pkt->getMediaFragment(pkt->getNumFragments() - 1, last_frag);

    switch (iSNPos)
    {
        case 0:
//		not used currently
            break;
        case 1:
            SeqNum = (uint16)(((uint8*)first_frag.getMemFragPtr())[0] >> (sizeof(uint8)));
            VrMax = PV2WAY_H223_AL3_SNPOS1_VRMAX;			    /* 0-127 */
            break;
        case 2:
            SeqNum = (uint16)((((uint8*)first_frag.getMemFragPtr())[0] >> 1) << 8 | ((uint8*)first_frag.getMemFragPtr())[1]);
            VrMax = PV2WAY_H223_AL3_SNPOS2_VRMAX; /* 0-32767 */
            break;
    }

    if (last_frag.getMemFragSize() >= PV2WAY_H223_AL3_CRC_SIZE)
    {
        Crc = (uint16)(((*((uint8*)last_frag.getMemFragPtr() + last_frag.getMemFragSize() - 1)) << 8) |
                       (*((uint8*)last_frag.getMemFragPtr() + last_frag.getMemFragSize() - 2)));
        pkt->setMediaFragFilledLen(pkt->getNumFragments() - 1, last_frag.getMemFrag().len - PV2WAY_H223_AL3_CRC_SIZE);
    }
    else // in the rare case that the last fragment contains only 1 byte of the CRC
    {
        OsclRefCounterMemFrag second_last_frag;
        pkt->getMediaFragment(pkt->getNumFragments() - 2, second_last_frag);
        Crc = (uint16)(((*((uint8*)last_frag.getMemFragPtr())) << 8) |
                       (*((uint8*)second_last_frag.getMemFragPtr() + second_last_frag.getMemFragSize() - 1)));
        pkt->setMediaFragFilledLen(pkt->getNumFragments() - 1, last_frag.getMemFrag().len - 1);
        pkt->setMediaFragFilledLen(pkt->getNumFragments() - 2, second_last_frag.getMemFrag().len - 1);
    }
    if (Crc != crc.Crc16Check(pkt, false))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "AdaptationLayer3::ParsePacket CRC error, sn(%d)", iSeqNum));

        info.crc_error = true;

        //Update sequence number.
        if (iSeqNum == VrMax)
        {
            iSeqNum = 0;
        }
        else
        {
            iSeqNum++;
        }
    }
    else
    {
        //If sequence number is good.
        if (iSeqNum == SeqNum)
        {
            if (iSeqNum == VrMax)
            {
                iSeqNum = 0;
            }
            else
            {
                iSeqNum++;
            }
        }
        //Else sequence number is not good.
        else
        { /* missing or mis-delivered packets, send them anyway */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "AdaptationLayer3::ParsePacket Sequence number error - Expected(%d), Received(%d)", iSeqNum, SeqNum));
            //Check the difference between expected seq number and actual seq number.

            //Check if sequence number wrapped.
            if (iSeqNum > SeqNum)
            {
                info.seq_num_error = ((VrMax + 1) - iSeqNum) + SeqNum;
            }
            //Else no wrap.
            {
                info.seq_num_error = SeqNum - iSeqNum;
            }

            //Update
            iSeqNum = (uint16)((SeqNum + 1) % (VrMax + 1));
        }
    }

    if (iSNPos)
    {
        pkt->getMediaFragment(0, first_frag);// do this in case first frag == last frag
        first_frag.getMemFrag().len -= iSNPos;
        first_frag.getMemFrag().ptr = (uint8*)first_frag.getMemFrag().ptr + iSNPos;
        OsclRefCounterMemFrag frags[PV2WAY_MAX_PACKET_MEM_FRAG];
        unsigned int num_frags = pkt->getNumFragments();
        unsigned int fragnum;
        frags[0] = first_frag;
        for (fragnum = 1;fragnum < num_frags;fragnum++)
        {
            pkt->getMediaFragment(fragnum, frags[fragnum]);
        }
        pkt->clearMediaFragments();
        for (fragnum = 0;fragnum < num_frags;fragnum++)
        {
            pkt->appendMediaFragment(frags[fragnum]);
        }
    }

}
