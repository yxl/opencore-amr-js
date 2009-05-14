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
#include "level0.h"
#include "h324utils.h"
#include "h223.h"

#define HDLC	0x7e
#define NUM_ZERO_BIT_INSERTION_BUFFERS 32

Level0PduParcom::Level0PduParcom()
{
    iLogger = PVLogger::GetLoggerObject("3g324m.h223.Level0");
    iNumRecvBits = 0;
    iRecvBits = 0;
    iRecvByte = 0;
    iRecvBytePos = 0;
    iRecvByte0 = 0;
    iRecvByte0Pos = 0;
    iZeroBitRemovedOutputPos = 0;
    iZeroBitRemovedOutput = 0;
    iPduPosZeroRemoved = 0;

    iTxByte0 = 0;
    iTxByte0Pos = 0;
    iTxCheck0 = 0;
    iTxCheck0Pos = 0;

    uint8 HecCrc[18] =
    {
        0x00, 0x05, 0x07, 0x02, 0x03, 0x06, 0x04, 0x01,
        0x06, 0x03, 0x01, 0x04, 0x05, 0x00, 0x02, 0x07
    };

    oscl_memcpy(iHecCrc, HecCrc, 18);
}

void
Level0PduParcom::Construct(uint16 max_outstanding_pdus)
{
    // Do not Leave on allocation failure
    iPduFragmentAlloc.SetLeaveOnAllocFailure(false);
    iPduFragmentAlloc.size(NUM_ZERO_BIT_INSERTION_BUFFERS, H223_MAX_MUX_PDU_SIZE);
    // Do not Leave on allocation failure
    iHdrFragmentAlloc.SetLeaveOnAllocFailure(false);
    iHdrFragmentAlloc.size(max_outstanding_pdus, (uint16)H223GetMaxStuffingSz(H223_LEVEL0));
}

void Level0PduParcom::GetHdrFragment(OsclRefCounterMemFrag& frag)
{
    frag = iHdrFragmentAlloc.get();
}

uint32
Level0PduParcom::GetStuffing(uint8* pPdu, uint32 max_size, uint8 mux_code)
{
    OSCL_UNUSED_ARG(mux_code);
    OSCL_UNUSED_ARG(max_size);
    uint8* pos = pPdu;
    for (unsigned i = 0; i < max_size; i++)
    {
        pos += AppendTxOctet(HDLC, pos);
    }
    return (uint16)(pos -pPdu);
}

PVMFStatus Level0PduParcom::CompletePdu(OsclSharedPtr<PVMFMediaDataImpl>& pdu, int8 MuxTblNum, uint8 pm)
{
    uint8 flag = HDLC;
    // header
    OsclRefCounterMemFrag frag;
    pdu->getMediaFragment(0, frag);
    pdu->setMediaFragFilledLen(0, GetHeaderSz());

    uint8* pPdu = (uint8*)frag.getMemFragPtr();
    *pPdu = flag;
    *(pPdu + 1) = (uint8)(iHecCrc[MuxTblNum] << 5 | MuxTblNum << 1);


    OsclRefCounterMemFrag post_zb_frag = iPduFragmentAlloc.get();
    if (post_zb_frag.getMemFragPtr() == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Level0PduParcom::CompletePdu Failed to allocate post_zb_frag"));
        return PVMFErrNoMemory;
    }

    uint8* pos = (uint8*)post_zb_frag.getMemFragPtr();
    pos += AppendTxOctet(flag, pos);
    pos += AppendTxOctet(flag, pos);
    pos += Insert0Octet(*(pPdu + 1), 8, pos);

    // write out the payload
    for (uint frag_num = 1; frag_num < pdu->getNumFragments(); frag_num++)
    {
        pdu->getMediaFragment(frag_num, frag);
        pos += Insert0((uint8*)frag.getMemFragPtr(), frag.getMemFragSize(), pos);
    }
    pos += AppendTxOctet(flag, pos);
    if (pm)
    {
        pos += Insert0Octet(1, 8, pos);
        pos += AppendTxOctet(flag, pos);
    }
    pos += AppendTxOctet(flag, pos);

    // replace current fragments with the zero bit inserted fragment
    pdu->clearMediaFragments();
    post_zb_frag.getMemFrag().len = pos - (uint8*)post_zb_frag.getMemFragPtr();
    if (post_zb_frag.getMemFrag().len)
    {
        pdu->appendMediaFragment(post_zb_frag);
    }

    return PVMFSuccess;
}

uint32 Level0PduParcom::Parse(uint8* bsbuf, uint32 bsbsz)
{
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"Level0PduParcom::Parse %d\n", bsbsz));
    //printBuffer(iDebug, bsbuf, bsbsz);
    SearchForHdlcFlag(bsbuf, bsbsz);
    return 0;
}

/* This function writes an octet to the zero bit stuffed tx bitstream.  If there is pending bits for zero bit check,
   they are flushed out first */
uint16 Level0PduParcom::AppendTxOctet(uint8 octet, uint8* pos)
{
    uint8* write_pos = pos;
    if (iTxCheck0Pos)
    {
        write_pos += AppendTxBits0(iTxCheck0Pos, iTxCheck0, write_pos);
        iTxCheck0Pos = 0;
        iTxCheck0 = 0;
    }
    write_pos += AppendTxBits0(8, octet, write_pos);
    return (uint16)(write_pos -pos);
}

/* Writes out bits to the output stream */
uint16 Level0PduParcom::AppendTxBits0(unsigned num_bits, int bits, uint8* pos)
{
    uint8* write_pos = pos;
    iTxByte0 |= (bits << iTxByte0Pos);
    iTxByte0Pos += num_bits;

    while (iTxByte0Pos >= 8)
    {
        *write_pos++ = (uint8)iTxByte0;
        iTxByte0 >>= 8;
        iTxByte0Pos -= 8;
    }
    return (uint16)(write_pos - pos);
}

/* Inserts the octet while checking for 5 1's.  */
uint16 Level0PduParcom::Insert0Octet(uint8 octet, uint16 num_bits_from_octet, uint8* out_buffer)
{
    uint8* write_pos = out_buffer;

    iTxCheck0 = iTxCheck0 | (octet << iTxCheck0Pos);
    iTxCheck0Pos += num_bits_from_octet;
    int bits_consumed = 0;
    while (iTxCheck0Pos >= 5)
    {
        if ((iTxCheck0 & 0x1F) == 0x1F)
        {
            // 5 1's detected.  Insert 0 after them
            //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"Level0PduParcom::Insert0Octet - inserting 0\n"));
            write_pos += AppendTxBits0(5, iTxCheck0 & 0x1F, write_pos);
            write_pos += AppendTxBits0(1, 0, write_pos);
            bits_consumed = 5;
        }
        else
        {
            // just insert the lsb
            write_pos += AppendTxBits0(1, iTxCheck0 & 1, write_pos);
            bits_consumed = 1;
        }
        iTxCheck0 >>= bits_consumed;
        iTxCheck0Pos -= bits_consumed;
    }
    return (uint16)(write_pos -out_buffer);
}

/* Zero bit insertion from a buffer */
uint16
Level0PduParcom::Insert0(uint8* chunk, int chunk_size, uint8* out_buffer)
{
    uint8* pos = out_buffer;
    for (int bytenum = 0; bytenum < chunk_size; bytenum++)
    {
        pos += Insert0Octet(chunk[bytenum], 8, pos);
    }
    return (uint16)(pos -out_buffer);
}

/* Does more than the name implies.  It searches for the next flag, and if a valid pdu is found, does zero bit removal
   and indicates pdu to upper layer */
void Level0PduParcom::SearchForHdlcFlag(uint8* bsbuf, int bsbsz)
{
    for (int bytenum = 0; bytenum < bsbsz; bytenum++)
    {
        uint8 cur_byte = bsbuf[bytenum];
        uint8 cur_bit = 0;
        for (int bitnum = 0; bitnum < 8; bitnum++)
        {
            cur_bit = (uint8)(cur_byte & 0x1);
            cur_byte >>= 1;
            iRecvBits = (uint8)(iRecvBits | (cur_bit << iNumRecvBits++));
            if (iNumRecvBits == 8)
            {
                if ((iRecvBits&HDLC) == HDLC)
                {
                    // found flag
                    unsigned size = iPduPos - iPdu;
                    if (size)
                    {
                        // perform 0 bit removal within the same buffer
                        Remove0();
                    }
                    iNumRecvBits = 0;
                    iRecvBits = 0;
                }
                else
                {
                    // copy current bit to pdu
                    AppendBit(iRecvBits&0x1);
                    iRecvBits >>= 1;
                    iNumRecvBits--;
                }
            }
        }
    }
}

/* Appends a bit to the received bitstream.  If 8 bits are received, they are flushed to iPdu */
void Level0PduParcom::AppendBit(int c)
{
    iRecvByte |= (c << iRecvBytePos++);
    if (iRecvBytePos == 8)
    {
        if (iPduPos == iPduEndPos)
        {
            Remove0();
        }
        *iPduPos++ = (uint8)iRecvByte;
        iRecvByte = 0;
        iRecvBytePos = 0;
    }
}

/* Appends bits to the zero bit removed incoming bitstream */
void Level0PduParcom::AppendBits0(unsigned num_bits, int bits)
{
    iZeroBitRemovedOutput = iZeroBitRemovedOutput | (bits << iZeroBitRemovedOutputPos);
    iZeroBitRemovedOutputPos += num_bits;
    while (iZeroBitRemovedOutputPos >= 8)
    {
        OSCL_ASSERT(iPduPosZeroRemoved != (iPduEndPos + 1));
        *iPduPosZeroRemoved++ = (uint8)iZeroBitRemovedOutput;
        iZeroBitRemovedOutput >>= 8;
        iZeroBitRemovedOutputPos -= 8;
    }
}

/* Performs 0 bit removal from the current pdu (iPdu) */
unsigned Level0PduParcom::Remove0()
{
    int size = iPduPos - iPdu;
    OSCL_ASSERT(size >= 0);
    iPduPosZeroRemoved = iPdu;
    //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"Level0PduParcom::Remove0 size(%d)", size));
    for (int bytenum = 0; bytenum < size; bytenum++)
    {
        Remove0Octet(iPdu[bytenum], 8);
    }
    if (iRecvBytePos)
    {
        //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"Level0PduParcom::Remove0 iRecvBytePos(%d)", iRecvBytePos));
        Remove0Octet((uint8)iRecvByte, iRecvBytePos);
        iRecvBytePos = 0;
        iRecvByte = 0;
    }
    if (iRecvByte0Pos)
    {
        //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"Level0PduParcom::Remove0 iRecvByte0Pos(%d)", iRecvByte0Pos));
        AppendBits0(iRecvByte0Pos, iRecvByte0);
        iRecvByte0Pos = 0;
        iRecvByte0 = 0;
    }
    //OSCL_ASSERT(iZeroBitRemovedOutputPos == 0);
    if (iZeroBitRemovedOutputPos)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "Level0PduParcom::Remove0 Error - iZeroBitRemovedOutputPos(%d) != 0", iZeroBitRemovedOutputPos));
        iZeroBitRemovedOutputPos = 0;
    }
    /* Return MUX-PDU header data */
    int fClosing = 0;
    int muxCode = -1;
    iPduPos = iPdu;
    if (*iPduPos & 1)
    {
        fClosing = 1;
    }
    muxCode = (*iPduPos >> 1) & 0xf;
    iPduPos++;
    iObserver->MuxPduIndicate(iPduPos, (uint16)(iPduPosZeroRemoved - iPdu - 1), fClosing, muxCode);

    iPduPos = iPdu;
    return 0;
}

void Level0PduParcom::Remove0Octet(uint8 cur_byte, int num_bits_from_octet)
{
    uint8 cur_bit = 0;
    for (int bitnum = 0; bitnum < num_bits_from_octet; bitnum++)
    {
        cur_bit = (uint8)(cur_byte & 0x1);
        cur_byte >>= 1;
        iRecvByte0 = (uint8)(iRecvByte0 | (cur_bit << iRecvByte0Pos++));
        if (iRecvByte0Pos == 6)
        {
            if ((iRecvByte0 & 0x1F) == 0x1F)
            {
                //PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0,"Level0PduParcom::Remove0Octet - skipping 0\n"));
//				OSCL_ASSERT( (iRecvByte0 & 0x20) == 0);
                if ((iRecvByte0&0x20) != 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "Level0PduParcom::Remove0Octet - Bitstream error.\n"));
                }
                // write out 5 1's and skip the following 0 bit
                AppendBits0(5, 0x1F);
                iRecvByte0Pos = 0;
                iRecvByte0 = 0;
            }
            else
            {
                // write out the lsb
                AppendBits0(1, iRecvByte0&01);
                iRecvByte0Pos--;
                iRecvByte0 >>= 1;
            }

        }
    }
}
