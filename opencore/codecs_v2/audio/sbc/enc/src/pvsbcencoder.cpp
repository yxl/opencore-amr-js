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

#include "pvsbcencoder.h"
#include "sbc_encoder.h"

OSCL_EXPORT_REF CPVSbcEncoder* CPVSbcEncoder::New()
{
    return new(CPVSbcEncoder);
}

OSCL_EXPORT_REF CPVSbcEncoder::~CPVSbcEncoder()
{
}

OSCL_EXPORT_REF TPvSbcEncStatus CPVSbcEncoder::Init()
{
    iEncConfig = (TPvSbcEncConfig *)oscl_malloc(sizeof(TPvSbcEncConfig));
    if (!iEncConfig)
        return TPVSBCENC_FAIL;

    status = encoder_init(iEncConfig);
    if (status == false)
        return TPVSBCENC_FAIL;

    return TPVSBCENC_SUCCESS;
}

OSCL_EXPORT_REF TPvSbcEncStatus CPVSbcEncoder::SetInput(TPvSbcEncConfig *config)
{
    iEncConfig->sampling_frequency = config->sampling_frequency;
    iEncConfig->nrof_channels = config->nrof_channels;
    iEncConfig->block_len = config->block_len;
    iEncConfig->nrof_subbands = config->nrof_subbands;
    iEncConfig->allocation_method = config->allocation_method;
    iEncConfig->bitpool = config->bitpool;
    iEncConfig->channel_mode = config->channel_mode;
    iEncConfig->join = config->join;

    return TPVSBCENC_SUCCESS;
}


OSCL_EXPORT_REF TPvSbcEncStatus CPVSbcEncoder::Execute(uint16* pcmbuffer, uint inBufSize,
        uint8 *bitstream, uint *outBufSize)
{
    iEncConfig->pcmbuffer = pcmbuffer;
    iEncConfig->bitstream = bitstream;

    if (inBufSize != (uint)(iEncConfig->block_len * iEncConfig->nrof_subbands * iEncConfig->nrof_channels))
        return TPVSBCENC_INSUFFICIENT_INPUT_DATA;

    status = encoder_execute(iEncConfig, iEncConfig->pcmbuffer);
    if (status == false)
        return TPVSBCENC_FAIL;
    *outBufSize = iEncConfig->framelen;

    return TPVSBCENC_SUCCESS;
}

OSCL_EXPORT_REF TPvSbcEncStatus CPVSbcEncoder::Reset()
{
    status = encoder_delete(iEncConfig);
    if (status == false)
        return TPVSBCENC_FAIL;

    oscl_free(iEncConfig);

    return TPVSBCENC_SUCCESS;
}





