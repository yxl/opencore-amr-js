/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PVOMA1FFREC_PLUGIN_H_INCLUDED
#define PVOMA1FFREC_PLUGIN_H_INCLUDED

#include "pvmf_cpmplugin_kmj_oma1_types.h"
#include "pvmf_recognizer_plugin.h"

#define CONTENT_TYPE_LEN    64

#define PVMF_OMA_RECOGNIZER_AUDIO_3GPP              "audio/3gpp"
#define PVMF_OMA_RECOGNIZER_AUDIO_3GPP2             "audio/3gpp2"
#define PVMF_OMA_RECOGNIZER_VIDEO_3GPP              "video/3gpp"
#define PVMF_OMA_RECOGNIZER_VIDEO_3GPP2             "video/3gpp2"
#define PVMF_OMA_RECOGNIZER_AUDIO_MP4               "audio/mp4"
#define PVMF_OMA_RECOGNIZER_VIDEO_MP4               "video/mp4"

#define PVMF_OMA_RECOGNIZER_AUDIO_AMR               "audio/AMR"
#define PVMF_OMA_RECOGNIZER_AUDIO_AMR_WB            "audio/AMR-WB"
#define PVMF_OMA_RECOGNIZER_AUDIO_ARM_WBPLUS        "audio/amr-wb+"

#define PVMF_OMA_RECOGNIZER_AUDIO_MPEG              "audio/mpeg"
#define PVMF_OMA_RECOGNIZER_AUDIO_MPA               "audio/MPA"
#define PVMF_OMA_RECOGNIZER_AUDIO_MP3               "audio/mp3"

#define PVMF_OMA_RECOGNIZER_AUDIO_MPEG4_GENERIC     "audio/mpeg4-generic"

class PVOMA1KMJRecognizerPlugin : public PVMFRecognizerPluginInterface
{
public:
    PVOMA1KMJRecognizerPlugin()
    {
    };

    ~PVOMA1KMJRecognizerPlugin()
    {
    };

    PVMFStatus SupportedFormats(PVMFRecognizerMIMEStringList& aSupportedFormatsList);
    PVMFStatus Recognize(PVMFDataStreamFactory& aSourceDataStreamFactory,
                         PVMFRecognizerMIMEStringList* aFormatHint,
                         Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult);
    PVMFStatus GetRequiredMinBytesForRecognition(uint32& aBytes);

PRIVATE:
  void RecognizeContentType(char* mediaType, OSCL_HeapString<OsclMemAllocator>& aRecognizedFormat);
};

#endif
