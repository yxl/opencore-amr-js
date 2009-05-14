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
/*
* ==============================================================================
*  Name        : pv_2way_basic_interfaces.h
*  Part of     :
*  Interface   :
*  Description : Interface class and supporting definitions for the PV2Way SDK
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_2WAY_BASIC_INTERFACES_H_INCLUDED
#define PV_2WAY_BASIC_INTERFACES_H_INCLUDED


//  INCLUDES

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

/**
 * The UUID for PV2WayTrackInfoInterface extension interface in PVAsyncInformationalEvent
 **/
#define PV2WayTrackInfoInterfaceUUID PVUuid(0x0864f8a2,0xde16,0x4ced,0xaa,0x48,0xda,0x18,0x20,0xf0,0x75,0xfb) // TBD: generate a new UUID here

/**
 * PV2WayTrackInfoInterface
 *
 * PV2WayTrackInfoInterface is an optional extension interface exposed by the PVAsyncInformationalEvent class
 * to indicate extended track attributes to the application.
 *
 **/
class PV2WayTrackInfoInterface : public PVInterface
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
           Returns a reference to the format string.

           @return Reference to the format string
        */
        virtual void GetFormatString(PVMFFormatType& aMimeString) = 0;

        /*
           Returns the Format Specific Info associated with this track

           @return A pointer to the Format Specific Info.  NULL if FSI is not present
        */
        virtual uint8* GetFormatSpecificInfo(uint32& aFormatSpecificInfoLen) = 0;

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


#endif //


