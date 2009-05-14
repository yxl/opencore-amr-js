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
#define IMPLEMENT_AVCSampleToGroup

#include "avcsampletogroupbox.h"
#include "atomutils.h"
#include "atomdefs.h"

AVCSampleToGroup::AVCSampleToGroup(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, _size, type)
{
    OSCL_UNUSED_ARG(size);
    uint32 count = _size - DEFAULT_ATOM_SIZE;
    if (_success)
    {

        if (!AtomUtils::read32(fp, _grouping_type))
        {
            _success = false;
        }
        count -= 4;

        if (!AtomUtils::read32(fp, _entry_count))
        {
            _success = false;
        }
        count -= 4;

        PV_MP4_FF_ARRAY_NEW(NULL, uint32, (_entry_count), _samplecountVec);
        PV_MP4_FF_ARRAY_NEW(NULL, uint32, (_entry_count), _group_description_indexVec);

        uint32 samplecount = 0;
        uint32 GDIndex = 0;

        for (uint32 idx = 0; idx < _entry_count; idx++)
        {
            if (!AtomUtils::read32(fp, samplecount))
            {
                _success = false;
            }
            _samplecountVec[idx] = samplecount;

            if (!AtomUtils::read32(fp, GDIndex))
            {
                _success = false;
            }
            _group_description_indexVec[ idx ] = GDIndex;
        }
    }
    if (!_success)
    {
        _mp4ErrorCode = READ_AVC_SAMPLE_TO_GROUP_BOX_FAILED;
    }
}

AVCSampleToGroup::~AVCSampleToGroup()
{
    if (_samplecountVec != NULL)
        PV_MP4_ARRAY_DELETE(NULL, _samplecountVec);

    if (_group_description_indexVec != NULL)
        PV_MP4_ARRAY_DELETE(NULL, _group_description_indexVec);
}
