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
#if !defined(LEVEL1_H)
#define LEVEL1_H
#include "pduparcom.h"

#define LEVEL1_STUFFING_SZ 2
#define LEVEL1DF_STUFFING_SZ 4
#define LEVEL1_HDR_SZ 3
#define LEVEL1DF_HDR_SZ 5

class Level1PduParcom : public H223PduParcomBase
{
    public:
        typedef enum
        {
            ELookForStartFlag = 0,
            ESkipFlags,
            ELookForEndFlag
        } EOperationId;

        Level1PduParcom(bool df = false);
        ~Level1PduParcom()
        {
        }
        TPVH223Level GetLevel()
        {
            return H223_LEVEL1;
        }

        bool UseDf(bool df)
        {
            bool ret = iDf;
            iDf = df;
            return ret;
        }
        uint32 GetStuffing(uint8* buf, uint32 buf_size, uint8 mux_code = 0);
        uint32 GetHeaderSz()
        {
            return (uint16)((iDf) ? LEVEL1DF_HDR_SZ : LEVEL1_HDR_SZ);
        }
        uint32 GetStuffingSz()
        {
            return (uint16)((iDf) ? LEVEL1DF_STUFFING_SZ : LEVEL1_STUFFING_SZ);
        }
        uint32 Parse(uint8* bsbuf, uint32 bsbsz);
        PVMFStatus CompletePdu(OsclSharedPtr<PVMFMediaDataImpl>& pdu, int8 mt, uint8 pm);
    protected:
        void Construct(uint16 max_outstanding_pdus);
        void GetHdrFragment(OsclRefCounterMemFrag& frag);
    private:
        inline bool CheckForFlag(uint8* bsbuf)
        {
            if (iDf)
                return (bsbuf[0] == 0xe1 && bsbuf[1] == 0x4d && bsbuf[2] == 0xe1 && bsbuf[3] == 0x4d);
            return (bsbuf[0] == 0xe1 && bsbuf[1] == 0x4d);
        }
        inline void IndicatePdu();

        PVMFBufferPoolAllocator iHdrFragmentAlloc;

        uint8 iHecCrc[20];
        bool iDf;
        int iCurrentInt32;
        int iSkipBytes;
        int iClosingNext;
        EOperationId iCurrentOp;
};

#endif

