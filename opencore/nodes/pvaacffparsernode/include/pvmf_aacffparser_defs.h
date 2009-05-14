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

#ifndef PVMF_AACFFPARSER_DEFS_H_INCLUDED
#define PVMF_AACFFPARSER_DEFS_H_INCLUDED

///////////////////////////////////////////////
// Port tags
///////////////////////////////////////////////

/**
 UUID for PV AAC FF parser node error
 **/
#define PVMFAACParserNodeEventTypesUUID PVUuid(0x13f4c2f2,0x2c66,0x46ac,0x95,0x1d,0x7f,0x86,0xb7,0x58,0x14,0x67)

/** Enumerated list of port tags supported by the node,
** for the port requests.
*/
typedef enum
{
    PVMF_AAC_PARSER_NODE_PORT_TYPE_SOURCE
} PVMFAACFFParserOutPortType;

// Capability mime strings
#define PVMF_AAC_PARSER_PORT_OUTPUT_FORMATS "x-pvmf/port/formattype"
#define PVMF_AAC_PARSER_PORT_OUTPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"


// Temporary until actual max track data size if used.
#define MAXTRACKDATASIZE	4096

static const char PVAAC_ALL_METADATA_KEY[] = "all";

#define PVMF_AAC_NUM_METADATA_VALUES 14
// Constant character strings for metadata keys
static const char PVAACMETADATA_TITLE_KEY[] = "title";
static const char PVAACMETADATA_ARTIST_KEY[] = "artist";
static const char PVAACMETADATA_ALBUM_KEY[] = "album";
static const char PVAACMETADATA_YEAR_KEY[] = "year";
static const char PVAACMETADATA_COMMENT_KEY[] = "comment";
static const char PVAACMETADATA_COPYRIGHT_KEY[] = "copyright";
static const char PVAACMETADATA_GENRE_KEY[] = "genre";
static const char PVAACMETADATA_TRACKNUMBER_KEY[] = "tracknumber";
static const char PVAACMETADATA_DURATION_KEY[] = "duration";
static const char PVAACMETADATA_NUMTRACKS_KEY[] = "num-tracks";
static const char PVAACMETADATA_TRACKINFO_BITRATE_KEY[] = "track-info/bit-rate";
static const char PVAACMETADATA_TRACKINFO_SAMPLERATE_KEY[] = "track-info/sample-rate";
static const char PVAACMETADATA_TRACKINFO_AUDIO_FORMAT_KEY[] = "track-info/audio/format";
static const char PVAACMETADATA_SEMICOLON[] = ";";
static const char PVAACMETADATA_CHARENCUTF8[] = "char-encoding=UTF8";
static const char PVAACMETADATA_CHARENCUTF16BE[] = "char-encoding=UTF16BE";
static const char PVAACMETADATA_FORMATID3V1[] = "format=id3v1";
static const char PVAACMETADATA_FORMATID3V11[] = "format=id3v1.1";
static const char PVAACMETADATA_TIMESCALE1000[] = ";timescale=1000";
static const char PVAACMETADATA_INDEX0[] = "index=0";
static const char PVAACMETADATA_RANDOM_ACCESS_DENIED_KEY[] = "random-access-denied";

#define PVMF_AAC_PARSER_NODE_MAX_AUDIO_DATA_MEM_POOL_SIZE   64*1024
#define PVMF_AAC_PARSER_NODE_DATA_MEM_POOL_GROWTH_LIMIT     1
#define PVMF_AAC_PARSER_NODE_MAX_NUM_OUTSTANDING_MEDIA_MSGS 4
#define PVMF_AAC_PARSER_NODE_MEDIA_MSG_SIZE                 128
#define PVMF_AAC_PARSER_NODE_TS_DELTA_DURING_REPOS_IN_MS    10


#define PVMF_AAC_PARSER_NODE_MAX_CPM_METADATA_KEYS 256
// Temporary until actual max track data size if used.
#define MAX_PORTS			2
#define NUM_AAC_FRAMES		1

// using asf specific values until aac specific are known
#define AAC_MIN_DATA_SIZE_FOR_RECOGNITION  512
#define AAC_DATA_OBJECT_PARSING_OVERHEAD   512

#define PVMF_AAC_PARSER_NODE_INCLUDE_ADTS_HEADERS 0

#define PVMF_AAC_PARSER_NODE_TS_DELTA_DURING_REPOS_IN_MS 10	// 10ms


#endif

