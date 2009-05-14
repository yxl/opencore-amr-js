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
#ifndef PVMF_SOCKET_NODE_TUNABLES_H_INCLUDED
#define PVMF_SOCKET_NODE_TUNABLES_H_INCLUDED

/*!
//Tunable parameters for socket node
*/

/*!
//This defines the default buffer size for TCP receive operations.
//The value can be modified at runtime by the
//PVMFSocketNodeExtensionInterface::SetMaxTCPRecvBufferSize API.
*/
#define SNODE_DEFAULT_SOCKET_TCP_BUFFER_SIZE  (64000)

/*!
//This sets the allowed maximum for the buffer size for TCP receive operations.
*/
#define SNODE_DEFAULT_MAX_TCP_RECV_BUFFER_SIZE (1024*1024)

/*!
//This defines the default memory pool size for TCP receive operations.
//The value can be modified at runtime by the
//PVMFSocketNodeExtensionInterface::SetMaxTCPRecvBufferCount API.
*/
#define SNODE_DEFAULT_SOCKET_TCP_BUFFER_COUNT 8

/*!
//This defines the size of the memory pool for the RTCP ports.
*/
#define SNODE_DEFAULT_NUMBER_MEDIADATA_IN_MEMPOOL 4

/*!
// For the UDP multiple receives.
// This is a 1/0 switch to entirely enable or disable the feature
// This can be modified at compile-time by redefining the value
// in an Oscl config file.
*/
#ifndef SNODE_ENABLE_UDP_MULTI_PACKET
#define SNODE_ENABLE_UDP_MULTI_PACKET 1
#endif

/*!
// Define the number of messages in the PVMFMediaFragGroup allocator
// pool.  This is tailored to current JB node behavior.
// JB node will repackage the media messages right away, so the max number
// of messages just needs to match or exceed the JB input port queue depth
// to avoid running out of memory.
// This can be modified at compile-time by redefining the value
// in an Oscl config file.
*/
#ifndef SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_MSGS
#define SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_MSGS 16
#endif

/*!
// The max fragments per message defines the initial reserve on the oscl_vector
// that is used to hold the multiple packets.  It's not a hard limit on the
// number of packets.
// This can be modified at compile-time by redefining the value
// in an Oscl config file.
*/
#ifndef SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_FRAGS_PER_MSG
#define SNODE_UDP_MULTI_FRAG_ALLOCATOR_MAX_FRAGS_PER_MSG 10
#endif

/*!
// Define the max amount of UDP data to receive in one call.
// We may actually receive this much plus one more packet.
// Using a value of zero will effectively disable multi-packet
// receive, although the multi-recv logic will still be exercised.
// To really disable the feature, use the ENABLE switch above.
// This can be modified at compile-time by redefining the value
// in an Oscl config file.
*/
#ifndef SNODE_UDP_MULTI_MAX_BYTES_PER_RECV
#define SNODE_UDP_MULTI_MAX_BYTES_PER_RECV (16*1024)
#define SNODE_UDP_MULTI_MIN_BYTES_PER_RECV (2*1024)
#endif

#endif// PVMF_SOCKET_NODE_TUNABLES_H_INCLUDED



