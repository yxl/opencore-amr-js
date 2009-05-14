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
    This PVA_FF_DataReferenceAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/


#define IMPLEMENT_DataReferenceAtom

#include "datareferenceatom.h"
#include "dataentryurlatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

typedef Oscl_Vector<PVA_FF_DataEntryAtom*, OsclMemAllocator> PVA_FF_DataEntryAtomVecType;

// Constructor
PVA_FF_DataReferenceAtom::PVA_FF_DataReferenceAtom()
        : PVA_FF_FullAtom(DATA_REFERENCE_ATOM, (uint8)0, (uint32)0)
{
    init();
    recomputeSize();
}

// Destructor
PVA_FF_DataReferenceAtom::~PVA_FF_DataReferenceAtom()
{
    // Clean up vector of DataEntryAtoms
    for (uint32 i = 0; i < _pdataEntryVec->size(); i++)
    {
        PVA_FF_DataEntryUrlAtom *atom = (PVA_FF_DataEntryUrlAtom*)(*_pdataEntryVec)[i];
        PV_MP4_FF_DELETE(NULL, PVA_FF_DataEntryUrlAtom, atom);
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, PVA_FF_DataEntryAtomVecType, Oscl_Vector, _pdataEntryVec);
}

void
PVA_FF_DataReferenceAtom::init()
{
    _entryCount = 0;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DataEntryAtomVecType, (), _pdataEntryVec);

    // Add default URLDataEntry with flag set indicating that the
    // media data is local to the file
    PVA_FF_DataEntryUrlAtom *deua = NULL;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_DataEntryUrlAtom, ((uint32)1), deua);
    addDataEntryAtom(deua);

    recomputeSize();
}

void
PVA_FF_DataReferenceAtom::addDataEntryAtom(PVA_FF_DataEntryAtom *atom)
{
    _pdataEntryVec->push_back(atom);
    atom->setParent(this);
    _entryCount++;
}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_DataReferenceAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, getEntryCount()))
    {
        return false;
    }
    rendered += 4;

    for (uint32 i = 0; i < _pdataEntryVec->size(); i++)
    {
        if (!(*_pdataEntryVec)[i]->renderToFileStream(fp))
        {
            return false;
        }
        rendered += (*_pdataEntryVec)[i]->getSize();
    }

    return true;
}



void
PVA_FF_DataReferenceAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 4; // For entry count

    for (uint32 i = 0; i < _pdataEntryVec->size(); i++)
    {
        size += (*_pdataEntryVec)[i]->getSize();
    }

    _size = size;

    // Update the size of the parent atoms
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
