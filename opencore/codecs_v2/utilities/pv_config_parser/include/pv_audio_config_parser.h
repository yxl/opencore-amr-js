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
#ifndef PV_AUDIO_CONFIG_PARSER_H_INCLUDED
#define PV_AUDIO_CONFIG_PARSER_H_INCLUDED

#include "oscl_base.h"
#include "oscl_types.h"
#include "pvmf_format_type.h"

typedef struct
{
    uint8* inPtr;
    uint32 inBytes;
    PVMFFormatType iMimeType;
} pvAudioConfigParserInputs;


typedef struct
{
    uint16 Channels;
    uint16 BitsPerSample;
    uint32 SamplesPerSec;
} pvAudioConfigParserOutputs;


OSCL_IMPORT_REF int32 pv_audio_config_parser(pvAudioConfigParserInputs *aInputs, pvAudioConfigParserOutputs *aOutputs);


#endif //PV_AUDIO_CONFIG_PARSER_H_INCLUDED


