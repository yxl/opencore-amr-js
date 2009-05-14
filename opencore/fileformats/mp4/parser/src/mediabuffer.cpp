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
/*                            MPEG-4 Util: MediaBuffer                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaBuffer Class is the implementation of the IMediaBuffer interface
    that contains the guts of the data handling of media data.
*/


#define IMPLEMENT_MediaBuffer

#include "mediabuffer.h"
#include "atomutils.h"


// Constructor
MediaBuffer::MediaBuffer(int32 size)
{
    _owner = NULL;
    _pdata = (uint8 *) oscl_malloc(size); // Allocate buffer space
}

// Destructor
MediaBuffer::~MediaBuffer()
{
    oscl_free(_pdata);
}

void
MediaBuffer::freeBuf()
{
    if (_owner != NULL)
    {
        _owner->freeBuf(this);
    }
}

void
MediaBuffer::setOwner(MediaBufferManager *owner)
{
    _owner = owner;
}

void
MediaBuffer::readByteData(MP4_FF_FILE *fp, uint32 size)
{
    AtomUtils::readByteData(fp, size, _pdata);
    _size = size;
}
