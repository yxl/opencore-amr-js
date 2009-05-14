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
// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//               O S C L _ M E M P O O L _ A L L O C A T O R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclerror OSCL Error
 *
 * @{
 */


/*! \file oscl_mempool_allocator.cpp
    \brief This file contains the implementation of memory pool allocator for leave/trap
*/

/***************************************************************************************
File Name		: oscl_mempool_allocator.cpp
Description		: File containing implementation of class OsclMemPoolAllocator that provides
				  methods for creating, deleting memory pool.
Coding History	:
				  Achint Kaur			April 11, 2006		Initial Draft
***************************************************************************************/

#include "oscl_mempool_allocator.h"

#include "oscl_exception.h"

#include "oscl_mem_basic_functions.h"

/***************************************************************************************
Description		: Constructor method for memory pool allocation. Sets iCustomAllocator
			      to the passed allocator pointer. Also initializes iBaseAddress to NULL.
Arguments		: gen_alloc - Custom allocator for memory. This is an input argument.
Return Values	: None
Assumptions		: None
Known Issues	: None
***************************************************************************************/
OsclMemPoolAllocator::OsclMemPoolAllocator(Oscl_DefAlloc* gen_alloc)
        : iCustomAllocator(gen_alloc),
        iBaseAddress(0)
{
}

/***************************************************************************************
Description		: Virtual destructor for memory pool allocation. Calls DestroyMemPool, if
				  iBaseAddress is not NULL i.e. memory pool exists.
Arguments		: None
Return Values	: None
Assumptions		: None
Known Issues	: None
***************************************************************************************/
OsclMemPoolAllocator::~OsclMemPoolAllocator()
{
    if (iBaseAddress)
    {
        DestroyMemPool();
    }
}

/***************************************************************************************
Description		: Method for creating memory pool given the number of chunks and chunk size.
Arguments		: aNumChunk - Default number of chunks in a memory pool is 2.
				  aChunkSize - Default size of each chunk is 4.
Return Values	: OsclAny* - Base address for the memory pool
Assumptions		: It is assumed that memory audit is not required so, _oscl_malloc used directly.
				  Client can pass its custom allocator for memory.
				  Malloc will be used if custom allocator is not set.
				  Memory alignment is taken from osclmemory.
				  uint32 and int32 are assumed to be oscl compliant.
Known Issues	: Is there a naming convention for leaving methods in oscl ?
				  Is there a need for memory alignment as done ?
				  Is OSCL_MEM_FILL_WITH_PATTERN required ?
***************************************************************************************/
OsclAny* OsclMemPoolAllocator::CreateMemPool(const uint32 aNumChunk, const uint32 aChunkSize)
{
    if (aNumChunk == 0 || aChunkSize == 0)
    {
        OSCL_LEAVE(OsclErrArgument);

        // OSCL_UNUSED_RETURN(NULL);	This statement was removed to avoid compiler warning for Unreachable Code
    }

    // Heap memory alligned chunk size
    uint32 lChunkSizeMemAligned;

    lChunkSizeMemAligned = oscl_mem_aligned_size(aChunkSize);

    if (iCustomAllocator)
    {
        iBaseAddress = iCustomAllocator->ALLOCATE(aNumChunk * lChunkSizeMemAligned);
    }
    else
    {
        iBaseAddress = _oscl_malloc(aNumChunk * lChunkSizeMemAligned);
    }

    if (iBaseAddress == NULL)
    {
        OSCL_LEAVE(OsclErrNoMemory);

        // OSCL_UNUSED_RETURN(NULL);	This statement was removed to avoid compiler warning for Unreachable Code

    }

#if OSCL_MEM_FILL_WITH_PATTERN

    oscl_memset(iMemPool, 0x55, aNumChunk*lChunkSizeMemAligned);

#endif

    return iBaseAddress;

}

/***************************************************************************************
Description		: Performs memory alignment for the passed size argument.
Arguments		: x - uint representing the size to be memory aligned
Return Values	: uint - Memory aligned size
Assumptions		: None
Known Issues	: None
***************************************************************************************/
uint OsclMemPoolAllocator::oscl_mem_aligned_size(uint x)
{

    uint y;

    if (x & 0x7)
    {
        y = x & (~0x7);
        y += 8;
    }
    else
    {
        y = x;
    }

    return y;
}

/***************************************************************************************
Description		: Method for destroying memory pool
Arguments		: None
Return Values	: None
Assumptions		: It is assumed that memory audit is not required so, _oscl_free used directly.
Known Issues	: None
Condition		: Is there any naming convention for leaving methods in oscl ?
***************************************************************************************/
void OsclMemPoolAllocator::DestroyMemPool()
{
    // If client class calls DestroyMemPool without CreateMemPool i.e iBaseAddress is NULL then leave
    if (!iBaseAddress)
    {
        OSCL_LEAVE(OsclErrArgument);

    }

    if (iCustomAllocator)
    {
        iCustomAllocator->deallocate(iBaseAddress);
    }
    else
    {
        _oscl_free(iBaseAddress);
    }

    iCustomAllocator = 0;

    iBaseAddress = 0;

    return;
}



