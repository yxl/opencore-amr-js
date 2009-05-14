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

#ifndef PVMF_FIXEDSIZE_BUFFER_ALLOC_H_INCLUDED
#define PVMF_FIXEDSIZE_BUFFER_ALLOC_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif


/* Interface ID for the Fixed-size buffer allocator */
#define PVMFFixedSizeBufferAllocUUID PVUuid(0xC8DE4E11,0xF988,0x4E42,0xBA,0x2A,0xAE,0x1A,0xB6,0x99,0x4E,0x25)

/**
 * This interface is used to allocate a set of fixed-size buffers.
 */
class PVMFFixedSizeBufferAlloc
{
    public:

        virtual ~PVMFFixedSizeBufferAlloc() {};

        /**
         * This method allocates a fixed-size buffer as long as there are
         * buffers remaining.  Once the maximum number of buffers have been
         * allocated, further requests will fail.
         *
         * @returns a ptr to a fixed-size buffer
         * or NULL if there is an error.
         */
        virtual OsclAny* allocate() = 0;

        /**
         * This method deallocates a buffer ptr that was previously
         * allocated through the allocate method.
         *
         * @param ptr is a ptr to the previously allocated buffer to release.
         */
        virtual void deallocate(OsclAny* ptr) = 0;

        /**
         * This method returns the size of the buffers that
         * will be allocated.
         *
         * @returns the fixed size used for all buffers.
         */
        virtual uint32 getBufferSize() = 0;

        /**
         * This method returns the maximum number of buffers
         * available for allocation
         *
         * @returns the max number of buffers available for allocation.
         */
        virtual uint32 getNumBuffers() = 0;
};

#endif
