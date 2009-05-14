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
#ifndef PVMF_MP4FFPARSER_DEFS_H_INCLUDED
#define PVMF_MP4FFPARSER_DEFS_H_INCLUDED

///////////////////////////////////////////////
// Error Events
///////////////////////////////////////////////
//None--node uses the PVMF event codes.

///////////////////////////////////////////////
// Information Events
///////////////////////////////////////////////
//None--node uses the PVMF event codes.

///////////////////////////////////////////////
// Port type tags
///////////////////////////////////////////////
typedef enum
{
    PVMF_MP4FFPARSERNODE_PORT_TYPE_OUTPUT = 1
} PVMFMP4FFParserNodePortType;


// Capability mime strings
#define PVMF_MP4FFPARSER_PORT_INPUT_FORMATS "x-pvmf/parser/mp4ff/input_formats"
#define PVMF_MP4FFPARSER_PORT_INPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=char*"

#define PVMF_MP4FFPARSER_NODE_PSEUDO_STREAMING_BUFFER_DURATION_IN_MS 4000

#define PVMF_MP4FFPARSER_PARSER_NODE_TS_DELTA_DURING_REPOS_IN_MS 10	// 10ms

#define DEFAULT_CAHCE_SIZE 8*1024
#define DEFAULT_ASYNC_READ_BUFFER_SIZE 8*1024
#define DEFAULT_NATIVE_ACCESS_MODE 0
#define MIN_CACHE_SIZE 1024
#define MAX_CACHE_SIZE 64*1024
#define MIN_ASYNC_READ_BUFFER_SIZE 1024
#define MAX_ASYNC_READ_BUFFER_SIZE 64*1024
#define DEFAULT_FILE_HANDLE	NULL

#define MIN_JITTER_BUFFER_DURATION_IN_MS	   1000
#define MAX_JITTER_BUFFER_DURATION_IN_MS	   10000

// Format Type integer defines
#define PVMF_MP4_PARSER_NODE_FORMAT_UNKNOWN		0
#define PVMF_MP4_PARSER_NODE_AMR				1
#define PVMF_MP4_PARSER_NODE_AMRWB				2
#define PVMF_MP4_PARSER_NODE_AMR_IETF			3
#define PVMF_MP4_PARSER_NODE_AMRWB_IETF			4
#define PVMF_MP4_PARSER_NODE_AMR_IF2			5
#define PVMF_MP4_PARSER_NODE_ADIF				6
#define PVMF_MP4_PARSER_NODE_ADTS				7
#define PVMF_MP4_PARSER_NODE_MPEG4_AUDIO		8
#define PVMF_MP4_PARSER_NODE_M4V				9
#define PVMF_MP4_PARSER_NODE_H2631998			10
#define PVMF_MP4_PARSER_NODE_H2632000			11
#define PVMF_MP4_PARSER_NODE_H264_RAW			12
#define PVMF_MP4_PARSER_NODE_H264_MP4			13
#define PVMF_MP4_PARSER_NODE_H264				14
#define PVMF_MP4_PARSER_NODE_3GPP_TIMED_TEXT	15

#endif // PVMF_MP4FFPARSER_DEFS_H_INCLUDED

