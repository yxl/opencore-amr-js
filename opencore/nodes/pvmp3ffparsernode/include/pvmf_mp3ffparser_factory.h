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
#ifndef PVMF_MP3FFPARSER_FACTORY_H_INCLUDED
#define PVMF_MP3FFPARSER_FACTORY_H_INCLUDED

// Forward declaration
class PVMFNodeInterface;

#define KPVMFMP3FFParserNodeUuid PVUuid(0x03c8bdcc,0xe78a,0x4bd7,0x83,0xf9,0xae,0x00,0x07,0xee,0x7d,0x37)


/**
 * PVMFMP3FFParserNodeFactory Class
 *
 * PVMFMP3FFParserNodeFactory class is a singleton class which instantiates
 * and provides access to PVMFMP3FileFormatParser node. It returns a
 * PVMFNodeInterface reference, the interface class of the PVMFMP3FFParserNode.
 *
 * The client is expected to contain and maintain a pointer to the instance created
 * while the node is active.
 */
class PVMFMP3FFParserNodeFactory
{
    public:
        /**
         * Creates an instance of a PVMFMP3FFParserNode. If the creation fails, this function will leave.
         *
         * @param aPriority The active object priority for the node. Default is
         * standard priority if not specified
         * @returns A pointer to an instance of PVMFMP3FFParserNode as PVMFNodeInterface reference
         * or leaves if instantiation fails
         **/
        OSCL_IMPORT_REF static PVMFNodeInterface* CreatePVMFMP3FFParserNode(int32 aPriority = OsclActiveObject::EPriorityNominal);

        /**
         * Deletes an instance of PVMFMP3FFParserNode
         * and reclaims all allocated resources.  An instance can be deleted only in
         * the idle state. An attempt to delete in any other state will fail and return false.
         *
         * @param aNode The PVMFMP3FFParserNode instance to be deleted
         * @returns A status code indicating success or failure of deletion
         **/
        OSCL_IMPORT_REF static bool DeletePVMFMP3FFParserNode(PVMFNodeInterface* aNode);
};

#endif // end PVMF_MP3FFPARSER_FACTORY_H_INCLUDED

