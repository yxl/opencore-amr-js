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
#ifndef PVMF_SOCKET_NODE_H_INCLUDED
#include "pvmf_socket_node.h"
#endif

void PVMFSocketNodeExtensionInterfaceImpl::addRef()
{
    PVInterfaceImpl<PVMFSocketNodeAllocator>::addRef();
}
void PVMFSocketNodeExtensionInterfaceImpl::removeRef()
{
    PVInterfaceImpl<PVMFSocketNodeAllocator>::removeRef();
}

bool PVMFSocketNodeExtensionInterfaceImpl::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == Uuid())
    {
        addRef();
        iface = this;
        return true;
    }
    else
    {
        iface = NULL;
        return false;
    }
}

PVMFSocketNodeExtensionInterfaceImpl::PVMFSocketNodeExtensionInterfaceImpl(PVMFSocketNode* aContainer): PVInterfaceImpl<PVMFSocketNodeAllocator>(PVMF_SOCKET_NODE_EXTENSION_INTERFACE_UUID), iContainer(aContainer)
{
}

PVMFSocketNodeExtensionInterfaceImpl::~PVMFSocketNodeExtensionInterfaceImpl()
{
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNodeExtensionInterfaceImpl::AllocateConsecutivePorts(PvmfMimeString* aPortConfig,
        uint32& aLowerPortNum,
        uint32& aHigherPortNum, uint32& aStartPortNum)
{
    return iContainer->AllocateConsecutivePorts(aPortConfig, aLowerPortNum, aHigherPortNum, aStartPortNum);
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNodeExtensionInterfaceImpl::SetMaxTCPRecvBufferSize(uint32 aBufferSize)
{
    return iContainer->SetMaxTCPRecvBufferSize(aBufferSize);
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNodeExtensionInterfaceImpl::GetMaxTCPRecvBufferSize(uint32& aSize)
{
    return iContainer->GetMaxTCPRecvBufferSize(aSize);
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNodeExtensionInterfaceImpl::SetMaxTCPRecvBufferCount(uint32 aCount)
{
    return iContainer->SetMaxTCPRecvBufferCount(aCount);
}

OSCL_EXPORT_REF PVMFStatus PVMFSocketNodeExtensionInterfaceImpl::GetMaxTCPRecvBufferCount(uint32& aCount)
{
    return iContainer->GetMaxTCPRecvBufferCount(aCount);
}

OSCL_EXPORT_REF OsclMemPoolResizableAllocator* PVMFSocketNodeExtensionInterfaceImpl::CreateSharedBuffer(const PVMFPortInterface* aPort , uint32 aBufferSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes)
{
    return iContainer->CreateSharedBuffer(aPort, aBufferSize, aExpectedNumberOfBlocksPerBuffer, aResizeSize, aMaxNumResizes);
}
