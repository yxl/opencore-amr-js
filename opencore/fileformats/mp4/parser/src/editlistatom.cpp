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

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<int32, OsclMemAllocator> int32VecType;
typedef Oscl_Vector<uint16, OsclMemAllocator> uint16VecType;

//WMF
EditListAtom::EditListAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    uint32  i = 0, temp = 0;
    uint16 temp_16;

    _psegmentDurations = NULL;
    _pmediaTimes       = NULL;
    _pmediaRates       = NULL;
    _preserveds        = NULL;

    if (_success)
    {

        PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _psegmentDurations);
        PV_MP4_FF_NEW(fp->auditCB, int32VecType, (), _pmediaTimes);
        PV_MP4_FF_NEW(fp->auditCB, uint16VecType, (), _pmediaRates);
        PV_MP4_FF_NEW(fp->auditCB, uint16VecType, (), _preserveds);

        AtomUtils::read32(fp, _entryCount);

        uint32 dataSize = _size - (DEFAULT_FULL_ATOM_SIZE + 4);

        uint32 entrySize = (4 + 4 + 2 + 2);

        if ((_entryCount*entrySize) > dataSize)
        {
            _success = false;
            _mp4ErrorCode = READ_EDITLIST_ATOM_FAILED;
            return;
        }

        for (i = 0; i < _entryCount; i++)
        {//only assume version 0 by now.

            AtomUtils::read32(fp, temp);
            (*_psegmentDurations).push_back(temp);

            AtomUtils::read32(fp, temp);
            (*_pmediaTimes).push_back(temp);

            AtomUtils::read16(fp, temp_16);
            (*_pmediaRates).push_back(temp_16);

            AtomUtils::read16(fp, temp_16); //just for reserve
            (*_preserveds).push_back(temp_16);

        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_EDITLIST_ATOM_FAILED;
    }
}

uint32 EditListAtom::getInitialTimeOffset()
{
    if ((_pmediaTimes == NULL) ||
            (_psegmentDurations == NULL))
    {
        return 0;
    }

    if (_pmediaTimes->size() != 0)
    {
        if ((*_pmediaTimes)[0] == -1)
        {
            return((*_psegmentDurations)[0]);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

// Destructor
EditListAtom::~EditListAtom()
{
    if (_psegmentDurations != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _psegmentDurations);
    }
    if (_pmediaTimes != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, int32VecType, Oscl_Vector, _pmediaTimes);
    }
    if (_pmediaRates != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint16VecType, Oscl_Vector, _pmediaRates);
    }
    if (_preserveds != NULL)
    {
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint16VecType, Oscl_Vector, _preserveds);
    }
}
