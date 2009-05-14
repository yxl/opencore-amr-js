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
#include "pvmf_simple_media_buffer.h"

#include "oscl_mem.h"

// constants
const uint PVMF_SIMPLE_MEDIA_BUF_DEFAULT_SIZE = 200;


OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::appendMediaFragment(OsclRefCounterMemFrag& memfrag)
{
    buffer.ptr = memfrag.getMemFragPtr();
    buffer.len = memfrag.getMemFragSize();
    return true;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::clearMediaFragments()
{
    buffer.ptr = NULL;
    buffer.len = 0;
    return true;
}

OSCL_EXPORT_REF PVMFSimpleMediaBuffer::~PVMFSimpleMediaBuffer()
{
    refcnt->removeRef();
}

OSCL_EXPORT_REF uint32 PVMFSimpleMediaBuffer::getMarkerInfo()
{
    return marker_info;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::isRandomAccessPoint()
{
    return random_access_point;
}

OSCL_EXPORT_REF uint32 PVMFSimpleMediaBuffer::getErrorsFlag()
{
    return errors_flag;
}

OSCL_EXPORT_REF uint32 PVMFSimpleMediaBuffer::getNumFragments()
{
    return 1;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::getMediaFragment(uint32 index, OsclRefCounterMemFrag& memfrag)
{
    if (index != 0)
    {
        return false;
    }

    memfrag = OsclRefCounterMemFrag(buffer, refcnt, capacity);

    // explicitly addref
    refcnt->addRef();

    return true;

}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::getMediaFragmentSize(uint32 index, uint32& size)
{
    size = 0;
    if (index != 0)
    {
        return false;
    }
    size = buffer.len;
    return true;
}

OSCL_EXPORT_REF uint32 PVMFSimpleMediaBuffer::getFilledSize()
{
    return buffer.len;
}

OSCL_EXPORT_REF uint32 PVMFSimpleMediaBuffer::getCapacity()
{
    return capacity;
}

OSCL_EXPORT_REF void PVMFSimpleMediaBuffer::setCapacity(uint32 aCapacity)
{
    capacity = aCapacity;
    return;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::setMediaFragFilledLen(uint32 index, uint32 len)
{
    if (index != 0)
    {
        return false;
    }

    if (len > capacity)
    {
        return false;
    }

    buffer.len = len;
    return true;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::setMarkerInfo(uint32 in_marker)
{
    marker_info = in_marker;
    return true;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::setRandomAccessPoint(bool flag)
{
    random_access_point = flag;
    return true;
}

OSCL_EXPORT_REF bool PVMFSimpleMediaBuffer::setErrorsFlag(uint32 flag)
{
    errors_flag = flag;
    return true;
}


OSCL_EXPORT_REF PVMFSimpleMediaBuffer::PVMFSimpleMediaBuffer(void * ptr,
        uint32 in_capacity,
        OsclRefCounter *my_refcnt) :
        marker_info(0), random_access_point(false), errors_flag(0)
{
    buffer.ptr = ptr;
    buffer.len = 0;
    capacity = in_capacity;
    refcnt = my_refcnt;
}

class SimpleMediaBufferCombinedCleanupSA : public OsclDestructDealloc
{
    public:
        virtual ~SimpleMediaBufferCombinedCleanupSA() {};
        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // no need to call destructors in this case just dealloc
            OsclMemAllocator alloc;
            alloc.deallocate(ptr);
        }
};

class SimpleMediaBufferCombinedCleanupDA : public OsclDestructDealloc
{
    public:
        SimpleMediaBufferCombinedCleanupDA(Oscl_DefAlloc* in_gen_alloc) :
                gen_alloc(in_gen_alloc) {};
        virtual ~SimpleMediaBufferCombinedCleanupDA() {};

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            // no need to call destructors in this case just dealloc
            gen_alloc->deallocate(ptr);
        }

    private:
        Oscl_DefAlloc* gen_alloc;
};



OSCL_EXPORT_REF OsclSharedPtr<PVMFMediaDataImpl> PVMFSimpleMediaBufferCombinedAlloc::allocate(uint32 requested_size)
{
    OsclSharedPtr<PVMFMediaDataImpl> shared_media_data;
    if (requested_size == 0)
    {
        requested_size = PVMF_SIMPLE_MEDIA_BUF_DEFAULT_SIZE;
    }

    uint aligned_refcnt_size;
    uint aligned_class_size = oscl_mem_aligned_size(sizeof(PVMFSimpleMediaBuffer));
    OsclRefCounter* my_refcnt;
    uint8* my_ptr;

    uint aligned_cleanup_size = oscl_mem_aligned_size(sizeof(SimpleMediaBufferCombinedCleanupDA));
    aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size + aligned_cleanup_size +
                                          aligned_class_size + requested_size);

    if (my_ptr == NULL)
    {
        //we assume that gen_alloc (Oscl_DefAlloc implementation) provided to this
        //class does not throw an exception during the normal course of operation
        //viz. running out of memory in a memory pool temporarily
        return shared_media_data;
    }
    SimpleMediaBufferCombinedCleanupDA* cleanup_ptr = OSCL_PLACEMENT_NEW(my_ptr + aligned_refcnt_size, SimpleMediaBufferCombinedCleanupDA(gen_alloc));

    // create the recounter after the cleanup object
    my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, cleanup_ptr));
    my_ptr += aligned_refcnt_size + aligned_cleanup_size;



    void *ptr;
    ptr = my_ptr + aligned_class_size;
    PVMFMediaDataImpl* media_data_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFSimpleMediaBuffer(ptr, requested_size, my_refcnt));
    shared_media_data.Bind(media_data_ptr, my_refcnt);
    return shared_media_data;
}

