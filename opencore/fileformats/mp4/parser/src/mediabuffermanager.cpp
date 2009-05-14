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


#define IMPLEMENT_MediaBufferManager

#include "mediabuffermanager.h"

typedef Oscl_Vector<MediaBuffer*, OsclMemAllocator> mediaBufferVecType;

// Constructor
MediaBufferManager::MediaBufferManager(int32 bufSize, uint32 numBufs)
{
    _bufferSize = bufSize;

    // Create 'numBufs' initial buffers each of size 'bufSize'

    PV_MP4_FF_NEW(fp->auditCB, mediaBufferVecType, (), _pfreeBufVec);
    PV_MP4_FF_NEW(fp->auditCB, mediaBufferVecType, (), _pusedBufVec);

    // Allocate free buffers
    for (uint32 i = 0; i < numBufs; i++)
    {
        MediaBuffer *mb = NULL;
        PV_MP4_FF_NEW(fp->auditCB, MediaBuffer, (bufSize), mb);
        mb->setOwner(this);
        (*_pfreeBufVec).push_back(mb);
    }
}

MediaBufferManager::~MediaBufferManager()
{
    uint32 i;

    // Clean up the vectors of buffers
    for (i = 0; i < _pfreeBufVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, MediaBuffer, (*_pfreeBufVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, mediaBufferVecType, Oscl_Vector, _pfreeBufVec);

    for (i = 0; i < _pusedBufVec->size(); i++)
    {
        PV_MP4_FF_DELETE(NULL, MediaBuffer, (*_pusedBufVec)[i]);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, mediaBufferVecType, Oscl_Vector, _pusedBufVec);
}

// Called by buffer when it is freed
void
MediaBufferManager::freeBuf(MediaBuffer *buf)
{
    // Need to find buffer in _pusedBufVec

    int32 i = 0;
    MediaBuffer **x;
    for (x = _pusedBufVec->begin();x < _pusedBufVec->end(); x++)
    {
        MediaBuffer *used = (MediaBuffer*)(*_pusedBufVec)[i];
        if (used == buf)
        {
            break; // Found buffer in used vec
        }
        i++;
    }

    (*_pfreeBufVec).push_back(buf); //  buffer to free vec
    _pusedBufVec->erase(x); // Delete buffer from used vec
}


MediaBuffer*
MediaBufferManager::getBuf()
{
    if (_pfreeBufVec->size() != 0)
    {
        MediaBuffer *mb = (MediaBuffer *)(*_pfreeBufVec)[_pfreeBufVec->size()-1];  // get last buf
        _pfreeBufVec->pop_back();
        (*_pusedBufVec).push_back(mb);
        return mb;
    }
    else   // No free buffers - need to create one
    {
        MediaBuffer *mb = newBuf();
        (*_pusedBufVec).push_back(mb);
        return mb;
    }
}

// By default create a 32KB buffer - if want another size buffer, need to set
// the bufSize member for this class
MediaBuffer*
MediaBufferManager::newBuf()
{
    MediaBuffer *mb = NULL;
    PV_MP4_FF_NEW(fp->auditCB, MediaBuffer, (_bufferSize), mb);
    return mb;
}

