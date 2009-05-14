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
#define IMPLEMENT_TrackExtendsAtom

#include "trackextendsatom.h"
#include "atomutils.h"
#include "atomdefs.h"

TrackExtendsAtom ::TrackExtendsAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    OSCL_UNUSED_ARG(type);

    _trackID = 0;
    _default_sample_description_index = 0;
    _default_sample_duration = 0;
    _default_sample_size = 0;
    _default_sample_flag = 0;

    if (_success)
    {
        if (!AtomUtils::read32(fp, _trackID))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
            return;
        }
        if (!AtomUtils::read32(fp, _default_sample_description_index))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
            return;
        }
        if (!AtomUtils::read32(fp, _default_sample_duration))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
            return;
        }
        if (!AtomUtils::read32(fp, _default_sample_size))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
            return;
        }
        if (!AtomUtils::read32(fp, _default_sample_flag))
        {
            _success = false;
            _mp4ErrorCode = READ_TRACK_EXTENDS_ATOM_FAILED;
            return;
        }
    }
}
// Destructor
TrackExtendsAtom::~TrackExtendsAtom()
{
}
