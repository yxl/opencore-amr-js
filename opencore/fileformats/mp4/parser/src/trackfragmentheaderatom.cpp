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
#define IMPLEMENT_TrackFragmentHeaderAtom

#include "trackfragmentheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

TrackFragmentHeaderAtom ::TrackFragmentHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    OSCL_UNUSED_ARG(type);

    _trackID = 0;
    _base_data_offset = 0;
    _sample_description_index = 0;
    _default_sample_duration = 0;
    _default_sample_size = 0;
    _default_sample_flag = 0;
    tf_flag = getFlags();

    if (_success)
    {
        if (!AtomUtils::read32(fp, _trackID))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED;
            return;
        }

        if (tf_flag & 0x00001)
        {
            if (!AtomUtils::read64(fp, _base_data_offset))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED;
                return;
            }
        }
        if (tf_flag & 0x000002)
        {
            if (!AtomUtils::read32(fp, _sample_description_index))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED;
                return;
            }
        }

        if (tf_flag & 0x000008)
        {
            if (!AtomUtils::read32(fp, _default_sample_duration))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED;
                return;
            }
        }

        if (tf_flag & 0x000010)
        {
            if (!AtomUtils::read32(fp, _default_sample_size))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED;
                return;
            }
        }
        if (tf_flag & 0x000020)
        {
            if (!AtomUtils::read32(fp, _default_sample_flag))
            {
                _success = false;
                _mp4ErrorCode = READ_TRACK_FRAGMENT_HEADER_ATOM_FAILED;
                return;
            }
        }
        ///tf_flag & 0x010000 duration is empty
    }
}
// Destructor
TrackFragmentHeaderAtom::~TrackFragmentHeaderAtom()
{
}
