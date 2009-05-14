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

#include "pvmf_metadata_infomessage.h"
#include "oscl_mem.h"

OSCL_EXPORT_REF PVMFMetadataInfoMessage::PVMFMetadataInfoMessage() :
        iEventCode(0),
        iRefCount(1)
{
    iMetadataVector.reserve(14);
    iMetadataVector.clear();
}

OSCL_EXPORT_REF PVMFMetadataInfoMessage::PVMFMetadataInfoMessage(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aMetadataVector, int32 aCode, PVUuid aUuid) :
        iRefCount(1)
{
    // Save the event info
    iEventCode = aCode;
    iEventUuid = aUuid;
    iMetadataVector.clear();
    iMetadataVector = aMetadataVector;
}

OSCL_EXPORT_REF PVMFMetadataInfoMessage::~PVMFMetadataInfoMessage()
{
    // If the destructor is called directly (instead of via removeRef())
    // then the ref count for the next message in the list needs to decremented
    iMetadataVector.clear();
}

OSCL_EXPORT_REF void PVMFMetadataInfoMessage::destroy()
{
    OSCL_DELETE(this);
}

OSCL_EXPORT_REF void PVMFMetadataInfoMessage::SetEventCodeUUID(int32 aCode, PVUuid aUuid)
{
    // Set event code and UUID
    iEventCode = aCode;
    iEventUuid = aUuid;
}

OSCL_EXPORT_REF void PVMFMetadataInfoMessage::GetCodeUUID(int32& aCode, PVUuid& aUuid)
{
    // Return event code and UUID
    aCode = iEventCode;
    aUuid = iEventUuid;
}

OSCL_EXPORT_REF Oscl_Vector<PvmiKvp, OsclMemAllocator>& PVMFMetadataInfoMessage::GetMetadataVector()
{
    // Return the next message in the list
    return iMetadataVector;
}

OSCL_EXPORT_REF void PVMFMetadataInfoMessage::addRef()
{
    // Increment this object's ref count
    ++iRefCount;
}

OSCL_EXPORT_REF void PVMFMetadataInfoMessage::removeRef()
{
    // Decrement this object's ref count
    --iRefCount;

    // If ref count reaches 0 then destroy this object automatically
    if (iRefCount <= 0)
    {
        // The next message's refcount has already been decremented so
        // set to NULL so the destructor won't decrement again.
        // Destroy this instance.
        destroy();
    }
}

OSCL_EXPORT_REF bool PVMFMetadataInfoMessage::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    // Only returns the error/info message interface
    if (uuid == PVMFMetadataInfoMessageInterfaceUUID)
    {
        PVMFMetadataInfoMessageInterface* myInterface = OSCL_STATIC_CAST(PVMFMetadataInfoMessageInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }
    return true;
}

