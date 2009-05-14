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

//               P V M F _ M E M P O O L

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =



/*! \file pvmf_mempool.h
    \brief This file contains the implementation of memory pool allocator
*/


#ifndef PVMF_MEMPOOL_H_INCLUDED
#define PVMF_MEMPOOL_H_INCLUDED

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVMF_MEDIA_CMD_H_INCLUDED
#include "pvmf_media_cmd.h"
#endif

#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

class PVMFMemPoolFixedChunkAllocator : public OsclMemPoolFixedChunkAllocator
{
    public:
        /** This API throws an exception when the memory allocation for pool fails
          * If numchunk and chunksize parameters are not set, memory pool of 1 chunk will be created in the first call to allocate.
          * The chunk size will be set to the n passed in for allocate().
          * If numchunk parameter is set to 0, the memory pool will use 1 for numchunk.
          *
          * @return void
          *
          */
        OSCL_IMPORT_REF PVMFMemPoolFixedChunkAllocator(const char* name = NULL, const uint32 numchunk = 1, const uint32 chunksize = 0, Oscl_DefAlloc* gen_alloc = NULL);

        OSCL_IMPORT_REF virtual ~PVMFMemPoolFixedChunkAllocator();

        /** This API throws an exception when n is greater than the fixed chunk size or there are no free chunk available in the pool.
          * If the memory pool hasn't been created yet, the pool will be created with chunk size equal to n so n must be greater than 0. Exception will be thrown if memory allocation for the memory pool fails.
          *
          * @return pointer to available chunk from memory pool
          *
          */
        OSCL_IMPORT_REF OsclAny* allocate(const uint32 n);

        /** This API throws an exception when the pointer p passed in is not part of the memory pool.
           * Exception will be thrown if the memory pool is not set up yet.
           *
           * @return void
           *
           */
        OSCL_IMPORT_REF void deallocate(OsclAny* p);

        OSCL_IMPORT_REF void LogMediaDataInfo(PVMFSharedMediaDataPtr aMediaData);

        OSCL_IMPORT_REF void LogMediaCmdInfo(PVMFSharedMediaCmdPtr aMediaCmd);

    private:
        void createmempool();

        OSCL_HeapString<OsclMemAllocator> iName;
        PVLogger* iDatapathLogger;
        OsclErrorTrapImp* iOsclErrorTrapImp;
};

#endif

/*! @} */
