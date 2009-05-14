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
#ifndef TSC_NODE_INTERFACE_H_INCLUDED
#define TSC_NODE_INTERFACE_H_INCLUDED

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif


//memory allocator type for this node.
typedef OsclMemAllocator Tsc324mNodeAllocator;

#define Tsc324mNodeCommandBase PVMFGenericNodeCommand<Tsc324mNodeAllocator>  // to remove typedef warning on symbian

class Tsc324mNodeCommand: public Tsc324mNodeCommandBase
{
    public:
        Tsc324mNodeCommand() {}
};

typedef PVMFNodeCommandQueue<Tsc324mNodeCommand, Tsc324mNodeAllocator> Tsc324mNodeCmdQ;

#endif

