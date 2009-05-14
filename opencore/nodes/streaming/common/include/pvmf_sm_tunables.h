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
#ifndef PVMF_SM_TUNABLES_H_INCLUDED
#define PVMF_SM_TUNABLES_H_INCLUDED

#ifndef PVMF_STREAMING_MEM_CONFIG_H_INCLUDED
#include "pvmf_streaming_mem_config.h"
#endif

/**
 * This file contains all the tunable parameters in streaming manager
 * and its children nodes. These are compile time constants for now
 * and once we have the capability and config interface implemented
 * for all the nodes, these would be configured thru that interface
 */

/*
 * Streaming Manager Node config constants
 */


/*
 * Session Controller Node config constants
 */

/*
 * Jitter Buffer Node config constants
 */
#define PVMF_JITTER_BUFFER_NODE_MEDIA_MSG_SIZE 128

#define DEFAULT_MAX_INACTIVITY_DURATION_IN_MS 70000

#define DEFAULT_NUM_FRAGMENTS_IN_MEDIA_MSG 1

#define DEFAULT_NUM_MEDIA_MSGS_IN_JITTER_BUFFER    2048 //64

#define PVMF_JITTER_BUFFER_NODE_THINNING_PERCENT 10

#define PVMF_JITTER_BUFFER_NODE_THINNING_LIVE_INTERVAL_MS 300000

#define PVMF_JITTER_BUFFER_NODE_THINNING_MIN_DURATION_MS 300000

/* Percentage of the jitter buffer occupancy */
/* High water mark is in terms of buffer occupancy */
/* Low water mark is in terms of time */
#define DEFAULT_JITTER_BUFFER_HIGH_WATER_MARK 0.80
#define DEFAULT_JITTER_BUFFER_LOW_WATER_MARK  0.40

#if (PMVF_JITTER_BUFFER_NODE_USE_NO_RESIZE_ALLOC)
#define MAX_SOCKET_BUFFER_SIZE        1500
#else
//This should be the same size as DEFAULT_RTPPACKETSOURCE_TCP_BUFFER_SIZE.
#define MAX_SOCKET_BUFFER_SIZE        (16*1024)
#define MIN_SOCKET_BUFFER_SIZE        (2*1024)
#endif
#define MAX_FEEDBACK_SOCKET_BUFFER_SIZE   (1500)

/* Based on 128 Kbps for a jitter of 7 seconds */
#define DEFAULT_RTP_SOCKET_MEM_POOL_SIZE_IN_BYTES 112000
//This must be 2*MAX_SOCKET_BUFFER_SIZE because the PS Node allocates
//MAX_SOCKET_BUFFER_SIZE for receive data per request.  It must be at least 2x
//to prevent packet loss due to out-of-memory because the PS Node will
//immediately requests the next packet when one comes in but before the
//previous packet is consumed.
#define DEFAULT_RTCP_SOCKET_MEM_POOL_SIZE_IN_BYTES (2 * MAX_SOCKET_BUFFER_SIZE)
/* Expressed as a percentage to account for PVMF media msg overhead */
#define PVMF_JITTER_BUFFER_NODE_MEM_POOL_OVERHEAD  10
//This should be at least 2x the max socket buffer size to prevent
//packet loss due to waiting for free buffers.
#define MIN_RTP_SOCKET_MEM_POOL_SIZE_IN_BYTES      (8 * MAX_SOCKET_BUFFER_SIZE)
#define SOCKET_MEM_POOL_OVERHEAD_IN_BYTES          256

// ************* PVMFSMSharedBufferAllocWithReSize  constants

// default number of times we can add another buffer
#define DEFAULT_MAX_NUM_SOCKETMEMPOOL_RESIZES      1   // allow one resize only
#define MAX_NUM_SOCKETMEMPOOL_RESIZES              1000
#define MIN_NUM_SOCKETMEMPOOL_RESIZES              0

// default size of added buffers
#define DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_FEEDBACK_PORT  \
                                MIN_RTP_SOCKET_MEM_POOL_SIZE_IN_BYTES
#define DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT  \
                (MIN_RTP_SOCKET_MEM_POOL_SIZE_IN_BYTES + 2*MAX_SOCKET_BUFFER_SIZE)

#define MIN_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT     0
#define MAX_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT     128*1024

// *************

#define MAX_RTCP_SOURCES 31
#define MAX_RTCP_BLOCK_SIZE 512
#define DEFAULT_RTCP_MEM_POOL_BUFFERS 4
#define RTCP_RAND_SEED 666

#define MIN_RTSP_SERVER_INACTIVITY_TIMEOUT_IN_SEC	1
#define MAX_RTSP_SERVER_INACTIVITY_TIMEOUT_IN_SEC	300

// RTCP INTERVAL related
#define DEFAULT_RTCP_INTERVAL_USEC 1000000
#define DEFAULT_RTCP_INTERVAL_SEC  1
#define RTCP_FIXED_INTERVAL_MODE   1


#define DEFAULT_JITTER_BUFFER_DURATION_IN_MS 4000
#define MAX_JITTER_BUFFER_DURATION_IN_MS     100000
#define MIN_JITTER_BUFFER_DURATION_IN_MS     1000

#define DEFAULT_JITTER_BUFFER_UNDERFLOW_THRESHOLD_IN_MS 500
#define DEFAULT_PLAY_BACK_THRESHOLD_IN_MS    250
#define DEFAULT_ESTIMATED_SERVER_KEEPAHEAD_FOR_OOO_SYNC_IN_MS	500

/* Status event happends at 5 Hz (200 msec) intervals */
#define PVMF_JITTER_BUFFER_BUFFERING_STATUS_EVENT_FREQUENCY 5
#define PVMF_JITTER_BUFFER_BUFFERING_STATUS_EVENT_CYCLES 1

#define PVMF_JITTER_BUFFER_NODE_SESSION_DURATION_MONITORING_INTERVAL_MAX_IN_MS (10*60*1000)

/* Non-resizable allocator related constants */
#define PVMF_JB_NO_RESIZE_ALLOC_NUM_CHUNKS_RTP 256
#define PVMF_JB_NO_RESIZE_ALLOC_NUM_CHUNKS_FB  16
#define PVMF_JB_NO_RESIZE_ALLOC_CHUNK_SIZE (MAX_SOCKET_BUFFER_SIZE + 128)

#define PVMF_EOS_TIMER_GAURD_BAND_IN_MS 200

/* Firewall packet related */
#define PVMF_JITTER_BUFFER_NODE_DEFAULT_FIREWALL_PKT_ATTEMPTS 3
#define PVMF_JITTER_BUFFER_NODE_MAX_FIREWALL_PKT_SIZE       8
#define PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_MEMPOOL_SIZE   256
#define PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_DEFAULT_SERVER_RESPONSE_TIMEOUT_IN_MS 1000
#define PVMF_JITTER_BUFFER_NODE_MAX_RTP_FIREWALL_PKT_SIZE   12
#define PVMF_JITTER_BUFFER_NODE_FIREWALL_PKT_DEFAULT_PAUSE_DURATION_IN_MS 20000

#define JITTERBUFFERNODE_MAX_RUNL_TIME_IN_MS 25

#define PVMF_SM_MSHTTP_NODE_DEFAULT_JITTER_BUFFER_SIZE (2*1024*1024)
/* Media layer node related */
#define MEDIALAYERNODE_MAXNUM_MEDIA_DATA     10
#define MEDIALAYERNODE_MAX_RUNL_TIME_IN_MS   25

/* Jitter buffer overflow related */
#define CONSECUTIVE_LOW_BUFFER_COUNT_THRESHOLD 100
#define MIN_PERCENT_OCCUPANCY_THRESHOLD 10
#define JITTER_BUFFER_DURATION_MULTIPLIER_THRESHOLD 1.5
#endif


