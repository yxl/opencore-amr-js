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
#include "threadsafe_mempool.h"

OSCL_EXPORT_REF ThreadSafeMemPoolFixedChunkAllocator *ThreadSafeMemPoolFixedChunkAllocator::Create(const uint32 numchunk, const uint32 chunksize, Oscl_DefAlloc *gen_alloc)
{

    OsclAny *ptr = NULL;
    if (gen_alloc)
    {
        ptr = gen_alloc->ALLOCATE(sizeof(ThreadSafeMemPoolFixedChunkAllocator));


    }
    else
    {
        ptr = OSCL_MALLOC(sizeof(ThreadSafeMemPoolFixedChunkAllocator));

    }

    if (ptr == NULL)
    {
        return NULL;
    }

    ThreadSafeMemPoolFixedChunkAllocator *self = OSCL_PLACEMENT_NEW(ptr, ThreadSafeMemPoolFixedChunkAllocator(numchunk, chunksize, gen_alloc));
    return self;
}

OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::Delete()
{
    this->~ThreadSafeMemPoolFixedChunkAllocator();
    if (iMemPoolAllocator)
    {
        iMemPoolAllocator->deallocate(this);
    }
    else
    {
        OSCL_FREE(this);
    }

}

OSCL_EXPORT_REF ThreadSafeMemPoolFixedChunkAllocator::ThreadSafeMemPoolFixedChunkAllocator(const uint32 numchunk, const uint32 chunksize, Oscl_DefAlloc* gen_alloc) :
        iNumChunk(1), iChunkSize(0), iChunkSizeMemAligned(0),
        iMemPoolAllocator(gen_alloc), iMemPool(NULL),
        iCheckNextAvailableFreeChunk(false), iObserver(NULL),
        iNextAvailableContextData(NULL),
        iRefCount(1)
{
    iNumChunk = numchunk;
    iChunkSize = chunksize;

    if (iNumChunk == 0)
    {
        iNumChunk = 1;
    }

    if (iChunkSize > 0)
    {
        createmempool();
    }

    iMemPoolMutex.Create();


}

OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::addRef_internal()
{
    // Just increment the ref count
    ++iRefCount;
}

// could be called externally
OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::addRef()
{
    // Just increment the ref count
    iMemPoolMutex.Lock();
    addRef_internal();
    iMemPoolMutex.Unlock();
}

OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::removeRef_internal()
{
    --iRefCount;
}

OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::removeRef()
{
    // Decrement the ref count
    iMemPoolMutex.Lock();
    removeRef_internal();

    // If ref count reaches 0 then destroy this object automatically
    if (iRefCount <= 0)
    {
        iMemPoolMutex.Unlock();
        //OSCL_DELETE(this);
        Delete();
    }
    else
    {
        iMemPoolMutex.Unlock();
    }
}


OSCL_EXPORT_REF ThreadSafeMemPoolFixedChunkAllocator::~ThreadSafeMemPoolFixedChunkAllocator()
{
    // Decrement the ref count
    --iRefCount;

    // If ref count reaches 0 then destroy this object
    if (iRefCount <= 0)
    {
        destroymempool();
    }
    iMemPoolMutex.Close();
}


OSCL_EXPORT_REF OsclAny* ThreadSafeMemPoolFixedChunkAllocator::allocate(const uint32 n)
{
    // Create the memory pool if it hasn't been created yet.
    // Use the allocation size, n, as the chunk size for memory pool
    iMemPoolMutex.Lock();

    if (iChunkSize == 0)
    {
        iChunkSize = n;
        createmempool();
    }
    else if (n > iChunkSize)
    {
        OSCL_LEAVE(OsclErrArgument);

    }


    if (iFreeMemChunkList.empty())
    {
        // No free chunk is available

        iMemPoolMutex.Unlock();
        return NULL;
    }

    // Return the next available chunk from the pool
    OsclAny* freechunk = iFreeMemChunkList.back();
    // Remove the chunk from the free list
    iFreeMemChunkList.pop_back();
    addRef_internal();
    iMemPoolMutex.Unlock();

    return freechunk;
}


OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::deallocate(OsclAny* p)
{
    if (iMemPool == NULL)
    {
        // Memory pool hasn't been allocated yet so error
        OSCL_LEAVE(OsclErrNotReady);
    }

    uint8* ptmp = (uint8*)p;
    uint8* mptmp = (uint8*)iMemPool;

    if ((ptmp < mptmp) || ptmp >= (mptmp + iNumChunk*iChunkSizeMemAligned))
    {
        // Returned memory is not part of this memory pool
        OSCL_LEAVE(OsclErrArgument);
    }

    if (((ptmp - mptmp) % iChunkSizeMemAligned) != 0)
    {
        // Returned memory is not aligned to the chunk.
        OSCL_LEAVE(OsclErrArgument);
    }


    iMemPoolMutex.Lock();
    // Put the returned chunk in the free pool
    iFreeMemChunkList.push_back(p);

    removeRef_internal();

    if (iRefCount > 0)
    {
        iMemPoolMutex.Unlock();
        // Notify the observer about free chunk available if waiting for such callback
        if (iCheckNextAvailableFreeChunk)
        {
            iCheckNextAvailableFreeChunk = false;
            if (iObserver)
            {
                iObserver->freechunkavailable(iNextAvailableContextData);
            }
        }
    }
    else
    {

        iMemPoolMutex.Unlock();
        // when the mempool is about to be destroyed, no need to
        // notify the observer. By this time, the observer should not
        // be expecting it
        Delete();
    }

}


OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& obs, OsclAny* aContextData)
{
    iCheckNextAvailableFreeChunk = true;
    iObserver = &obs;
    iNextAvailableContextData = aContextData;
}

OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::CancelFreeChunkAvailableCallback()
{
    iCheckNextAvailableFreeChunk = false;
    iObserver = NULL;
    iNextAvailableContextData = NULL;
}

OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::createmempool()
{
    if (iChunkSize == 0 || iNumChunk == 0)
    {
        OSCL_LEAVE(OsclErrArgument);
    }

    // Create one block of memory for the memory pool
    iChunkSizeMemAligned = oscl_mem_aligned_size(iChunkSize);
    int32 leavecode = 0;
    if (iMemPoolAllocator)
    {
        OSCL_TRY(leavecode, iMemPool = iMemPoolAllocator->ALLOCATE(iNumChunk * iChunkSizeMemAligned));
    }
    else
    {
        OSCL_TRY(leavecode, iMemPool = OSCL_MALLOC(iNumChunk * iChunkSizeMemAligned));
    }

    if (leavecode || iMemPool == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }

#if OSCL_MEM_FILL_WITH_PATTERN
    oscl_memset(iMemPool, 0x55, iNumChunk*iChunkSizeMemAligned);
#endif

    // Set up the free mem chunk list vector
    iFreeMemChunkList.reserve(iNumChunk);
    uint8* chunkptr = (uint8*)iMemPool;

    for (uint32 i = 0; i < iNumChunk; ++i)
    {
        iFreeMemChunkList.push_back((OsclAny*)chunkptr);
        chunkptr += iChunkSizeMemAligned;
    }
}


OSCL_EXPORT_REF void ThreadSafeMemPoolFixedChunkAllocator::destroymempool()
{
    // If ref count reaches 0 then destroy this object
    if (iRefCount <= 0)
    {
#if OSCL_MEM_CHECK_ALL_MEMPOOL_CHUNKS_ARE_RETURNED
        // Assert if all of the chunks were not returned
        OSCL_ASSERT(iFreeMemChunkList.size() == iNumChunk);
#endif

        iFreeMemChunkList.clear();

        if (iMemPool)
        {
            if (iMemPoolAllocator)
            {
                iMemPoolAllocator->deallocate(iMemPool);
            }
            else
            {
                OSCL_FREE(iMemPool);
            }

            iMemPool = NULL;
        }
    }
}
