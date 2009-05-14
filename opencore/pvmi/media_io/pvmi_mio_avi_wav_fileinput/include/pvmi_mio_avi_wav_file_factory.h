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
#ifndef PVMI_MIO_AVIFILE_FACTORY_H_INCLUDED
#define PVMI_MIO_AVIFILE_FACTORY_H_INCLUDED

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
#ifndef PV_AVIFILE_H_INCLUDED
#include "pv_avifile.h"
#endif

// Forward declarations
class PvmiMIOControl;
class OsclMemAllocator;

/**
 * Structure containing configuration info for this node
 */
class PvmiMIOAviWavFileSettings
{
    public:
        PvmiMIOAviWavFileSettings()
        {
            iMediaFormat = PVMF_MIME_FORMAT_UNKNOWN;
            iNumLoops = 0;
            iRecModeSyncWithClock = false;
            iStreamNumber = 0;
            iSamplingFrequency = 0;
            iNumChannels = 0;
            iTimescale = 0;
            iFrameHeight = 0;
            iFrameWidth = 0;
            iFrameRate = 0;
            iSampleSize = 0;
            iByteRate = 0;
            iFrameDuration = 0;
            iDataBufferSize = 0;
            iPicBottomUp = false;
        }

        PvmiMIOAviWavFileSettings(const PvmiMIOAviWavFileSettings& aSettings)
        {
            iMediaFormat = aSettings.iMediaFormat;
            iMimeType = aSettings.iMimeType;
            iNumLoops = 0;
            iRecModeSyncWithClock = aSettings.iRecModeSyncWithClock;
            iStreamNumber = aSettings.iStreamNumber;
            iSamplingFrequency = aSettings.iSamplingFrequency;
            iNumChannels = aSettings.iNumChannels;
            iTimescale = aSettings.iTimescale;
            iFrameHeight = aSettings.iFrameHeight;
            iFrameWidth = aSettings.iFrameWidth;
            iFrameRate = aSettings.iFrameRate;
            iSampleSize = aSettings.iSampleSize;
            iByteRate = aSettings.iByteRate;
            iFrameDuration = aSettings.iFrameDuration;
            iDataBufferSize = aSettings.iDataBufferSize;
            iPicBottomUp = aSettings.iPicBottomUp;

        }

        ~PvmiMIOAviWavFileSettings() {}

        // General settings
        PVMFFormatType  iMediaFormat;
        OSCL_HeapString<OsclMemAllocator> iMimeType;
        uint32			iNumLoops;
        bool iRecModeSyncWithClock;
        uint32 iStreamNumber;

        // Settings for audio stream
        OsclFloat iSamplingFrequency;
        uint32 iNumChannels;

        // Settings for video stream
        uint32 iTimescale;
        uint32 iFrameHeight;
        uint32 iFrameWidth;
        OsclFloat iFrameRate;
        uint32 iSampleSize;
        uint32 iByteRate;
        uint32 iFrameDuration;
        uint32 iDataBufferSize;
        bool	        iPicBottomUp;
};

/**
 * enum for file type information
 */

typedef enum
{
    FILE_FORMAT_WAV,
    FILE_FORMAT_AVI
}FileFormatType;

/**
 * Factory class for PvmiMIOAviWavFile
 */
class PvmiMIOAviWavFileFactory
{
    public:
        /**
         * Creates an instance of PvmiMIOAviWavFile. If the creation fails,
         * this function will leave.
         *
         * @param aLoopIn set if required to record in loop
         * @param aRecordingMode true if Authoring in sync with clock. false if Authoring as soon as data is available.
         * @param aStreamNo stream number for avi file, if there are multiple streams in the file. There will be one MIO Component and MIO node per stream.
         * @param aFileParser Pointer to file parser (AVI or WAV)
         * @param aFileType file type (AVI or WAV)
         * @returns A pointer to an PvmiMIOControl for the file input media input module
         * @return arError error code. 1 if success
         * @throw Leaves with OsclErrNoMemory if memory allocation failed.
         */
        OSCL_IMPORT_REF static PvmiMIOControl* Create(uint32 aNumLoops, bool aRecordingMode, uint32 aStreamNo,
                OsclAny* aFileParser, FileFormatType aFileType, int32& arError);

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

#endif // PVMI_MIO_AVIFILE_FACTORY_H_INCLUDED
