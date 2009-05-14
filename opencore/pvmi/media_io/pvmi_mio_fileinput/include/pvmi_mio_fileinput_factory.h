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
#ifndef PVMI_MIO_FILEINPUT_FACTORY_H_INCLUDED
#define PVMI_MIO_FILEINPUT_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

// Forward declarations
class PvmiMIOControl;
class OsclMemAllocator;

/**
 * Structure containing configuration info for this node
 */
class PvmiMIOFileInputSettings
{
    public:
        PvmiMIOFileInputSettings()
        {
            iMediaFormat = PVMF_MIME_FORMAT_UNKNOWN;
            iLoopInputFile = true;
            iSamplingFrequency = 8000;
            iNumChannels = 1;
            iNum20msFramesPerChunk = 1;
            iTimescale = 1000;
            iFrameHeight = 144;
            iFrameWidth = 176;
            iFrameRate = 15;
        }

        PvmiMIOFileInputSettings(const PvmiMIOFileInputSettings& aSettings)
        {
            iMediaFormat = aSettings.iMediaFormat;
            iLoopInputFile = aSettings.iLoopInputFile;
            iSamplingFrequency = aSettings.iSamplingFrequency;
            iNumChannels = aSettings.iNumChannels;
            iNum20msFramesPerChunk = aSettings.iNum20msFramesPerChunk;
            iTimescale = aSettings.iTimescale;
            iFrameHeight = aSettings.iFrameHeight;
            iFrameWidth = aSettings.iFrameWidth;
            iFrameRate = aSettings.iFrameRate;
            iFileName = aSettings.iFileName;
            iLogFileName = aSettings.iLogFileName;
            iTextFileName = aSettings.iTextFileName;
        }

        ~PvmiMIOFileInputSettings()
        {
            iMediaFormat = PVMF_MIME_FORMAT_UNKNOWN;
            iLoopInputFile = true;
            iSamplingFrequency = 8000;
            iNumChannels = 1;
            iNum20msFramesPerChunk = 1;
            iTimescale = 1000;
            iFrameHeight = 144;
            iFrameWidth = 176;
            iFrameRate = 15;
        }

        // General settings
        PVMFFormatType iMediaFormat;
        bool iLoopInputFile;
        OSCL_wStackString<512> iFileName;
        OSCL_wStackString<512> iLogFileName;
        OSCL_wStackString<512> iTextFileName;
        // Settings for audio files
        uint32 iSamplingFrequency;
        uint32 iNumChannels;
        uint32 iNum20msFramesPerChunk;
        // Settings for video files
        uint32 iTimescale;
        uint32 iFrameHeight;
        uint32 iFrameWidth;
        OsclFloat iFrameRate;
};


/**
 * Factory class for PvmiMIOFileInput
 */
class PvmiMIOFileInputFactory
{
    public:
        /**
         * Creates an instance of PvmiMIOFileInput. If the creation fails,
         * this function will leave.
         *
         * @param aSettings Settings for PvmiMIOFileInput
         * @returns A pointer to an PvmiMIOControl for the file input media input module
         * @throw Leaves with OsclErrNoMemory if memory allocation failed.
         */
        OSCL_IMPORT_REF static PvmiMIOControl* Create(const PvmiMIOFileInputSettings& aSettings);

        /**
         * This function allows the application to delete an instance of file input MIO module
         * and reclaim all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aNode The file input MIO module to be deleted.
         * @returns A status code indicating success or failure.
         */
        OSCL_IMPORT_REF static bool Delete(PvmiMIOControl* aMio);
};

#endif // PVMI_MIO_FILEINPUT_FACTORY_H_INCLUDED
