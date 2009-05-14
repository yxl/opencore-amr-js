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

#include "pvmf_duration_infomessage.h"
#include "oscl_mem.h"

OSCL_EXPORT_REF PVMFDurationInfoMessage::PVMFDurationInfoMessage() :
        iEventCode(0),
        iDuration(0),
        iRefCount(1)
{
}

OSCL_EXPORT_REF PVMFDurationInfoMessage::PVMFDurationInfoMessage(uint32 aDuration, int32 aCode, PVUuid aUuid) :
        iRefCount(1)
{
    // Save the event info
    iEventCode = aCode;
    iEventUuid = aUuid;
    iDuration = aDuration;
}

OSCL_EXPORT_REF PVMFDurationInfoMessage::~PVMFDurationInfoMessage()
{
    // If the destructor is called directly (instead of via removeRef())
    // then the ref count for the next message in the list needs to decremented
}
OSCL_EXPORT_REF void PVMFDurationInfoMessage::destroy()
{
    OSCL_DELETE(this);
}

OSCL_EXPORT_REF void PVMFDurationInfoMessage::SetEventCodeUUID(int32 aCode, PVUuid aUuid)
{
    // Set event code and UUID
    iEventCode = aCode;
    iEventUuid = aUuid;
}

OSCL_EXPORT_REF void PVMFDurationInfoMessage::GetCodeUUID(int32& aCode, PVUuid& aUuid)
{
    // Return event code and UUID
    aCode = iEventCode;
    aUuid = iEventUuid;
}

OSCL_EXPORT_REF uint32 PVMFDurationInfoMessage::GetDuration()
{
    // Return the next message in the list
    return iDuration;
}

OSCL_EXPORT_REF void PVMFDurationInfoMessage::addRef()
{
    // Increment this object's ref count
    ++iRefCount;
}

OSCL_EXPORT_REF void PVMFDurationInfoMessage::removeRef()
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

OSCL_EXPORT_REF bool PVMFDurationInfoMessage::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    // Only returns the error/info message interface
    if (uuid == PVMFDurationInfoMessageInterfaceUUID)
    {
        PVMFDurationInfoMessageInterface* myInterface = OSCL_STATIC_CAST(PVMFDurationInfoMessageInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }
    return true;
}

