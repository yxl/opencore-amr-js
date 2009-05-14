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
#ifndef PVMFRTSPNODEREG_H_INCLUDED
#define PVMFRTSPNODEREG_H_INCLUDED

#ifndef OSCL_SHARED_LIBRARY_H_INCLUDED
#include "oscl_shared_library.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#define KPVMFRTSPStreamingModuleUuid PVUuid(0x608aea40,0x9ca0,0x11dd,0xbd,0x92,0x00,0x02,0xa5,0xd5,0xc5,0x1b)
class StreamingNodesCoreLibraryLoader
{
    public:
        static PVMFNodeInterface* CreateStreamingManagerNode(int32 aPriority);
        static bool DeleteStreamingManagerNode(PVMFNodeInterface* aNode);
};

#endif // PVMFRTSPNODEREG_H_INCLUDED

