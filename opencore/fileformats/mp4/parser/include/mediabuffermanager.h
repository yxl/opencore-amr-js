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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                     MPEG-4 Util: MediaBufferManager Class                     */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
	This MediaBufferManager Class is responsible for maintaining list of used
	and free MediaBuffers.  This is promarily used by the Mpeg4File class when
	it is retrieving media data from disk.  It needs buffers to store data and
	to pass to the consumers of the media data - namesly PVServer and PVPlayer.
*/


#ifndef MEDIABUFFERMANAGER_H_INCLUDED
#define MEDIABUFFERMANAGER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef MEDIABUFFER_H_INCLUDED
#include "mediabuffer.h"
#endif

#ifndef IMEDIABUFFER_H_INCLUDED
#include "imediabuffer.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif


class MediaBuffer; // Forward declaration for circular includes

class MediaBufferManager
{

    public:
        MediaBufferManager(int32 bufSize = 32728, uint32 numBufs = 1); // Constructor
        // Create 'numBufs' initial buffers each of size 'bufSize'

        virtual ~MediaBufferManager();

        void freeBuf(MediaBuffer *buf); // Called by buffer when it is freed
        MediaBuffer *getBuf();


    private:
        MediaBuffer *newBuf(); // By default create a 32KB buffer

        Oscl_Vector<MediaBuffer*, OsclMemAllocator> *_pfreeBufVec;
        Oscl_Vector<MediaBuffer*, OsclMemAllocator> *_pusedBufVec;

        int32 _bufferSize; // Size of buffers (in bytes)
};

#endif // MEDIABUFFERMANAGER_H_INCLUDED


