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
#ifndef PV_THREADSAFE_MEMPOOL_H_INCLUDED
#define PV_THREADSAFE_MEMPOOL_H_INCLUDED


#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef OSCL_MUTEX_H_INCLUDED
#include "oscl_mutex.h"
#endif

class ThreadSafeMemPoolFixedChunkAllocator: public OsclMemPoolFixedChunkAllocator
{
    public:
        /**
          * If numchunk and chunksize parameters are not set,
          * memory pool of 1 chunk will be created in the first call to allocate.
          * The chunk size will be set to the n passed in for allocate().
          * If numchunk parameter is set to 0, the memory pool will use 1 for numchunk.
          *
          * @return void
          *
          */
        OSCL_IMPORT_REF static ThreadSafeMemPoolFixedChunkAllocator *Create(const uint32 numchunk = 1, const uint32 chunksize = 0, Oscl_DefAlloc* gen_alloc = NULL);
        OSCL_IMPORT_REF void Delete();

        OSCL_IMPORT_REF virtual ~ThreadSafeMemPoolFixedChunkAllocator();


        /** This API throws an exception when n is greater than the fixed chunk size or there are no free chunk available in the pool.
          * If the memory pool hasn't been created yet, the pool will be created with chunk size equal to n so n must be greater than 0.
          * Exception will be thrown if memory allocation for the memory pool fails.
          *
          * @return pointer to available chunk from memory pool
          *
          */
        OSCL_IMPORT_REF virtual OsclAny* allocate(const uint32 n);

        /** This API throws an exception when the pointer p passed in is not part of the memory pool.
           * Exception will be thrown if the memory pool is not set up yet.
           *
           * @return void
           *
           */
        OSCL_IMPORT_REF virtual void deallocate(OsclAny* p);

        /** This API will set the flag to send a callback via specified observer object when the
          * next memory chunk is deallocated by deallocate() call.
          * The observer must be implemented in a THREADSAFE manner
          * Note that if the mempool to be destroyed (due to refcount going to 0)
          * there will be NO callback to the observer
          * @return void
          *
          */
        OSCL_IMPORT_REF virtual void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& obs, OsclAny* aContextData = NULL);

        /** This API will cancel any past callback requests..
          *
          * @return void
          *
          */
        OSCL_IMPORT_REF virtual void CancelFreeChunkAvailableCallback();

        /** Increments the reference count for this memory pool allocator
          *
          * @return void
          *
          */
        OSCL_IMPORT_REF void addRef();

        /** Decrements the reference count for this memory pool allocator
          * When the reference count goes to 0, this instance of the memory pool object is deleted
          *
          * @return void
          *
          */
        OSCL_IMPORT_REF void removeRef();

    protected:
        OSCL_IMPORT_REF ThreadSafeMemPoolFixedChunkAllocator(const uint32 numchunk = 1, const uint32 chunksize = 0, Oscl_DefAlloc* gen_alloc = NULL);

        /* copy constructor - declared protected to prevent usage of the default copy constructor */
        /* NOTE: This copy constructor should never be used */
        ThreadSafeMemPoolFixedChunkAllocator(const ThreadSafeMemPoolFixedChunkAllocator &alloc): OsclMemPoolFixedChunkAllocator(alloc) {};


    protected:
        OSCL_IMPORT_REF virtual void createmempool();
        OSCL_IMPORT_REF virtual void destroymempool();

        OSCL_IMPORT_REF void addRef_internal();
        OSCL_IMPORT_REF void removeRef_internal();

        uint32 iNumChunk;
        uint32 iChunkSize;
        uint32 iChunkSizeMemAligned;
        Oscl_DefAlloc* iMemPoolAllocator;
        OsclAny* iMemPool;

        Oscl_Vector<OsclAny*, OsclMemAllocator> iFreeMemChunkList;

        // For thread-safe applications, e.g. when one thread does allocate() and the other deallocate()
        // the mempool needs to create a mutex object
        //	- Each sensitive shared data access is then protected by this lock to ensure thread-safety
        OsclMutex iMemPoolMutex;

        bool iCheckNextAvailableFreeChunk;
        OsclMemPoolFixedChunkAllocatorObserver* iObserver;
        OsclAny* iNextAvailableContextData;

        int32 iRefCount;


};

#endif
