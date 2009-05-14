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
/*                            MPEG-4 FullAtom Class                              */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This FullAtom Class is the base class for some Atoms in the MPEG-4 File
    Format.
*/


#define IMPLEMENT_FullAtom

#include "fullatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Constructor
FullAtom::FullAtom(uint32 type, uint8 version, uint32 flags)
        : Atom(type)
{
    _version = version;
    _flags = flags;
}

FullAtom::FullAtom(MP4_FF_FILE *fp)
        : Atom(fp)
{
    if (_success)
    {

        // Size and type set in Atom constructor
        uint32 data;

        if (!AtomUtils::read32(fp, data))
        {
            _mp4ErrorCode = READ_FAILED;
            _success = false;
            return;
        }

        _version = (uint8)(data >> 24);
        _flags = data & 0x00ffffff;

        // Version 1 atoms supported
        //we support version 0 & 1
        if (_version)
        {
            if ((_type == MOVIE_HEADER_ATOM) || (_type == TRACK_HEADER_ATOM) || (_type == MEDIA_HEADER_ATOM))
                ;
            else
            {
                _success = false;
                _mp4ErrorCode = ATOM_VERSION_NOT_SUPPORTED;
            }
        }
    }
}

FullAtom::FullAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    if (_success)
    {
        // Size and type set in Atom constructor
        uint32 data;

        if (!AtomUtils::read32(fp, data))
        {
            _mp4ErrorCode = READ_FAILED;
            _success = false;
            return;
        }

        _version = (uint8)(data >> 24);
        _flags = data & 0x00ffffff;

        // Version 1 atoms supported
        //we support version 0 & 1
        if (_version)
        {
            if ((_type == MOVIE_HEADER_ATOM) || (_type == TRACK_HEADER_ATOM) || (_type == MEDIA_HEADER_ATOM))
                ;
            else
            {
                _success = false;
                _mp4ErrorCode = ATOM_VERSION_NOT_SUPPORTED;
            }
        }
    }
}


// Destructor
OSCL_EXPORT_REF FullAtom::~FullAtom()
{
    // Empty
}

//Note: ARM linker can't find this symbol if it's declared "inline"
#ifndef __CC_ARM
inline
#endif
OSCL_EXPORT_REF uint32
FullAtom::getDefaultSize() const
{
    return DEFAULT_FULL_ATOM_SIZE;
}


