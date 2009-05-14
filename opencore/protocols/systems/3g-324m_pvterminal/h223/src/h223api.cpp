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
#include "h223_api.h"
#include "h223.h"
const uint gMuxStuffingSz[] = {1, 2, 4, 5, 6, 0};
const uint gMuxFlagSz[] = {1, 2, 4, 2, 2, 0};
const uint gMaxStuffingSz[] = {3, 3, 5, 5, 6, 0};
const uint8 gNumOnes[] = {0, 1, 1, 2, 1, 2, 2, 3, // 0
                          1, 2, 2, 3, 2, 3, 3, 4, // 8
                          1, 2, 2, 3, 2, 3, 3, 4, // 16
                          2, 3, 3, 4, 3, 4, 4, 5, // 24
                          1, 2, 2, 3, 2, 3, 3, 4, // 32
                          2, 3, 3, 4, 3, 4, 4, 5, // 40
                          2, 3, 3, 4, 3, 4, 4, 5, // 48
                          3, 4, 4, 5, 4, 5, 5, 6, // 56
                          1, 2, 2, 3, 2, 3, 3, 4, // 64
                          2, 3, 3, 4, 3, 4, 4, 5, // 72
                          2, 3, 3, 4, 3, 4, 4, 5, // 80
                          3, 4, 4, 5, 4, 5, 5, 6, // 88
                          2, 3, 3, 4, 3, 4, 4, 5, // 96
                          3, 4, 4, 5, 4, 5, 5, 6, // 104
                          3, 4, 4, 5, 4, 5, 5, 6, // 112
                          4, 5, 5, 6, 5, 6, 6, 7, // 120
                          1, 2, 2, 3, 2, 3, 3, 4, // 128
                          2, 3, 3, 4, 3, 4, 4, 5, // 136
                          2, 3, 3, 4, 3, 4, 4, 5, // 144
                          3, 4, 4, 5, 4, 5, 5, 6, // 152
                          2, 3, 3, 4, 3, 4, 4, 5, // 160
                          3, 4, 4, 5, 4, 5, 5, 6, // 168
                          3, 4, 4, 5, 4, 5, 5, 6, // 176
                          4, 5, 5, 6, 5, 6, 6, 7, // 184
                          2, 3, 3, 4, 3, 4, 4, 5, // 192
                          3, 4, 4, 5, 4, 5, 5, 6, // 200
                          3, 4, 4, 5, 4, 5, 5, 6, // 208
                          4, 5, 5, 6, 5, 6, 6, 7, // 216
                          3, 4, 4, 5, 4, 5, 5, 6, // 224
                          4, 5, 5, 6, 5, 6, 6, 7, // 232
                          4, 5, 5, 6, 5, 6, 6, 7, // 240
                          5, 6, 6, 7, 6, 7, 7, 8
                         }; // 248

/* Delete this */
OSCL_EXPORT_REF CPVH223Multiplex* AllocateH223Mux(TPVLoopbackMode aLoopbackMode)
{
    return OSCL_NEW(CPVH223Multiplex, (aLoopbackMode));
}

OSCL_EXPORT_REF void DeallocateH223Mux(CPVH223Multiplex* h223mux)
{
    OSCL_DELETE(h223mux);
}

uint H223GetMuxStuffingSz(TPVH223Level flat_level)
{
    return gMuxStuffingSz[flat_level];
}

uint H223GetMuxFlagSz(TPVH223Level flat_level)
{
    return gMuxFlagSz[flat_level];
}

uint H223GetMaxStuffingSz(TPVH223Level flat_level)
{
    return gMaxStuffingSz[flat_level];
}

uint H223MuxStuffing(TPVH223Level level, uint8* pPdu, uint bufSz)
{
    int32 stuffing_size = (int32)H223GetMuxStuffingSz(level);
    if (stuffing_size == 0)
        return 0;

    int32 size_left = bufSz;
    while (size_left >= stuffing_size)
    {
        switch (level)
        {
            case H223_LEVEL0: // level 0
                *(pPdu) = 0x7e;
                break;
            case H223_LEVEL1: // level 1
            case H223_LEVEL1_DF: // H223DoubleFlag == TRUE
                *(pPdu) = 0xe1;
                *(pPdu + 1) = 0x4d;
                if (level == H223_LEVEL1)
                    break;
                *(pPdu + 2) = 0xe1;
                *(pPdu + 3) = 0x4d;
                break;
            case H223_LEVEL2: // level 2
            case H223_LEVEL2_OH: // H223OptionalHeader==1
                *(pPdu) = 0xe1;
                *(pPdu + 1) = 0x4d;
                *(pPdu + 2) = 0;
                *(pPdu + 3) = 0;
                *(pPdu + 4) = 0;
                if (level == H223_LEVEL2)
                    break;
                *(pPdu + 5) = 0;
                break;
            default:
                return 0;
        }
        pPdu += stuffing_size;
        size_left -= stuffing_size;
    }
    return (bufSz -size_left);
}

bool H223CheckSync(TPVH223Level level, uint8* buf, uint bufSz, uint tolerance)
{
    if (bufSz < H223GetMuxFlagSz(level))
        return false;
    uint mismatch_cnt = 0;
    switch (level)
    {
        case H223_LEVEL0:
            mismatch_cnt = gNumOnes[(buf[0] ^ 0x7E)];
            break;
        case H223_LEVEL1:
        case H223_LEVEL1_DF:
            mismatch_cnt = (uint16)(gNumOnes[(buf[0] ^ 0xE1)]  + gNumOnes[(buf[1] ^ 0x4D)]);
            if (level == H223_LEVEL1)
                break;
            mismatch_cnt += (uint16)(gNumOnes[(buf[2] ^ 0xE1)]  + gNumOnes[(buf[3] ^ 0x4D)]);
            break;
        case H223_LEVEL2_OH:
            if (bufSz >= 6)
                mismatch_cnt = (uint16)(gNumOnes[buf[5]]);
        case H223_LEVEL2:
            mismatch_cnt += (uint16)(gNumOnes[(buf[0] ^ 0xE1)]  + gNumOnes[(buf[1] ^ 0x4D)]);
            if (bufSz >= 5)
            {
                mismatch_cnt += (uint16)(gNumOnes[buf[2]]);
                mismatch_cnt += (uint16)(gNumOnes[buf[3]]);
                mismatch_cnt += (uint16)(gNumOnes[buf[4]]);
            }
            break;
        default:
            return false;
    }
    return (mismatch_cnt <= tolerance);
}
