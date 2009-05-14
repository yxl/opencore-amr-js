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
 * @file pvmf_clientserver_socket_factory.cpp
 * @brief Singleton factory for PVMFClientServerSocketNode
 */
#ifndef PVMF_CLIENTSERVER_SOCKET_NODE_FACTORY_H_INCLUDED
#include "pvmf_clientserver_socket_factory.h"
#endif
#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

// Use default DLL entry point for Symbian
#include "oscl_dll.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMFClientServerSocketNodeFactory::CreateClientServerSocketNode(OsclTCPSocket* aSocketHandle, int32 aPriority)
{
    PVMFNodeInterface* node = NULL;
    node = OSCL_NEW(PVMFClientServerSocketNode, (aSocketHandle, aPriority));
    if (node == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }
    return node;
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVMFClientServerSocketNodeFactory::DeleteClientServerSocketNode(PVMFNodeInterface* aNode)
{
    if (aNode)
    {
        OSCL_DELETE(aNode);
        return true;
    }
    return false;
}

