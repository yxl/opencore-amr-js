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
#include "pvoma1_kmj_recognizer.h"
#include "pv_mime_string_utils.h"
#include "pvfile.h"
#include "oscl_file_io.h"
#include "objmng/svc_drm.h"
#include "log.h"
#include "pvmf_cpmplugin_kmj_oma1_data_access.h"

PVMFStatus PVOMA1KMJRecognizerPlugin::Recognize( PVMFDataStreamFactory& aSourceDataStreamFactory,
                                                 PVMFRecognizerMIMEStringList* aFormatHint,
                                                 Oscl_Vector<PVMFRecognizerResult,OsclMemAllocator>& aRecognizerResult
                                                )
{
  Oscl_FileServer fileServ;
    PVFile pvfile;

    // set data stream
    pvfile.SetCPM(&aSourceDataStreamFactory);

    // open data stream
    int32 ret = pvfile.Open( NULL,Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,fileServ);
    if(ret)  // open data stream failed
    {
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize pvfile.Open failed");
        return PVMFFailure;
    }

    DrmPluginDataAccess dataAccess(&pvfile);

    T_DRM_Input_Data drmInputData =
    {
        (int32_t)&dataAccess,
        TYPE_DRM_UNKNOWN,
        DrmPluginGetDataLen,
        DrmPluginReadData,
        DrmPluginSeekData
    };

    int32_t session = SVC_drm_openSession(drmInputData);

    if((DRM_MEDIA_DATA_INVALID == session) || (DRM_FAILURE == session))
    {
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize SVC_drm_openSession failed");
    pvfile.Close();
        return PVMFFailure;
    }

    char mediaType[CONTENT_TYPE_LEN] = {0};
    ret = SVC_drm_getContentType(session,(uint8_t*)mediaType);

    SVC_drm_closeSession(session);
    pvfile.Close();

    if(DRM_SUCCESS != ret)  // get DRM content type failed
    {
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize SVC_drm_getTypeDirectly failed");
        return PVMFFailure;
    }

    // save content type for use by outer
    PVMFRecognizerResult result;

    RecognizeContentType(mediaType,result.iRecognizedFormat);
    result.iRecognitionConfidence = PVMFRecognizerConfidenceCertain;
    aRecognizerResult.push_back(result);

    return PVMFSuccess;
}


PVMFStatus PVOMA1KMJRecognizerPlugin::SupportedFormats(PVMFRecognizerMIMEStringList& aSupportedFormatsList)
{
    // do nothing
    return PVMFSuccess;
}

PVMFStatus PVOMA1KMJRecognizerPlugin::GetRequiredMinBytesForRecognition(uint32& aBytes)
{
    // do nothing
    return PVMFSuccess;
}

void PVOMA1KMJRecognizerPlugin::RecognizeContentType(char* mediaType, OSCL_HeapString<OsclMemAllocator>& aRecognizedFormat)
{
    if (0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_VIDEO_MP4)
        || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_MP4)
        || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_VIDEO_3GPP2)
        || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_VIDEO_3GPP)
        || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_3GPP2)
        || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_3GPP))
    {
        aRecognizedFormat = PVMF_MIME_MPEG4FF;
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize MP4");
    }
    else if (0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_MP3)
             || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_MPEG)
             || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_MPA))
    {
        aRecognizedFormat = PVMF_MIME_MP3FF;
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize MP3");
    }
    else if(0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_AMR)
            || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_AMR_WB)
            || 0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_ARM_WBPLUS))
    {
        aRecognizedFormat = PVMF_MIME_AMRFF;
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize AMR");
    }
    else if(0 == oscl_CIstrcmp( mediaType,PVMF_OMA_RECOGNIZER_AUDIO_MPEG4_GENERIC))
    {
        aRecognizedFormat = PVMF_MIME_AACFF;
        LOGD("PVOMA1KMJRecognizerPlugin:Recognize AAC");
    }
    else
    {
        aRecognizedFormat = PVMF_MIME_FORMAT_UNKNOWN;
    LOGD("PVOMA1KMJRecognizerPlugin:Recognize unknown");
    }
}

