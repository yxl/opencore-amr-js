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
#define IMPLEMENT_MovieExtendsHeaderAtom

#include "movieextendsheaderatom.h"
#include "a_isucceedfail.h"
#include "atomutils.h"


// constructor
PVA_FF_MovieExtendsHeaderAtom::PVA_FF_MovieExtendsHeaderAtom()
        : PVA_FF_FullAtom(MOVIE_EXTENDS_HEADER_ATOM, (uint8)0, (uint32)0)
{
    _fragmentDuration = 0;
    _fileOffset = 0;
    _oRendered = false;
    recomputeSize();

}

//destructor
PVA_FF_MovieExtendsHeaderAtom::~PVA_FF_MovieExtendsHeaderAtom()
{
    // do nothing
}


// return fragment duration
uint32
PVA_FF_MovieExtendsHeaderAtom::getFragmentDuration(void)
{
    return _fragmentDuration;
}


// set fragment duration
void
PVA_FF_MovieExtendsHeaderAtom::setFragmentDuration(uint32 duration)
{
    _fragmentDuration = duration;
}


// update the value of fragment duration for new sample
void
PVA_FF_MovieExtendsHeaderAtom::updateMovieFragmentDuration(uint32 ts)
{
    if (ts > _fragmentDuration)
    {
        _fragmentDuration = ts;
    }
}

// write the duration value at the file offset stored during rendering
void
PVA_FF_MovieExtendsHeaderAtom::writeMovieFragmentDuration(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 fileWriteOffset;

    fileWriteOffset = PVA_FF_AtomUtils::getCurrentFilePosition(fp);

    PVA_FF_AtomUtils::seekFromStart(fp, _fileOffset);

    if (_oRendered == true)
    {
        PVA_FF_AtomUtils::render32(fp, _fragmentDuration);
    }

    PVA_FF_AtomUtils::seekFromStart(fp, fileWriteOffset);
}


// recompute size of atom
void
PVA_FF_MovieExtendsHeaderAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4; // For fragment duration

    _size = size;

    // Update the parent atom size
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


// write atom to the target file
bool
PVA_FF_MovieExtendsHeaderAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp)
{
    uint32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // store file offset to modify duration later
    _fileOffset = PVA_FF_AtomUtils::getCurrentFilePosition(fp);

    if (!PVA_FF_AtomUtils::render32(fp, _fragmentDuration))
    {
        return false;
    }

    rendered += 4;

    _oRendered = true;

    return true;
}
