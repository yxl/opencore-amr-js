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
#ifndef PVMF_NODE_SHARED_LIB_INTERFACE_H_INCLUDED
#define PVMF_NODE_SHARED_LIB_INTERFACE_H_INCLUDED

#ifndef OSCL_SHARED_LIB_INTERFACE_H_INCLUDED
#include "oscl_shared_lib_interface.h"
#endif
#ifndef OSCL_SHARED_LIBRARY_H_INCLUDED
#include "oscl_shared_library.h"
#endif
#ifndef PV_PLAYER_NODE_REGISTRY_INTERFACE_H_INCLUDED
#include "pv_player_node_registry_interface.h"
#endif

#define PV_CREATE_NODE_INTERFACE OsclUuid(0xac8703a0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define PV_RELEASE_NODE_INTERFACE OsclUuid(0xac8703a1,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define PV_NODE_INTERFACE OsclUuid(0xac8703a2,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x66)


class NodeSharedLibraryInterface
{
    public:
        /**
         * Query for the instance of a particular interface based on the request the node UUID
         *
         * @param aNodeUuid This is the UUID to identify which node to retrieve the interface for.
         *
         * @param aInterfacePtr - output parameter filled in with the requested interface
         *                        pointer or NULL if the interface pointer is not supported.
         **/
        virtual OsclAny* QueryNodeInterface(const PVUuid& aNodeUuid, const OsclUuid& aInterfaceId) = 0;
};

#endif // PVMF_NODE_SHARED_LIB_INTERFACE_H_INCLUDED

