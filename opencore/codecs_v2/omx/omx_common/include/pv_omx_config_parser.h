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
#ifndef PV_OMX_CONFIG_PARSER_H_INCLUDED
#define PV_OMX_CONFIG_PARSER_H_INCLUDED

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#ifndef PV_OMX_QUEUE_H_INCLUDED
#include "pv_omx_queue.h"
#endif

#ifndef OMX_Types_h
#include "OMX_Types.h"
#endif

#ifndef OSCL_BASE_INCLUDED_H
#include "oscl_base.h"
#endif

#ifndef OMX_Core_h
#include "OMX_Core.h"
#endif

#ifndef PV_AUDIO_CONFIG_PARSER_H
#include "pv_audio_config_parser.h"
#endif

#ifndef PV_VIDEO_CONFIG_PARSER_H
#include "pv_video_config_parser.h"
#endif


#ifdef __cplusplus
extern "C"
{
#endif
    OSCL_IMPORT_REF OMX_BOOL OMXConfigParser(
        OMX_PTR aInputParameters,
        OMX_PTR aOutputParameters);

#ifdef __cplusplus
}
#endif


typedef struct
{
    OMX_U8* inPtr;             //pointer to codec configuration header
    OMX_U32 inBytes;           //length of codec configuration header
    OMX_STRING cComponentRole; //OMX component codec type
    OMX_STRING cComponentName;  //OMX component name
} OMXConfigParserInputs;

typedef struct
{
    OMX_U16 Channels;
    OMX_U16 BitsPerSample;
    OMX_U32 SamplesPerSec;
} AudioOMXConfigParserOutputs;

typedef struct
{
    OMX_U32 width;
    OMX_U32 height;
    OMX_U32 profile;
    OMX_U32 level;
} VideoOMXConfigParserOutputs;

#endif

