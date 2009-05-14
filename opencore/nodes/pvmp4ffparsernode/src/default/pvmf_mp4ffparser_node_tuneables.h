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
#ifndef PVMF_MP4FFPARSER_NODE_TUNEABLES_H_INCLUDED
#define PVMF_MP4FFPARSER_NODE_TUNEABLES_H_INCLUDED

// Track data size and buffer depth values
#define M4V_MAXTRACKDATASIZE			20000
#define M4V_MAXTRACKQUEUEDEPTH			2
#define H263_MAXTRACKDATASIZE			20000
#define H263_MAXTRACKQUEUEDEPTH			2
#define H264_MP4_MAXTRACKDATASIZE		50000
#define H264_MP4_MAXTRACKQUEUEDEPTH		2
#define MPEG4_AUDIO_MAXTRACKDATASIZE	8192
#define MPEG4_AUDIO_MAXTRACKQUEUEDEPTH	2
#define AMR_IETF_MAXTRACKDATASIZE		(5120)
#define AMR_IETF_MAXTRACKQUEUEDEPTH		2
#define AMRWB_IETF_MAXTRACKDATASIZE		(61*10*2)
#define AMRWB_IETF_MAXTRACKQUEUEDEPTH		2
#define TIMEDTEXT_MAXTRACKDATASIZE		4096
#define TIMEDTEXT_MAXTRACKQUEUEDEPTH	2
#define UNKNOWN_MAXTRACKDATASIZE		10000
#define UNKNOWN_MAXTRACKQUEUEDEPTH		3

// PVMF media data memory pool settings
#define PVMP4FF_MEDIADATA_POOLNUM 8
#define PVMP4FF_MEDIADATA_CHUNKSIZE 128

// PVMF media data memory pool settings
#define PVMP4FF_TEXT_TRACK_MEDIADATA_POOLNUM 1

// Number of samples to retrieve per media data
#define M4V_NUMSAMPLES			1
#define H263_NUMSAMPLES			1
#define H264_MP4_NUMSAMPLES		1
#define MPEG4_AUDIO_NUMSAMPLES	4
#define AMR_IETF_NUMFRAMES		10
#define AMRWB_IETF_NUMFRAMES	10
#define TIMEDTEXT_NUMSAMPLES	1
#define UNKNOWN_NUMSAMPLES		1

//max number of attempts at growing the resizable mem pool
#define PVMF_MP4FF_PARSER_NODE_MEM_POOL_GROWTH_LIMIT 2

//optimized parsing mode
#define PVMF_MP4FF_PARSER_NODE_ENABLE_PARSER_OPTIMIZATION 1
#define PVMF_MP4FF_PARSER_NODE_ENABLE_PARSING_MOOF_UPFRONT 0

// EOS duration in seconds to prevent EOS timestamp overlapping with beginning of next playback
#define PVMP4FF_DEFAULT_EOS_DURATION_IN_SEC 1

#define PVMP4FF_UNDERFLOW_THRESHOLD_IN_MS   3000
#define PVMP4FF_UNDERFLOW_STATUS_EVENT_FREQUENCY 10 //every 100 ms
#define PVMP4FF_UNDERFLOW_STATUS_EVENT_CYCLES 1

#define PVMFFF_DEFAULT_THUMB_NAIL_SAMPLE_NUMBER 2

#define PVMP4FF_OMA2_DECRYPTION_BUFFER_SIZE 20*1024

// Number of samples to be retrieved for Best Thumbnail Mode
#define NUMSAMPLES_BEST_THUMBNAIL_MODE 10

//flag, enable(1) or disable(0) functionality to break up aac frames into multiple media messages
#define PVMFMP4FF_BREAKUP_AAC_FRAMES_INTO_MULTIPLE_MEDIA_FRAGS 1

#endif // PVMF_MP4FFPARSER_NODE_TUNEABLES_H_INCLUDED





