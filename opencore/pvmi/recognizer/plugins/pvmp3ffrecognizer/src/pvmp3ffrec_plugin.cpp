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
#include "pvmp3ffrec_plugin.h"
#include "imp3ff.h"
#include "oscl_file_io.h"

#define PVMF_MP3_REC_PLUGIN_MIN_SIZE_FOR_RECOGNITION 128
//intial scan length for recognition, for now 200kb
#define PVMF_MP3_REC_PLUGIN_FILE_SCAN_SIZE (200*1024)


PVMFStatus PVMP3FFRecognizerPlugin::SupportedFormats(PVMFRecognizerMIMEStringList& aSupportedFormatsList)
{
    // Return MP3 as supported type
    OSCL_HeapString<OsclMemAllocator> supportedformat = PVMF_MIME_MP3FF;
    aSupportedFormatsList.push_back(supportedformat);
    return PVMFSuccess;
}


PVMFStatus PVMP3FFRecognizerPlugin::Recognize(PVMFDataStreamFactory& aSourceDataStreamFactory, PVMFRecognizerMIMEStringList* aFormatHint,
        Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult)
{
    OSCL_UNUSED_ARG(aFormatHint);
    // Instantiate the IMpeg3File object, which is the class representing the mp3 ff parser library.
    OSCL_wStackString<1> tmpfilename;
    MP3ErrorType eSuccess = MP3_SUCCESS;
    uint32 initSearchSize = PVMF_MP3_REC_PLUGIN_FILE_SCAN_SIZE;
    IMpeg3File* mp3File = OSCL_NEW(IMpeg3File, (eSuccess));
    if (!mp3File || eSuccess != MP3_SUCCESS)
    {
        // unable to construct parser object
        return PVMFSuccess;
    }

    eSuccess = mp3File->IsMp3File(tmpfilename, &aSourceDataStreamFactory, initSearchSize);

    PVMFRecognizerResult result;
    if (eSuccess == MP3_SUCCESS)
    {
        // It is an MP3 file so add positive result
        result.iRecognizedFormat = PVMF_MIME_MP3FF;
        result.iRecognitionConfidence = PVMFRecognizerConfidenceCertain;
        aRecognizerResult.push_back(result);
    }
    else if (eSuccess == MP3_INSUFFICIENT_DATA)
    {
        // It could be an MP3 file, but not sure
        result.iRecognizedFormat = PVMF_MIME_MP3FF;
        result.iRecognitionConfidence = PVMFRecognizerConfidencePossible;
        aRecognizerResult.push_back(result);
    }
    if (mp3File)
    {
        OSCL_DELETE(mp3File);
        mp3File = NULL;
    }
    return PVMFSuccess;
}

PVMFStatus PVMP3FFRecognizerPlugin::GetRequiredMinBytesForRecognition(uint32& aBytes)
{
    aBytes = PVMF_MP3_REC_PLUGIN_MIN_SIZE_FOR_RECOGNITION;
    return PVMFSuccess;
}






