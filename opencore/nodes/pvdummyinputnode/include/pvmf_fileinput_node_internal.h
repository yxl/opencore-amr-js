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

#ifndef PVMF_FILEINPUT_NODE_INTERNAL_H_INCLUDED
#define PVMF_FILEINPUT_NODE_INTERNAL_H_INCLUDED

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif
#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif


//memory allocator type for this node.
typedef OsclMemAllocator PVMFFileInputNodeAllocator;


///////////////////////////////////////////////////////
// For Command implementation
///////////////////////////////////////////////////////

//Default vector reserve size
#define PVMF_FILEINPUT_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_FILEINPUT_NODE_COMMAND_ID_START 6000

//Mimetypes for the custom interface
#define PVMF_FILEINPUT_NODE_EXTENSION_INTERFACE_MIMETYPE "pvxxx/FileInputNode/ExtensionInterface"
#define PVMF_FILEINPUT_NODE_MIMETYPE "pvxxx/FileInputNode"
#define PVMF_BASEMIMETYPE "pvxxx"

//Node command type.
//typedef PVMFGenericNodeCommand<PVMFFileInputNodeAllocator> PVMFFileInputNodeCommandBase;
#define PVMFFileInputNodeCommandBase PVMFGenericNodeCommand<PVMFFileInputNodeAllocator>  // to remove typedef warning on symbian
class PVMFFileInputNodeCommand: public PVMFFileInputNodeCommandBase
{
    public:
        //constructor for Custom2 command
        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, const OsclAny*aContext)
        {
            PVMFFileInputNodeCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        }
        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        }
};


//Command queue type
typedef PVMFNodeCommandQueue<PVMFFileInputNodeCommand, PVMFFileInputNodeAllocator> PVMFFileInputNodeCmdQ;

#endif // PVMF_FILEINPUT_NODE_INTERNAL_H_INCLUDED


