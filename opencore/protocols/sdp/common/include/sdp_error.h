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
#ifndef __SDP_ERROR_H__
#define __SDP_ERROR_H__


typedef enum
{
    SDP_SUCCESS,
    SDP_VALUE_OUT_OF_RANGE,
    SDP_MISSING_ORIGIN_FIELD,
    SDP_MISSING_SESSION_NAME_FIELD,
    SDP_MISSING_AUTHOR_VERSION_FIELD,
    SDP_MISSING_AUTHOR_FIELD,
    SDP_MISSING_COPYRIGHT_FIELD,
    SDP_MISSING_RATING_FIELD,
    SDP_MISSING_CREATION_DATE_FIELD,
    SDP_MISSING_RANGE_FIELD,
    SDP_MISSING_WMF_VERSION_FIELD = 10,
    SDP_MISSING_CONTROL_URL_FIELD,
    SDP_INVALID_MEDIA_PARAMETER,
    SDP_MISSING_MEDIA_DESCRIPTION,
    SDP_MISSING_RTP_MAP,
    SDP_OUTPUT_BUFFER_OVERFLOW,
    SDP_BAD_FORMAT,
    SDP_BAD_MEDIA_FORMAT,
    SDP_BAD_SESSION_FORMAT = 18,
    SDP_PAYLOAD_MISMATCH,
    SDP_FAILURE,
    SDP_BAD_MEDIA_FRAMESIZE,
    SDP_BAD_MEDIA_WIDTH,
    SDP_BAD_MEDIA_HEIGHT,
    SDP_NO_MEMORY,
    SDP_BAD_MEDIA_MISSING_RTPMAP,
    SDP_FAILURE_NO_C_FIELD,
    SDP_BAD_MEDIA_ALT_ID,
    SDP_BAD_MEDIA_FMTP,
    SDP_FAILURE_NO_FMTP_FIELD,
    SDP_BAD_MEDIA_LANG_FIELD,
    SDP_BAD_MEDIA_MAXPTIME,
    SDP_BAD_MEDIA_RTP_MAP,
    SDP_BAD_MEDIA_CONTROL_FIELD,
    SDP_BAD_MEDIA_RANGE_FIELD,
    SDP_BAD_MEDIA_FRAME_RATE,
    SDP_BAD_MEDIA_FRAME_INTERVAL,
} SDP_ERROR_CODE;

//----------------------------------------------------------------------
// Global Data Declarations
//----------------------------------------------------------------------
#define MAXIMUM_OUTPUT_STRING_SIZE	1000
#define SDP_INT_STRING_LENGTH	9
#define SDP_FLOAT_STRING_LENGTH	10
#define SDP_CR_LF 2
#define SDP_SPACE 1
#define MAX_PARSERS_SUPPORTED 9
#define MAX_CODEC_MODE_LIST 13
#define MAX_PAYLOADS 10		/*Defined arbitrarily*/

//======================================================================
//  CLASS DEFINITIONS and FUNCTION DECLARATIONS
//======================================================================

#endif  // __SDP_ERROR_H__
