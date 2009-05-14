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
#ifndef PVMF_LOCAL_DATA_SOURCE_H_INCLUDED
#define PVMF_LOCAL_DATA_SOURCE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#define PVMF_LOCAL_DATASOURCE_UUID PVUuid(0xee849325,0x158a,0x4eb5,0xbd,0x4a,0xb8,0xb4,0x9d,0x77,0x4b,0x92)

class OsclFileHandle;
class PVMFCPMPluginAccessInterfaceFactory;

#define BITMASK_PVMF_SOURCE_INTENT_PLAY          0x00000001
#define BITMASK_PVMF_SOURCE_INTENT_GETMETADATA   0x00000002
#define BITMASK_PVMF_SOURCE_INTENT_PREVIEW       0x00000004
#define BITMASK_PVMF_SOURCE_INTENT_THUMBNAILS    0x00000008

//Source data for local file playback (format type PVMF_MPEG4FF & others)
class PVMFLocalDataSource : public PVInterface
{
    public:
        //default constructor
        PVMFLocalDataSource(OsclFileHandle*aFileHandle = NULL)
                : iFileHandle(aFileHandle)
                , iPreviewMode(false)
                , iIntent(BITMASK_PVMF_SOURCE_INTENT_PLAY)
                , iContentAccessFactory(NULL)
        {
        }

        //copy constructor
        PVMFLocalDataSource(const PVMFLocalDataSource& source) : PVInterface(source)
                , iFileHandle(source.iFileHandle)
                , iPreviewMode(source.iPreviewMode)
                , iIntent(source.iIntent)
                , iContentAccessFactory(source.iContentAccessFactory)
        {}

        /* From PVInterface */
        void addRef()
        {
            iRefCounter++;
        }
        void removeRef()
        {
            iRefCounter--;
        }
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            if (uuid == PVUuid(PVMF_LOCAL_DATASOURCE_UUID))
            {
                iface = this;
                return true;
            }
            else
            {
                iface = NULL;
                return false;
            }
        }
        int32 iRefCounter;

        OsclFileHandle* iFileHandle;
        //Optional file handle.
        //When not NULL, the file will be accessed using this handle.
        //When NULL, the file will be opened using its string URL.

        bool iPreviewMode;
        //Optional field to indicate if the source that is being
        //passed in will be played back in a preview mode.

        uint32 iIntent;
        //Optional field to indicate if the source that is being
        //passed in will be used for play back or just for metadata retrieval

        PVMFCPMPluginAccessInterfaceFactory* iContentAccessFactory;
};



#endif //PVMF_LOCAL_DATA_SOURCE_H_INCLUDED

