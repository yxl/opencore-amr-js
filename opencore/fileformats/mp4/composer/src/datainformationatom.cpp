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
    This PVA_FF_DataInformationAtom Class contains objects that declare the location
    of the media information within the stream.
*/


#define IMPLEMENT_DataInformationAtom

#include "datainformationatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"


// Constructor
PVA_FF_DataInformationAtom::PVA_FF_DataInformationAtom()
        : PVA_FF_Atom(DATA_INFORMATION_ATOM)
{
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DataReferenceAtom, (), _pdataReferenceAtom);

    _pdataReferenceAtom->setParent(this);
    recomputeSize();
}

PVA_FF_DataInformationAtom::~PVA_FF_DataInformationAtom()
{
    PV_MP4_FF_DELETE(NULL, PVA_FF_DataReferenceAtom, _pdataReferenceAtom);
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_DataInformationAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    // Render base PVA_FF_Atom members
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render the dataReferenceAtom
    if (!_pdataReferenceAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += getDataReferenceAtom().getSize();

    return true;
}


void
PVA_FF_DataInformationAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += getDataReferenceAtom().getSize();

    _size = size;

    // Update the size of the parent atom
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}


