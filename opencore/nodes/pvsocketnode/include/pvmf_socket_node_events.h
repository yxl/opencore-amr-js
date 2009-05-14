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
#ifndef PVMF_SOCKET_NODE_EVENTS_H_INCLUDED
#define PVMF_SOCKET_NODE_EVENTS_H_INCLUDED

#define PVMFSocketNodeEventTypeUUID PVUuid(0xbe846567,0xae17,0x44c8,0x9c,0x9a,0x87,0xb8,0x33,0xa0,0xf9,0x1d)

/* Debug macros*/
#define ENABLE_SOCKET_NODE_DEBUG_INFO_EVENT	0

/**
 * An enumeration of error codes from PVMFSocketNode
 **/
typedef enum
{
    PVMFSocketNodeErrorEventStart = 1024,
    PVMFSocketNodeErrorSocketServerCreateError,
    PVMFSocketNodeErrorSocketServConnectError,
    PVMFSocketNodeErrorUDPSocketRecvError,
    PVMFSocketNodeErrorTCPSocketRecvError,
    PVMFSocketNodeErrorUDPSocketSendError,
    PVMFSocketNodeErrorTCPSocketSendError,
    PVMFSocketNodeError_TCPSocketConnect,
    PVMFSocketNodeError_DNSLookup,
    PVMFSocketNodeErrorSocketFailure,
    PVMFSocketNodeErrorSocketTimeOut,
    PVMFSocketNodeErrorInvalidPortTag,
    PVMFSocketNodeErrorInvalidPortConfig,
    PVMFSocketNodeErrorConnectCancelled,
    PVMFSocketNodeErrorDNSCancelled,
    PVMFSocketNodeErrorEventEnd
} PVMFSocketNodeErrorEventType;

/**
 * An enumeration of info codes from PVMFSocketNode
 **/
typedef enum
{
    PVMFSocketNodeInfoEventStart = 8192,
    PVMFSocketNodeInfoEventPacketTruncated,

    PVMFSocketNodeInfoEventRequestedDNSResolution,
    PVMFSocketNodeInfoEventConnectRequestPending,

    PVMFSocketNodeInfoEventEnd
} PVMFSocketNodeInfoEventType;

#endif //PVMF_SOCKET_NODE_EVENTS_H_INCLUDED


