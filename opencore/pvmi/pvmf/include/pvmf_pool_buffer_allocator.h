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
/**
 *  @file pvmf_pool_buffer_allocator.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media message
 *  header class which is used to hold the basic information such as timestamp,
 *  sequence number, etc for every media message.
 *
 */

#ifndef PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED
#define PVMF_POOL_BUFFER_ALLOCATOR_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

class PVLogger;

//typedef OSCLMemAutoPtr<OsclRefCounterMemFrag*> OsclRefCounterMemFrag_ptr;
typedef Oscl_TAlloc<OsclRefCounterMemFrag, OsclMemAllocator> OsclRefCounterMemFrag_Alloc;

class PVMFBufferPoolAllocator : public OsclDestructDealloc
{
    public:
        OSCL_IMPORT_REF PVMFBufferPoolAllocator(bool aLeaveOnAllocFailure = true);
        OSCL_IMPORT_REF virtual ~PVMFBufferPoolAllocator();
        OSCL_IMPORT_REF void SetLeaveOnAllocFailure(bool aLeaveOnAllocFailure);
        OSCL_IMPORT_REF virtual void size(uint16 num_frags, uint16 frag_size);
        OSCL_IMPORT_REF virtual void clear();
        OSCL_IMPORT_REF virtual OsclRefCounterMemFrag get();
        OSCL_IMPORT_REF virtual void release(OsclRefCounterMemFrag&);
        // OsclDestructDealloc virtual
        OSCL_IMPORT_REF virtual void destruct_and_dealloc(OsclAny* ptr);
        OSCL_IMPORT_REF void SetFailFrequency(uint16 aFrequency);
    protected:
        uint32 iFragSize;
        bool iDestroyPool;
        Oscl_Vector<OsclRefCounterMemFrag, OsclRefCounterMemFrag_Alloc> iAvailFragments;
        PVLogger *iLogger;
        uint32 iAllocNum;
        uint32 iFailFrequency;
        bool iLeaveOnAllocFailure;
};


#endif
