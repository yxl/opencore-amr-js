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
#define IMPLEMENT_AVCSampleDependency

#include "avcsampledependency.h"
#include "atomutils.h"
#include "atomdefs.h"

AVCSampleDependency::AVCSampleDependency(MP4_FF_FILE *fp, uint32 size, uint32 type, uint32 sample_count)
        : FullAtom(fp, size, type)
{
    if (_success)
    {

        PV_MP4_FF_ARRAY_NEW(NULL, uint16, (sample_count), _dependencycount);
        if (_dependencycount == NULL)
        {
            _success = false;
            _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
            return;
        }

        PV_MP4_FF_ARRAY_NEW(NULL, uint16, (sample_count), _relativesamplenumber);
        if (_relativesamplenumber == NULL)
        {
            _success = false;
            _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
            return;
        }

        for (uint32 idx = 0; idx < sample_count;idx++)
        {
            if (!AtomUtils::read16(fp, _dependencycount[idx]))
            {
                _success = false;
            }
            for (uint32 i = 0; i < _dependencycount[idx]; i++)
            {
                if (!AtomUtils::read16(fp, _relativesamplenumber[i]))
                {
                    _success = false;
                }
            }
        }
    }
    if (!_success)
    {
        _mp4ErrorCode = READ_AVC_SAMPLE_DEPENDENCY_BOX_FAILED;
    }
}
// Destructor
AVCSampleDependency::~AVCSampleDependency()
{
}
