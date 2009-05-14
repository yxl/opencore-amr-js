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
#include "sbc_encoder.h"
#include "sbcenc_allocation.h"

/*$F
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *    A2DP specification Adopted version 1.0: Appendix B, Section 12.8, Page 69
 *    offset4 and offset8 tables for AM_LOUDNESS bit allocation method
 *
 *    Offset table for four subbands
 *    -----------------------------------------------------------
 *    offset4 | fs = 16000 | fs = 32000 | fs = 44100 | fs = 48000
 *    -----------------------------------------------------------
 *    sb = 0  | -1         | -2         | -2         | -2
 *    sb = 1  |  0         |  0         |  0         |  0
 *    sb = 2  |  0         |  0         |  0         |  0
 *    sb = 3  |  0         |  1         |  1         |  1
 *    -----------------------------------------------------------
 *
 *    Offset table for eight subbands
 *    -----------------------------------------------------------
 *    offset8 | fs = 16000 | fs = 32000 | fs = 44100 | fs = 48000
 *    -----------------------------------------------------------
 *    sb = 0  | -2         | -3         | -4         | -4
 *    sb = 1  |  0         |  0         |  0         |  0
 *    sb = 2  |  0         |  0         |  0         |  0
 *    sb = 3  |  0         |  0         |  0         |  0
 *    sb = 4  |  0         |  0         |  0         |  0
 *    sb = 5  |  0         |  0         |  0         |  0
 *    sb = 6  |  0         |  1         |  1         |  1
 *    sb = 7  |  1         |  2         |  2         |  2
 *    -----------------------------------------------------------
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
static const Int    sbc_offset4[4][4] =
{
    { -1, 0, 0, 0 },
    { -2, 0, 0, 1 },
    { -2, 0, 0, 1 },
    { -2, 0, 0, 1 }
};

static const Int    sbc_offset8[4][8] =
{
    { -2, 0, 0, 0, 0, 0, 0, 1 },
    { -3, 0, 0, 0, 0, 0, 1, 2 },
    { -4, 0, 0, 0, 0, 0, 1, 2 },
    { -4, 0, 0, 0, 0, 0, 1, 2 }
};

void derive_allocation(const sbc_t * sbc, Int bits[2][8])
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    const UInt  sf = sbc->sf_index;
    Int	  t_var;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (sbc->channel_mode == CM_MONO || sbc->channel_mode == CM_DUAL_CHANNEL)
    {
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        Int bitneed[2][8], loudness, max_bitneed, bitcount, slicecount, bitslice;
        Int ch, sb, tmp1, tmp2, tmp3, tmp4;
        Int *ptr1;
        const UWord32 *ptr2;
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        for (ch = 0; ch < sbc->channels; ch++)
        {
            ptr1 = &bitneed[ch][0];
            ptr2 = &sbc->scale_factor[ch][0];
            if (sbc->allocation_method == AM_SNR)
            {
                for (sb = sbc->subbands >> 2; sb != 0; sb--)
                {
                    tmp1 = *ptr2++;
                    tmp2 = *ptr2++;
                    tmp3 = *ptr2++;
                    tmp4 = *ptr2++;

                    *ptr1++ = tmp1;
                    *ptr1++ = tmp2;
                    *ptr1++ = tmp3;
                    *ptr1++ = tmp4;
                }
            }

            else
            {
                for (sb = 0; sb < sbc->subbands; sb++)
                {
                    if (sbc->scale_factor[ch][sb] == 0)
                    {
                        bitneed[ch][sb] = -5;
                    }
                    else
                    {
                        if (sbc->subbands == 4)
                        {
                            loudness = sbc->scale_factor[ch][sb] - sbc_offset4[sf][sb];
                        }
                        else
                        {
                            loudness = sbc->scale_factor[ch][sb] - sbc_offset8[sf][sb];
                        }

                        if (loudness > 0)
                        {
                            bitneed[ch][sb] = loudness >> 1;
                        }
                        else
                        {
                            bitneed[ch][sb] = loudness;
                        }
                    }
                }
            }

            max_bitneed = 0;
            for (sb = 0; sb < sbc->subbands; sb++)
            {
                if (bitneed[ch][sb] > max_bitneed) max_bitneed = bitneed[ch][sb];
            }

            bitcount = 0;
            slicecount = 0;
            bitslice = max_bitneed + 1;
            do
            {
                bitslice--;
                bitcount += slicecount;
                slicecount = 0;
                for (sb = 0; sb < sbc->subbands; sb++)
                {
                    t_var = bitneed[ch][sb];
                    if ((t_var > bitslice + 1) && (t_var < bitslice + 16))
                    {
                        slicecount++;
                    }
                    else if (t_var == bitslice + 1)
                    {
                        slicecount += 2;
                    }
                }
            }
            while (bitcount + slicecount < sbc->bitpool);

            if (bitcount + slicecount == sbc->bitpool)
            {
                bitcount += slicecount;
                bitslice--;
            }

            for (sb = 0; sb < sbc->subbands; sb++)
            {
                if (bitneed[ch][sb] < bitslice + 2)
                {
                    bits[ch][sb] = 0;
                }
                else
                {
                    bits[ch][sb] = bitneed[ch][sb] - bitslice;
                    if (bits[ch][sb] > 16) bits[ch][sb] = 16;
                }
            }

            sb = 0;
            while (bitcount < sbc->bitpool && sb < sbc->subbands)
            {
                t_var = bits[ch][sb];
                if ((t_var >= 2) && (t_var < 16))
                {
                    t_var++;
                    bitcount++;
                }
                else if ((bitneed[ch][sb] == bitslice + 1) && (sbc->bitpool > bitcount + 1))
                {
                    t_var = 2;
                    bitcount += 2;
                }
                bits[ch][sb] = t_var;
                sb++;
            }

            sb = 0;
            while (bitcount < sbc->bitpool && sb < sbc->subbands)
            {
                if (bits[ch][sb] < 16)
                {
                    bits[ch][sb]++;
                    bitcount++;
                }

                sb++;
            }
        }

    }
    else if (sbc->channel_mode == CM_STEREO || sbc->channel_mode == CM_JOINT_STEREO)
    {
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        Int bitneed[2][8], loudness, max_bitneed, bitcount, slicecount, bitslice;
        Int ch, sb, tmp1, tmp2, tmp3, tmp4, tmp;
        Int * ptr1, *ptr3, *ptr4, *ptr5;
        const UWord32 * ptr2;
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        max_bitneed = 0;

        if (sbc->allocation_method == AM_SNR)
        {
            tmp = sbc->subbands >> 2;
            ptr1 =  &bitneed[0][0];
            ptr2 =  &sbc->scale_factor[0][0];

            for (sb = tmp; sb != 0; sb--)
            {
                tmp1 = *ptr2++;
                tmp2 = *ptr2++;
                tmp3 = *ptr2++;
                tmp4 = *ptr2++;

                *ptr1++ = tmp1;
                *ptr1++ = tmp2;
                *ptr1++ = tmp3;
                *ptr1++ = tmp4;

                if (tmp1 > max_bitneed) max_bitneed = tmp1;
                if (tmp2 > max_bitneed) max_bitneed = tmp2;
                if (tmp3 > max_bitneed) max_bitneed = tmp3;
                if (tmp4 > max_bitneed) max_bitneed = tmp4;
            }

            ptr1 =  &bitneed[1][0];
            ptr2 =  &sbc->scale_factor[1][0];

            for (sb = tmp; sb != 0; sb--)
            {
                tmp1 = *ptr2++;
                tmp2 = *ptr2++;
                tmp3 = *ptr2++;
                tmp4 = *ptr2++;

                *ptr1++ = tmp1;
                *ptr1++ = tmp2;
                *ptr1++ = tmp3;
                *ptr1++ = tmp4;

                if (tmp1 > max_bitneed) max_bitneed = tmp1;
                if (tmp2 > max_bitneed) max_bitneed = tmp2;
                if (tmp3 > max_bitneed) max_bitneed = tmp3;
                if (tmp4 > max_bitneed) max_bitneed = tmp4;
            }

        }
        else
        {
            for (ch = 0; ch < 2; ch++)
            {
                for (sb = 0; sb < sbc->subbands; sb++)
                {
                    if (sbc->scale_factor[ch][sb] == 0)
                    {
                        bitneed[ch][sb] = -5;
                    }
                    else
                    {
                        if (sbc->subbands == 4)
                        {
                            loudness = sbc->scale_factor[ch][sb] - sbc_offset4[sf][sb];
                        }
                        else
                        {
                            loudness = sbc->scale_factor[ch][sb] - sbc_offset8[sf][sb];
                        }

                        if (loudness > 0)
                        {
                            bitneed[ch][sb] = loudness >> 1;
                        }
                        else
                        {
                            bitneed[ch][sb] = loudness;
                        }
                    }

                    if (bitneed[ch][sb] > max_bitneed) max_bitneed = bitneed[ch][sb];
                }
            }
        }


        bitcount = 0;
        slicecount = 0;
        bitslice = max_bitneed + 1;

        do
        {
            bitslice--;
            bitcount += slicecount;
            slicecount = 0;

            ptr1 = &bitneed[0][0];
            ptr3 = &bitneed[1][0];
            for (sb = sbc->subbands; sb != 0; sb--)
            {
                t_var = *ptr1++;
                tmp1 = *ptr3++;

                if ((t_var > bitslice + 1) && (t_var < bitslice + 16))
                    slicecount++;
                else if (t_var == bitslice + 1)
                    slicecount += 2;

                if ((tmp1 > bitslice + 1) && (tmp1 < bitslice + 16))
                    slicecount++;

                else if (tmp1 == bitslice + 1)
                    slicecount += 2;
            }

        }
        while (bitcount + slicecount < sbc->bitpool);


        if (bitcount + slicecount == sbc->bitpool)
        {
            bitcount += slicecount;
            bitslice--;
        }

        ptr1 = &bitneed[0][0];
        ptr3 = &bits[0][0];

        ptr4 = &bitneed[1][0];
        ptr5 = &bits[1][0];
        for (sb = sbc->subbands; sb != 0 ; sb--)
        {
            tmp1 = *ptr1++;
            tmp2 = *ptr3;

            if (tmp1 < bitslice + 2)
                tmp2 = 0;
            else
            {
                tmp2 = tmp1 - bitslice;
                if (tmp2 > 16) tmp2 = 16;
            }
            *ptr3++ = tmp2;

            tmp1 = *ptr4++;
            tmp2 = *ptr5;

            if (tmp1 < bitslice + 2)
                tmp2 = 0;
            else
            {
                tmp2 = tmp1 - bitslice;
                if (tmp2 > 16) tmp2 = 16;
            }
            *ptr5++ = tmp2;
        }

        for (sb = 0; (sb < sbc->subbands) && (bitcount < sbc->bitpool) ; sb++)
        {
            t_var = bits[0][sb];
            if ((t_var >= 2) && (t_var < 16))
            {
                t_var++;
                bitcount++;
            }
            else if ((bitneed[0][sb] == bitslice + 1) && (sbc->bitpool > bitcount + 1))
            {
                t_var = 2;
                bitcount += 2;
            }
            bits[0][sb] = t_var;

            if (bitcount < sbc->bitpool)
            {
                t_var = bits[1][sb];
                if ((t_var >= 2) && (t_var < 16))
                {
                    t_var++;
                    bitcount++;
                }
                else if ((bitneed[1][sb] == bitslice + 1) && (sbc->bitpool > bitcount + 1))
                {
                    t_var = 2;
                    bitcount += 2;
                }
                bits[1][sb] = t_var;
            }

        }

        for (sb = 0; (sb < sbc->subbands) && (bitcount < sbc->bitpool) ; sb++)
        {
            for (ch = 0; (ch < 2 &&  bitcount < sbc->bitpool) ; ch++)
            {
                if (bits[ch][sb] < 16)
                {
                    bits[ch][sb]++;
                    bitcount++;
                }
            }
        }
    }
}

