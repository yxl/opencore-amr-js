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
/*                     MPEG-4 TrackReferenceTypeAtom Class                       */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackReferenceTypeAtom Class provides a reference from the containing stream
    to another stream in the MPEG-4 presentation.
*/


#define IMPLEMENT_TrackReferenceTypeAtom

#include "trackreferencetypeatom.h"
#include "atomutils.h"
#include "atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

// Stream-in Constructor
TrackReferenceTypeAtom::TrackReferenceTypeAtom(MP4_FF_FILE *fp,
        uint32 size,
        uint32 type)
        : Atom(fp, size, type)
{
    if (_success)
    {

        _pparent = NULL;

        uint32 base = getDefaultSize(); // Get size of base Atom class
        uint32 actual = getSize(); // Get size of actual atom
        uint32 delta = actual - base;

        PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _trackIDs);

        uint32 id;
        for (uint32 i = 0; i < delta; i += 4)
        {
            // Read in the vector of trackIDs
            if (!AtomUtils::read32(fp, id))
            {
                _mp4ErrorCode = READ_TRACK_REFERENCE_TYPE_ATOM_FAILED;
                _success = false;
                break;
            }
            (*_trackIDs).push_back(id);
        }
    }
    else
    {
        _mp4ErrorCode = READ_TRACK_REFERENCE_TYPE_ATOM_FAILED;
    }
}

// Destructor
TrackReferenceTypeAtom::~TrackReferenceTypeAtom()
{
    // Clean up vector of track reference ids
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _trackIDs);
}


uint32
TrackReferenceTypeAtom::getTrackReferenceAt(int32 index) const
{
    if ((uint32)index < _trackIDs->size())
    {
        if (_type == DPND_TRACK_REFERENCE_TYPE)
        {
            return (*_trackIDs)[index];
        }
        else
        {
            return 0;
        }
    }
    return 0; // ERROR condition
}


