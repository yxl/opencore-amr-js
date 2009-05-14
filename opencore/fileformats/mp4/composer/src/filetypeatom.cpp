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
    This PVA_FF_MovieAtom Class is the main atom class in the MPEG-4 File that stores
    all the meta data about the MPEG-4 presentation.
*/


#define IMPLEMENT_WMFSetSessionAtom

#include "filetypeatom.h"
#include "atomutils.h"
#include "a_atomdefs.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

PVA_FF_FileTypeAtom::PVA_FF_FileTypeAtom()
        :	PVA_FF_Atom(FILE_TYPE_ATOM)
{
    _pCompatibleBrandVec = NULL;

    _size = getDefaultSize() + 8;

    PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pCompatibleBrandVec);
}


bool
PVA_FF_FileTypeAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, _Brand))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _Version))
    {
        return false;
    }
    rendered += 4;

    for (uint32 i = 0; i < _pCompatibleBrandVec->size(); i++)
    {
        if (!PVA_FF_AtomUtils::render32(fp, (*_pCompatibleBrandVec)[i]))
        {
            return false;
        }
    }

    return true;
}


void
PVA_FF_FileTypeAtom::addCompatibleBrand(uint32 brand)
{
    if (_pCompatibleBrandVec != NULL)
    {
        _pCompatibleBrandVec->push_back(brand);
        recomputeSize();
    }
}

void
PVA_FF_FileTypeAtom::recomputeSize()
{
    _size = getDefaultSize() + 8;

    for (uint32 i = 0; i < _pCompatibleBrandVec->size(); i++)
    {
        _size += 4;
    }
}
