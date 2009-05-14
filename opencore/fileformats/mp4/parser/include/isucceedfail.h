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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                               ISucceedFail Class                              */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
	This ISucceedFail Class is the base class for all other classes here
	- this provides the ability to check for failure when reading in an
	mp4 file
*/


#ifndef ISUCCEEDFAIL_H_INCLUDED
#define ISUCCEEDFAIL_H_INCLUDED

#ifndef PV_MP4FFPARSER_CONFIG_H_INCLUDED
#include "pv_mp4ffparser_config.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif


#define MAX_ERROR_CODES 113

typedef enum {  READ_FAILED	= -1,
                EVERYTHING_FINE = 0,
                DEFAULT_ERROR = 1,
                READ_USER_DATA_ATOM_FAILED = 2,
                READ_MEDIA_DATA_ATOM_FAILED = 3,
                READ_MOVIE_ATOM_FAILED = 4,
                READ_MOVIE_HEADER_ATOM_FAILED = 5,
                READ_TRACK_ATOM_FAILED = 6,
                READ_TRACK_HEADER_ATOM_FAILED = 7,
                READ_TRACK_REFERENCE_ATOM_FAILED = 8,
                READ_TRACK_REFERENCE_TYPE_ATOM_FAILED = 9,
                READ_OBJECT_DESCRIPTOR_ATOM_FAILED = 10,
                READ_INITIAL_OBJECT_DESCRIPTOR_FAILED = 11,
                READ_OBJECT_DESCRIPTOR_FAILED = 12,
                READ_MEDIA_ATOM_FAILED = 13,
                READ_MEDIA_HEADER_ATOM_FAILED = 14,
                READ_HANDLER_ATOM_FAILED = 15,
                READ_MEDIA_INFORMATION_ATOM_FAILED = 16,
                READ_MEDIA_INFORMATION_HEADER_ATOM_FAILED = 17,
                READ_VIDEO_MEDIA_HEADER_ATOM_FAILED = 18,
                READ_SOUND_MEDIA_HEADER_ATOM_FAILED = 19,
                READ_HINT_MEDIA_HEADER_ATOM_FAILED = 20,
                READ_MPEG4_MEDIA_HEADER_ATOM_FAILED = 21,
                READ_DATA_INFORMATION_ATOM_FAILED = 22,
                READ_DATA_REFERENCE_ATOM_FAILED = 23,
                READ_DATA_ENTRY_URL_ATOM_FAILED = 24,
                READ_DATA_ENTRY_URN_ATOM_FAILED = 25,
                READ_SAMPLE_TABLE_ATOM_FAILED = 26,
                READ_TIME_TO_SAMPLE_ATOM_FAILED = 27,
                READ_SAMPLE_DESCRIPTION_ATOM_FAILED = 28,
                READ_SAMPLE_SIZE_ATOM_FAILED = 29,
                READ_SAMPLE_TO_CHUNK_ATOM_FAILED = 30,
                READ_CHUNK_OFFSET_ATOM_FAILED = 31,
                READ_SYNC_SAMPLE_ATOM_FAILED = 32,
                READ_SAMPLE_ENTRY_FAILED = 33,
                READ_AUDIO_SAMPLE_ENTRY_FAILED = 34,
                READ_VISUAL_SAMPLE_ENTRY_FAILED = 35,
                READ_HINT_SAMPLE_ENTRY_FAILED = 36,
                READ_MPEG_SAMPLE_ENTRY_FAILED = 37,
                READ_AUDIO_HINT_SAMPLE_FAILED = 38,
                READ_VIDEO_HINT_SAMPLE_FAILED = 39,
                READ_ESD_ATOM_FAILED = 40,
                READ_ES_DESCRIPTOR_FAILED = 41,
                READ_SL_CONFIG_DESCRIPTOR_FAILED = 42,
                READ_DECODER_CONFIG_DESCRIPTOR_FAILED  = 43,
                READ_DECODER_SPECIFIC_INFO_FAILED = 44,
                DUPLICATE_MOVIE_ATOMS = 45,
                NO_MOVIE_ATOM_PRESENT = 46,
                DUPLICATE_OBJECT_DESCRIPTORS = 47,
                NO_OBJECT_DESCRIPTOR_ATOM_PRESENT = 48,
                DUPLICATE_MOVIE_HEADERS = 49,
                NO_MOVIE_HEADER_ATOM_PRESENT = 50,
                DUPLICATE_TRACK_REFERENCE_ATOMS = 51,
                DUPLICATE_TRACK_HEADER_ATOMS = 52,
                NO_TRACK_HEADER_ATOM_PRESENT = 53,
                DUPLICATE_MEDIA_ATOMS = 54,
                NO_MEDIA_ATOM_PRESENT = 55,
                READ_UNKNOWN_ATOM = 56,
                NON_PV_CONTENT = 57,
                FILE_NOT_STREAMABLE = 58,
                INSUFFICIENT_BUFFER_SIZE = 59,
                INVALID_SAMPLE_SIZE = 60,
                INVALID_CHUNK_OFFSET = 61,
                END_OF_TRACK = 62,
                MEMORY_ALLOCATION_FAILED = 63,
                READ_FILE_TYPE_ATOM_FAILED = 64,
                ZERO_OR_NEGATIVE_ATOM_SIZE = 65,
                NO_MEDIA_TRACKS_IN_FILE = 66,
                NO_META_DATA_FOR_MEDIA_TRACKS = 67,
                MEDIA_DATA_NOT_SELF_CONTAINED = 68,
                READ_PVTI_SESSION_INFO_FAILED = 69,
                READ_PVTI_MEDIA_INFO_FAILED = 70,
                READ_CONTENT_VERSION_FAILED = 71,
                READ_DOWNLOAD_ATOM_FAILED = 72,
                READ_TRACK_INFO_ATOM_FAILED = 73,
                READ_REQUIREMENTS_ATOM_FAILED = 74,
                READ_WMF_SET_MEDIA_ATOM_FAILED = 75,
                READ_WMF_SET_SESSION_ATOM_FAILED = 76,
                READ_PV_USER_DATA_ATOM_FAILED = 77,
                READ_VIDEO_INFORMATION_ATOM_FAILED = 78,
                READ_RANDOM_ACCESS_ATOM_FAILED = 79,
                READ_AMR_SAMPLE_ENTRY_FAILED = 80,
                READ_H263_SAMPLE_ENTRY_FAILED = 81,
                FILE_OPEN_FAILED = 82,
                READ_UUID_ATOM_FAILED = 83,
                FILE_VERSION_NOT_SUPPORTED = 84,
                TRACK_VERSION_NOT_SUPPORTED = 85,


                READ_COPYRIGHT_ATOM_FAILED = 88,
                READ_FONT_TABLE_ATOM_FAILED = 89,
                READ_FONT_RECORD_FAILED = 90,
                FILE_PSEUDO_STREAMABLE = 91,
                FILE_NOT_PSEUDO_STREAMABLE = 92,
                READ_PV_ENTITY_TAG_ATOM_FAILED = 93,
                DUPLICATE_FILE_TYPE_ATOMS = 94,
                UNSUPPORTED_FILE_TYPE = 95,
                FILE_TYPE_ATOM_NOT_FOUND = 96,
                READ_EDIT_ATOM_FAILED = 97,
                READ_EDITLIST_ATOM_FAILED = 98,
                ATOM_VERSION_NOT_SUPPORTED = 99,
                READ_UDTA_TITL_FAILED = 100,
                READ_UDTA_DSCP_FAILED = 101,
                READ_UDTA_CPRT_FAILED = 102,
                READ_UDTA_PERF_FAILED = 103,
                READ_UDTA_AUTH_FAILED = 104,
                READ_UDTA_GNRE_FAILED = 105,
                READ_UDTA_RTNG_FAILED = 106,
                READ_UDTA_CLSF_FAILED = 107,
                READ_UDTA_KYWD_FAILED = 108,
                READ_PV_CONTENT_TYPE_ATOM_FAILED = 109,
                READ_TEXT_SAMPLE_MODIFIERS_FAILED = 110,
                READ_AVC_SAMPLE_ENTRY_FAILED = 111,
                READ_AVC_CONFIG_BOX_FAILED = 112,
                READ_MPEG4_BITRATE_BOX_FAILED = 113,
                READ_AVC_SAMPLE_DEPENDENCY_TYPE_BOX_FAILED = 114,
                READ_AVC_SAMPLE_TO_GROUP_BOX_FAILED = 115,
                READ_AVC_SAMPLE_GROUP_DESCRIPTION_BOX_FAILED = 116,
                READ_AVC_SUBSEQUENCE_DESCRIPTION_BOX_FAILED = 117,
                READ_AVC_LAYER_DESCRIPTION_BOX_FAILED = 118,
                READ_AVC_SAMPLE_DEPENDENCY_BOX_FAILED = 119,


                READ_TRACK_EXTENDS_ATOM_FAILED = 127,
                READ_MOVIE_EXTENDS_HEADER_FAILED = 128,
                READ_MOVIE_EXTENDS_ATOM_FAILED = 129,
                DUPLICATE_MOOF_ATOM = 130,
                READ_MOVIE_FRAGMENT_ATOM_FAILED = 131,
                READ_MOVIE_FRAGMENT_HEADER_FAILED = 132,
                READ_TRACK_FRAGMENT_ATOM_FAILED = 133,
                READ_TRACK_FRAGMENT_RUN_ATOM_FAILED = 134,
                READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED = 135,
                READ_MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_FAILED = 136,
                READ_TRACK_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED = 137,
                READ_MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM_FAILED = 138,
                READ_META_DATA_FAILED = 139,


                READ_ITUNES_ILST_META_DATA_FAILED = 140,


                INSUFFICIENT_DATA = 141,
                NOT_SUPPORTED = 142,
                READ_OMADRM_KMS_BOX_FAILED = 143,
                DUPLICATE_OMADRM_KMS_BOX = 144,
                READ_SCHEME_INFORMATION_BOX_FAILED = 145,
                NO_SAMPLE_IN_CURRENT_MOOF = 146,
                SUFFICIENT_DATA_IN_FILE = 147,
                LAST_SAMPLE_IN_MOOV = 148,
                READ_UDTA_LOC_FAILED = 149,
                NOT_PROGRESSIVE_STREAMABLE = 150,
                READ_PIXELASPECTRATIO_BOX_FAILED = 151,

                EXCEED_MAX_LIMIT_SUPPORTED_FOR_TOTAL_TRACKS

             } MP4_ERROR_CODE;


class ISucceedFail
{

    public:
        bool MP4Success()
        {
            return _success;
        }
        int32 GetMP4Error()
        {
            return _mp4ErrorCode;
        }

    protected:
        bool _success;
        int32 _mp4ErrorCode;
};


#endif // ISUCCEEDFAIL_H_INCLUDED
