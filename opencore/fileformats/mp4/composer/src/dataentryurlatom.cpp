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
    This PVA_FF_DataEntryUrlAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/

#define IMPLEMENT_DataEntryUrlAtom_H__

#include "dataentryurlatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

// Constructor
PVA_FF_DataEntryUrlAtom::PVA_FF_DataEntryUrlAtom(uint32 selfContained)
        : PVA_FF_DataEntryAtom(DATA_ENTRY_URL_ATOM, (uint8)0, selfContained)
{
    recomputeSize();
}

// Destructor
PVA_FF_DataEntryUrlAtom::~PVA_FF_DataEntryUrlAtom()
{
    // Empty
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_DataEntryUrlAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (getFlags() != 1)
    {
        // media NOT self contained
        if (!PVA_FF_AtomUtils::renderNullTerminatedString(fp, getLocation()))
        {
            return false;
        }
        rendered += _location.get_size() + 1; // 1 for null termination
    }

    return true;
}



void
PVA_FF_DataEntryUrlAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    if (getFlags() != 1)  // media NOT self contained
    {
        size += _location.get_size() + 1; // 1 for null termination
    }

    _size = size;

}


