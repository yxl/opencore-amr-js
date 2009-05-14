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
    This PVA_FF_TrackReferenceAtom Class provides a reference from the containing stream
    to another stream in the MPEG-4 presentation.
*/


#define IMPLEMENT_TrackReferenceAtom

#include "trackreferenceatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_TrackReferenceAtom::PVA_FF_TrackReferenceAtom(uint32 refType)
        : PVA_FF_Atom(TRACK_REFERENCE_ATOM)
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_TrackReferenceTypeAtom, (refType), _ptrackReferenceTypeAtom);

    recomputeSize();
    _ptrackReferenceTypeAtom->setParent(this);
}

// Destructor
PVA_FF_TrackReferenceAtom::~PVA_FF_TrackReferenceAtom()
{
    PV_MP4_FF_DELETE(NULL, PVA_FF_TrackReferenceTypeAtom, _ptrackReferenceTypeAtom);
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_TrackReferenceAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!_ptrackReferenceTypeAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _ptrackReferenceTypeAtom->getSize();

    return true;
}


void
PVA_FF_TrackReferenceAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += _ptrackReferenceTypeAtom->getSize();
    _size = size;

    // Update size of parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
