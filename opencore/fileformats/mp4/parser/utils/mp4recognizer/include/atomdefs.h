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
#ifndef ATOMDEFS_H_INCLUDED
#define ATOMDEFS_H_INCLUDED

#ifndef PV_MP4FFPARSER_CONFIG_H_INCLUDED
#include "pv_mp4ffparser_config.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef OSCL_MEM__H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#ifndef OSCL_INT64_UTILS_H_INCLUDED
#include "oscl_int64_utils.h"
#endif
static const int32 MEDIA_DATA_IN_MEMORY = 0;
static const int32 MEDIA_DATA_ON_DISK = 1;

static const uint32 DEFAULT_PRESENTATION_TIMESCALE = 1000; // For milliseconds
static const uint32 INITIAL_TRACK_ID = 1; // Initial track ID for first track added to this movie

static const int32 MEDIA_TYPE_UNKNOWN = 10;

// MediaInformationHeaderAtom types
static const int32 MEDIA_INFORMATION_HEADER_TYPE_AUDIO = 0;
static const int32 MEDIA_INFORMATION_HEADER_TYPE_VISUAL = 1;
static const int32 MEDIA_INFORMATION_HEADER_TYPE_HINT = 2;
static const int32 MEDIA_INFORMATION_HEADER_TYPE_MPEG4 = 3;

static const uint32 UNKNOWN_ATOM = 0;
static const int32 UNKNOWN_DESCRIPTOR = 1;
static const int32 UNKNOWN_HANDLER = 2;

static const int32 CODING_TYPE_I = 0;
static const int32 CODING_TYPE_P = 1;
static const int32 CODING_TYPE_B = 2;
static const int32 CODING_TYPE_SPRITE = 3;

// Mpeg-4 file types
static const int32 FILE_TYPE_AUDIO = 1;
static const int32 FILE_TYPE_VIDEO = 2;
static const int32 FILE_TYPE_AUDIO_VIDEO = 3; // Logical ORing of the two
static const int32 FILE_TYPE_STILL_IMAGE = 4;
static const int32 FILE_TYPE_STILL_IMAGE_AUDIO = 5; // Logical ORing of the two
static const int32 FILE_TYPE_TEXT = 8;
static const int32 FILE_TYPE_TEXT_AUDIO = 9;
static const int32 FILE_TYPE_TEXT_VIDEO = 10;
static const int32 FILE_TYPE_TEXT_AUDIO_VIDEO = 11; // Logical ORing


// Scalability settings on the Mpeg4 file
static const int32 STREAM_SCALABILITY_NONE = 0;
static const int32 STREAM_SCALABILITY_TEMPORAL = 1;
static const int32 STREAM_SCALABILITY_SPATIAL = 2;
static const int32 STREAM_SCALABILITY_BOTH = 3; // Logical ORing of the two

#define FourCharConstToUint32(a, b, c, d) ( (uint32(a) << 24) | (uint32(b) << 16) | (uint32(c) << 8) | uint32(d) )

const uint32 PACKETVIDEO_FOURCC = FourCharConstToUint32('p', 'v', 'm', 'm');
const uint32 PVUSER_DATA_ATOM = FourCharConstToUint32('p', 'v', 'm', 'm');

const uint32    FILE_TYPE_ATOM = FourCharConstToUint32('f', 't', 'y', 'p');

const uint32    UUID_ATOM = FourCharConstToUint32('u', 'u', 'i', 'd');

const uint32    MOVIE_ATOM = FourCharConstToUint32('m', 'o', 'o', 'v');
const uint32    MOVIE_HEADER_ATOM = FourCharConstToUint32('m', 'v', 'h', 'd');
const uint32    TRACK_ATOM = FourCharConstToUint32('t', 'r', 'a', 'k');
const uint32    ESD_ATOM = FourCharConstToUint32('e', 's', 'd', 's');
const uint32    TRACK_HEADER_ATOM = FourCharConstToUint32('t', 'k', 'h', 'd');
const uint32    TRACK_REFERENCE_ATOM = FourCharConstToUint32('t', 'r', 'e', 'f');

//movie fragments
const uint32    MOVIE_FRAGMENT_ATOM = FourCharConstToUint32('m', 'o', 'o', 'f');
const uint32	MOVIE_EXTENDS_ATOM = FourCharConstToUint32('m', 'v', 'e', 'x');
const uint32	MOVIE_EXTENDS_HEADER_ATOM = FourCharConstToUint32('m', 'e', 'h', 'd');
const uint32	TRACK_EXTENDS_ATOM = FourCharConstToUint32('t', 'r', 'e', 'x');
const uint32	MOVIE_FRAGMENT_HEADER_ATOM = FourCharConstToUint32('m', 'f', 'h', 'd');
const uint32	TRACK_FRAGMENT_ATOM = FourCharConstToUint32('t', 'r', 'a', 'f');
const uint32	TRACK_FRAGMENT_HEADER_ATOM = FourCharConstToUint32('t', 'f', 'h', 'd');
const uint32	TRACK_FRAGMENT_RUN_ATOM = FourCharConstToUint32('t', 'r', 'u', 'n');
const uint32	MOVIE_FRAGMENT_RANDOM_ACCESS_ATOM = FourCharConstToUint32('m', 'f', 'r', 'a');
const uint32	MOVIE_FRAGMENT_RANDOM_ACCESS_OFFSET_ATOM = FourCharConstToUint32('m', 'f', 'r', 'o');
const uint32	TRACK_FRAGMENT_RANDOM_ACCESS_ATOM = FourCharConstToUint32('t', 'f', 'r', 'a');

const uint32    HINT_TRACK_REFERENCE_TYPE = FourCharConstToUint32('h', 'i', 'n', 't');
const uint32    DPND_TRACK_REFERENCE_TYPE = FourCharConstToUint32('d', 'p', 'n', 'd');
const uint32    IPIR_TRACK_REFERENCE_TYPE = FourCharConstToUint32('i', 'p', 'i', 'r');
const uint32    MPOD_TRACK_REFERENCE_TYPE = FourCharConstToUint32('m', 'p', 'o', 'd');
const uint32    SYNC_TRACK_REFERENCE_TYPE = FourCharConstToUint32('s', 'y', 'n', 'c');

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
const uint32    CHUNK_OFFSET_ATOM = FourCharConstToUint32('s', 't', 'c', 'o');
const uint32    SYNC_SAMPLE_ATOM = FourCharConstToUint32('s', 't', 's', 's');
const uint32    SHADOW_SYNC_SAMPLE_ATOM = FourCharConstToUint32('s', 't', 's', 'h');
const uint32    DEGRADATION_PRIORITY_ATOM = FourCharConstToUint32('s', 't', 'd', 'p');
const uint32    OBJECT_DESCRIPTOR_ATOM = FourCharConstToUint32('i', 'o', 'd', 's');
const uint32    MEDIA_DATA_ATOM = FourCharConstToUint32('m', 'd', 'a', 't');
const uint32    FREE_SPACE_ATOM = FourCharConstToUint32('f', 'r', 'e', 'e');
const uint32    SKIP_ATOM = FourCharConstToUint32('s', 'k', 'i', 'p');
const uint32    USER_DATA_ATOM = FourCharConstToUint32('u', 'd', 't', 'a');
const uint32    MEDIA_TYPE_AUDIO = FourCharConstToUint32('s', 'o', 'u', 'n');
const uint32    MEDIA_TYPE_VISUAL = FourCharConstToUint32('v', 'i', 'd', 'e');
const uint32    MEDIA_TYPE_HINT = FourCharConstToUint32('h', 'i', 'n', 't');
const uint32    MEDIA_TYPE_OBJECT_DESCRIPTOR = FourCharConstToUint32('o', 'd', 's', 'm');
const uint32    MEDIA_TYPE_CLOCK_REFERENCE = FourCharConstToUint32('c', 'r', 's', 'm');
const uint32    MEDIA_TYPE_SCENE_DESCRIPTION = FourCharConstToUint32('s', 'd', 's', 'm');
const uint32    MEDIA_TYPE_MPEG7 = FourCharConstToUint32('m', '7', 's', 'm');
const uint32    MEDIA_TYPE_OBJECT_CONTENT_INFO = FourCharConstToUint32('o', 'c', 's', 'm');
const uint32    MEDIA_TYPE_IPMP = FourCharConstToUint32('i', 'p', 's', 'm');
const uint32    MEDIA_TYPE_MPEG_J = FourCharConstToUint32('m', 'j', 's', 'm');
const uint32    MEDIA_TYPE_SCALABILITY = FourCharConstToUint32('p', 'v', 's', 'c');
const uint32    MEDIA_TYPE_TEXT = FourCharConstToUint32('t', 'e', 'x', 't');

const uint32    MPEG_SAMPLE_ENTRY = FourCharConstToUint32('m', 'p', '4', 's');
const uint32    AUDIO_SAMPLE_ENTRY = FourCharConstToUint32('m', 'p', '4', 'a');
const uint32    VIDEO_SAMPLE_ENTRY = FourCharConstToUint32('m', 'p', '4', 'v');

const uint32    AMR_SAMPLE_ENTRY_ATOM = FourCharConstToUint32('s', 'a', 'm', 'r');
const uint32    AMRWB_SAMPLE_ENTRY_ATOM = FourCharConstToUint32('s', 'a', 'w', 'b');
const uint32    H263_SAMPLE_ENTRY_ATOM = FourCharConstToUint32('s', '2', '6', '3');

const uint32    AMR_SPECIFIC_ATOM = FourCharConstToUint32('d', 'a', 'm', 'r');
const uint32    H263_SPECIFIC_ATOM = FourCharConstToUint32('d', '2', '6', '3');
const uint32    H263_BITRATE_ATOM = FourCharConstToUint32('b', 'i', 't', 'r');


const uint32    COPYRIGHT_ATOM = FourCharConstToUint32('c', 'p', 'r', 't');

const uint32    NULL_MEDIA_HEADER_ATOM = FourCharConstToUint32('n', 'm', 'h', 'd');
const uint32    FONT_TABLE_ATOM = FourCharConstToUint32('f', 't', 'a', 'b');
const uint32    TEXT_SAMPLE_ENTRY = FourCharConstToUint32('t', 'x', '3', 'g');

// Part of udta at file level
const uint32    PV_CONTENT_TYPE_ATOM = FourCharConstToUint32('p', 'v', 'c', 't');

const uint32    ASSET_INFO_TITLE_ATOM = FourCharConstToUint32('t', 'i', 't', 'l');
const uint32    ASSET_INFO_DESCP_ATOM = FourCharConstToUint32('d', 's', 'c', 'p');
const uint32    ASSET_INFO_PERF_ATOM = FourCharConstToUint32('p', 'e', 'r', 'f');
const uint32    ASSET_INFO_AUTHOR_ATOM = FourCharConstToUint32('a', 'u', 't', 'h');
const uint32    ASSET_INFO_GENRE_ATOM = FourCharConstToUint32('g', 'n', 'r', 'e');
const uint32    ASSET_INFO_RATING_ATOM = FourCharConstToUint32('r', 't', 'n', 'g');
const uint32    ASSET_INFO_CLSF_ATOM = FourCharConstToUint32('c', 'l', 's', 'f');
const uint32    ASSET_INFO_KEYWORD_ATOM = FourCharConstToUint32('k', 'y', 'w', 'd');
const uint32    ASSET_INFO_LOCATION_ATOM = FourCharConstToUint32('l', 'o', 'c', 'i');
const uint32    ASSET_INFO_ALBUM_ATOM = FourCharConstToUint32('a', 'l', 'b', 'm');
const uint32    ASSET_INFO_YRRC_ATOM = FourCharConstToUint32('y', 'r', 'r', 'c');

//AVC related atoms
const uint32    AVC_SAMPLE_ENTRY = FourCharConstToUint32('a', 'v', 'c', '1');
const uint32    AVC_CONFIGURATION_BOX = FourCharConstToUint32('a', 'v', 'c', 'C');
const uint32    AVC_SAMPLE_DEPENDENCY_TYPE_BOX = FourCharConstToUint32('s', 'd', 't', 'p');
const uint32    AVC_SAMPLE_TO_GROUP_BOX = FourCharConstToUint32('s', 'b', 'g', 'p');
const uint32    AVC_SAMPLE_GROUP_DESCRIPTION_BOX = FourCharConstToUint32('s', 'g', 'p', 'd');
const uint32    AVC_SUBSEQUENCE_DESCRIPTION_BOX = FourCharConstToUint32('a', 'v', 's', 's');
const uint32    AVC_LAYER_DESCRIPTION_BOX = FourCharConstToUint32('a', 'v', 'l', 'l');
const uint32    AVC_SAMPLE_DEPENDENCY_BOX = FourCharConstToUint32('s', 'd', 'e', 'p');

const uint32    MPEG4_BITRATE_BOX = FourCharConstToUint32('b', 't', 'r', 't');
const uint32    MPEG4_EXTENSION_DESCRIPTORS_BOX = FourCharConstToUint32('m', '4', 'd', 's');

//Pixel Aspect Ratio
const uint32  PIXELASPECTRATIO_BOX = FourCharConstToUint32('p', 'a', 's', 'p');


const uint32    WMF_BRAND = FourCharConstToUint32('w', 'm', 'f', ' ');
const uint32    BRAND_3GPP4 = FourCharConstToUint32('3', 'g', 'p', '4');
const uint32    MOBILE_MP4 = FourCharConstToUint32('m', 'm', 'p', '4');
const uint32    BRAND_3GPP5 = FourCharConstToUint32('3', 'g', 'p', '5');
const uint32    BRAND_3GP6 = FourCharConstToUint32('3', 'g', 'p', '6');
const uint32    BRAND_3GR6 = FourCharConstToUint32('3', 'g', 'r', '6');
const uint32    BRAND_MP41 = FourCharConstToUint32('m', 'p', '4', '1');
const uint32    BRAND_MP42 = FourCharConstToUint32('m', 'p', '4', '2');
const uint32    BRAND_ISOM = FourCharConstToUint32('i', 's', 'o', 'm');


/* OMA2 DRM Atoms */
const uint32    ENCRYPTED_AUDIO_SAMPLE_ENTRY = FourCharConstToUint32('e', 'n', 'c', 'a');
const uint32    ENCRYPTED_VIDEO_SAMPLE_ENTRY = FourCharConstToUint32('e', 'n', 'c', 'v');
const uint32    ENCRYPTED_TEXT_SAMPLE_ENTRY = FourCharConstToUint32('e', 'n', 'c', 't');

const uint32    PROTECTION_SCHEME_INFO_BOX = FourCharConstToUint32('s', 'i', 'n', 'f');
const uint32    ORIGINAL_FORMAT_BOX = FourCharConstToUint32('f', 'r', 'm', 'a');
const uint32    SCHEME_TYPE_BOX = FourCharConstToUint32('s', 'c', 'h', 'm');
const uint32    SCHEME_INFORMATION_BOX = FourCharConstToUint32('s', 'c', 'h', 'i');
const uint32    MUTABLE_DRM_INFORMATION = FourCharConstToUint32('m', 'd', 'r', 'i');
const uint32    OMADRM_TRANSACTION_TRACKING_BOX = FourCharConstToUint32('o', 'd', 't', 't');
const uint32    OMADRM_RIGHTS_OBJECT_BOX = FourCharConstToUint32('o', 'd', 'r', 'b');
const uint32    OMADRM_KMS_BOX = FourCharConstToUint32('o', 'd', 'k', 'm');
// Text Sample Modifier Atoms
const uint32    TEXT_STYLE_BOX = FourCharConstToUint32('s', 't', 'y', 'l');
const uint32    TEXT_HIGHLIGHT_BOX = FourCharConstToUint32('h', 'l', 'i', 't');
const uint32    TEXT_HILIGHT_COLOR_BOX = FourCharConstToUint32('h', 'c', 'l', 'r');
const uint32    TEXT_KARAOKE_BOX = FourCharConstToUint32('k', 'r', 'o', 'k');
const uint32    TEXT_SCROLL_DELAY_BOX = FourCharConstToUint32('d', 'l', 'a', 'y');
const uint32    TEXT_HYPER_TEXT_BOX = FourCharConstToUint32('h', 'r', 'e', 'f');
const uint32    TEXT_OVER_RIDE_BOX = FourCharConstToUint32('t', 'b', 'o', 'x');
const uint32    TEXT_BLINK_BOX = FourCharConstToUint32('b', 'l', 'n', 'k');




const uint32	ITUNES_ALBUM_ATOM  = FourCharConstToUint32('©', 'a', 'l', 'b');
const uint32	ITUNES_ARTIST1_ATOM = FourCharConstToUint32('©', 'a', 'r', 't');
const uint32	ITUNES_ARTIST2_ATOM = FourCharConstToUint32('©', 'A', 'R', 'T');
const uint32	ITUNES_ALBUM_ARTIST_ATOM = FourCharConstToUint32('a', 'A', 'R', 'T');
const uint32	ITUNES_COMMENT_ATOM = FourCharConstToUint32('©', 'c', 'm', 't');
const uint32	ITUNES_YEAR_ATOM = FourCharConstToUint32('©', 'd', 'a', 'y');
const uint32	ITUNES_SONG_TITLE_ATOM = FourCharConstToUint32('©', 'n', 'a', 'm');
const uint32	ITUNES_GENRE1_ATOM = FourCharConstToUint32('©', 'g', 'e', 'n');
const uint32	ITUNES_GENRE2_ATOM = FourCharConstToUint32('g', 'n', 'r', 'e');
const uint32	ITUNES_TRACK_NUMBER_ATOM = FourCharConstToUint32('t', 'r', 'k', 'n');
const uint32	ITUNES_DISK_NUMBER_ATOM = FourCharConstToUint32('d', 'i', 's', 'k');
const uint32	ITUNES_COMPOSER_ATOM = FourCharConstToUint32('©', 'w', 'r', 't');
const uint32	ITUNES_ENCODER_TOOL_ATOM = FourCharConstToUint32('©', 't', 'o', 'o');
const uint32    ITUNES_ENCODEDBY_ATOM = FourCharConstToUint32('@', 'e', 'n', 'c');
const uint32	ITUNES_BPM_ATOM = FourCharConstToUint32('t', 'm', 'p', 'o');
const uint32	ITUNES_COMPILATION_ATOM = FourCharConstToUint32('c', 'p', 'i', 'l');
const uint32	ITUNES_ART_WORK_ATOM = FourCharConstToUint32('c', 'o', 'v', 'r');
const uint32	ITUNES_GROUPING1_ATOM = FourCharConstToUint32('©', 'g', 'r', 'p');
const uint32	ITUNES_GROUPING2_ATOM = FourCharConstToUint32('g', 'r', 'u', 'p');
const uint32	ITUNES_LYRICS_ATOM = FourCharConstToUint32('©', 'l', 'y', 'r');
const uint32	ITUNES_DESCRIPTION_ATOM = FourCharConstToUint32('d', 'e', 's', 'c');
const uint32	ITUNES_COPYRIGHT_ATOM = FourCharConstToUint32('c', 'p', 'r', 't');
const uint32	META_DATA_ATOM = FourCharConstToUint32('m', 'e', 't', 'a');
const uint32	ITUNES_FREE_FORM_DATA_NAME_ATOM = FourCharConstToUint32('n', 'a', 'm', 'e');
const uint32	ITUNES_MDIRAPPL_HDLR_PART1 = FourCharConstToUint32('m', 'd', 'i', 'r');
const uint32	ITUNES_MDIRAPPL_HDLR_PART2 = FourCharConstToUint32('a', 'p', 'p', 'l');
const uint32    ITUNES_ILST_ATOM = FourCharConstToUint32('i', 'l', 's', 't');
const uint32	ITUNES_ILST_DATA_ATOM = FourCharConstToUint32('d', 'a', 't', 'a');
const uint32	ITUNES_MEAN_ATOM = FourCharConstToUint32('m', 'e', 'a', 'n');
const uint32	ITUNES_FREE_FORM_ATOM = FourCharConstToUint32('-', '-', '-', '-');
const uint32    ITUNES_TRACK_SUBTITLE_ATOM = FourCharConstToUint32('@', 's', 't', '3');
const uint32    ITUNES_CONTENT_RATING_ATOM = FourCharConstToUint32('r', 't', 'n', 'g');  // Same tag as ASSET_INFO_RATING_ATOM



//For iTunes
#define ITUNES_FREE_FORM_DATA_ATOM_TYPE_NORM "iTunNORM"
#define ITUNES_FREE_FORM_DATA_ATOM_TYPE_TOOL "tool"
#define ITUNES_FREE_FORM_DATA_ATOM_TYPE_CDDB1 "iTunes_CDDB_1"
#define ITUNES_FREE_FORM_DATA_ATOM_TYPE_CDDB_TRACKNUMBER "iTunes_CDDB_TrackNumber"
#define ITUNES_FREE_FORM_DATA_ATOM_TYPE_CDDB_IDS "iTunes_CDDB_IDs"


// VARIOUS DESCRIPTOR TAGS CURRENTLY IN USE

#define ES_DESCRIPTOR_TAG             0x03
#define ES_ID_INC_TAG                 0x0E
#define ES_ID_REF_TAG                 0x0F
#define DECODER_CONFIG_DESCRIPTOR_TAG 0x04
#define DECODER_SPECIFIC_INFO_TAG     0x05
#define SL_CONFIG_DESCRIPTOR          0x06

#define STREAM_TYPE_AUDIO      0x05
#define STREAM_TYPE_VISUAL     0x04

#define DEFAULT_AUTHORING_MODE 0x00000000

typedef enum
{
    ENoFileType	  = 0x0000,
    E3GP4		  = 0x0001,
    E3GP5		  = 0x0002,
    EISOM		  = 0x0010,
    EMP41		  = 0x0020,
    EMP42		  = 0x0040,
    EMMP4		  = 0x0100,
    EWMF		  = 0x0200,
    EUNKNOWN_TYPE = 0x8000
} FILE_TYPE_MASKS;

typedef enum
{
    CODEC_TYPE_AMR_AUDIO = 1,
    CODEC_TYPE_AAC_AUDIO

} AUDIO_CODEC_TYPES;

typedef enum
{
    CODEC_TYPE_MPEG4_VIDEO = 1,
    CODEC_TYPE_BASELINE_H263_VIDEO,
    CODEC_TYPE_MPEG4_IMAGE

} VIDEO_CODEC_TYPES;

typedef enum
{
    AMR_AUDIO   = 0xd0,
    QCELP_MP4	= 0xE1,
    MPEG4_AUDIO = 0x40,
    MPEG2_AUDIO_LC = 0x67,
    MPEG4_VIDEO = 0x20,
    H263_VIDEO  = 0xc0,
    AMRWB_AUDIO_3GPP = 0xFA,
    AVC_VIDEO   = 0xFB,
    AMR_AUDIO_3GPP = 0xFC,
    TIMED_TEXT  = 0xFD,
    MPEG4_SYS_OD = 0x01,
    MPEG4_SYS_BIFS = 0x01,
    TYPE_UNKNOWN = 0

} OTI_VALUES;

typedef enum
{
    STRING_GENRE = 0,
    INTEGER_GENRE
}GnreVersion;

typedef struct
{
    OSCL_wHeapString<OsclMemAllocator> title;
    OSCL_wHeapString<OsclMemAllocator> author;
    OSCL_wHeapString<OsclMemAllocator> description;
    OSCL_wHeapString<OsclMemAllocator> rating;
    OSCL_wHeapString<OsclMemAllocator> copyRight;
    OSCL_wHeapString<OsclMemAllocator> version;
    OSCL_wHeapString<OsclMemAllocator> creationDate;
} ClipInfo;

typedef enum
{

    ORIGINAL_CHAR_TYPE_UNKNOWN,
    ORIGINAL_CHAR_TYPE_UTF8,
    ORIGINAL_CHAR_TYPE_UTF16

} MP4FFParserOriginalCharEnc;


#define JPEG	0x0000
#define GIF		0x0001
#define MP4_MIN_BYTES_FOR_GETTING_MOVIE_HDR_SIZE 1024

#define DISABLE_REPOS_ON_CLIPS_HAVING_UNEQUAL_TFRA_ENTRY_COUNT 0

#define COUNT_OF_TRUNS_PARSED_THRESHOLD 3000

// when playing from a file
#define MAX_CACHED_TABLE_ENTRIES_FILE		4096
#define TABLE_ENTRIES_THRESHOLD_FILE            1024

// when playing from a memory buffer data stream
#define MAX_CACHED_TABLE_ENTRIES_MBDS		16384
#define TABLE_ENTRIES_THRESHOLD_MBDS		4096

#define BYTE_ORDER_MASK 0xFEFF
#define BYTE_ORDER_MASK_SIZE 2
#define MAX_UTF8_REPRESENTATION 4

// Max limit for number of tracks in a file

#define MAX_LIMIT_FOR_NUMBER_OF_TRACKS 1024

#endif //ATOMDEFS_H_INCLUDED
