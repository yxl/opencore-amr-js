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
#if !defined(LEVEL0_H)
#define LEVEL0_H
#include "pduparcom.h"

#define LEVEL0_STUFFING_SZ 1
#define LEVEL0_HDR_SZ 2

class Level0PduParcom : public H223PduParcomBase
{
    public:
        Level0PduParcom();
        TPVH223Level GetLevel()
        {
            return H223_LEVEL0;
        }
        uint32 GetStuffing(uint8* buf, uint32 buf_size, uint8 mux_code = 0);
        uint32 GetHeaderSz()
        {
            return LEVEL0_HDR_SZ;
        }
        uint32 GetStuffingSz()
        {
            return LEVEL0_STUFFING_SZ;
        }
        uint32 Parse(uint8* bsbuf, uint32 bsbsz);
        PVMFStatus CompletePdu(OsclSharedPtr<PVMFMediaDataImpl>& pdu, int8 mt, uint8 pm);

    protected:
        void Construct(uint16 max_outstanding_pdus);
        void GetHdrFragment(OsclRefCounterMemFrag& frag);
        uint16 Insert0(uint8* pPdu, int pdu_size, uint8* pRetPdu);
        uint16 Insert0Octet(uint8 cur_byte, uint16 num_bits_from_octet, uint8* out_buffer);
        uint16 AppendTxBits0(unsigned num_bits, int bits, uint8* pos);
        // appends an octet to the output stream without checking for 0 bit.  Flushes the current check bits
        uint16 AppendTxOctet(uint8 octet, uint8* pos);

        void SearchForHdlcFlag(uint8* bsbuf, int bsbsz);
        unsigned Remove0();
        void Remove0Octet(uint8 cur_byte, int num_bits_from_octet);
        void AppendBit(int c);
        void AppendBits0(unsigned num_bits, int bits);

        int iPmNext;
        PVMFBufferPoolAllocator iPduFragmentAlloc;
        PVMFBufferPoolAllocator iHdrFragmentAlloc;

        uint8 iHecCrc[20];
        unsigned iRecvBits;
        unsigned iNumRecvBits;
        unsigned iRecvByte;
        int iRecvBytePos;
        unsigned iRecvByte0;
        int iRecvByte0Pos;
        int iZeroBitRemovedOutputPos;
        unsigned iZeroBitRemovedOutput;
        uint8* iPduPosZeroRemoved;

        int iTxByte0;
        int iTxByte0Pos;
        int iTxCheck0;
        int iTxCheck0Pos;
};

#endif


