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
#ifndef PVMF_DURATIONINFOMESSAGE_EXTENSION_H_INCLUDED
#define PVMF_DURATIONINFOMESSAGE_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif

#define PVMFDurationInfoMessageInterfaceUUID PVUuid(0x28e9d201,0xd566,0x4b06,0xa5,0x68,0x8d,0x65,0x7a,0x2d,0x92,0x64)

class PVMFDurationInfoMessageInterface : public PVInterface
{
    public:
        /*
           Returns the code and space UUID for this info message

           @param aCode Reference to an integer which will be filled in with event code
           @param aUuid Reference to a PVUuid which will be filled in with UUID assocated to event code

           @return None
        */
        virtual void GetCodeUUID(int32& aCode, PVUuid& aUuid) = 0;

        /*
           Returns a pointer to the next info message in the linked list. If NULL,
           the current info message is the last element in the list.

           @return Pointer to the next message in the list. NULL if at the end of list
        */
        virtual uint32 GetDuration() = 0;

        // From PVInterface
        /*
           Increments the reference count for this info message object
        */
        virtual void addRef() = 0;
        /*
           Decrements the reference count for this info message object and deletes
           this object if count goes to 0.
        */
        virtual void removeRef() = 0;
        /*
           Returns the extension interface for the specified UUID if this info
           message object supports it. If the requested extension interface is supported
           true is returned, else false.
        */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
};


#endif // PVMF_DURATIONINFOMESSAGE_EXTENSION_H_INCLUDED

