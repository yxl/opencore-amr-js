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
 * @file pvmf_clientserver_socket_tuneables.h
 */

#ifndef PVMF_CLIENTSERVER_SOCKET_TUNEABLES_H_INCLUDED
#define PVMF_CLIENTSERVER_SOCKET_TUNEABLES_H_INCLUDED

#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_UTILS_H_INCLUDED
#include "pvmi_config_and_capability_utils.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif
#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif


#define PVMFSocketNodeEventTypeUUID PVUuid(0xbe846567,0xae17,0x44c8,0x9c,0x9a,0x87,0xb8,0x33,0xa0,0xf9,0x1d)


#define DEFAULT_SOCKET_NODE_MEDIA_MSGS_NUM 1
#define PVMF_SOCKET_NODE_PORT_VECTOR_RESERVE 10

const PVUid32 PVMF_MEDIA_CMD_SOCKET_CONNECT_FORMAT_ID    = 0x00000200;
const PVUid32 PVMF_MEDIA_CMD_SOCKET_DISCONNECT_FORMAT_ID = 0x00000201;

#define SNODE_DEFAULT_SOCKET_TCP_BUFFER_SIZE		(4096)
#define SNODE_DEFAULT_MAX_TCP_RECV_BUFFER_SIZE		(4*1024)

#define SNODE_DEFAULT_SOCKET_TCP_BUFFER_COUNT		8
#define SNODE_DEFAULT_NUMBER_MEDIADATA_IN_MEMPOOL	4

#define MAX_SOCKET_BUFFER_SIZE						(16*1024)
#define DEFAULT_MAX_NUM_SOCKETMEMPOOL_RESIZES		 100

#define DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_FEEDBACK_PORT  32*1024
#define DEFAULT_MAX_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT  \
                (32*1024 + 2*MAX_SOCKET_BUFFER_SIZE)

#define MIN_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT		0
#define MAX_SOCKETMEMPOOL_RESIZELEN_INPUT_PORT		128*1024

/**
 * An enumeration of error codes from PVMFSocketNode
 **/
typedef enum
{
    PVMFSocketNodeErrorEventStart = 1024,
    PVMFSocketNodeErrorSocketServerCreateError,
    PVMFSocketNodeErrorSocketServConnectError,
    PVMFSocketNodeErrorTCPSocketRecvError,
    PVMFSocketNodeErrorTCPSocketSendError,
    PVMFSocketNodeError_TCPSocketConnect,
    PVMFSocketNodeErrorSocketFailure,
    PVMFSocketNodeErrorSocketTimeOut,
    PVMFSocketNodeErrorInvalidPortTag,
    PVMFSocketNodeErrorInvalidPortConfig,
    PVMFSocketNodeErrorConnectCancelled,
    PVMFSocketNodeErrorEventEnd
} PVMFSocketNodeErrorEventType;

#endif	//PVMF_CLIENTSERVER_SOCKET_TUNEABLES_H_INCLUDED
