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
#include "editatom.h"
#include "editlistatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

// Constructor
PVA_FF_EditAtom::PVA_FF_EditAtom()
        : PVA_FF_Atom(FourCharConstToUint32('e', 'd', 't', 's'))
{
    _size = getDefaultSize();

    _peditList = NULL;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_EditListAtom, (), _peditList);

    _peditList->setParent(this);
}

bool
PVA_FF_EditAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    recomputeSize();

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render the data of the members once they get implemented
    if (!_peditList->renderToFileStream(fp))
    {
        return false;
    }

    return true;
}


void PVA_FF_EditAtom::addEditEntry(uint32 duration, int32 time, uint16 rate)
{
    _peditList->addEditEntry(duration, time, rate);
    recomputeSize();
}

void PVA_FF_EditAtom::addEmptyEdit(uint32 duration)
{
    _peditList->addEditEntry(duration, -1, 1);
    recomputeSize();
}


void PVA_FF_EditAtom::recomputeSize()
{
    _size = getDefaultSize();
    _size += _peditList->getSize();

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

PVA_FF_EditAtom::~PVA_FF_EditAtom()
{
    if (_peditList != NULL)
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_EditListAtom, _peditList);
    }
}

