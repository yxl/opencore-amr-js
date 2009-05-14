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
#ifndef PVMF_WAVFFPARSER_FACTORY_H_INCLUDED
#define PVMF_WAVFFPARSER_FACTORY_H_INCLUDED

// Forward declaration
class PVMFNodeInterface;

#define KPVMFWavFFParserNodeUuid PVUuid(0xddc1b58e, 0x8404, 0x4b89, 0x82, 0x79, 0x8f, 0xfb, 0x3b, 0x8e, 0xba, 0x8b)



/**
 * PVMFWAVFFParserNodeFactory Class
 *
 * PVMFWAVFFParserNodeFactory class is a singleton class which instantiates and provides
 * access to PVMF GSM-WAV NB audio decoder node. It returns a PVMFNodeInterface
 * reference, the interface class of the PVMFWAVFFParserNode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVMFWAVFFParserNodeFactory
{
    public:
        /**
         * Creates an instance of a PVMFWAVFFParserNode. If the creation fails, this function will leave.
         *
         * @param aPriority The active object priority for the node. Default is standard priority if not specified
         * @returns A pointer to an instance of PVMFWAVFFParserNode as PVMFNodeInterface reference or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreatePVMFWAVFFParserNode(int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * Deletes an instance of PVMFWAVFFParserNode
         * and reclaims all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aNode The PVMFWAVFFParserNode instance to be deleted
         * @returns A status code indicating success or failure of deletion
         **/
        OSCL_IMPORT_REF static bool DeletePVMFWAVFFParserNode(PVMFNodeInterface* aNode);
};

#endif // PVMF_WAVFFPARSER_FACTORY_H_INCLUDED

