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

#include "pvmf_basic_errorinfomessage.h"
#include "oscl_mem.h"

OSCL_EXPORT_REF PVMFBasicErrorInfoMessage::PVMFBasicErrorInfoMessage() :
        iEventCode(0),
        iNextMessage(NULL),
        iRefCount(1)
{
}

OSCL_EXPORT_REF PVMFBasicErrorInfoMessage::PVMFBasicErrorInfoMessage(int32 aCode, PVUuid& aUuid, PVMFErrorInfoMessageInterface* aNextMsg) :
        iRefCount(1)
{
    // Save the event info
    iEventCode = aCode;
    iEventUuid = aUuid;
    iNextMessage = aNextMsg;

    // Increment the ref count for the next message in the list
    if (iNextMessage)
    {
        iNextMessage->addRef();
    }
}

OSCL_EXPORT_REF PVMFBasicErrorInfoMessage::~PVMFBasicErrorInfoMessage()
{
    // If the destructor is called directly (instead of via removeRef())
    // then the ref count for the next message in the list needs to decremented
    if (iNextMessage)
    {
        iNextMessage->removeRef();
        iNextMessage = NULL;
    }
}
OSCL_EXPORT_REF void PVMFBasicErrorInfoMessage::destroy()
{
    OSCL_DELETE(this);
}

OSCL_EXPORT_REF void PVMFBasicErrorInfoMessage::SetEventCodeUUID(int32 aCode, PVUuid aUuid)
{
    // Set event code and UUID
    iEventCode = aCode;
    iEventUuid = aUuid;
}

OSCL_EXPORT_REF void PVMFBasicErrorInfoMessage::SetNextMessage(PVMFErrorInfoMessageInterface& aNextMsg)
{
    // Remove reference to existing one if present
    if (iNextMessage)
    {
        iNextMessage->removeRef();
    }

    // Save the reference and increment ref for it
    iNextMessage = &aNextMsg;
    iNextMessage->addRef();
}

OSCL_EXPORT_REF void PVMFBasicErrorInfoMessage::GetCodeUUID(int32& aCode, PVUuid& aUuid)
{
    // Return event code and UUID
    aCode = iEventCode;
    aUuid = iEventUuid;
}

OSCL_EXPORT_REF PVMFErrorInfoMessageInterface* PVMFBasicErrorInfoMessage::GetNextMessage()
{
    // Return the next message in the list
    return iNextMessage;
}

OSCL_EXPORT_REF void PVMFBasicErrorInfoMessage::addRef()
{
    // Increment this object's ref count
    ++iRefCount;

    // Increment ref count for the next message in the list
    if (iNextMessage)
    {
        iNextMessage->addRef();
    }
}

OSCL_EXPORT_REF void PVMFBasicErrorInfoMessage::removeRef()
{
    // Decrement ref count for the next message in the list
    if (iNextMessage)
    {
        iNextMessage->removeRef();
    }

    // Decrement this object's ref count
    --iRefCount;

    // If ref count reaches 0 then destroy this object automatically
    if (iRefCount <= 0)
    {
        // The next message's refcount has already been decremented so
        // set to NULL so the destructor won't decrement again.
        iNextMessage = NULL;
        // Destroy this instance.
        destroy();
    }
}

OSCL_EXPORT_REF bool PVMFBasicErrorInfoMessage::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    // Only returns the error/info message interface
    if (uuid == PVMFErrorInfoMessageInterfaceUUID)
    {
        PVMFErrorInfoMessageInterface* myInterface = OSCL_STATIC_CAST(PVMFErrorInfoMessageInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }

    return true;
}

