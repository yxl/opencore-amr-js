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
#include "oscl_mem.h"
#include "sbc_encoder.h"
#include "sbc.h"
#include "sbcenc_crc8.h"
#include "sbcenc_filter.h"
#include "sbcenc_bitstream.h"
#include "sbcenc_allocation.h"
#include "scalefactors.h"


/*
 ===============================================================================
 *    Encoder Initialization
 ===============================================================================
 */

bool encoder_init(TPvSbcEncConfig *config)
{

    if (NULL != config)
    {
        oscl_memset(config, 0, sizeof(TPvSbcEncConfig));
        if (NULL != (config->state = (enc_state_t *) oscl_malloc(sizeof(enc_state_t))))
        {
            oscl_memset(config->state, 0, sizeof(enc_state_t));
        }
        else
        {
            //fprintf(stderr, "not enough memory\n");
            return false;
        }
    }
    else
    {
        // fprintf(stderr, "not enough memory\n");
        return false;
    }

    // default encoding parameters
    config->sampling_frequency = 44100;
    config->allocation_method = AM_SNR;
    config->nrof_channels = 1;
    config->channel_mode = CM_MONO;
    config->nrof_subbands = 8;
    config->block_len = 16;
    config->bitpool = 53;
    config->join = 0;

    return true;
}



/*
 ===============================================================================
 *    Encoder Deletion
 ===============================================================================
 */

bool encoder_delete(TPvSbcEncConfig *config)
{
    if (!config) return false;
    if (config->state)
        oscl_free(config->state);

    return true;
}

/*
 ===============================================================================
 *    Encoder Execution
 ===============================================================================
 */
bool encoder_execute(TPvSbcEncConfig *config, UWord16 *data)
{
    /*~~~~~~~~~~~~~~~~~~~~~~~~*/
    enc_state_t *state;
    Int         i, framelen;
    Int *ptr, *ptr1;
    /*~~~~~~~~~~~~~~~~~~~~~~~~*/

    state = (enc_state_t *)config->state;

    if (!state->init)
    {
        switch (config->sampling_frequency)
        {
            case 16000:
                state->sbc.sf_index = SF_16;
                break;
            case 32000:
                state->sbc.sf_index = SF_32;
                break;
            case 44100:
                state->sbc.sf_index = SF_44;
                break;
            case 48000:
                state->sbc.sf_index = SF_48;
                break;
            default:    ;   /* return (-2); */
        }

        state->sbc.channels = config->nrof_channels;
        state->sbc.channel_mode = config->channel_mode;
        state->sbc.allocation_method = config->allocation_method;
        state->sbc.subbands = config->nrof_subbands;
        state->sbc.blocks = config->block_len;
        state->sbc.bitpool = config->bitpool;

        if
        (
            ((state->sbc.channel_mode == CM_MONO || state->sbc.channel_mode == CM_DUAL_CHANNEL) && state->sbc.bitpool > 16 * state->sbc.subbands) ||
            ((state->sbc.channel_mode == CM_STEREO || state->sbc.channel_mode == CM_JOINT_STEREO) && state->sbc.bitpool > 32 * state->sbc.subbands)
        )
        {
            encoder_delete(config);
            return false;
        }

        state->init = Btrue;
    }

    if (config->nrof_channels == 1)
    {
        if (state->sbc.subbands == 4)
        {
            ptr = &state->filter.X[0][63];
            for (i = state->sbc.blocks; i != 0; i--)
            {
                Word16 s1 = *data++;
                Word16 s2 = *data++;
                Word16 s3 = *data++;
                Word16 s4 = *data++;

                *ptr-- = s1;
                *ptr-- = s2;
                *ptr-- = s3;
                *ptr-- = s4;
            }

            analysis_filter_4(&state->filter, &state->sbc);
        }
        else
        {
            ptr = &state->filter.X[0][127];
            for (i = (state->sbc.blocks << 1); i != 0 ; i--)
            {
                Word16 s1 = *data++;
                Word16 s2 = *data++;
                Word16 s3 = *data++;
                Word16 s4 = *data++;

                *ptr-- = s1;
                *ptr-- = s2;
                *ptr-- = s3;
                *ptr-- = s4;
            }

            analysis_filter_8(&state->filter, &state->sbc);
        }

    }
    else
    {
        if (state->sbc.subbands == 4)
        {
            ptr = &state->filter.X[0][63];
            ptr1 = &state->filter.X[1][63];
            for (i = (state->sbc.blocks << 1); i != 0; i--)
            {
                Word16 s1 = *data++;
                Word16 s2 = *data++;
                Word16 s3 = *data++;
                Word16 s4 = *data++;

                *ptr-- = s1;
                *ptr-- = s3;
                *ptr1-- = s2;
                *ptr1-- = s4;
            }

            analysis_filter_4(&state->filter, &state->sbc);
        }
        else
        {
            ptr = &state->filter.X[0][127];
            ptr1 = &state->filter.X[1][127];
            for (i = (state->sbc.blocks << 1); i != 0 ; i--)
            {
                Word16 s1 = *data++;
                Word16 s2 = *data++;
                Word16 s3 = *data++;
                Word16 s4 = *data++;

                Word16 s5 = *data++;
                Word16 s6 = *data++;
                Word16 s7 = *data++;
                Word16 s8 = *data++;

                *ptr-- = s1;
                *ptr-- = s3;
                *ptr-- = s5;
                *ptr-- = s7;

                *ptr1-- = s2;
                *ptr1-- = s4;
                *ptr1-- = s6;
                *ptr1-- = s8;
            }
            analysis_filter_8(&state->filter, &state->sbc);
        }
    }

    compute_scalefactors(state);

    derive_allocation(&state->sbc, state->sbc.bits);

    framelen = pack_bitstream(config->bitstream, state, MAX_SZOF_BS_BUFF);

    config->framelen = framelen;

    return true;
}
