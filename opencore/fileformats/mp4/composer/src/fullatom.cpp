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
    This PVA_FF_FullAtom Class fp the base class for some Atoms in the MPEG-4 File
    Format.
*/


#define IMPLEMENT_FullAtom

#include "fullatom.h"
#include "atomutils.h"

const uint32 DEFAULT_FULL_ATOM_SIZE = 12; // (8 bytes from PVA_FF_Atom + 1 for
//  version and 3 for flags)

// Constructor
PVA_FF_FullAtom::PVA_FF_FullAtom(uint32 type, uint8 version, uint32 flags)
        : PVA_FF_Atom(type)
{
    _version = version;
    _flags = flags;
}

// Desctuctor
PVA_FF_FullAtom::~PVA_FF_FullAtom()
{
    // Empty
}

inline uint32
PVA_FF_FullAtom::getDefaultSize() const
{
    return DEFAULT_FULL_ATOM_SIZE;
}

bool
PVA_FF_FullAtom::renderAtomBaseMembers(MP4_AUTHOR_FF_FILE_IO_WRAP *fp) const
{
    // Render PVA_FF_Atom size
    if (!PVA_FF_AtomUtils::render32(fp, getSize()))
    {
        return false;
    }

    // Render PVA_FF_Atom type (MOVIE_ATOM)
    if (!PVA_FF_AtomUtils::render32(fp, getType()))
    {
        return false;
    }

    // Render Version
    if (!PVA_FF_AtomUtils::render8(fp, getVersion()))
    {
        return false;
    }

    // Render flags
    if (!PVA_FF_AtomUtils::render24(fp, getFlags()))
    {
        return false;
    }

    return true;
}


