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
#include "sbcenc_crc8.h"
#include "oscl_mem.h"
#include "sbcenc_filter.h"
#include "sbc_encoder.h"
#include "sbcenc_bitstream.h"


Int pack_bitstream(UWord8 *data, enc_state_t *state, UInt len)
{
    Int ch, sb, blk, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, t_var1, t_var2, temp ;
    Int *ptr1, *ptr2, levels[16], audio_samp;
    sbc_t *sbc;
    crc_t *crc;

    sbc = &state->sbc;
    crc = &state->crc;

    oscl_memset(crc, 0, sizeof(crc_t));
    oscl_memset(data, 0, len);

    tmp0 = (sbc->sf_index << 6);

    switch (sbc->blocks)
    {
        case 8:
            tmp0 |= 16;
            break;

        case 12:
            tmp0 |= 32;
            break;

        case 16:
            tmp0 |= 48;
            break;
    }

    tmp0 |= (sbc->channel_mode & 0x03) << 2;
    tmp0 |= (sbc->allocation_method & 0x01) << 1;
    tmp2 = sbc->subbands;

    if (tmp2 == 8)
        tmp0 |= 0x01;

    data[0] = 0x9C;
    data[1] = tmp0;
    data[2] = sbc->bitpool;
    data[3] = 0x00;

    sbc->bitpointer = 32;
    crc->crc_buffer[0] = data[1];
    crc->crc_buffer[1] = data[2];
    crc->crc_consumed = 16;

    if (sbc->channel_mode == CM_JOINT_STEREO)
    {
        if ((len << 3) < (UInt)(32 + tmp2))
            return(-1);
        else
        {
            tmp0 = sbc->join;
            tmp1 = 0;

            for (sb = 0; sb < tmp2 - 1  ; sb ++)
                tmp1 |= ((tmp0 >> sb) & 0x01) << (7 - sb);

            if (tmp2 == 4)
                crc->crc_buffer[2] = tmp1 & 0xf0;
            else
                crc->crc_buffer[2] = tmp1;

            sbc->bitpointer = tmp2 + 32;
            crc->crc_consumed += tmp2;
        }

        data[4] = tmp1;
    }

    tmp0 = crc->crc_consumed;
    tmp1 = sbc->bitpointer;

    if ((len << 3) < (UInt)(tmp1 + ((tmp2 * sbc->channels) << 2)))
        return -1;
    else
    {
        for (ch = 0; ch < sbc->channels; ch ++)
        {
            for (sb = 0; sb < tmp2; sb ++)
            {
                tmp3 = sbc->scale_factor[ch][sb];
                tmp3 &= 0x0f;

                tmp4 = tmp1 & 0x07;
                tmp4 = 4 - tmp4;
                tmp4 = tmp3 << tmp4;
                tmp5 = tmp1 >> 3;
                data [tmp5] |= tmp4;

                crc->crc_buffer[tmp0 >> 3] |= (tmp3 << (4 - (tmp0 & 0x07)));

                tmp1 += 4;
                tmp0 += 4;
            }
        }
    }
    sbc->bitpointer = tmp1;
    crc->crc_consumed = tmp0;

    data[3] = crc8(crc->crc_buffer, crc->crc_consumed);

    ptr1 = &levels[0];
    for (ch = 0; ch < sbc->channels; ch++)
    {
        ptr2 = &sbc->bits[ch][0];

        for (sb = (tmp2 >> 2); sb != 0 ; sb--)
        {
            tmp1 = *ptr2++;
            tmp3 = *ptr2++;

            *ptr1++ = (1 << tmp1) - 1;
            *ptr1++ = (1 << tmp3) - 1;

            tmp4 = *ptr2++;
            tmp5 = *ptr2++;
            *ptr1++ = (1 << tmp4) - 1;
            *ptr1++ = (1 << tmp5) - 1;
        }
    }

    t_var1 = sbc->bitpointer;
    for (blk = 0; blk < sbc->blocks; blk++)
    {
        ptr1 = &levels[0];
        for (ch = 0; ch < sbc->channels; ch++)
        {
            for (sb = 0; sb < sbc->subbands; sb++)
            {
                tmp1 = *ptr1++;

                if (tmp1 > 0)
                {
                    //Changed the order of multiplication & right shifting

                    temp = (sbc->sb_sample[blk][ch][sb] >> 1);
                    tmp5 = (32768 << sbc->scale_factor[ch][sb]);
                    audio_samp = FMULT_2((temp + tmp5), tmp1) ;
                    audio_samp = audio_samp >> sbc->scale_factor[ch][sb] ;

                    t_var2 = sbc->bits[ch][sb];

                    if (8 - (t_var1 & 0x7) >= t_var2)
                    {
                        data[t_var1 >> 3] |= audio_samp << (8 - (t_var1 & 0x7) - t_var2);
                        t_var1 += t_var2;
                    }
                    else if (16 - (t_var1 & 0x7) >= t_var2)
                    {
                        data[t_var1 >> 3] |= audio_samp >> (t_var2 + (t_var1 & 0x7) - 8);
                        data[(t_var1 >> 3) + 1] = audio_samp << (16 - (t_var1 & 0x7) - t_var2);
                        t_var1 += t_var2;
                    }
                    else
                    {
                        data[t_var1 >> 3] |= audio_samp >> (t_var2 + (t_var1 & 0x7) - 8);
                        data[(t_var1 >> 3) + 1] = (audio_samp >> ((t_var1 & 0x7) + t_var2 - 16)) & 0xff;
                        data[(t_var1 >> 3) + 2] = audio_samp << (24 - (t_var1 & 0x7) - t_var2);
                        t_var1 += t_var2;
                    }
                }
            }
        }
    }

    if (t_var1 & 0x7)
        t_var1 += 8 - (t_var1 & 0x7);

    sbc->bitpointer = t_var1;

    return(sbc->bitpointer >> 3);
}
