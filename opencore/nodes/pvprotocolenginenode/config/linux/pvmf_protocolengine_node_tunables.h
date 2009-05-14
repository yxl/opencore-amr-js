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
#ifndef PVMF_PROTOCOL_ENGINE_NODE_TUNABLES_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_NODE_TUNABLES_H_INCLUDED

#define PDL_HTTP_USER_AGENT			_STRLIT_CHAR("PVPLAYER 04.07.00.01")
#define PDL_HTTP_VERSION_NUMBER		0 // 0 => Http Version1.0 ; 1 => Http Version1.1

// macros for memory pool
#define PVHTTPDOWNLOADOUTPUT_CONTENTDATA_POOLNUM 16
#define PVHTTPSTREAMINGOUTPUT_CONTENTDATA_POOLNUM 1024
#define PVHTTPDOWNLOADOUTPUT_MEDIADATA_CHUNKSIZE 128
#ifndef PVHTTPDOWNLOADOUTPUT_CONTENTDATA_CHUNKSIZE
#define PVHTTPDOWNLOADOUTPUT_CONTENTDATA_CHUNKSIZE 8000
#endif

// macros for timer
#define DEFAULT_MAX_SERVER_INACTIVITY_DURATION_IN_SEC 60
#define DEFAULT_MAX_SERVER_RESPONSE_DURATION_IN_SEC 60
#define DEFAULT_KEEPALIVE_TIMEOUT_IN_SEC 60
#define DEFAULT_MAX_SERVER_RESPONSE_DURATION_IN_SEC_FOR_STOPEOS_LOGGING 4
#define DEFAULT_WALLCLOCK_TIMEOUT_IN_SEC 2
#define DEFAULT_BUFFER_STATUS_CLOCK_TIMEOUT_IN_SEC 2
#define DEFAULT_TIMER_VECTOR_RESERVE_NUMBER 8
#define PVPROTOCOLENGINENODE_TIMER_FREQUENCY 1 // 1 means 1 sec, 1000 means 1msec

// bitmask for http method for extension header
#define BITMASK_HTTPGET 1
#define BITMASK_HTTPPOST 2
#define BITMASK_HTTPHEAD 4

// macros for user-agent
#define DEFAULT_DL_USER_AGNET			_STRLIT_CHAR("PVPlayer/4.0 (Beta release)")
#define DEFAULT_MS_STREAMING_USER_AGENT _STRLIT_CHAR("NSPlayer/10.0.0.3646")

// marcos for download algorithm pre-condition checks
#define PVPROTOCOLENGINE_INIT_DOWNLOAD_TIME_THRESHOLD  1000 // 1000msec
#define PVPROTOCOLENGINE_INIT_DOWNLOAD_SIZE_PERCENTAGE_THRESHOLD  10 // 10% of file size
#define PVPROTOCOLENGINE_INIT_DOWNLOAD_SIZE_THRESHOLD  4096 // 4Kbytes
#define PVPROTOCOLENGINE_INIT_DOWNLOAD_TIME_THRESHOLD_WITH_CLIPBITRATE  10 // 10sec
#define PVPROTOCOLENGINE_JITTER_BUFFER_SIZE_BYTES  10240 // 10Kbytes
#define PVPROTOCOLENGINE_JITTER_BUFFER_SIZE_TIME   4	// 4sec worth of data
#define PVPROTOCOLENGINE_JITTER_BUFFER_SIZE_DLPERCENTAGE   15	// 15% of file size

// marcos for the fixed-point calculation in auto-resume decision
#define PVPROTOCOLENGINE_AUTO_RESUME_FIXED_CALCULATION_MAX_LIMIT_RIGHT_SHIFT_FACTOR  16 // 2^16=65536
#define PVPROTOCOLENGINE_AUTO_RESUME_FIXED_CALCULATION_RIGHT_SHIFT 10

// macros for download byte percentage calculation: downloadSize*100/fileSize
#define PVPROTOCOLENGINE_DOWNLOAD_BYTE_PERCENTAGE_CONVERTION_100 100 // 100
#define PVPROTOCOLENGINE_DOWNLOAD_BYTE_PERCENTAGE_DLSIZE_LIMIT_RIGHT_SHIFT_FACTOR   25 //2^25 =33554432 // 33554432 = 2^32 / 2^7 = 2^25
#define PVPROTOCOLENGINE_DOWNLOAD_BYTE_PERCENTAGE_DLSIZE_RIGHTSHIFT_FACTOR 7 // right shift 7 bits, 2^7 > 100
#define PVPROTOCOLENGINE_MINIMAL_BUFFERING_TIME_FOR_FASTTRACK  7000		// 7000msec

// macros for duration calcuation: fileSize*1000/playback rate
#define PVPROTOCOLENGINE_DOWNLOAD_DURATION_CALCULATION_LIMIT_RIGHT_SHIFT_FACTOR   22 //2^21 =4194304 
#define PVPROTOCOLENGINE_DOWNLOAD_DURATION_CALCULATION_RIGHTSHIFT_FACTOR 10 // right shift 7 bits, 2^7 > 100



// macros for misc stuff
#define PVPROTOCOLENGINE_RESERVED_NUMBER_OF_FRAMES 4
#define PROTOCOLENGINENODE_SPACE_ASCIICODE 0x20
#define BUFFER_SIZE_FOR_ZERO_OUT_DATA_STREAM_OBJECT 10240
#ifndef EVENT_HANDLER_TOTAL
#define EVENT_HANDLER_TOTAL 9
#endif

#endif // PVMF_PROTOCOL_ENGINE_NODE_TUNABLES_H_INCLUDED

