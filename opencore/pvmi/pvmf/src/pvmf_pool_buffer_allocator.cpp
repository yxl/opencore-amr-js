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
#include "pvlogger.h"
#include "pvmf_pool_buffer_allocator.h"
#include "oscl_mem.h"
#include "oscl_time.h"

OSCL_EXPORT_REF PVMFBufferPoolAllocator::PVMFBufferPoolAllocator(bool aLeaveOnAllocFailure) :
        iFragSize(0),
        iDestroyPool(false),
        iLeaveOnAllocFailure(aLeaveOnAllocFailure)
{
    iLogger = PVLogger::GetLoggerObject("pvmf.bufferpoolallocator");
    iAllocNum = 0;
    iFailFrequency = 0;
}

OSCL_EXPORT_REF PVMFBufferPoolAllocator::~PVMFBufferPoolAllocator()
{
    iDestroyPool = true;
    iAvailFragments.clear();
}

OSCL_EXPORT_REF void PVMFBufferPoolAllocator::SetLeaveOnAllocFailure(bool aLeaveOnAllocFailure)
{
    iLeaveOnAllocFailure = aLeaveOnAllocFailure;
}

OSCL_EXPORT_REF void PVMFBufferPoolAllocator::size(uint16 num_frags, uint16 frag_size)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFBufferPoolAllocator::size num_frags(%d), frag_size(%d)", num_frags, frag_size));
    iFragSize = frag_size;
    iAvailFragments.reserve(num_frags);

    for (uint16 frag_num = 0; frag_num < num_frags; frag_num++)
    {
        unsigned aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
        uint8* buf = (uint8*)OSCL_MALLOC(aligned_refcnt_size + frag_size);

        // check for out-of-memory
        if (!buf)
        {
            iAvailFragments.clear();
            OSCL_LEAVE(OSCL_BAD_ALLOC_EXCEPTION_CODE);
        }

        // ref counter will delete itself when refcount goes to 0
        OsclRefCounterDA* ref_counter = OSCL_PLACEMENT_NEW(buf, OsclRefCounterDA(buf, this));
        OsclMemoryFragment m;
        m.ptr = buf + aligned_refcnt_size;
        m.len = iFragSize;
        OsclRefCounterMemFrag frag(m, ref_counter, iFragSize);
        iAvailFragments.push_back(frag);
    }
}

OSCL_EXPORT_REF void PVMFBufferPoolAllocator::clear()
{
    iDestroyPool = true;
    iAvailFragments.clear();
}

OSCL_EXPORT_REF void PVMFBufferPoolAllocator::destruct_and_dealloc(OsclAny* ptr)
{
    if (!iDestroyPool)
    {
        // Create another ref counter for the buffer and add it back to the available list
        OsclRefCounterDA* ref_counter = OSCL_PLACEMENT_NEW(ptr, OsclRefCounterDA(ptr, this));
        unsigned aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
        OsclMemoryFragment m;
        m.ptr = (uint8*)ptr + aligned_refcnt_size;
        m.len = iFragSize;
        OsclRefCounterMemFrag frag(m, ref_counter, iFragSize);
        release(frag);
    }
    else
    {
        OSCL_FREE(ptr);
    }
}

OSCL_EXPORT_REF OsclRefCounterMemFrag PVMFBufferPoolAllocator::get()
{
    OsclRefCounterMemFrag ret;
#if _DEBUG
    ++iAllocNum;
    if (iFailFrequency)
    {
        TimeValue timenow;
        int ms = timenow.to_msec();
        if (iAllocNum % ((ms & 0xF) + 1) == 0)
        {
            // throw exception
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NONFATAL_ERROR, (0, "PVMFBufferPoolAllocator::get - Simulating out of fragment exception !! \n"));
            if (iLeaveOnAllocFailure)
            {
                OSCL_LEAVE(OSCL_BAD_ALLOC_EXCEPTION_CODE);
            }
            return ret;
        }
    }
#endif

    if (!iAvailFragments.empty())
    {
        ret = iAvailFragments.back();
        iAvailFragments.pop_back();
        ret.getMemFrag().len = iFragSize;
        return ret;
    }
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NONFATAL_ERROR, (0, "PVMFBufferPoolAllocator::get - Out of fragments !! \n"));

    if (iLeaveOnAllocFailure)
    {
        OSCL_LEAVE(OSCL_BAD_ALLOC_EXCEPTION_CODE);
    }
    return ret;
}

OSCL_EXPORT_REF void PVMFBufferPoolAllocator::release(OsclRefCounterMemFrag& frag)
{
    iAvailFragments.push_back(frag);
}

OSCL_EXPORT_REF void PVMFBufferPoolAllocator::SetFailFrequency(uint16 aFrequency)
{
    iFailFrequency = aFrequency;
}

