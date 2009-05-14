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
#ifndef PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED
#define PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_BASE_ALLOC_H_INCLUDED
#include "oscl_base_alloc.h"
#endif

#ifndef OSCL_REFCOUNTER_H_INCLUDED
#include "oscl_refcounter.h"
#endif

#ifndef OSCL_MEM_BASIC_FUNCTIONS_H
#include "oscl_mem_basic_functions.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif


class OsclMemPoolResizableAllocatorCleanupDA : public OsclDestructDealloc
{
    public:
        OsclMemPoolResizableAllocatorCleanupDA(Oscl_DefAlloc* in_gen_alloc) :
                gen_alloc(in_gen_alloc) {};

        virtual ~OsclMemPoolResizableAllocatorCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            gen_alloc->deallocate(ptr);
        }
    private:
        Oscl_DefAlloc* gen_alloc;
};

class PVMFResizableSimpleMediaMsgAlloc
{
    public:
        PVMFResizableSimpleMediaMsgAlloc(OsclMemPoolResizableAllocator* in_gen_alloc)
        {
            if (in_gen_alloc)
            {
                gen_alloc = in_gen_alloc;
                gen_alloc->enablenullpointerreturn();
                iBufferOverhead = 0;
                uint aligned_class_size =
                    oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));
                uint aligned_cleanup_size =
                    oscl_mem_aligned_size(sizeof(OsclMemPoolResizableAllocatorCleanupDA));
                uint aligned_refcnt_size =
                    oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
                iBufferOverhead = (aligned_refcnt_size +
                                   aligned_cleanup_size +
                                   aligned_class_size);

            }
            else
            {
                OSCL_LEAVE(OsclErrArgument);
            }
        };

        virtual ~PVMFResizableSimpleMediaMsgAlloc()
        {
        };

        OsclSharedPtr<PVMFMediaDataImpl> allocate(uint32 size)
        {
            OsclSharedPtr<PVMFMediaDataImpl> shared_media_data;

            if (size == 0)
            {
                OSCL_ASSERT(false);
            }

            uint aligned_in_size = oscl_mem_aligned_size(size);

            uint8* my_ptr;
            OsclRefCounter* my_refcnt;

            uint aligned_class_size =
                oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));

            uint aligned_cleanup_size =
                oscl_mem_aligned_size(sizeof(OsclMemPoolResizableAllocatorCleanupDA));

            uint aligned_refcnt_size =
                oscl_mem_aligned_size(sizeof(OsclRefCounterDA));

            my_ptr = (uint8*) gen_alloc->allocate(aligned_refcnt_size +
                                                  aligned_cleanup_size +
                                                  aligned_class_size +
                                                  aligned_in_size);

            if (my_ptr == NULL)
            {
                return shared_media_data;
            }

            OsclMemPoolResizableAllocatorCleanupDA *my_cleanup =
                OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, OsclMemPoolResizableAllocatorCleanupDA(gen_alloc));

            my_refcnt =
                OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, my_cleanup));

            my_ptr += aligned_refcnt_size + aligned_cleanup_size;

            void* ptr;
            ptr = my_ptr + aligned_class_size;

            PVMFMediaDataImpl* media_data_ptr =
                new(my_ptr) PVMFSimpleMediaBuffer(ptr,
                                                  aligned_in_size,
                                                  my_refcnt);

            shared_media_data.Bind(media_data_ptr, my_refcnt);
            return shared_media_data;
        }

        void ResizeMemoryFragment(OsclSharedPtr<PVMFMediaDataImpl>& aSharedBuffer)
        {
            OsclRefCounterMemFrag memFrag;
            aSharedBuffer->getMediaFragment(0, memFrag);
            uint32 currCapacity = memFrag.getCapacity();
            uint32 bytesUsed = memFrag.getMemFragSize();

            //uint32 alignedBytesUsed = bytesUsed;
            uint32 alignedBytesUsed = oscl_mem_aligned_size(bytesUsed);

            if (alignedBytesUsed < currCapacity)
            {
                uint32 bytesToReclaim = (currCapacity - alignedBytesUsed);
                OsclMemPoolResizableAllocator* dataAllocator =
                    reinterpret_cast<OsclMemPoolResizableAllocator*>(gen_alloc);
                /* Account for the overhead */
                uint8* memFragPtr = (uint8*)(memFrag.getMemFragPtr());
                uint8* ptr = (memFragPtr - iBufferOverhead);
                dataAllocator->trim((OsclAny*)ptr, bytesToReclaim);
                aSharedBuffer->setCapacity(alignedBytesUsed);
            }
        }

        uint32 GetMediaMsgAllocationOverheadBytes()
        {
            return iBufferOverhead;
        }

    private:
        uint iBufferOverhead;
        OsclMemPoolResizableAllocator* gen_alloc;
};

class PVMFResizableSimpleMediaMsgAllocCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~PVMFResizableSimpleMediaMsgAllocCleanupSA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            uint8* tmp_ptr = (uint8*) ptr;

            uint aligned_refcnt_size =
                oscl_mem_aligned_size(sizeof(OsclRefCounterSA<PVMFResizableSimpleMediaMsgAllocCleanupSA>));

            tmp_ptr += aligned_refcnt_size;
            PVMFResizableSimpleMediaMsgAlloc* dataBufferAlloc =
                reinterpret_cast<PVMFResizableSimpleMediaMsgAlloc*>(tmp_ptr);
            dataBufferAlloc->~PVMFResizableSimpleMediaMsgAlloc();
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};

#endif //PVMF_RESIZABLE_SIMPLE_MEDIAMSG_H_INCLUDED


