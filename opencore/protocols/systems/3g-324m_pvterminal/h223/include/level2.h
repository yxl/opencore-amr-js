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
#if !defined(LEVEL2_H)
#define LEVEL2_H
#include "pduparcom.h"

#define PDU_HDR_SIZE_MAX 8
#define LEVEL2_STUFFING_SZ 5
#define LEVEL2OH_STUFFING_SZ 6
#define LEVEL2_HDR_SZ 5
#define LEVEL2OH_HDR_SZ 6
#define LEVEL2_THRESHOLD_SYNC 15
#define LEVEL2_THRESHOLD_DATA 13

#define LEVEL2_FLAG_SZ 2
#define LEVEL2_MAX_PDU_SZ 254
#define LEVEL2_FLAG 0xE14D
#define LEVEL2_CLOSING_FLAG 0x1EB2

class Level2PduParcom : public H223PduParcomBase
{
    public:
        typedef enum
        {
            ECopyHdr = 0,
            ECopyData
        } EOperationId;

        Level2PduParcom(bool oh = false, int closing_cur = 0);
        ~Level2PduParcom()
        {
            OSCL_DEFAULT_FREE(iEncTab);
            OSCL_DEFAULT_FREE(iDecTab);
            OSCL_DEFAULT_FREE(iBsbuf[0]);
            OSCL_DEFAULT_FREE(iBsbuf[1]);
        }
        TPVH223Level GetLevel()
        {
            if (iOh)
                return H223_LEVEL2_OH;
            return H223_LEVEL2;
        }

        bool UseOh(bool oh)
        {
            bool ret = iOh;
            iOh = oh;
            return ret;
        }

        uint32 GetStuffing(uint8* buf, uint32 buf_size, uint8 mux_code = 0);
        uint32 GetHeaderSz()
        {
            return (uint16)((iOh) ? LEVEL2OH_HDR_SZ : LEVEL2_HDR_SZ);
        }
        uint32 GetStuffingSz()
        {
            return (uint16)((iOh) ? LEVEL2OH_STUFFING_SZ : LEVEL2_STUFFING_SZ);
        }
        uint32 Parse(uint8* bsbuf, uint32 bsbsz);
        PVMFStatus CompletePdu(OsclSharedPtr<PVMFMediaDataImpl>& pdu, int8 mt, uint8 pm);
        void ResetStats();
        void LogStats(TPVDirection dir);
        void SetClosingCur(int32 closing);
    protected:
        void Construct(uint16 max_outstanding_pdus);
        void GetHdrFragment(OsclRefCounterMemFrag& frag);
    private:
        PVMFBufferPoolAllocator iHdrFragmentAlloc;

        uint8 iHecCrc[20];
        int iThreshold;
        bool iOh;
        bool iUseOh;
        int iCurrentInt32;
        int iCnt;
        int iRecoverCnt;
        int iMpl;
        int iMplRemaining;
        int iPktsInUse;
        int iMuxCode;
        int iClosingCur;
        int iClosingNext;
        int iClosingNextRx;

        uint8 iPduHdr[PDU_HDR_SIZE_MAX];
        uint8* iPduHdrPos;
        uint8* iBsbuf[2];
        unsigned iCurBsBuf;

        int iPrevMuxTblNum;
        int iPrevPm;
        EOperationId iCurrentOp;
        bool CheckFlag(uint8* buf, int* fClosing, int trsld);
        void IndicatePdu(uint8 optional_header);
        int ParseHdr(int* fClosing, int* mpl, int* muxCode, int* numErrors, uint8* optional_header);

        inline void ResetPdu()
        {
            iPduPos = iPdu;
            iMpl = 0;
            iMplRemaining = 0;
            iMuxCode = -1;
            iClosingCur = 0;
        }

        inline void ResetPduHdr()
        {
            iPduHdrPos = iPduHdr;
        }

        inline void CopyOctetToPdu(uint8 c)
        {
            if (iPduPos >= iPduEndPos)
            {
                /* Reset the pdu */
                ResetPdu();
            }
            /* Copy the byte to the pdu */
            *iPduPos++ = c;
        }

        uint8* FindSync(uint8* data, int len, int* closing);
        bool RecoverPduData(uint8*& bsbuf, int* bsbsz);

        int GolayDec(int received, int* num_errors)
        {
            received = ((received >> 1) & 0xfffff800) + (received & 0x7ff);
            int syndrome = get_syndrome(received);
            received ^= iDecTab[syndrome];
            *num_errors = iNumOnes[syndrome&0xFF] + iNumOnes[(syndrome >> 8)&0xFF] +
                          iNumOnes[(syndrome >> 16)&0xFF] + iNumOnes[(syndrome >> 24)&0xFF];
            return received >> 11;
        }

        int get_syndrome(int pattern)
        {

            //static const long gen_poly = 0x00000c75;

            long junk = 0x00400000;
            if (pattern >= 0x00000800)
            {
                while (pattern & 0xfffff800)
                {
                    while (!(junk & pattern)) junk >>= 1;
                    pattern ^= (junk + (junk >> 1) + (junk >> 5) + (junk >> 6) + (junk >> 7) + (junk >> 9) + (junk >> 11));
                }
            }
            return(pattern);
        }

        uint8 iLastHdr[8];
        int* iEncTab;
        int* iDecTab;
        uint8 iNumOnes[256];
        // Outgoing
        uint32 iNumClosingFlagsTx;
        uint32 iNumFlagEmulation;

        // Incoming
        uint32 iNumClosingFlagsRx;
        uint32 iNumGolayCblePduHdrErrorsRx;
        uint32 iNumPduHdrErrorsMplRx;
        uint32 iNumCorruptedOhRx;
        uint32 iNumHeadersCorrectedByOh;
        bool iCopyPduWithSync;
};

#endif


