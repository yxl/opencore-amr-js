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
#ifndef PVMF_STREAMING_DATA_SOURCE_H_INCLUDED
#define PVMF_STREAMING_DATA_SOURCE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

#define PVMF_STREAMING_DATASOURCE_UUID PVUuid(0x0b3fabc5,0x9f20,0x4197,0xa8,0x1c,0x32,0x54,0x0f,0xb6,0xbe,0x2c)

class OsclFileHandle;
class PVMFCPMPluginAccessInterfaceFactory;

#define BITMASK_PVMF_SOURCE_INTENT_PLAY          0x00000001
#define BITMASK_PVMF_SOURCE_INTENT_GETMETADATA   0x00000002
#define BITMASK_PVMF_SOURCE_INTENT_PREVIEW       0x00000004
#define BITMASK_PVMF_SOURCE_INTENT_THUMBNAILS    0x00000008

//Source data for local file playback (format type PVMF_MPEG4FF & others)
class PVMFStreamingDataSource : public PVInterface
{
    public:
        //default constructor
        PVMFStreamingDataSource(OsclFileHandle*aFileHandle = NULL)
                : iFileHandle(aFileHandle)
                , iPreviewMode(false)
                , iIntent(BITMASK_PVMF_SOURCE_INTENT_PLAY)
        {
        }

        //copy constructor
        PVMFStreamingDataSource(const PVMFStreamingDataSource& source) : PVInterface(source)
                , iFileHandle(source.iFileHandle)
                , iStreamStatsLoggingURL(source.iStreamStatsLoggingURL)
                , iPreviewMode(source.iPreviewMode)
                , iIntent(source.iIntent)
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
            if (uuid == PVUuid(PVMF_STREAMING_DATASOURCE_UUID))
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
        //When not NULL, the sdp file will be accessed using this handle.
        //When NULL, the file will be opened using its string URL.

        OSCL_wHeapString<OsclMemAllocator> iStreamStatsLoggingURL;
        //Optional logging url.
        //When present, streaming stats will be sent to this URL.
        //Typically applies to MS HTTP Streaming sessions

        bool iPreviewMode;
        //Optional field to indicate if the source that is being
        //passed in will be played back in a preview mode.

        uint32 iIntent;
        //Optional field to indicate if the source that is being
        //passed in will be used for play back or just for metadata retrieval

        OSCL_wHeapString<OsclMemAllocator> iProxyName;
        //HTTP proxy name, either ip or dns
        int32	iProxyPort;
        //HTTP proxy port
};


#endif //PVMF_STREAMING_DATA_SOURCE_H_INCLUDED

