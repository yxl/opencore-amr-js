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
#ifndef PVMF_DURATION_INFOMESSAGE_H_INCLUDED
#define PVMF_DURATION_INFOMESSAGE_H_INCLUDED

#ifndef PVMF_DURATIONINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_durationinfomessage_extension.h"
#endif

/*
 This class that provides basic implementation of PVMFDurationInfoMessageInterface.
 This object can carry the code, UUID, and duration information associated with an
 info event.
 */
class PVMFDurationInfoMessage : public PVMFDurationInfoMessageInterface
{
    public:
        /*
         Default constructor. Ref count is initialized to 1.
         This object should always be created with OSCL_NEW.

         Event code is initialized to 0.
         Event UUId is initialized to 0.
         Event data is NULL.
         Next message pointer is NULL
         */
        OSCL_IMPORT_REF PVMFDurationInfoMessage();

        /*
         Constructor with initialization parameters. Ref count is initialized to 1
         This object should always be created with OSCL_NEW.

         @param aCode Event code value
         @param aUuid UUID for the event code's group
         @param aData Pointer to the event's opaque data (optional)
         @param aNextMsg Pointer to the next message in the list (optional)
         */
        OSCL_IMPORT_REF PVMFDurationInfoMessage(uint32 aDuration, int32 aCode = 0, PVUuid aUuid = PVMFDurationInfoMessageInterfaceUUID);

        /*
         A derived class should override this in order to destroy itself
         properly by calling OSCL_DELETE with its own type name.
         */
        OSCL_IMPORT_REF virtual void destroy();

        /*
         Method to set the event code and UUID

         @param aCode Event code value
         @param aUuid UUID for the event code's group
         */
        OSCL_IMPORT_REF void SetEventCodeUUID(int32 aCode, PVUuid aUuid);

        // From PVMFDurationInfoMessageInterface
        OSCL_IMPORT_REF void GetCodeUUID(int32& aCode, PVUuid& aUuid);
        OSCL_IMPORT_REF uint32 GetDuration();

        // From PVInterface
        OSCL_IMPORT_REF virtual void addRef();
        OSCL_IMPORT_REF virtual void removeRef();
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

    protected:
        /* Destructor
        */
        OSCL_IMPORT_REF virtual ~PVMFDurationInfoMessage();

    private:
        int32 iEventCode;
        PVUuid iEventUuid;
        uint32 iDuration;
        int32 iRefCount;
};

#endif // PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED


