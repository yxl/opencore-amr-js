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
#ifndef PVMF_MP4FFPARSER_FACTORY_H_INCLUDED
#define PVMF_MP4FFPARSER_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

// Forward declaration
class PVMFNodeInterface;

#define KPVMFMP4FFParserNodeUuid PVUuid(0xb7d84b4a,0xfa69,0x4e4a,0x9d,0x43,0xa2,0x22,0x83,0x3f,0xb1,0x2f)

/**
 * PVMFMP4FFParserNodeFactory Class
 *
 * PVMFMP4FFParserNodeFactory class is a singleton class which instantiates and provides
 * access to PVMF MP4 file format parser node. It returns a PVMFNodeInterface
 * reference, the interface class of the PVMFMP4FFParserNode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVMFMP4FFParserNodeFactory
{
    public:
        /**
         * Creates an instance of a PVMFMP4FFParserNode. If the creation fails, this function will leave.
         *
         * @param aPriority The active object priority for the node. Default is standard priority if not specified
         * @returns A pointer to an instance of PVMFMP4FFParserNode as PVMFNodeInterface reference or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreatePVMFMP4FFParserNode(int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * Deletes an instance of PVMFMP4FFParserNode
         * and reclaims all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aNode The PVMFMP4FFParserNode instance to be deleted
         * @returns A status code indicating success or failure of deletion
         **/
        OSCL_IMPORT_REF static bool DeletePVMFMP4FFParserNode(PVMFNodeInterface* aNode);
};

#endif // PVMF_MP4FFPARSER_FACTORY_H_INCLUDED
