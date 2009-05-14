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
#include "pvmf_node_shared_lib_interface.h"

#include "pvmf_mp4ffparser_factory.h"

#include "osclconfig.h"

class Mp4NodesInterface: public OsclSharedLibraryInterface,
            public NodeSharedLibraryInterface
{
    public:
        Mp4NodesInterface() {};

        // From NodeSharedLibraryInterface
        OsclAny* QueryNodeInterface(const PVUuid& aNodeUuid, const OsclUuid& aInterfaceId)
        {
            if (KPVMFMP4FFParserNodeUuid == aNodeUuid)
            {
                if (PV_CREATE_NODE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(PVMFMP4FFParserNodeFactory::CreatePVMFMP4FFParserNode));
                }
                else if (PV_RELEASE_NODE_INTERFACE == aInterfaceId)
                {
                    return ((OsclAny*)(PVMFMP4FFParserNodeFactory::DeletePVMFMP4FFParserNode));
                }
            }

            return NULL;
        };

        // From OsclSharedLibraryInterface
        OsclAny* SharedLibraryLookup(const OsclUuid& aInterfaceId)
        {
            if (aInterfaceId == PV_NODE_INTERFACE)
            {
                return OSCL_STATIC_CAST(NodeSharedLibraryInterface*, this);
            }

            return NULL;
        };
};


extern "C"
{
    OsclSharedLibraryInterface *PVGetInterface(void)
    {
        return OSCL_NEW(Mp4NodesInterface, ());
    }
    void PVReleaseInterface(OsclSharedLibraryInterface* aInstance)
    {
        OSCL_DELETE(aInstance);
    }
}

