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
#ifndef PVMFMP4NODEREG_H_INCLUDED
#define PVMFMP4NODEREG_H_INCLUDED

#ifndef OSCL_SHARED_LIBRARY_H_INCLUDED
#include "oscl_shared_library.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

class Mp4NodesCoreLibraryLoader
{
    public:
        static PVMFNodeInterface* CreateMp4ParserNode(int32 aPriority);
        static bool DeleteMp4ParserNode(PVMFNodeInterface* aNode);
};

#endif // PVMFMP4NODEREG_H_INCLUDED

