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
*  Name        : pv_2way_track_info_impl.h
*  Part of     :
*  Interface   :
*  Description : Interface class and supporting definitions for the PV2Way SDK
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_2WAY_TRACK_INFO_IMPL_H_INCLUDED
#define PV_2WAY_TRACK_INFO_IMPL_H_INCLUDED


#include "pv_2way_basic_interfaces.h"


class PV2WayTrackInfoImpl : public PV2WayTrackInfoInterface

{

    private:

        PVMFFormatType iMimeString;
        uint8* iFormatSpecificInfo;
        uint32 iFormatSpecificInfoLen;
        int32 iEventCode;
        PVUuid iEventUuid;
        int32 iRefCount;

    public:

        PV2WayTrackInfoImpl(PVMFFormatType aMimeString, uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen, int32 aCode, PVUuid aUuid): iRefCount(1)
        {
            iMimeString = aMimeString;

            iFormatSpecificInfo = aFormatSpecificInfo;
            iFormatSpecificInfoLen = aFormatSpecificInfoLen;
            iEventCode = aCode;
            iEventUuid = aUuid;
        }

        /*
             Returns the code and space UUID for this info message

        	 @param aCode Reference to an integer which will be filled in with event code
        	 @param aUuid Reference to a PVUuid which will be filled in with UUID assocated to event code

        	 @return None
          */
        virtual void GetCodeUUID(int32& aCode, PVUuid& aUuid)
        {
            aCode = iEventCode;
            aUuid = iEventUuid;
        }

        /*
           Returns a reference to the format string.

           @return Reference to the format string
        */

        virtual void GetFormatString(PVMFFormatType& aMimeString)
        {

            aMimeString = iMimeString;
        }


        /*    Returns the Format Specific Info associated with this track

            @return A pointer to the Format Specific Info.  NULL if FSI is not present
         */

        virtual uint8* GetFormatSpecificInfo(uint32& aFormatSpecificInfoLen)
        {
            if (aFormatSpecificInfoLen == 0 || iFormatSpecificInfo == NULL)
            {
                aFormatSpecificInfoLen = 0;
                return NULL;
            }
            aFormatSpecificInfoLen = iFormatSpecificInfoLen;
            return iFormatSpecificInfo;
        }

        // From PVInterface

        /*
           Increments the reference count for this info message object
         */

        virtual void addRef()
        {
            ++iRefCount;
        }

        /*
           Decrements the reference count for this info message object and deletes
           this object if count goes to 0.
        */

        virtual void removeRef()
        {
            --iRefCount;
            if (iRefCount == 0)
            {
                OSCL_DELETE(this);
            }
        }

        /*
             Returns the extension interface for the specified UUID if this info
             message object supports it. If the requested extension interface is supported
             true is returned, else false.
         */

        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            if (uuid == PV2WayTrackInfoInterfaceUUID)
            {
                PV2WayTrackInfoInterface* myInterface = OSCL_STATIC_CAST(PV2WayTrackInfoInterface*, this);
                iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
            }
            else
            {
                return false;
            }

            return true;
        }
};

#endif
