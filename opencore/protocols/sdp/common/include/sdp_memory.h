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

#ifndef SDP_MEMORY_H
#define SDP_MEMORY_H

#include "oscl_mem.h"

#include "oscl_base_alloc.h"

typedef OsclMemAllocator SDPParserAlloc;

class SDPMemory
{
    public:
        SDPMemory() {};
        ~SDPMemory() {};
        template <class T> T* alloc(uint32 size, T* temp)
        {
            T* ptr = (T*)oscl_malloc(size);
            return ptr;
        };
        template <class T> void dealloc(T* p)
        {
            oscl_free(p);
        };
};

template <class T> class SDPAllocDestructDealloc: public OsclDestructDealloc
{
    public:
        virtual ~SDPAllocDestructDealloc() {}

        T* allocate(uint32 size)
        {
            SDPParserAlloc alloc;
            T* ptr = OSCL_PLACEMENT_NEW(alloc.ALLOCATE(size), T());
            return ptr;
        }

        virtual void destruct_and_dealloc(OsclAny* ptr)
        {
            T* tptr ;
            tptr = reinterpret_cast<T*>(ptr);
            tptr->~T();
            SDPParserAlloc alloc;
            alloc.deallocate(ptr);
        }
};

#endif


