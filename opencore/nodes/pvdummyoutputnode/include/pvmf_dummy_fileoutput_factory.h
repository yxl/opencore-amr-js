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
#ifndef PVMF_DUMMY_FILEOUTPUT_FACTORY_H_INCLUDED
#define PVMF_DUMMY_FILEOUTPUT_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

// Forward declaration
class PVMFNodeInterface;

//Port tags for this node.
typedef enum
{
    PVMF_DUMMY_FILE_OUTPUT_NODE_PORT_TYPE_SINK
} PVMFDummyFileOutputNodePortTag;

#define KPVFileOutputNodeUuid PVUuid(0xa9fb43bb,0xfed4,0x4914,0xa4,0xbf,0x7d,0x04,0x3f,0x45,0x0c,0x24)

/**
 * PVMFDummyFileOutputNodeFactory Class
 *
 * PVMFDummyFileOutputNodeFactory class is a singleton class which instantiates and provides
 * access to PV FileOutput node. It returns a PVMFNodeInterface
 * reference, the interface class of the PVMFDummyFileOutputNode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVMFDummyFileOutputNodeFactory
{
    public:
        /**
         * Creates an instance of a FileOutput. If the creation fails, this function will leave.
         *
         * @param aPriority The active object priority for the node. Default is standard priority if not specified
         * @returns A pointer to an author or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreateDummyFileOutput(int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * Creates an instance of a FileOutput. If the creation fails, this function will leave.
         *
         * @param aFileName pathname of file to write to.
         * @param aFormat media format for file.
         * @param aPriority The active object priority for the node. Default is standard priority if not specified
         * @returns A pointer to an author or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreateDummyFileOutput(OSCL_wString &aFileName, PVMFFormatType aFormat, int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * This function allows the application to delete an instance of a PVMFDummyFileOutputNode
         * and reclaim all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aComposer The file format composer to be deleted.
         * @returns A status code indicating success or failure.
         **/
        OSCL_IMPORT_REF static bool DeleteDummyFileOutput(PVMFNodeInterface*& aNode);
};

#endif // PVMF_FILEOUTPUT_FACTORY_H_INCLUDED
