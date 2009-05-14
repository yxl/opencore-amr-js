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
#ifndef PVMF_MEDIA_FRAG_GROUP_H_INCLUDED
#define PVMF_MEDIA_FRAG_GROUP_H_INCLUDED

#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#include "pvmf_media_data_impl.h"
#endif

#ifndef OSCL_SHARED_PTR_H_INCLUDED
#include "oscl_shared_ptr.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

// constants
const uint PVMF_MEDIA_FRAG_GROUP_DEFAULT_CAPACITY = 10;

class MediaDataEntry
{
    public:
        MediaDataEntry() : next(NULL) {}
        ~MediaDataEntry()
        {
            mediaDataImpl.Unbind();
        }
        OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
        MediaDataEntry* next;
};

/**
 * The PVMFMediaFragGroup class is a MediaDataImpl implementation that
 * stores multiple OsclRefCounterMemFrag which have been separately allocated.
 * It takes an allocator as a templatized parameter to allow the user to determine
 * how memory for storing the fragments internally is allocated.
 * The PVMFMediaFragGroup is created with a fixed capacity that is passed in at
 * construction time.
 */
template<class Alloc>
class PVMFMediaFragGroup : public PVMFMediaDataImpl
{

    public:
        virtual uint32 getMarkerInfo();
        virtual bool isRandomAccessPoint();
        virtual uint32 getErrorsFlag();
        virtual uint32 getNumFragments();
        virtual bool getMediaFragment(uint32 index, OsclRefCounterMemFrag& memfrag);
        virtual bool getMediaFragmentSize(uint32 index, uint32& size);
        virtual uint32 getFilledSize();
        virtual uint32 getCapacity();

        virtual void setCapacity(uint32 aCapacity);
        virtual bool setMediaFragFilledLen(uint32 index, uint32 len);
        virtual bool setMarkerInfo(uint32 marker);
        virtual bool setRandomAccessPoint(bool flag);
        virtual bool setErrorsFlag(uint32 flag);

        virtual bool appendMediaFragment(OsclRefCounterMemFrag& memfrag);
        virtual bool clearMediaFragments();

        /* NOTE!!:  The constructor assumes the refcnt has already been incremented
         * to reflect this class holding a reference to the buffer. Increment it
         * externally if you aren't simply passing ownership of a reference
         */
        PVMFMediaFragGroup(uint32 capacity = PVMF_MEDIA_FRAG_GROUP_DEFAULT_CAPACITY);

        virtual ~PVMFMediaFragGroup();

    private:
        uint32 marker_info;
        bool random_access_point;
        uint32 errors_flag;
        Oscl_Vector<OsclRefCounterMemFrag, Alloc> iFragments;
        uint32 capacity;
        uint32 filled_size;
};


/**
 * The PVMFMediaFragGroupCombinedAlloc allocator class
 * takes care of allocating the refcounter, PVMFMediaFragGroup container,
 * and the actual buffer space in a single block of memory.
 */
template<class Alloc>
class PVMFMediaFragGroupCombinedAlloc : public OsclDestructDealloc
{

    public:
        PVMFMediaFragGroupCombinedAlloc(uint in_capacity, uint in_fragment_capacity,
                                        Oscl_DefAlloc* opt_gen_alloc = 0)
                : capacity(in_capacity)
                , fragment_capacity(in_fragment_capacity)
                , gen_alloc(opt_gen_alloc)
                , available_mfgs(NULL)
                , num_available_mfgs(0)
                , iRefCnt(0)
                , iDestroy(false)
                , iCheckNextAvailableFreeChunk(false)
                , iObserver(NULL)
                , iNextAvailableContextData(NULL)
        {
            addRef();
        };
        virtual ~PVMFMediaFragGroupCombinedAlloc();
        void create();
        void destroy();
        void addRef();
        void removeRef();
        void do_allocate(uint32 requested_fragment_capacity = 0);
        OsclSharedPtr<PVMFMediaDataImpl> allocate(uint32 requested_fragment_capacity = 0);
        void destruct_and_dealloc(OsclAny* ptr);
        bool IsMsgAvailable()
        {
            if (available_mfgs == NULL)
            {
                return false;
            }
            return true;
        }

        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& obs, OsclAny* aContextData = NULL)
        {
            iCheckNextAvailableFreeChunk = true;
            iObserver = &obs;
            iNextAvailableContextData = aContextData;
        }

        void CancelFreeChunkAvailableCallback()
        {
            iCheckNextAvailableFreeChunk = false;
            iObserver = NULL;
            iNextAvailableContextData = NULL;
        }

        unsigned NumMsgAvail()
        {
            return num_available_mfgs;
        }

    protected:
        void append(OsclSharedPtr<PVMFMediaDataImpl> & media_data);
        void append(uint8* my_ptr);

        uint capacity;
        uint fragment_capacity;
        Oscl_DefAlloc* gen_alloc;
        MediaDataEntry* available_mfgs;
        unsigned num_available_mfgs;
        OsclMemPoolFixedChunkAllocator* media_data_entry_alloc;
        int32 iRefCnt;
        bool iDestroy;
        bool iCheckNextAvailableFreeChunk;
        OsclMemPoolFixedChunkAllocatorObserver* iObserver;
        OsclAny* iNextAvailableContextData;
};


template<class Alloc>
PVMFMediaFragGroup<Alloc>::~PVMFMediaFragGroup()
{
    // clear all the fragments
    iFragments.clear();
}

template<class Alloc>
uint32 PVMFMediaFragGroup<Alloc>::getMarkerInfo()
{
    return marker_info;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::isRandomAccessPoint()
{
    return random_access_point;
}

template<class Alloc>
uint32 PVMFMediaFragGroup<Alloc>::getErrorsFlag()
{
    return errors_flag;
}

template<class Alloc>
uint32 PVMFMediaFragGroup<Alloc>::getNumFragments()
{
    return iFragments.size();
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::getMediaFragment(uint32 index, OsclRefCounterMemFrag& memfrag)
{
    if (index >= iFragments.size())
    {
        return false;
    }

    memfrag = OsclRefCounterMemFrag(iFragments[index]);

    return true;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::getMediaFragmentSize(uint32 index, uint32& size)
{
    size = 0;
    if (index >= iFragments.size())
    {
        return false;
    }

    size = iFragments[index].getMemFrag().len;

    return true;
}

template<class Alloc>
uint32 PVMFMediaFragGroup<Alloc>::getFilledSize()
{
    return filled_size;
}

template<class Alloc>
uint32 PVMFMediaFragGroup<Alloc>::getCapacity()
{
    return capacity;
}

template<class Alloc>
void PVMFMediaFragGroup<Alloc>::setCapacity(uint32 aCapacity)
{
    capacity = aCapacity;
    return;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::setMediaFragFilledLen(uint32 index, uint32 len)
{
    if (index >= iFragments.size())
    {
        return false;
    }

    if (len > iFragments[index].getCapacity())
    {
        return false;
    }

    filled_size -= iFragments[index].getMemFrag().len;
    iFragments[index].getMemFrag().len = len;
    filled_size += len;
    return true;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::setMarkerInfo(uint32 in_marker)
{
    marker_info = in_marker;
    return true;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::setRandomAccessPoint(bool flag)
{
    random_access_point = flag;
    return true;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::setErrorsFlag(uint32 flag)
{
    errors_flag = flag;
    return true;
}

template<class Alloc>
PVMFMediaFragGroup<Alloc>::PVMFMediaFragGroup(uint32 in_capacity) :
        marker_info(0), random_access_point(false), errors_flag(0)
{
    iFragments.reserve(in_capacity);
    capacity = 0;
    filled_size = 0;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::appendMediaFragment(OsclRefCounterMemFrag& memfrag)
{
    iFragments.push_back(memfrag);
    capacity += memfrag.getCapacity();
    filled_size += memfrag.getMemFrag().len;
    return true;
}

template<class Alloc>
bool PVMFMediaFragGroup<Alloc>::clearMediaFragments()
{
    iFragments.clear();
    capacity = 0;
    filled_size = 0;
    return true;
}

template<class Alloc>
PVMFMediaFragGroupCombinedAlloc<Alloc>::~PVMFMediaFragGroupCombinedAlloc()
{
    removeRef();
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::addRef()
{
    iRefCnt++;
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::removeRef()
{
    //we need to make sure that this allocator persists as long as
    //all buffers allocated it have been released. Therefore we need
    //a notion of reference count for the allocator itself. iRefCnt
    //is incremented in addRef and is set decremented here.
    iRefCnt--;
    if ((iRefCnt == 0) &&
            (num_available_mfgs == capacity))
    {
        //implies that all messages are have been returned
        //if not, wait for all messages to return
        //this allocator needs to live till all messages allocated by
        //it have been released
        destroy();
    }
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::create()
{
    media_data_entry_alloc = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (capacity));
    for (uint num = 0; num < capacity; num++)
    {
        do_allocate(fragment_capacity);
    }
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::do_allocate(uint32 requested_size)
{
    if (requested_size == 0)
    {
        requested_size = PVMF_MEDIA_FRAG_GROUP_DEFAULT_CAPACITY;
    }

    uint aligned_class_size = oscl_mem_aligned_size(sizeof(PVMFMediaFragGroup<Alloc>));
    OsclRefCounter* my_refcnt = NULL;
    uint8* my_ptr = NULL;

    uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    my_ptr = (uint8*) gen_alloc->ALLOCATE(aligned_refcnt_size + aligned_class_size);
    // create the recounter after the cleanup object
    my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, this));
    my_ptr += aligned_refcnt_size;

    PVMFMediaDataImpl* media_data_ptr = OSCL_PLACEMENT_NEW(my_ptr, PVMFMediaFragGroup<Alloc>(requested_size));

    OsclSharedPtr<PVMFMediaDataImpl> shared_media_data(media_data_ptr, my_refcnt);
    append(shared_media_data);
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::append(OsclSharedPtr<PVMFMediaDataImpl> & media_data)
{
    void* memory_for_entry = media_data_entry_alloc->ALLOCATE(sizeof(MediaDataEntry));
    MediaDataEntry* entry = OSCL_PLACEMENT_NEW(memory_for_entry, MediaDataEntry());
    entry->mediaDataImpl = media_data;

    MediaDataEntry* first = entry;
    if (available_mfgs)
    {
        first = available_mfgs->next;
        available_mfgs->next = entry;
    }
    available_mfgs = entry;
    entry->next = first;
    num_available_mfgs++;
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::append(uint8* my_ptr)
{
    uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    PVMFMediaDataImpl* media_data_ptr = (PVMFMediaDataImpl*)(my_ptr + aligned_refcnt_size);
    OsclRefCounter* my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, this));
    void* memory_for_entry = media_data_entry_alloc->ALLOCATE(sizeof(MediaDataEntry));
    MediaDataEntry* entry = OSCL_PLACEMENT_NEW(memory_for_entry, MediaDataEntry());
    entry->mediaDataImpl.Bind(media_data_ptr, my_refcnt);

    MediaDataEntry* first = entry;
    if (available_mfgs)
    {
        first = available_mfgs->next;
        available_mfgs->next = entry;
    }
    available_mfgs = entry;
    entry->next = first;
    num_available_mfgs++;
    if ((iRefCnt == 0) && (num_available_mfgs == capacity))
    {
        //iRefCnt of zero implies that owner of PVMFMediaFragGroupCombinedAlloc
        //has given up ownership (meaning no more allocations)
        //But we need to wait for all msgs to be released before we can destroy
        //the allocators.
        destroy();
    }
}

template<class Alloc>
OsclSharedPtr<PVMFMediaDataImpl> PVMFMediaFragGroupCombinedAlloc<Alloc>::allocate(uint32 requested_size)
{
    OsclSharedPtr<PVMFMediaDataImpl> media_data;

    if (requested_size > fragment_capacity)
    {
        OSCL_LEAVE(OSCL_BAD_ALLOC_EXCEPTION_CODE);
    }

    if (available_mfgs == NULL)
    {
        //all required memory must have been allocated in "do_allocate"
        //which is called as part of "create". any exception in either of
        //these methods means that the allocator could not be created, but
        //upon successful creation if "available_mfgs" is NULL then it means
        //we are temporarily out of memory, and there is no need to throw
        //an exception in this case
        return media_data;
    }

    MediaDataEntry* first = available_mfgs->next;
    media_data = first->mediaDataImpl;

    if (first == available_mfgs)
    {
        available_mfgs = NULL;
    }
    else
    {
        available_mfgs->next = first->next;
    }
    first->~MediaDataEntry();
    media_data_entry_alloc->deallocate(first);
    num_available_mfgs--;
    return media_data;
}

template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::destroy()
{
    iDestroy = true;
    MediaDataEntry* first = NULL;
    if (available_mfgs)
    {
        first = available_mfgs->next;
        available_mfgs->next = NULL;
    }

    while (first)
    {
        MediaDataEntry* cur = first;
        first = first->next;
        cur->~MediaDataEntry();
        media_data_entry_alloc->deallocate(cur);
    }

    if (media_data_entry_alloc)
    {
        OSCL_DELETE(media_data_entry_alloc);
    }
    OSCL_DELETE(this);
}


template<class Alloc>
void PVMFMediaFragGroupCombinedAlloc<Alloc>::destruct_and_dealloc(OsclAny* ptr)
{
    uint8* my_ptr = (uint8*)ptr;
    uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    PVMFMediaDataImpl* media_data_ptr = (PVMFMediaDataImpl*)(my_ptr + aligned_refcnt_size);
    media_data_ptr->clearMediaFragments();

    if (!iDestroy)
    {
        if (iCheckNextAvailableFreeChunk == true)
        {
            iCheckNextAvailableFreeChunk = false;
            if (iObserver)
            {
                iObserver->freechunkavailable(iNextAvailableContextData);
            }
        }
        append(my_ptr);
    }
    else
    {
        media_data_ptr->~PVMFMediaDataImpl();
        gen_alloc->deallocate(ptr);
    }
}

#endif

