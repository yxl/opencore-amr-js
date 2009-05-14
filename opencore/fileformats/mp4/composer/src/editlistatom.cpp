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

#include "editlistatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"
#include "fullatom.h"

//PVA_FF_EditAtom()

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<int32, OsclMemAllocator> int32VecType;
typedef Oscl_Vector<uint16, OsclMemAllocator> uint16VecType;
// Constructor
PVA_FF_EditListAtom::PVA_FF_EditListAtom()
        : PVA_FF_FullAtom(FourCharConstToUint32('e', 'l', 's', 't'), 0, 0)//currently I sent both version and flag to 0
{
    _entryCount = 0;

    _size = getDefaultSize();
    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psegmentDurations);
    PV_MP4_FF_NEW(fp->auditCB, int32VecType, (), _pmediaTimes);
    PV_MP4_FF_NEW(fp->auditCB, uint16VecType, (), _pmediaRates);

    recomputeSize();
}

void PVA_FF_EditListAtom::addEditEntry(uint32 duration, int32 time, uint16 rate)
{
    _psegmentDurations->push_back(duration);
    _pmediaTimes->push_back(time);
    _pmediaRates->push_back(rate);
    _entryCount++;
    recomputeSize();
}

bool
PVA_FF_EditListAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;
    uint32 i = 0; // Keep track of number of bytes rendered

    recomputeSize();

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _entryCount))
    {
        return false;
    }
    rendered += 4;

    for (i = 0; i < _entryCount; i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, (*_psegmentDurations)[i]))
        {
            return false;
        }
        rendered += 4;

        if (!PVA_FF_AtomUtils::render32(fp, (*_pmediaTimes)[i]))
        {
            return false;
        }
        rendered += 4;

        if (!PVA_FF_AtomUtils::render16(fp, (*_pmediaRates)[i]))
        {
            return false;
        }
        rendered += 2;

        if (!PVA_FF_AtomUtils::render16(fp, 0))
        {
            return false;
        }
        rendered += 2;
    }

    return true;
}

void PVA_FF_EditListAtom::recomputeSize()
{
    _size = getDefaultSize();
    _size += (_entryCount * 12) + 4; // 4 for _entryCount

    // Update the size of the parent atom since this child atom may have changed
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

PVA_FF_EditListAtom::~PVA_FF_EditListAtom()
{
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psegmentDurations);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, int32VecType, Oscl_Vector, _pmediaTimes);
    PV_MP4_FF_TEMPLATED_DELETE(NULL, uint16VecType, Oscl_Vector, _pmediaRates);
}
