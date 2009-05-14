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
#ifndef PVMF_PROTOCOL_ENGINE_NODE_SHARED_LIB_INTERFACE_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_NODE_SHARED_LIB_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_UUID_H_INCLUDED
#include "oscl_uuid.h"
#endif


#define PENODE_SHARED_LIBRARY_INTERFACE OsclUuid(0x8cb21f0c,0xa2fa,0x41e6,0x9a,0xbd,0x1a,0x7e,0xfb,0xd9,0x95,0xef)
#define PENODE_CREATE_LIB_INTERFACE OsclUuid(0x9c9ab8ca,0xf872,0x4449,0x90,0xfd,0xda,0x0a,0x66,0xf8,0x1f,0x0d)
#define PENODE_RELEASE_LIB_INTERFACE OsclUuid(0x78e44ccf,0xb537,0x42d1,0x96,0x66,0xeb,0x17,0x34,0x92,0x17,0xab)


class ProtocolEngineNodeSharedLibraryInterface
{
    public:
        /**
         * Query for the instance of a particular interface based on the interface UUID
         *
         * @param aInterfaceId - give interface UUID.
         **/
        virtual OsclAny* QueryLibInterface(const OsclUuid& aInterfaceId) = 0;
};

#endif // PVMF_PROTOCOL_ENGINE_NODE_SHARED_LIB_INTERFACE_H_INCLUDED

