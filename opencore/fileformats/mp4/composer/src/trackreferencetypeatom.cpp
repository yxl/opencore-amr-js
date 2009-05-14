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
    This PVA_FF_TrackReferenceTypeAtom Class provides a reference from the containing stream
    to another stream in the MPEG-4 presentation.
*/


#define IMPLEMENT_TrackReferenceTypeAtom

#include "trackreferencetypeatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
// Constructor
PVA_FF_TrackReferenceTypeAtom::PVA_FF_TrackReferenceTypeAtom(uint32 refType)
        : PVA_FF_Atom(refType)
{
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _trackIDs);
    recomputeSize();
}


// Destructor
PVA_FF_TrackReferenceTypeAtom::~PVA_FF_TrackReferenceTypeAtom()
{
    // Clean up vector of track reference ids
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _trackIDs);
}

int32
PVA_FF_TrackReferenceTypeAtom::addTrackReference(uint32 ref)
{
    _trackIDs->push_back(ref);
    recomputeSize();

    return _trackIDs->size();
}

uint32
PVA_FF_TrackReferenceTypeAtom::getTrackReferenceAt(int32 index) const
{
    if (index < (int32)_trackIDs->size())
    {
        return (*_trackIDs)[index];
    }
    return 0; // ERROR condition
}


void
PVA_FF_TrackReferenceTypeAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 4 * _trackIDs->size();

    _size = size;

    // Update size of parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_TrackReferenceTypeAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    for (uint32 i = 0; i < _trackIDs->size(); i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, (*_trackIDs)[i]))
        {
            return false;
        }
    }
    rendered += (4 * _trackIDs->size());

    return true;
}

