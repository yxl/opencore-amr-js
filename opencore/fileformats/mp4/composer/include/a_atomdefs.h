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
/*
    This PVA_FF_Atom Class is the base class for all other Atoms in the MPEG-4 File
    Format.
*/


#ifndef __AtomDefs_H__
#define __AtomDefs_H__

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_BIN_STREAM_H_INCLUDED
#include "oscl_bin_stream.h"
#endif

#include "pv_mp4ffcomposer_config.h"
static const int32 MEDIA_DATA_IN_MEMORY = 0;
static const int32 MEDIA_DATA_ON_DISK = 1;

static const uint32 DEFAULT_PRESENTATION_TIMESCALE = 1000; // For milliseconds
static const uint32 INITIAL_TRACK_ID = 1; // Initial track ID for first track added to this movie

static const int32 MEDIA_TYPE_UNKNOWN = 10;

// PVA_FF_MediaInformationHeaderAtom types
static const int32 MEDIA_INFORMATION_HEADER_TYPE_AUDIO = 0;
static const int32 MEDIA_INFORMATION_HEADER_TYPE_VISUAL = 1;
static const int32 MEDIA_INFORMATION_HEADER_TYPE_HINT = 2;
static const int32 MEDIA_INFORMATION_HEADER_TYPE_MPEG4 = 3;

static const int32 UNKNOWN_ATOM = 0;
static const int32 UNKNOWN_DESCRIPTOR = 1;
static const int32 UNKNOWN_HANDLER = 2;

static const int32 CODING_TYPE_I = 0;
static const int32 CODING_TYPE_P = 1;
static const int32 CODING_TYPE_B = 2;
static const int32 CODING_TYPE_SPRITE = 3;

// Mpeg-4 file types
static const int32 FILE_TYPE_AUDIO = 1;
static const int32 FILE_TYPE_VIDEO = 2;
static const int32 FILE_TYPE_AUDIO_VIDEO = 3; // logical ORing of above
static const int32 FILE_TYPE_STILL_IMAGE = 4;
static const int32 FILE_TYPE_STILL_IMAGE_AUDIO = 5; // logical ORing of above
static const int32 FILE_TYPE_TIMED_TEXT = 8;
static const int32 FILE_TYPE_AUDIO_TEXT = 9;
static const int32 FILE_TYPE_VIDEO_TEXT = 10;
static const int32 FILE_TYPE_AUDIO_VIDEO_TEXT = 11;

// Scalability settings on the Mpeg4 file
static const int32 STREAM_SCALABILITY_NONE = 0;
static const int32 STREAM_SCALABILITY_TEMPORAL = 1;
static const int32 STREAM_SCALABILITY_SPATIAL = 2;
static const int32 STREAM_SCALABILITY_BOTH = 3; // logical ORing of above

static const uint32 INVALID_TRACK_ID = 0;

// The following are the allowable protocols with respect to the hint tracks:
// HINT_PROTOCOL_TEMPORAL_SCALABILITY - Video stream encoded with PacketVideo Temporal Scalability
// HINT_PROTOCOL_SPATIAL_SCALABILITY - Video stream encoded with PacketVideo Spatial Scalability
// 'pvst' - Video stream encoded with both PacketVideo Spatial and Temporal scalability
// HINT_PROTOCOL_BASE_LAYER_ONLY - Video stream encoded with Base Layer only
// *** Note that only HINT_PROTOCOL_TEMPORAL_SCALABILITY and HINT_PROTOCOL_BASE_LAYER_ONLY video protocols are supported in version 1.0 ***
// 'pvau' - Audio encoding with simple hint track


static const int32 OBJ_DESCR_TAG			= 0x1;
static const int32 INITIAL_OBJ_DESCR_TAG	= 0x2;
static const int32 ES_DESCR_TAG			= 0x3;
static const int32 DEC_CONFIG_DESCR_TAG	= 0x4;
static const int32 DEC_SPEC_INFO_TAG		= 0x5;
static const int32 SL_CONFIG_DESCR_TAG	= 0x6;
//
static const int32 IPMP_DESCR_PTR_TAG		= 0xa;
static const int32 IPMP_DESCR_TAG			= 0xb;
//
static const int32 ES_ID_INC_DESCR_TAG	= 0xe;
static const int32 ES_ID_REF_DESCR_TAG	= 0xf;
//
static const int32 MP4_IOD_TAG	= 0x10;
static const int32 MP4_OD_TAG	= 0x11;



#define FourCharConstToUint32(a, b, c, d) ( (uint32(a) << 24) | (uint32(b) << 16) | (uint32(c) << 8) | uint32(d) )

const uint32 HINT_PROTOCOL_BASE_LAYER_ONLY = FourCharConstToUint32('p', 'v', 'b', 'l');
const uint32 HINT_PROTOCOL_TEMPORAL_SCALABILITY = FourCharConstToUint32('p', 'v', 't', 's');
const uint32 HINT_PROTOCOL_SPATIAL_SCALABILITY = FourCharConstToUint32('p', 'v', 's', 's');
const uint32 HINT_PROTOCOL_SPATIAL_TEMPORAL = FourCharConstToUint32('p', 'v', 's', 't');
const uint32 HINT_PROTOCOL_STILL_IMAGE = FourCharConstToUint32('p', 'v', 's', 'i');
const uint32 HINT_PROTOCOL_AUDIO = FourCharConstToUint32('p', 'v', 'a', 'u');
const uint32 PACKETVIDEO_FOURCC = FourCharConstToUint32('p', 'v', 'm', 'm');


const uint32    MOVIE_ATOM = FourCharConstToUint32('m', 'o', 'o', 'v');
const uint32    MOVIE_HEADER_ATOM = FourCharConstToUint32('m', 'v', 'h', 'd');
const uint32    TRACK_ATOM = FourCharConstToUint32('t', 'r', 'a', 'k');
const uint32    ESD_ATOM = FourCharConstToUint32('e', 's', 'd', 's');
const uint32    TRACK_HEADER_ATOM = FourCharConstToUint32('t', 'k', 'h', 'd');
const uint32    TRACK_REFERENCE_ATOM = FourCharConstToUint32('t', 'r', 'e', 'f');
const uint32    MEDIA_ATOM = FourCharConstToUint32('m', 'd', 'i', 'a');
const uint32    EDIT_ATOM = FourCharConstToUint32('e', 'd', 't', 's');
const uint32    EDIT_LIST_ATOM = FourCharConstToUint32('e', 'l', 's', 't');
const uint32    MEDIA_HEADER_ATOM = FourCharConstToUint32('m', 'd', 'h', 'd');
const uint32    HANDLER_ATOM = FourCharConstToUint32('h', 'd', 'l', 'r');
const uint32    MEDIA_INFORMATION_ATOM = FourCharConstToUint32('m', 'i', 'n', 'f');
const uint32    VIDEO_MEDIA_HEADER_ATOM = FourCharConstToUint32('v', 'm', 'h', 'd');
const uint32    SOUND_MEDIA_HEADER_ATOM = FourCharConstToUint32('s', 'm', 'h', 'd');
const uint32    HINT_MEDIA_HEADER_ATOM = FourCharConstToUint32('h', 'm', 'h', 'd');
const uint32    MPEG4_MEDIA_HEADER_ATOM = FourCharConstToUint32('n', 'm', 'h', 'd');
const uint32    DATA_INFORMATION_ATOM = FourCharConstToUint32('d', 'i', 'n', 'f');
const uint32    DATA_REFERENCE_ATOM = FourCharConstToUint32('d', 'r', 'e', 'f');
const uint32    DATA_ENTRY_URL_ATOM = FourCharConstToUint32('u', 'r', 'l', ' ');
const uint32    DATA_ENTRY_URN_ATOM = FourCharConstToUint32('u', 'r', 'n', ' ');
const uint32    SAMPLE_TABLE_ATOM = FourCharConstToUint32('s', 't', 'b', 'l');
const uint32    TIME_TO_SAMPLE_ATOM = FourCharConstToUint32('s', 't', 't', 's');
const uint32    COMPOSITION_OFFSET_ATOM = FourCharConstToUint32('c', 't', 't', 's');
const uint32    SAMPLE_DESCRIPTION_ATOM = FourCharConstToUint32('s', 't', 's', 'd');
const uint32    SAMPLE_SIZE_ATOM = FourCharConstToUint32('s', 't', 's', 'z');
const uint32    SAMPLE_TO_CHUNK_ATOM = FourCharConstToUint32('s', 't', 's', 'c');
const uint32    CHUNK_OFFSET_ATOM = FourCharConstToUint32('s', 'r', 'c', 'o');
const uint32    SYNC_SAMPLE_ATOM = FourCharConstToUint32('s', 't', 's', 's');
const uint32    SHADOW_SYNC_SAMPLE_ATOM = FourCharConstToUint32('s', 't', 's', 'h');
const uint32    DEGRADATION_PRIORITY_ATOM = FourCharConstToUint32('s', 't', 'd', 'p');
const uint32    OBJECT_DESCRIPTOR_ATOM = FourCharConstToUint32('i', 'o', 'd', 's');
const uint32    MEDIA_DATA_ATOM = FourCharConstToUint32('m', 'd', 'a', 't');
const uint32    SKIP_ATOM = FourCharConstToUint32('f', 'r', 'e', 'e');
const uint32    FREE_SPACE_ATOM = FourCharConstToUint32('s', 'k', 'i', 'p');
const uint32    USER_DATA_ATOM = FourCharConstToUint32('u', 'd', 't', 'a');
const uint32    MEDIA_TYPE_AUDIO = FourCharConstToUint32('s', 'o', 'u', 'n');
const uint32    MEDIA_TYPE_VISUAL = FourCharConstToUint32('v', 'i', 'd', 'e');
const uint32	MEDIA_TYPE_TEXT = FourCharConstToUint32('t', 'e', 'x', 't');

const uint32    MPEG_SAMPLE_ENTRY = FourCharConstToUint32('m', 'p', '4', 's');
const uint32    AUDIO_SAMPLE_ENTRY = FourCharConstToUint32('m', 'p', '4', 'a');
const uint32    VIDEO_SAMPLE_ENTRY = FourCharConstToUint32('m', 'p', '4', 'v');

const uint32    FILE_TYPE_ATOM = FourCharConstToUint32('f', 't', 'y', 'p');
const uint32    PVMM_BRAND = FourCharConstToUint32('p', 'v', 'm', 'm');
const uint32    BRAND_3GPP4 = FourCharConstToUint32('3', 'g', 'p', '4');
const uint32    BRAND_3GPP5 = FourCharConstToUint32('3', 'g', 'p', '5');
const uint32    BRAND_3GPP6 = FourCharConstToUint32('3', 'g', 'p', '6');
const uint32    BRAND_MPEG4 = FourCharConstToUint32('m', 'p', '4', '1');

const uint32    TREF_TYPE_OD = FourCharConstToUint32('m', 'p', 'o', 'd');
const uint32    TREF_TYPE_DEPEND = FourCharConstToUint32('d', 'p', 'n', 'd');

const uint32    TEXT_SAMPLE_ENTRY = FourCharConstToUint32('t', 'x', '3', 'g');
const uint32    AMR_SAMPLE_ENTRY = FourCharConstToUint32('s', 'a', 'm', 'r');
const uint32    AMR_WB_SAMPLE_ENTRY = FourCharConstToUint32('s', 'a', 'w', 'b');
const uint32    H263_SAMPLE_ENTRY = FourCharConstToUint32('s', '2', '6', '3');
const uint32    AMR_SPECIFIC_ATOM = FourCharConstToUint32('d', 'a', 'm', 'r');
const uint32    H263_SPECIFIC_ATOM = FourCharConstToUint32('d', '2', '6', '3');
const uint32    AVC_SAMPLE_ENTRY = FourCharConstToUint32('a', 'v', 'c', '1');
const uint32    AVC_CONFIGURATION_BOX = FourCharConstToUint32('a', 'v', 'c', 'C');

const uint32    BRAND_MMP4 = FourCharConstToUint32('m', 'm', 'p', '4');


const uint32    ASSET_INFO_TITLE_ATOM = FourCharConstToUint32('t', 'i', 't', 'l');
const uint32    ASSET_INFO_DESCP_ATOM = FourCharConstToUint32('d', 's', 'c', 'p');
const uint32    ASSET_INFO_PERF_ATOM = FourCharConstToUint32('p', 'e', 'r', 'f');
const uint32    ASSET_INFO_AUTHOR_ATOM = FourCharConstToUint32('a', 'u', 't', 'h');
const uint32    ASSET_INFO_GENRE_ATOM = FourCharConstToUint32('g', 'n', 'r', 'e');
const uint32    ASSET_INFO_RATING_ATOM = FourCharConstToUint32('r', 't', 'n', 'g');
const uint32    ASSET_INFO_CLSF_ATOM = FourCharConstToUint32('c', 'l', 's', 'f');
const uint32    ASSET_INFO_KEYWORD_ATOM = FourCharConstToUint32('k', 'y', 'w', 'd');
const uint32    ASSET_INFO_LOCINFO_ATOM = FourCharConstToUint32('l', 'o', 'c', 'i');
const uint32	ASSET_INFO_CPRT_ATOM =  FourCharConstToUint32('c', 'p', 'r', 't');
const uint32	ASSET_INFO_ALBUM_TITLE_ATOM =  FourCharConstToUint32('a', 'l', 'b', 'm');
const uint32	ASSET_INFO_RECORDING_YEAR_ATOM =  FourCharConstToUint32('y', 'r', 'r', 'c');

// Movie fragment atoms
const uint32    MOVIE_EXTENDS_ATOM = FourCharConstToUint32('m', 'v', 'e', 'x');
const uint32    MOVIE_EXTENDS_HEADER_ATOM = FourCharConstToUint32('m', 'e', 'h', 'd');
const uint32    TRACK_EXTENDS_ATOM = FourCharConstToUint32('t', 'r', 'e', 'x');
const uint32    MOVIE_FRAGMENT_ATOM = FourCharConstToUint32('m', 'o', 'o', 'f');
const uint32    MOVIE_FRAGMENT_HEADER_ATOM = FourCharConstToUint32('m', 'f', 'h', 'd');
const uint32    TRACK_FRAGMENT_ATOM = FourCharConstToUint32('t', 'r', 'a', 'f');
const uint32    TRACK_FRAGMENT_HEADER_ATOM = FourCharConstToUint32('t', 'f', 'h', 'd');
const uint32    TRACK_RUN_ATOM = FourCharConstToUint32('t', 'r', 'u', 'n');
const uint32    MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM = FourCharConstToUint32('m', 'f', 'r', 'a');
const uint32    TRACK_FRAGMENT_RANDOM_ACCESS_ATOM = FourCharConstToUint32('t', 'f', 'r', 'a');
const uint32    MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_ATOM = FourCharConstToUint32('m', 'f', 'r', 'o');

// MORE TBA - add specific protocols into type value

#define PVMM_VERSION  0x00010000
#define WMF_VERSION   0x00010001
#define VERSION_3GPP4 0x00000300
#define VERSION_MPEG4 0x00000000
#define VERSION_MMP4  0x00000001
#define VERSION_3GPP5 0x00000100
#define VERSION_3GPP6 0x00000600

typedef enum
{
    CODEC_TYPE_AMR_AUDIO = 1,
    CODEC_TYPE_AAC_AUDIO = 2,
    CODEC_TYPE_AMR_WB_AUDIO = 3
} AUDIO_CODEC_TYPES;

typedef enum
{
    CODEC_TYPE_MPEG4_VIDEO = 4,
    CODEC_TYPE_BASELINE_H263_VIDEO = 5,
    CODEC_TYPE_AVC_VIDEO = 6
} VIDEO_CODEC_TYPES;

typedef enum
{
    CODEC_TYPE_TIMED_TEXT = 7

} TEXT_CODEC_TYPES;

#define MPEG4_SP_L0    0x08
#define MPEG4_SP_L1    0x01
#define MPEG4_SP_L2    0x02
#define MPEG4_SP_L3    0x03
#define MPEG4_SSP_L0   0x10
#define MPEG4_SSP_L1   0x11
#define MPEG4_SSP_L2   0x12

#define MIN_NUM_MEDIA_TRACKS 0

#define AMR_INTERLEAVE_BUFFER_SIZE    2048
#define AMR_WB_INTERLEAVE_BUFFER_SIZE 4096
#define AAC_INTERLEAVE_BUFFER_SIZE   12000  // Calc with 96 Kbps as max
#define VIDEO_INTERLEAVE_BUFFER_SIZE 128000 // 2 x Bitrate @ 256 kbps
#define TEXT_INTERLEAVE_BUFFER_SIZE  12000

#define MAX_PV_BASE_SIMPLE_PROFILE_VOL_HEADER_SIZE 28

#define LANGUAGE_CODE_UNKNOWN   0x55C4
#define RATING_ENTITY_UNKNOWN   0
#define RATING_CRITERIA_UNKNOWN 0

#define DEFAULT_INTERLEAVE_INTERVAL 1000

//Encoding mode defines

//Please note that only SOME COMBINATIONS are allowed

//b31.....b4b3b2b1b0 - Bit Mask Definitions
//No bits are set - default mode - no interleaving, meta data at the end
//b0 is set - Media data is interleaved, BIFS and OD are still seperate tracks
//b1 is set - Meta data is upfront, this implies temp files are needed while authoring
//b2 - undefined
//b3 is set - Do not use temp files while authoring
//b4-b31 - Reserved for future use

/**
 * This mode authors non Progressive Downloadable output files using temp files
 * during authoring:
 * Meta data towards the end of the clip
 * Media data is not interleaved. Temp files are used.
 * Media data is authored in separate media atoms for each track
 * Temporary files are written to the same directory as the output file.
 */
#define	PVMP4FF_SET_MEDIA_INTERLEAVE_MODE   0x00000001

#define	PVMP4FF_SET_META_DATA_UPFRONT_MODE  0x00000002

/**
 * This mode authors 3GPP Progressive Downloadable output files:
 * Meta Data is upfront.
 * Media Data is interleaved. Temp files are used.
 * Temporary files are written to the same directory as the output file.
 */
#define	PVMP4FF_3GPP_PROGRESSIVE_DOWNLOAD_MODE 0x00000003

/**
 * This mode authors 3GPP Downloadable output files:
 * Meta Data is towards the end of the clip.
 * Media Data is interleaved.
 * No temp files are used.
 */
#define	PVMP4FF_3GPP_DOWNLOAD_MODE  0x00000009

#define	PVMP4FF_SET_FIRST_SAMPLE_EDIT_MODE  0x00000010

// movie fragment mode
// 6th bit is now reserved movie fragment mode and last bit is reserved for interleaving
#define PVMP4FF_MOVIE_FRAGMENT_MODE 0x00000021

#define DEFAULT_MOVIE_FRAGMENT_DURATION_IN_MS 10000

#endif


