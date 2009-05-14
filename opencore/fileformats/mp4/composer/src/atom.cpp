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
    This PVA_FF_Atom Class is the base class for all other Atoms in the MPEG-4 File
    Format.
*/



#define IMPLEMENT_Atom

#include "atom.h"
#include "atomutils.h"

// Normal constructor
PVA_FF_Atom::PVA_FF_Atom(uint32 type)
{
    _type = type;
    _pparent = NULL;
}

// Destructor
PVA_FF_Atom::~PVA_FF_Atom()
{
    // Empty
}

bool
PVA_FF_Atom::renderAtomBaseMembers(MP4_AUTHOR_FF_FILE_IO_WRAP *fp) const
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
    return true;
}


uint32
PVA_FF_Atom::getDefaultSize() const
{
    return DEFAULT_ATOM_SIZE;
}


