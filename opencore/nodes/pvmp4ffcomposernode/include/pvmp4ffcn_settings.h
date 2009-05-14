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

#ifndef PVMP4FFCN_SETTINGS_H_INCLUDED
#define PVMP4FFCN_SETTINGS_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

/**
 * Structure containing configuration info for this node
 */
class PVMp4FFComposerNodeSettings
{
    public:
        PVMp4FFComposerNodeSettings()
        {
            iMediaFormat = PVMF_MIME_FORMAT_UNKNOWN;
            iLoopInputFile = true;
            iSamplingFrequency = 8000;
            iNumChannels = 1;
            iNum20msFramesPerChunk = 1;
            iTimescale = 1000;
            iFrameHeight = 144;
            iFrameWidth = 176;
            iFrameRateSimulation = false;
            iFrameRate = 15;
            iFirstFrameTimestamp = 0;
            iBitrate = 0;
            iFrameInterval = 0;
        }

        // General settings
        PVMFFormatType iMediaFormat;
        bool iLoopInputFile;
        OSCL_wHeapString<OsclMemAllocator> iFileName;
        OSCL_wHeapString<OsclMemAllocator> iLogFileName;
        OSCL_wHeapString<OsclMemAllocator> iTextFileName;

        // Settings for audio files
        uint32 iSamplingFrequency;
        uint32 iNumChannels;
        uint32 iNum20msFramesPerChunk;
        // Settings for video files
        uint32 iTimescale;
        uint32 iFrameHeight;
        uint32 iFrameWidth;
        bool iFrameRateSimulation;
        OsclFloat iFrameRate;
        uint32 iFirstFrameTimestamp;
        uint32 iBitrate;
        uint32 iFrameInterval;
};

#endif


