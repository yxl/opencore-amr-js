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
#include "oscl_types.h"
#include "sbc.h"
#include "sbcenc_filter.h"
#include "oscl_mem.h"


/*$F
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *    A2DP specification Adopted version 1.0: Appendix B, Section 12.8. Page 70
 *    proto_4_40 and proto_8_80 tables are prototype filter coefficients for
 *    analysis and synthesis filters
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */


static const Word32  sbc_proto_4_40[40] =
{
    0 ,			576115 ,    1601898 ,   2935298 ,
    4120164,	4179059 ,   2003406 ,  -3285782 ,
    11718563, 	21945682 ,	31005088 ,  34567968 ,
    27784982, 	6584670 ,  -30947094 , -83372136 ,
    145592176,	209366608 , 264824096 , 302610720 ,
    316018688,	302610720 , 264824096 , 209366608 ,
    -145592176, -83372136 , -30947094 ,  6584670 ,
    27784982, 	34567968 ,  31005088 ,  21945682 ,
    -11718563,  -3285782 ,   2003406 ,   4179059 ,
    4120164, 	2935298 ,   1601898 ,   576115
};

static const Word32 sbc_proto_8_80[80] =
{
    0 ,			168122 ,    368569 ,    595519 ,    884677 ,
    1223985 ,   1585274 ,   1915252 ,   2160181 ,   2258852 ,
    2141627 , 	1735771 ,   968681 ,   -191991 ,   -1771385 ,
    -3755063 , 	6076836 ,   8621515 ,   11229669 ,  13687238 ,
    15733030 , 	17077392 ,  17417002 ,  16448018 ,  13891192 ,
    9510749 , 	3139712 ,  -5278279 ,  -15720018 , -28035266 ,
    -41956612 , -57109432 ,  73013312 ,  89104208 ,  104770776 ,
    119396536 , 132354304,  143091584 , 151132928 , 156111152 ,
    157791808 , 156111152 , 151132928 , 143091584 , 132354304 ,
    119396536 , 104770776 , 89104208 , -73013312 , -57109432 ,
    -41956612 , -28035266 , -15720018 , -5278279 ,   3139712 ,
    9510749 ,   13891192 ,  16448018 ,  17417002 ,  17077392 ,
    15733030 , 	13687238 ,  11229669 ,  8621515 ,  -6076836 ,
    -3755063 ,  -1771385 ,  -191991 ,    968681 ,    1735771 ,
    2141627 ,   2258852 ,   2160181 ,   1915252 ,   1585274 ,
    1223985 , 	884677 ,    595519 ,    368569 ,    168122
};

/*$F
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *    A2DP specification Adopted version 1.0: Appendix B, Section 12.7.1 . Page 67-68
 *
 *    SBC Analysis matrix for 4 subbands
 *
 *                         /      1           4      pi \
 *       M_4x8[i][k] = cos| (i + ---) × (k - ---) × ---  |
 *                         \      2           2      4  /
 *
 *    SBC Analysis matrix for 8 subbands
 *
 *                          /      1           8      pi \
 *       M_8x16[i][k] = cos| (i + ---) × (k - ---) × ---  |
 *                          \      2           2      8  /
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
//Q30 format for the table
static const Word32 M_8x16[11] =
{
    759250112,  410903200,  992008064,  892783680,
    596539008,  209476640, 1053110144, -209476640,
    -1053110144, -596539008,  892783680
};

/*$F
 ===============================================================================
 *    A2DP specification Adopted version 1.0: Appendix B, Section 12.7.1 Page 67-68
 *    Analysis Filter
 *
 *  ___________________________
 *  SBC ANALYSIS FOR 8 SUBBANDS
 *  ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 *   ____
 *  |    |  Input 8 new audio samples
 *   ¯¯¯¯
 *    |
 *    V
 *   _________________________________________________
 *  | »» | »» | »» |  X-FIFO    ···    | »» | »» | »» |
 *   ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 *  0  |  8 |    |                        |    |    |79
 *     |    |    |                        |    |    |
 *     |    |    |                        |    |    |
 *     V    V    V                        V    V    V
 *   _________________________________________________
 *  |              sbc_proto_8_80                     |
 *   ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 *  0  |    |    |                        |    |    |79
 *     |    |    |                        |    |    |
 *     |    |    |                        |    |    |
 *     V    V    V                        V    V    V
 *   _________________________________________________
 *  |        |        |    Z-vector   ···    |        |  Partial calculation
 *   ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 *  0 |     15 |                               |
 *    |        |                               |
 *    V        V                               V
 *    _        _        _                      _        _
 *   | |0     | |      | |                    | |      | |0
 *   | |   +  | |   +  | |        ···         | |  =   | |
 *   | |      | |      | |                    | |      | | S-Vector
 *   | |15    | |      | |                    | |      | |15
 *    ¯        ¯        ¯                      ¯        ¯
 *                                                      |
 *                                                      |
 *                   _            __________            |
 * next 8 output    | |          |          |           |
 * subband samples  | |<---------|          | <---------·
 *                  | |          | M[i][k]  |
 *                  | |          |          |  i = 0 to 7
 *                   ¯           |          |  k = 0 to 15
 *                                ¯¯¯¯¯¯¯¯¯¯
 *
 ===============================================================================
 */
/*
 ===============================================================================
 *    analysis filter bank
 ===============================================================================
 */

void analysis_filter_4(analysis_filter_t *filter, sbc_t *sbc)
{
    Int *ptr, *ptr1, t_var2, t_var1, arr_tmp[16], ch, blk, i, *X_ptr;
    Int  t_var4, tmp1, tmp2, tmp3, tmp4;
    const Word32 *ptr2;
    const Word32 *ptr3;

    for (ch = 0; ch < sbc->channels; ch++)
    {
        X_ptr = &filter->X[ch][60];
        for (blk = 0; blk < sbc->blocks; blk++)
        {
            ptr1 = X_ptr;

            ptr2 = &sbc_proto_4_40[0];
            ptr = &arr_tmp[0];

            for (i = 4; i != 0; i --)
            {
                tmp1 = ptr2[0];
                tmp2 = ptr2[1];
                tmp3 = ptr1[0];
                tmp4 = ptr1[1];

                t_var2 = FMULT(tmp1 , tmp3);
                t_var1 = FMULT(tmp2 , tmp4);

                tmp1 = ptr2[8];
                tmp2 = ptr2[9];
                tmp3 = ptr1[8];
                tmp4 = ptr1[9];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                tmp1 = ptr2[16];
                tmp2 = ptr2[17];
                tmp3 = ptr1[16];
                tmp4 = ptr1[17];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                tmp1 = ptr2[24];
                tmp2 = ptr2[25];
                tmp3 = ptr1[24];
                tmp4 = ptr1[25];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                tmp1 = ptr2[32];
                tmp2 = ptr2[33];
                tmp3 = ptr1[32];
                tmp4 = ptr1[33];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                *ptr++ = t_var2;
                *ptr++ = t_var1;

                ptr2 += 2;
                ptr1 += 2;

            }

            ptr -= 8;
            ptr3 = M_8x16;

            tmp1 = ptr[0] + ptr[4];
            tmp2 = ptr[1] + ptr[3];
            tmp3 = ptr[5] - ptr[7];

            t_var4  = FMULT_1(tmp1, *ptr3++);
            t_var2  = ptr[2] + t_var4;
            t_var4  = ptr[2] - t_var4;

            tmp4     = FMULT_1(tmp3, *ptr3);
            tmp1     = FMULT_1(tmp2, *ptr3++);
            tmp4    += FMULT_1(tmp2, *ptr3);
            tmp1    -= FMULT_1(tmp3, *ptr3);

            sbc->sb_sample[blk][ch][0]  = t_var2 + tmp4;
            sbc->sb_sample[blk][ch][3]  = t_var2 - tmp4;
            sbc->sb_sample[blk][ch][1]  = t_var4 + tmp1;
            sbc->sb_sample[blk][ch][2]  = t_var4 - tmp1;

            X_ptr -= 4;
        }
    }

    tmp2 = 64 - (sbc->blocks << 2);
    for (ch = 0; ch < sbc->channels; ch++)
    {
        ptr =  &filter->X[ch][tmp2];
        ptr1 = &filter->X[ch][64];

        oscl_memmove(ptr1, ptr, 36 * sizeof(Int));
    }

}

void analysis_filter_8(analysis_filter_t *filter, sbc_t *sbc)
{
    Int *ptr, *ptr1, t_var2, t_var1, arr_tmp[16], ch, blk, i, *X_ptr;
    Int t_var3;
    Int t_var4;
    Int t_var5;
    Int t_var6;
    Int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    const Word32 *ptr2;
    const Word32 *ptr3;

    for (ch = 0; ch < sbc->channels; ch++)
    {
        X_ptr = &filter->X[ch][120];
        for (blk = 0; blk < sbc->blocks; blk++)
        {
            ptr = X_ptr;

            /* Windowing by 80 coefficients */
            ptr1 = &arr_tmp[0];
            ptr2 = &sbc_proto_8_80[0];

            /* Partial calculation */
            for (i = 8; i != 0; i--)
            {

                tmp1 = ptr2[0];
                tmp2 = ptr2[1];
                tmp3 = ptr[0];
                tmp4 = ptr[1];

                t_var2  = FMULT(tmp1 , tmp3);
                t_var1  = FMULT(tmp2 , tmp4);

                tmp1 = ptr2[16];
                tmp2 = ptr2[17];
                tmp3 = ptr[16];
                tmp4 = ptr[17];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                tmp1 = ptr2[32];
                tmp2 = ptr2[33];
                tmp3 = ptr[32];
                tmp4 = ptr[33];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                tmp1 = ptr2[48];
                tmp2 = ptr2[49];
                tmp3 = ptr[48];
                tmp4 = ptr[49];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                tmp1 = ptr2[64];
                tmp2 = ptr2[65];
                tmp3 = ptr[64];
                tmp4 = ptr[65];

                t_var2 += FMULT(tmp1 , tmp3);
                t_var1 += FMULT(tmp2 , tmp4);

                ptr2 += 2;
                ptr += 2 ;

                *ptr1++ = t_var2;
                *ptr1++ = t_var1;
            }
            /* Calculate 8 subband samples by Matrixing */
            ptr = & sbc->sb_sample[blk][ch][0];
            ptr3 = M_8x16;
            ptr1 -= 16;

            tmp1 = ptr1[ 0] + ptr1[ 8];
            tmp3 = ptr1[ 2] + ptr1[ 6];
            tmp6 = ptr1[10] - ptr1[14];


            t_var5  = FMULT_1(tmp1, *ptr3++);
            t_var2  = ptr1[4] + t_var5 ;
            t_var5  = ptr1[4] - t_var5;
            t_var3  = t_var2;
            t_var6  = t_var5;
            tmp8    = FMULT_1(tmp6, *ptr3);
            tmp1    = FMULT_1(tmp3, *ptr3++);
            tmp8   += FMULT_1(tmp3, *ptr3);
            tmp1   -= FMULT_1(tmp6, *ptr3++);
            t_var2 += tmp8;
            t_var3 -= tmp8;
            t_var5 += tmp1;
            t_var6 -= tmp1;

            tmp2 = ptr1[ 1] + ptr1[ 7];
            tmp5 = ptr1[ 9] - ptr1[15];

            t_var1  = -FMULT_1(tmp2, *ptr3);
            t_var4  = -FMULT_1(tmp5, *ptr3++);
            t_var1 -=  FMULT_1(tmp5, *ptr3);
            t_var4 +=  FMULT_1(tmp2, *ptr3++);

            tmp7 = ptr1[11] - ptr1[13];
            tmp4 = ptr1[ 3] + ptr1[ 5];

            t_var1 -=  FMULT_1(tmp7, *ptr3);
            t_var4 -=  FMULT_1(tmp4, *ptr3++);
            t_var1 -=  FMULT_1(tmp4, *ptr3);
            t_var4 +=  FMULT_1(tmp7, *ptr3++);

            ptr[0]  = t_var2 - t_var1;
            ptr[7]  = t_var2 + t_var1;
            ptr[3]  = t_var3 - t_var4;
            ptr[4]  = t_var3 + t_var4;


            t_var1  = -FMULT_1(tmp2, *ptr3);
            t_var4  =  FMULT_1(tmp5, *ptr3++);
            t_var1 -=  FMULT_1(tmp5, *ptr3);
            t_var4 -=  FMULT_1(tmp2, *ptr3++);

            t_var1 -=  FMULT_1(tmp7, *ptr3);
            t_var4 +=  FMULT_1(tmp4, *ptr3++);
            t_var1 -=  FMULT_1(tmp4, *ptr3);
            t_var4 -=  FMULT_1(tmp7, *ptr3);

            ptr[1]  = t_var5 - t_var1;
            ptr[6]  = t_var5 + t_var1;
            ptr[2]  = t_var6 - t_var4;
            ptr[5]  = t_var6 + t_var4;


            X_ptr -= 8;
        }
    }

    tmp2 = 128 - (sbc->blocks << 3);
    for (ch = 0; ch < sbc->channels; ch++)
    {
        ptr =  &filter->X[ch][tmp2];
        ptr1 = &filter->X[ch][128];

        oscl_memmove(ptr1, ptr, 72 * sizeof(Int));
    }

}
