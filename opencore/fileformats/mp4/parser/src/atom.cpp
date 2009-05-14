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
/*                    MPEG-4 Base Atom Class Implementation                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This Atom Class is the base class for all other Atoms in the MPEG-4 File
    Format.
*/



#define IMPLEMENT_Atom

#include "atom.h"
#include "atomutils.h"

// Normal constructor from fullatom constructor
Atom::Atom(uint32 type)
{
    _type = type;
    _pparent = NULL;
}

Atom::Atom(MP4_FF_FILE *fp)
{
    int32 currfilePointer; // current file position

    _success = false;
    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;

    // SAVE THE CURRENT FILE POSITION
    currfilePointer = AtomUtils::getCurrentFilePosition(fp);

    if (AtomUtils::read32read32(fp, _size, _type))
    {
        if (((int32)_size >= DEFAULT_ATOM_SIZE) &&
                ((int32)(currfilePointer + _size) <= fp->_fileSize))
        {
            _success = true;
            _mp4ErrorCode = EVERYTHING_FINE;
        }
    }
}


Atom::Atom(MP4_FF_FILE *fp, uint32 size, uint32 type)
{
    int32 currfilePointer; // current file position

    _success = false;
    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;

    // SAVE THE CURRENT FILE POSITION
    currfilePointer = AtomUtils::getCurrentFilePosition(fp);

    _size = size;
    _type = type;

    int32 temp = (int32)(_size - DEFAULT_ATOM_SIZE);

    if ((temp >= 0) &&
            ((currfilePointer + temp) <= fp->_fileSize))
    {
        _success = true;
        _mp4ErrorCode = EVERYTHING_FINE;
    }
}

Atom::Atom(uint8 *&buf)
{
    _success = true;
    _mp4ErrorCode = EVERYTHING_FINE;

    if (!AtomUtils::read32read32(buf, _size, _type))
    {
        _success = false;
    }

    int32 size = (int32)_size;

    if (size <= 0)
    {
        _success = false;
        _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
    }

    if (_success)
    {
        /* atom size must atleast be DEFAULT_ATOM_SIZE */
        if (size < DEFAULT_ATOM_SIZE)
        {
            _success = false;
        }
    }
}


// Destructor
Atom::~Atom()
{
    // Empty
}

//Note: ARM linker can't find this symbol if it's declared "inline"
#ifndef __CC_ARM
inline
#endif
uint32
Atom::getDefaultSize() const
{
    return DEFAULT_ATOM_SIZE;
}


