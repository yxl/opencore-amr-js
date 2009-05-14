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
/*
 * File:	muxpdu.c
 * Comment:	Mux level 1 specific routines
 */
#include "level1.h"
#include "h223.h"

Level1PduParcom::Level1PduParcom(bool df):
        iDf(df)
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.Level1");
    iCurrentInt32 = 0;
    iCurrentOp = ELookForStartFlag;
    iSkipBytes = 0;
    iPduPos = NULL;
    uint8 HecCrc[18] =
    {
        0x00, 0x05, 0x07, 0x02, 0x03, 0x06, 0x04, 0x01,
        0x06, 0x03, 0x01, 0x04, 0x05, 0x00, 0x02, 0x07
    };

    oscl_memcpy(iHecCrc, HecCrc, 18);
}

void
Level1PduParcom::Construct(uint16 max_outstanding_pdus)
{
    iHdrFragmentAlloc.SetLeaveOnAllocFailure(false);
    iHdrFragmentAlloc.size(max_outstanding_pdus, (uint16)H223GetMaxStuffingSz(H223_LEVEL1));
}

void Level1PduParcom::GetHdrFragment(OsclRefCounterMemFrag& frag)
{
    frag = iHdrFragmentAlloc.get();
}

unsigned
Level1PduParcom::GetStuffing(uint8* pPdu, uint32 buf_size, uint8 mux_code)
{
    OSCL_UNUSED_ARG(mux_code);
    OSCL_UNUSED_ARG(buf_size);
    int32 stuffing_size = iDf ? 4 : 2;
    int32 size_left = buf_size;
    while (size_left >= stuffing_size)
    {
        *(pPdu) = 0xe1;
        *(pPdu + 1) = 0x4d;
        pPdu += 2;
        size_left -= 2;
        if (iDf)
        {
            *(pPdu) = 0xe1;
            *(pPdu + 1) = 0x4d;
            pPdu += 2;
            size_left -= 2;
        }
    }
    return (buf_size -size_left);
}

PVMFStatus Level1PduParcom::CompletePdu(OsclSharedPtr<PVMFMediaDataImpl>& pdu, int8 MuxTblNum, uint8 pm)
{
    iClosingNext = 0;
    OSCL_UNUSED_ARG(pm);
    OsclRefCounterMemFrag frag;
    pdu->getMediaFragment(0, frag);
    pdu->setMediaFragFilledLen(0, GetHeaderSz());

    // header
    uint8* pPdu = (uint8*)frag.getMemFrag().ptr;

    *pPdu = 0xe1;
    *(pPdu + 1) = 0x4d;
    if (iDf)
    {
        *(pPdu + 2) = 0xe1;
        *(pPdu + 3) = 0x4d;
        *(pPdu + 4) = (uint8)(iHecCrc[MuxTblNum] << 5 | MuxTblNum << 1);
    }
    else
    {
        *(pPdu + 2) = (uint8)(iHecCrc[MuxTblNum] << 5 | MuxTblNum << 1);
    }

    // trailer
    if (pm)
    {
        OsclRefCounterMemFrag frag = iHdrFragmentAlloc.get();
        if (frag.getMemFragPtr() == NULL)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Level1PduParcom::CompletePdu Failed to allocate frag"));
            return PVMFErrNoMemory;
        }

        frag.getMemFrag().len = GetHeaderSz();
        pPdu = (uint8*)frag.getMemFragPtr();
        *pPdu = 0xe1;
        *(pPdu + 1) = 0x4d;
        if (iDf) 	/* L1 Double Flag */
        {
            *(pPdu + 2) = 0xe1;
            *(pPdu + 3) = 0x4d;
            *(pPdu + 4) = (uint8)(iHecCrc[MuxTblNum] << 5 | MuxTblNum << 1 | 1);
        }
        else
        {
            *(pPdu + 2) = (uint8)(iHecCrc[MuxTblNum] << 5 | MuxTblNum << 1 | 1);
        }
        pdu->appendMediaFragment(frag);
    }
    return PVMFSuccess;
}

inline void Level1PduParcom::IndicatePdu()
{
    int pdu_size = iPduPos - iPdu;
    uint8* pdu = iPdu;

    if (pdu_size >= 1)
    {
        if (iHecCrc[((*pdu)>>1)&0xf] != (*pdu) >> 5)
        {
            iObserver->MuxPduErrIndicate(EHeaderErr);
#ifdef PVANALYZER
            nCrcError++;
            PVAnalyzer(ANALYZER_MUXHDR, "Mux Header Error #=%d, %%=%f", nCrcError, (double)nCrcError / (double)nPdu);
#endif
        }
        else
        {
            int fClosing = 0;
            int muxCode = -1;
            if ((*pdu) & 1)
                fClosing = 1;
            muxCode = ((*pdu) >> 1) & 0xf;
            pdu++;
            pdu_size--;
            iObserver->MuxPduIndicate(pdu, (uint16)pdu_size, fClosing, muxCode);
        }
    }
    iPduPos = NULL;
}

uint32 Level1PduParcom::Parse(uint8* bsbuf, uint32 bsbsz)
{
    const int FLAG = (iDf) ? 0xE14DE14D : 0xE14D;
    const int FLAG_SZ = (iDf) ? 4 : 2;
    const int FLAG_MASK = (iDf) ? 0xFFFFFFFF : 0xFFFF;
#ifdef PVANALYZER
    static unsigned int nPdu = 0;
    static unsigned int nCrcError = 0;
    nPdu++;
#endif

    for (unsigned num = 0; num < bsbsz; num++)
    {
        iCurrentInt32 <<= 8;
        iCurrentInt32  |= bsbuf[num];
        switch (iCurrentOp)
        {
            case ELookForStartFlag:
                if ((iCurrentInt32 & FLAG_MASK) == FLAG)
                {
                    iCurrentOp = ESkipFlags;
                    iPduPos = iPdu;
                }
                break;
            case ESkipFlags:
                iSkipBytes++;
                if (iSkipBytes == FLAG_SZ)
                {
                    if ((iCurrentInt32 & FLAG_MASK) != FLAG)
                    {
                        if (!iPduPos)
                            iPduPos = iPdu;
                        // Check if iPduPos+FLAG_SZ-1 overflows
                        int copy_from = iCurrentInt32;
                        for (uint8* write_pos = iPduPos + FLAG_SZ - 1; write_pos >= iPduPos; write_pos--)
                        {
                            *write_pos = (uint8)copy_from;
                            copy_from >>= 8;
                        }
                        iPduPos += FLAG_SZ;
                        iCurrentOp = ELookForEndFlag;
                    }
                    iSkipBytes = 0;
                }
                break;
            case ELookForEndFlag:
                if ((iCurrentInt32 & FLAG_MASK) == FLAG)
                {
                    iPduPos -= (FLAG_SZ - 1);
                    // found end flag
                    IndicatePdu();
                    iCurrentOp = ESkipFlags;
                }
                else
                {
                    *iPduPos++ = bsbuf[num];
                    // check for pdu size
                    if ((iPduPos - iPdu) == H223_MAX_MUX_PDU_SIZE)
                    {
                        iObserver->MuxPduErrIndicate(ESizeErr);
                        iPduPos = NULL;
                        iCurrentOp = ELookForStartFlag;
                    }
                }
                break;
        }
    }
    return 1;
}
