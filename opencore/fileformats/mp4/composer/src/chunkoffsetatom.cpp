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
/*
    This PVA_FF_ChunkOffsetAtom Class gives the index of each chunk into the
    containing FILE.
*/

#define IMPLEMENT_ChunkOffsetAtom

#include "chunkoffsetatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
// Constructor
PVA_FF_ChunkOffsetAtom::PVA_FF_ChunkOffsetAtom(uint32 mediaType,
        uint32 fileAuthoringFlags)
        : PVA_FF_FullAtom(FourCharConstToUint32('s', 't', 'c', 'o'), (uint8)0, (uint32)0),
        _mediaType(mediaType)
{
    _oInterLeaveMode = false;
    _modified = true; // True for a new atom
    _entryCount = 0;
    _currentDataOffset = 0;

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pchunkOffsets);

    if (fileAuthoringFlags & PVMP4FF_SET_MEDIA_INTERLEAVE_MODE)
    {
        _oInterLeaveMode = true;
    }

    recomputeSize();
}

// Destructor
PVA_FF_ChunkOffsetAtom::~PVA_FF_ChunkOffsetAtom()
{
    // Cleanup vector
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pchunkOffsets);
}

void
PVA_FF_ChunkOffsetAtom::nextSample(uint32 size,
                                   bool isChunkStart, uint32 baseOffset)
{
    // If sample is the start of a new chunk, add the current offset entry
    if (isChunkStart)
    {
        if (_oInterLeaveMode)
        {
            addChunkOffset(baseOffset);
            return;
        }
        else
        {
            addChunkOffset(_currentDataOffset);
        }
    }

    // Increment the size of the offset by the length of this sample
    switch (_mediaType)
    {
        case MEDIA_TYPE_TEXT: //for timed text track
        case MEDIA_TYPE_AUDIO:
        case MEDIA_TYPE_VISUAL:
        {
            _currentDataOffset += size;
        }
        break;
        case MEDIA_TYPE_UNKNOWN:
        default:
            break;
    }
}

// Adding to and getting first chunk offset values
void
PVA_FF_ChunkOffsetAtom::addChunkOffset(uint32 offset)
{
    _pchunkOffsets->push_back(offset);
    _entryCount += 1;

    recomputeSize();
}


// Updating all the chunk entries based on the current file offset at which
// the corresponding mediaDataAtom MEDIA_DATA_ATOM gets rendered to disk
void
PVA_FF_ChunkOffsetAtom::updateChunkEntries(uint32 fileOffset)
{
    if (_modified)
    {
        for (uint32 i = 0; i < _pchunkOffsets->size(); i++)
        {
            (*_pchunkOffsets)[i] += fileOffset;
        }
        _modified = false;
        return;
    }
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_ChunkOffsetAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, getEntryCount()))
    {
        return false;
    }
    rendered += 4;

    if (_pchunkOffsets->size() < getEntryCount())
    {
        return false;
    }
    for (uint32 i = 0; i < getEntryCount(); i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, (*_pchunkOffsets)[i]))
        {
            return false;
        }
    }
    rendered += 4 * getEntryCount();

    return true;
}

void
PVA_FF_ChunkOffsetAtom::recomputeSize()
{
    int size = getDefaultSize();
    size += 4; // For entryCount
    size += 4 * getEntryCount();

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

void
PVA_FF_ChunkOffsetAtom::reAuthorFirstChunkOffset(uint32 offset)
{
    (*_pchunkOffsets)[0] = offset;
}
