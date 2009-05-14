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
    This PVA_FF_DecoderSpecificInfo Class that holds the Mpeg4 VOL header for the
	video stream
*/

#define __IMPLEMENT_AMRDecoderSpecificInfo3GPP__

#include "amrdecoderspecificinfo3gpp.h"

#include "atom.h"
#include "atomutils.h"

// Default constructor
PVA_FF_AMRSpecificAtom::PVA_FF_AMRSpecificAtom()
        : PVA_FF_Atom(FourCharConstToUint32('d', 'a', 'm', 'r'))
{
    _mode_set = 0;

    _VendorCode = PACKETVIDEO_FOURCC;

    _mode_change_period = 0;
    _frames_per_sample = 1;

    recomputeSize();
}

bool
PVA_FF_AMRSpecificAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    renderAtomBaseMembers(fp);
    rendered += getDefaultSize();

    // Render decoder specific info payload
    if (!PVA_FF_AtomUtils::render32(fp, _VendorCode))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render8(fp, _encoder_version))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render16(fp, _mode_set))
    {
        return false;
    }
    rendered += 2;

    if (!PVA_FF_AtomUtils::render8(fp, _mode_change_period))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render8(fp, _frames_per_sample))
    {
        return false;
    }
    rendered += 1;

    return true;
}

void
PVA_FF_AMRSpecificAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 9; // FOR DECODER SPECIFIC STRUCT

    _size = size;

    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

