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
#include "pvmf_format_type.h"

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

#include "pv_mime_string_utils.h"

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

OSCL_EXPORT_REF bool PVMFFormatType::isCompressed() const
{
    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_PCM) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_PCM8) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_PCM16) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_PCM16_BE) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ULAW) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ALAW) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_YUV420) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_YUV422) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB8) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB12) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB16) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB24) == 0))
    {
        // all uncompressed formats
        return false;
    }

    return true;
}


OSCL_EXPORT_REF bool PVMFFormatType::isAudio() const
{
    //Standard audio media types from IANA.org
    if (oscl_strstr(iMimeStr.c_str(), "audio/") != NULL)
        return true;

    //PV internal audio media types
    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_PCM) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_PCM16_BE) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AMR_IETF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AMRWB_IETF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AMR_IF2) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ADIF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ADTS) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AAC_SIZEHDR) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_MPEG4_AUDIO) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_3640) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_G726) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ASF_AMR) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ASF_MPEG4_AUDIO) == 0))
    {
        return true;
    }

    return  false;
}

OSCL_EXPORT_REF bool PVMFFormatType::isVideo() const
{
    //Standard video media types from IANA
    if (oscl_strstr(iMimeStr.c_str(), "video/") != NULL)
    {
        // 3gp timed text has a mime as "video/3gpp-tt" we should return false in this case
        if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_3GPP_TIMEDTEXT) == 0))
        {
            return false;
        }
        return true;
    }

    //PV internal video media types
    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_YUV420) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_YUV422) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB8) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB12) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB16) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RGB24) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_H264_VIDEO_RAW) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_H264_VIDEO_MP4) == 0))
    {
        return true;
    }

    return  false;
}

OSCL_EXPORT_REF bool PVMFFormatType::isImage() const
{
    //Standard video media types from IANA
    if (oscl_strstr(iMimeStr.c_str(), "image/") != NULL)
        return true;

    //PV internal image media type
    if (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_M4V_IMAGE) == 0)
        return true;

    return false;
}

OSCL_EXPORT_REF bool PVMFFormatType::isText() const
{
    if (oscl_strstr(iMimeStr.c_str(), "text/") != NULL)
        return true;

    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_3GPP_TIMEDTEXT) == 0))
    {
        return true;
    }

    return false;
}

OSCL_EXPORT_REF bool PVMFFormatType::isFile() const
{
    //PV internal file format type
    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_MPEG4FF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_H223) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RTP) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AMRFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AACFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_MP3FF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_WAVFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_ASFFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_RMFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DIVXFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_MIDIFF) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_AVIFF) == 0))
        return true;

    return false;
}

OSCL_EXPORT_REF bool PVMFFormatType::isRaw() const
{
    //PV internal raw format type
    return (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_8BIT_RAW) == 0);
}

//return true if a media type is PV internal data source.
OSCL_EXPORT_REF bool PVMFFormatType::isDataSource() const
{
    //PV internal data source format type
    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_RTSP_URL) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_HTTP_URL) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_SDP_FILE) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_PVX_FILE) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_DATA_SOURCE_RTP_PACKET_SOURCE) == 0))
        return true;

    return false;
}

//return true if a media type is PV user input.
OSCL_EXPORT_REF bool PVMFFormatType::isUserInput() const
{
    //PV user input format type
    if ((pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_USERINPUT_BASIC_STRING) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_USERINPUT_IA5_STRING) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_USERINPUT_GENERAL_STRING) == 0) ||
            (pv_mime_strcmp(iMimeStr.c_str(), PVMF_MIME_USERINPUT_DTMF) == 0))
        return true;

    return false;
}
