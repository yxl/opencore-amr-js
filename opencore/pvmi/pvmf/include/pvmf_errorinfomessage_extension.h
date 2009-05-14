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
#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#define PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif


#define PVMFErrorInfoMessageInterfaceUUID PVUuid(0xbddbb012,0x07e0,0x455e,0x8b,0xe3,0xdc,0x1e,0x4e,0xc1,0xbc,0xaa)

class PVMFErrorInfoMessageInterface : public PVInterface
{
    public:
        /*
           Returns the code and space UUID for this error/info message

           @param aCode Reference to an integer which will be filled in with event code
           @param aUuid Reference to a PVUuid which will be filled in with UUID assocated to event code

           @return None
        */
        virtual void GetCodeUUID(int32& aCode, PVUuid& aUuid) = 0;

        /*
           Returns a pointer to the next error/info message in the linked list. If NULL,
           the current error/info message is the last element in the list.

           @return Pointer to the next message in the list. NULL if at the end of list
        */
        virtual PVMFErrorInfoMessageInterface* GetNextMessage() = 0;

        // From PVInterface
        /*
           Increments the reference count for this error message object
        */
        virtual void addRef() = 0;
        /*
           Decrements the reference count for this error message object and deletes
           this object if count goes to 0.
        */
        virtual void removeRef() = 0;
        /*
           Returns the extension interface for the specified UUID if this error
           message object supports it. If the requested extension interface is supported
           true is returned, else false.
        */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;
};


#endif // PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED

