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
/**
 *  @file pvmf_format_type.h
 *  @brief This file defines known format types and MIME strings,
 *    and some utilities for converting between them.
 *
 */

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#define PVMF_FORMAT_TYPE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifdef __cplusplus
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef OSCL_STR_PTR_LEN_H_INCLUDED
#include "oscl_str_ptr_len.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

/** PvmfMimeString is used in several PVMF APIs */
typedef OSCL_String PvmfMimeString;
#endif

// MIME string for unknown format
#define PVMF_MIME_FORMAT_UNKNOWN	"FORMATUNKNOWN"

/**
** Note on format MIME strings:
** The MIME strings that start with "X" were made up by PV.
** The others came from:
** http://www.iana.org/assignments/media-types/index.html.
** The names were chosen to match the names of the PV format
** index values.
**/

/* Following are the recognized formats  */

// MIME strings for uncompressed audio formats
#define PVMF_MIME_PCM   	"X-PCM-GEN"
#define PVMF_MIME_PCM8		"audio/L8"
#define PVMF_MIME_PCM16		"audio/L16"
#define PVMF_MIME_PCM16_BE	"X-PCM16-BE"
#define PVMF_MIME_ULAW		"audio/PCMU"
#define PVMF_MIME_ALAW		"audio/PCMA"

// MIME strings for uncompressed video formats
#define PVMF_MIME_YUV420	"X-YUV-420"
#define PVMF_MIME_YUV422	"X-YUV-422"
#define PVMF_MIME_RGB8		"X-RGB-8"
#define PVMF_MIME_RGB12		"X-RGB-12"
#define PVMF_MIME_RGB16		"X-RGB-16"
#define PVMF_MIME_RGB24		"X-RGB-24"

// MIME strings for uncompressed sub-video formats
#define PVMF_MIME_YUV420_PLANAR				"X-YUV-420-PLANAR"
#define PVMF_MIME_YUV420_PACKEDPLANAR		"X-YUV-420-PACKEDPLANAR"
#define PVMF_MIME_YUV420_SEMIPLANAR			"X-YUV-420-SEMIPLANAR"
#define PVMF_MIME_YUV420_PACKEDSEMIPLANAR	"X-YUV-420-PACKEDSEMIPLANAR"
#define PVMF_MIME_YUV422_PLANAR				"X-YUV-422-PLANAR"
#define PVMF_MIME_YUV422_PACKEDPLANAR		"X-YUV-422-PACKEDPLANAR"
#define PVMF_MIME_YUV422_SEMIPLANAR			"X-YUV-422-SEMIPLANAR"
#define PVMF_MIME_YUV422_PACKEDSEMIPLANAR	"X-YUV-422-PACKEDSEMIPLANAR"
#define PVMF_MIME_YUV420_SEMIPLANAR_YVU		"X-YUV-420-SEMIPLANAR-YUV"

// MIME strings for compressed audio formats
#define PVMF_MIME_AMR			"audio/AMR" // Streaming AMR format, aka IETF_COMBINED_TOC
#define PVMF_MIME_AMRWB			"audio/AMR-WB" // AMR Wide Band
#define PVMF_MIME_AMR_IETF		"X-AMR-IETF-SEPARATE" // Today's IETF
#define PVMF_MIME_AMRWB_IETF	"X-AMRWB-IETF-SEPARATE" // Today's IETF
#define PVMF_MIME_AMR_IF2		"X-AMR-IF2"
#define PVMF_MIME_EVRC			"audio/EVRC" // Streaming EVRC format
#define PVMF_MIME_MP3			"audio/MPEG"
#define PVMF_MIME_ADIF			"X-AAC-ADIF" //.aac file format
#define PVMF_MIME_ADTS			"X-AAC-ADTS" //.aac file format
#define PVMF_MIME_AAC_SIZEHDR	"X-AAC-SIZEHDR"
#define PVMF_MIME_LATM			"audio/MP4A-LATM" // Streaming AAC format
#define PVMF_MIME_MPEG4_AUDIO	"X-MPEG4-AUDIO"// MPEG4 Audio (AAC) as stored in MPEG4 File
#define PVMF_MIME_G723          "audio/G723"
#define PVMF_MIME_G726			"x-pvmf/audio/g726"
//WMA Audio
#define PVMF_MIME_WMA			"audio/x-ms-wma"
// AMR Audio from a asf file
#define PVMF_MIME_ASF_AMR		"x-pvmf/audio/asf-amr"
// real audio
#define PVMF_MIME_REAL_AUDIO	"audio/vnd.rn-realaudio"
// MPEG4 Audio from a asf file
#define PVMF_MIME_ASF_MPEG4_AUDIO   "x-pvmf/audio/asf-mpeg4-audio"
#define PVMF_MIME_3640		 "audio/mpeg4-generic" // Streaming AAC format

// MIME strings for Compressed video formats
#define PVMF_MIME_M4V		"video/MP4V-ES"// MPEG4 Video
#define PVMF_MIME_H2631998	"video/H263-1998"
#define PVMF_MIME_H2632000	"video/H263-2000"
//Raw 264 files (.264 - stored as per byte stream format)
#define PVMF_MIME_H264_VIDEO_RAW "X-H264-BYTE-STREAM"
//H264 tracks as stored in MP4/3GP files
#define PVMF_MIME_H264_VIDEO_MP4  "X-H264-VIDEO"
//H263 streamed as per RFC 3984
#define PVMF_MIME_H264_VIDEO "video/H264"
// WMV7, WMV8, WMV9
#define PVMF_MIME_WMV         "video/x-ms-wmv"
// RV8, RV0
#define PVMF_MIME_REAL_VIDEO		  "video/vnd.rn-realvideo"

// MIME strings for still image formats
#define PVMF_MIME_M4V_IMAGE	"X-M4V-IMAGE" // PV Proprietary Still Image
#define PVMF_MIME_IMAGE_FORMAT "image"

// MIME strings for Multiplexed formats and single-media file formats.
#define PVMF_MIME_MPEG4FF		"video/MP4"
#define PVMF_MIME_H223			"X-H223"
#define PVMF_MIME_RTP			"X-RTP"
#define PVMF_MIME_AMRFF			"X-AMR-FF"
#define PVMF_MIME_AACFF			"X-AAC-FF"
#define PVMF_MIME_MP3FF			"X-MP3-FF"
#define PVMF_MIME_WAVFF			"X-WAV-FF"
#define PVMF_MIME_ASFFF			"x-pvmf/mux/asf"
#define PVMF_MIME_RMFF			"x-pvmf/mux/rm"
#define PVMF_MIME_DIVXFF		"x-pvmf/mux/divx"
#define PVMF_MIME_MIDIFF		"application/x-midi"
#define PVMF_MIME_AVIFF			"x-pvmf/mux/avi"
#define PVMF_MIME_DATA_PVRFF    "video/x-pvmf-pvr "

// MIME strings for RAW data formats
#define PVMF_MIME_8BIT_RAW   "X-RAW-8"

// MIME strings for PVMF data source types
#define PVMF_MIME_DATA_SOURCE_RTSP_URL  "X-PVMF-DATA-SRC-RTSP-URL"
#define PVMF_MIME_DATA_SOURCE_RTSP_PVR_FCS_URL "X-PVMF-DATA-SRC-RTSP-PVR-FCS-URL"
#define PVMF_MIME_DATA_SOURCE_HTTP_URL  "X-PVMF-DATA-SRC-HTTP-URL"
#define PVMF_MIME_DATA_SOURCE_SDP_FILE  "X-PVMF-DATA-SRC-SDP-FILE"
#define PVMF_MIME_DATA_SOURCE_SDP_PVR_FCS_FILE "X-PVMF-DATA-SRC-SDP-PVR-FCS-FILE"
#define PVMF_MIME_DATA_SOURCE_SDP_BROADCAST "X-PVMF-DATA-SRC-SDP-BROADCAST"
#define PVMF_MIME_DATA_SOURCE_PVX_FILE  "X-PVMF-DATA-SRC-PVX-FILE"
#define PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL "X-PVMF-DATA-SRC-MS-HTTP-STREAMING-URL"
#define PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL "X-PVMF-DATA-SRC-REAL-HTTP-CLOAKING-URL"
#define PVMF_MIME_DATA_SOURCE_RTP_PACKET_SOURCE "X-PVMF-DATA-SRC-RTP-PACKET"
#define PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL "X-PVMF-DATA-SRC-SHOUTCAST-URL"
#define PVMF_MIME_DATA_SOURCE_UNKNOWN_URL "X-PVMF-DATA-SRC-UNKNOWN-URL"


// MIME string for miscellaneous media data formats
#define PVMF_MIME_3GPP_TIMEDTEXT		"video/3gpp-tt"

// MIME string for protocol formats
#define PVMF_MIME_INET_UDP				"X-PVMF-NET-INET-UDP"
#define PVMF_MIME_INET_TCP				"X-PVMF-NET-INET-TCP"
#define PVMF_MIME_RTSP					"X-PVMF-NET-INET-RTSP"
#define PVMF_MIME_RTCP					"X-PVMF-NET-INET-RTCP"

// reserved for 2way
#define PVMF_MIME_USERINPUT_BASIC_STRING	"x-pvmf/userinput/basic-string"
#define PVMF_MIME_USERINPUT_IA5_STRING		"x-pvmf/userinput/ia5-string"
#define PVMF_MIME_USERINPUT_GENERAL_STRING	"x-pvmf/userinput/general-string"
#define PVMF_MIME_USERINPUT_DTMF			"x-pvmf/userinput/dtmf"


class PVMFFormatType
{
    public:
        // default constructor
        PVMFFormatType() : iCompressed(false), iMimeStr(PVMF_MIME_FORMAT_UNKNOWN)
        {}

        PVMFFormatType(const char* mimeStrPtr, bool compressed = false)
        {
            iStr = OSCL_HeapString<OsclMemAllocator> (mimeStrPtr);
            iMimeStr = iStr.get_cstr();
            iCompressed = compressed;
        }

        PVMFFormatType(const char* mimeStrPtr, int32 mimeStrLen, bool compressed = false)
        {
            iStr = OSCL_HeapString<OsclMemAllocator> (mimeStrPtr, mimeStrLen);
            iMimeStr.setPtrLen(iStr.get_cstr(), iStr.get_size());
            iCompressed = compressed;
        }

        PVMFFormatType& operator=(const PVMFFormatType& rhs)
        {
            iStr = OSCL_HeapString<OsclMemAllocator> (rhs.getMIMEStrPtr(), rhs.getMIMEStrLen());
            iMimeStr.setPtrLen(iStr.get_cstr(), iStr.get_size());
            iCompressed = rhs.isCompressed();
            return *this;
        }

        PVMFFormatType& operator=(const char* mimeStrPtr)
        {
            iStr = OSCL_HeapString<OsclMemAllocator> (mimeStrPtr);
            iMimeStr = iStr.get_cstr();
            return *this;
        }

        virtual ~PVMFFormatType() {};

        // case insensitive comparision
        c_bool operator==(const PVMFFormatType& rhs) const
        {
            return iMimeStr.isCIEquivalentTo(rhs.getStrCSumPtr());
        }

        bool operator!=(const PVMFFormatType& rhs) const
        {
            return !(*this == rhs);
        }

        void setCompressed(bool compressed = false)
        {
            iCompressed = compressed;
        }

        const char* getMIMEStrPtr() const
        {
            return iMimeStr.c_str();
        }

        int32 getMIMEStrLen() const
        {
            return iMimeStr.length();
        }

        const StrCSumPtrLen& getStrCSumPtr() const
        {
            return iMimeStr;
        }

        //return true if it is a compressed Media
        OSCL_IMPORT_REF bool isCompressed() const;

        //return true if a media type is IANA audio format or PV internal audio format.
        OSCL_IMPORT_REF bool isAudio() const;

        //return true if a media type is IANA video format or PV internal video format.
        OSCL_IMPORT_REF bool isVideo() const;

        //return true if a media type is image format or PV internal image format.
        OSCL_IMPORT_REF bool isImage() const;

        //return true if a media type is IANA text format.
        OSCL_IMPORT_REF bool isText() const;

        //return true if a media type is PV internal file format.
        OSCL_IMPORT_REF bool isFile() const;

        //return true if a media type is PV internal raw format.
        OSCL_IMPORT_REF bool isRaw() const;

        //return true if a media type is PV internal data source.
        OSCL_IMPORT_REF bool isDataSource() const;

        //return true if a media type is PV internal data source.
        OSCL_IMPORT_REF bool isUserInput() const;

        // a flag to indicate if a media type described by the MIME is compressed
        // this value should be passed in because this class does not have the
        // ability to determine if a given media type, especially a format from
        // the third party???
        bool								iCompressed;

    protected:
        // a local storage for MIME string
        OSCL_HeapString<OsclMemAllocator>	iStr;
        StrCSumPtrLen						iMimeStr;

};

#endif
