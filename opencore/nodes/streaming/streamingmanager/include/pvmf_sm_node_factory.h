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
#ifndef PVMF_SM_NODE_FACTORY_H_INCLUDED
#define PVMF_SM_NODE_FACTORY_H_INCLUDED

#ifndef PVMF_SM_NODE_EVENTS_H_INCLUDED
#include "pvmf_sm_node_events.h"
#endif

/* Forward declaration */
class PVMFNodeInterface;

#define KPVMFStreamingManagerNodeUuid PVUuid(0xd4756fd8,0x7bc0,0x417e,0xb3,0xa4,0x13,0xbd,0xbe,0xa0,0x5d,0x49)

/**
 * PVMFStreamingManagerNodeFactory Class
 *
 * PVMFStreamingManagerNodeFactory class is a singleton class
 * which instantiates and provides access to PVMF streaming manager node.
 * It returns a PVMFNodeInterface reference, the interface class of the
 * PVMFStreamingManagerNode.
 *
 * The client is expected to contain and maintain a pointer to the instance
 * created while the node is active.
 */

class PVMFStreamingManagerNodeFactory
{
    public:
        /**
         * Creates an instance of a PVMFStreamingManagerNode.
         * If the creation fails, this function will leave.
         *
         * @param priority for the underlying node active object
         *
         * @returns A pointer to an instance of PVMFStreamingManagerNode
         * as PVMFNodeInterface reference or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreateStreamingManagerNode(int32 aPriority);

        /**
         * Deletes an instance of PVMFStreamingManagerNode
         * and reclaims all allocated resources.  An instance can be deleted
         * only in the created state. An attempt to delete in any other state
         * will result in a leave with error code OsclErrInvalidState.
         *
         * @param aNode The PVMFStreamingManagerNode instance to be deleted
         * @returns None
         **/
        OSCL_IMPORT_REF static bool DeleteStreamingManagerNode(PVMFNodeInterface* aNode);
};

const int PVMFFSPNotCreated = -1;

#endif


