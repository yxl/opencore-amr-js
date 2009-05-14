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
#include "scalefactors.h"

/*
 ===============================================================================
 *
 ===============================================================================
 */
void compute_scalefactors(enc_state_t *state)
{
    Int      sb, blk;           /* channel, subband, block and bit counters */
    sbc_t    *sbc = &state->sbc;
    UWord32  tmp1, tmp2, tmp3, tmp4, *ptr1, *ptr3;

    if (sbc->channels == 1)
    {
        ptr1 = &sbc->scale_factor[0][0];

        for (sb = 0; sb < sbc->subbands; sb++)
        {
            tmp1 = 0;
            tmp2 = 2;
            for (blk = 0; blk < sbc->blocks ; blk += 4)
            {
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk+1][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk+2][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk+3][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
            }

            *ptr1++ = tmp1;
        }
    }
    else
    {
        ptr1 = &sbc->scale_factor[0][0];
        ptr3 = &sbc->scale_factor[1][0];

        for (sb = 0; sb < sbc->subbands; sb++)
        {
            tmp1 = 0;
            tmp2 = 2;

            tmp3 = 0;
            tmp4 = 2;
            for (blk = 0; blk < sbc->blocks ; blk += 4)
            {
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk+1][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk+2][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp2 <= (UWord32)(fabs(sbc->sb_sample[blk+3][0][sb]) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }
                while (tmp4 <= (UWord32)(fabs(sbc->sb_sample[blk][1][sb]) >> 15))
                {
                    tmp3++;
                    tmp4 <<= 1;
                }
                while (tmp4 <= (UWord32)(fabs(sbc->sb_sample[blk+1][1][sb]) >> 15))
                {
                    tmp3++;
                    tmp4 <<= 1;
                }
                while (tmp4 <= (UWord32)(fabs(sbc->sb_sample[blk+2][1][sb]) >> 15))
                {
                    tmp3++;
                    tmp4 <<= 1;
                }
                while (tmp4 <= (UWord32)(fabs(sbc->sb_sample[blk+3][1][sb]) >> 15))
                {
                    tmp3++;
                    tmp4 <<= 1;
                }
            }

            *ptr1++ = tmp1;
            *ptr3++ = tmp3;
        }

    }

    /********************************************************************************/
    //Optimized code

    if (sbc->channel_mode == CM_JOINT_STEREO)
    {
        Int		sb_sample_j_0[16], sb_sample_j_1[16];
        UWord32 tmp1, tmp2, tmp3, tmp4;
        Int		*ptr1, *ptr2, tmp5, tmp6;

        sbc->join = 0;
        for (sb = 0; sb < sbc->subbands - 1; sb++)
        {
            tmp1 = 0;
            tmp2 = 2;
            tmp3 = 0;
            tmp4 = 2;

            ptr1 = &sb_sample_j_0[0];
            ptr2 = &sb_sample_j_1[0];
            for (blk = 0; blk < sbc->blocks; blk++)
            {
                tmp5 = (sbc->sb_sample[blk][0][sb] + sbc->sb_sample[blk][1][sb]) >> 1;
                tmp6 = (sbc->sb_sample[blk][0][sb]  - sbc->sb_sample[blk][1][sb]) >> 1 ;

                while (tmp2 <= (UWord32)(fabs(tmp5) >> 15))
                {
                    tmp1++;
                    tmp2 <<= 1;
                }

                while (tmp4 <= (UWord32)(fabs(tmp6) >> 15))
                {
                    tmp3++;
                    tmp4 <<= 1;
                }
                *ptr1++ = tmp5;
                *ptr2++ = tmp6;
            }

            ptr1 -= sbc->blocks;
            ptr2 -= sbc->blocks;

            if ((sbc->scale_factor[0][sb] + sbc->scale_factor[1][sb]) > (tmp1 + tmp3))
            {
                sbc->join |= 1 << sb;
                sbc->scale_factor[0][sb] = tmp1;
                sbc->scale_factor[1][sb] = tmp3;

                for (blk = 0; blk < sbc->blocks ; blk += 2)
                {
                    tmp2 = *ptr1++;
                    tmp4 = *ptr1++;
                    tmp5 = *ptr2++;
                    tmp6 = *ptr2++;

                    sbc->sb_sample[blk][0][sb] = tmp2;
                    sbc->sb_sample[blk][1][sb] = tmp5;

                    sbc->sb_sample[blk+1][0][sb] = tmp4;
                    sbc->sb_sample[blk+1][1][sb] = tmp6;
                }
            }
        }
    }
}
