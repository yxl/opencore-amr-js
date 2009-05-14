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
#ifndef PVMF_COUNTED_PTR_H_INCLUDED
#define PVMF_COUNTED_PTR_H_INCLUDED


#ifndef   OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef   OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef   OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif


template <class T>
class PVMFReferenceCounter : public OsclRefCounter
{
    public:
        //This is the surrogate constructor.
        //It allocates the ReferenceCounter from it's own pool
        //so it can manage the deallocation later, when the time is right.
        static PVMFReferenceCounter<T>* create(void)
        {
            //Check if the memory pools need to be setup.
            if (!poolrc)
            {
                rcPool = OSCL_NEW
                         (
                             OsclMemPoolFixedChunkAllocator,
                             (
                                 poolDepth/*chunks*/
                                 , sizeof(PVMFReferenceCounter<T>)/*chunk size*/
                                 , NULL/*allocator*/
                             )
                         );
                tPool = OSCL_NEW
                        (
                            OsclMemPoolFixedChunkAllocator,
                            (
                                poolDepth/*chunks*/
                                , sizeof(T)/*chunk size*/
                                , NULL/*allocator*/
                            )
                        );
            }
            //Increment the pool's ref counter each time we allocate from it.
            poolrc++;

            //Allocate the template object from the pool.
            T* thisObj = OSCL_STATIC_CAST(T*, tPool->allocate(sizeof(T)));
            //Construct the object in place using it's default constructor.
            thisObj = OSCL_PLACEMENT_NEW(thisObj, T());

            //Allocate "this" ReferenceCounter from the pool.
            PVMFReferenceCounter<T>* thisRC =
                OSCL_STATIC_CAST(PVMFReferenceCounter<T>*,
                                 rcPool->allocate(sizeof(PVMFReferenceCounter<T>)));
            //Construct the ReferenceCounter in place.
            thisRC = OSCL_PLACEMENT_NEW(thisRC, PVMFReferenceCounter<T>(thisObj));

            return thisRC;
        }

        virtual void addRef()
        {
            rc++;
        }

        virtual void removeRef()
        {
            rc--;
            if (!rc)
            {
                obj->~T();
                tPool->deallocate(obj);
                rcPool->deallocate(this);
                poolrc--;
                if (!poolrc)
                {
                    OSCL_DELETE(rcPool);
                    OSCL_DELETE(tPool);
                }
            }
        }

        virtual uint32 getCount()
        {
            return rc;
        }

        //The actual reference count.
        uint32   rc;

        //The object to which were are counting references.
        T*       obj;

    private:
        //These are the memory pools from which the reference counter
        //and template object are instantiated.  These are fixed-chunk
        //allocators to optimize allocation speed.
        static OsclMemPoolFixedChunkAllocator* rcPool;
        static OsclMemPoolFixedChunkAllocator* tPool;

        //This is the reference counter for the memory pools so we know when to free them.
        static uint32                          poolrc;

        //This is the size of the memory pool in whole-unit allocations.
        static uint32                          poolDepth;

        //The default constructor is private to prevent accidentally
        //allocating one incorrectly.  The proper way to allocate a
        //ReferenceCounter is to call the "create" method AFTER initializing
        //the OSCL memory subsystem.
        PVMFReferenceCounter(T* aObj = NULL)
                : rc(1)
                , obj(aObj)
        {
        }
};


/**
 *@note: Because CounterPtr creates a ReferenceCounter,
 * you cannot instantiate a CounterPtr before calling OsclMem::Init().
 */
template <class T>
class PVMFCountedPtr
{
    public:
        PVMFReferenceCounter<T>* cnt;

        PVMFCountedPtr(void)
        {
            cnt = PVMFReferenceCounter<T>::create();
        }

        ~PVMFCountedPtr()
        {
            cnt->removeRef();
        }

        PVMFCountedPtr<T>& operator=(const PVMFCountedPtr<T>& r)
        {
            //Order of statements is important.
            //Must add ref first in case r==this.
            r.cnt->addRef();
            //Detach the old ref counter.
            cnt->removeRef();
            //Attach the new ref counter.
            cnt = r.cnt;
            return *this;
        }

        inline T& operator*() const
        {
            return *(cnt->obj);
        }

        inline T* operator->() const
        {
            return cnt->obj;
        }
};


#endif //PVMF_COUNTED_PTR_H_INCLUDED


