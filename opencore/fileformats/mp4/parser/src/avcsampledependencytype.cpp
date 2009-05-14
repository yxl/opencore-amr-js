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

#include "avcsampledependencytype.h"
#include "atomutils.h"
#include "atomdefs.h"

AVCSampleDependencyType::AVCSampleDependencyType(MP4_FF_FILE *fp, uint32 size, uint32 type, uint32 sample_count)
        : FullAtom(size, _version, flags)
{
    OSCL_UNUSED_ARG(type);
    if (_success)
    {
        if (_version == 0)
        {

            PV_MP4_FF_ARRAY_NEW(NULL, uint8, (sample_count), _reserved);

            if (_reserved == NULL)
            {
                _success = false;
                _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                return;
            }

            PV_MP4_FF_ARRAY_NEW(NULL, uint8, (sample_count), _sample_depends_on);

            if (_sample_depends_on == NULL)
            {
                _success = false;
                _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                return;
            }

            PV_MP4_FF_ARRAY_NEW(NULL, uint8, (sample_count), _sample_is_depended_on);

            if (_sample_is_depended_on == NULL)
            {
                _success = false;
                _mp4ErrorCode = MEMORY_ALLOCATION_FAILED;
                return;
            }

            for (uint32 idx = 0; idx < sample_count;idx++)
            {
                if (!AtomUtils::read8(fp, _reserved[idx]))
                {
                    _success = false;
                }

                _sample_depends_on[idx] = (uint8)(_reserved[idx] & ~0xF3);
                _sample_is_depended_on[idx] = (uint8)(_reserved[idx] & ~0xFC);
            }
        }
    }
    if (!_success)
    {
        _mp4ErrorCode = READ_AVC_SAMPLE_DEPENDENCY_TYPE_BOX_FAILED;
    }
}
// Destructor
AVCSampleDependencyType::~AVCSampleDependencyType()
{
    if (_reserved != NULL)
        PV_MP4_ARRAY_DELETE(NULL, _reserved);

    if (_sample_depends_on != NULL)
        PV_MP4_ARRAY_DELETE(NULL, _sample_depends_on);

    if (_sample_is_depended_on != NULL)
        PV_MP4_ARRAY_DELETE(NULL, _sample_is_depended_on);
}
