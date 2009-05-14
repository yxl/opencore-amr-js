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
#ifndef PVMF_DOWNLOADMANAGER_FACTORY_H_INCLUDED
#define PVMF_DOWNLOADMANAGER_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

// Forward declaration
class PVMFNodeInterface;

#define KPVMFDownloadManagerNodeUuid PVUuid(0x9eb46c30,0x7ef2,0x11d9,0x96,0x69,0x08,0x00,0x20,0x0c,0x9a,0x66)

/**
 * PVMFDownloadManagerNodeFactory Class
 *
 * PVMFDownloadManagerNodeFactory class is a singleton class which instantiates and provides
 * access to PVMF MP4 file format parser node. It returns a PVMFNodeInterface
 * reference, the interface class of the PVMFDownloadManagerNode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVMFDownloadManagerNodeFactory
{
    public:
        /**
         * Creates an instance of a PVMFDownloadManagerNode. If the creation fails, this function will leave.
         *
         * @param aPriority The active object priority for the node. Default is standard priority if not specified
         * @returns A pointer to an instance of PVMFDownloadManagerNode as PVMFNodeInterface reference or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreatePVMFDownloadManagerNode(int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * Deletes an instance of PVMFDownloadManagerNode
         * and reclaims all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aNode The PVMFDownloadManagerNode instance to be deleted
         * @returns A status code indicating success or failure of deletion
         **/
        OSCL_IMPORT_REF static bool DeletePVMFDownloadManagerNode(PVMFNodeInterface* aNode);
};

#endif // PVMF_DOWNLOADMANAGER_FACTORY_H_INCLUDED
