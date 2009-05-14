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
#define IMPLEMENT_AVCConfigurationBox

#include "avcsampleentry.h"
#include "atomutils.h"
#include "atomdefs.h"

#define LENGTH_SIZE_MINUS_ONE_MASK 0x03
#define NUM_SEQUENCE_PARAM_SETS_MASK 0x01F

MPEG4BitRateBox::MPEG4BitRateBox(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _mp4ErrorCode = READ_MPEG4_BITRATE_BOX_FAILED;

    if (_success)
    {
        _success = false;
        _pparent = NULL;

        if (!AtomUtils::read32(fp, _bufferSizeDB))
        {
            return;
        }
        if (!AtomUtils::read32(fp, _maxBitRate))
        {
            return;
        }
        if (!AtomUtils::read32(fp, _avgBitRate))
        {
            return;
        }
        _success = true;
        _mp4ErrorCode = EVERYTHING_FINE;

    }
}

