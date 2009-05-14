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
#ifndef PVMP4FFCN_FACTORY_H_INCLUDED
#define PVMP4FFCN_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVMP4FFCN_TYPES_H_INCLUDED
#include "pvmp4ffcn_types.h"
#endif

// Forward declaration
class PVMFNodeInterface;

#define KPVMp4FFComposerNodeUuid PVUuid(0x0d270304,0x2c46,0x4fe7,0x92,0xa1,0xa7,0xaa,0x1f,0x95,0xa0,0x7a)

/**
 * PVMp4FFComposerNodeFactory Class
 *
 * PVMp4FFComposerNodeFactory class is a singleton class which instantiates and provides
 * access to PV MPEG4 file format composer. It returns a PVMFNodeInterface
 * reference, the interface class of the PVMp4FFComposerNode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVMp4FFComposerNodeFactory
{
    public:
        /**
         * Creates an instance of a PV MPEG4 file format composer. If the creation fails, this function will leave.
         *
         * @param aPriority Priority of the active object of this node
         * @returns A pointer to an author or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreateMp4FFComposer(int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * This function allows the application to delete an instance of a PVMp4FFComposerNode
         * and reclaim all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aComposer The file format composer to be deleted.
         * @returns A status code indicating success or failure.
         **/
        OSCL_IMPORT_REF static bool DeleteMp4FFComposer(PVMFNodeInterface* aComposer);
};

#endif // PVMP4FFCN_FACTORY_H_INCLUDED
