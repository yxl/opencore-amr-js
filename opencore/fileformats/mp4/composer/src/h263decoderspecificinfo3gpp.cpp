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

#define __IMPLEMENT_H263DecoderSpecificInfo3GPP__

#include "h263decoderspecificinfo3gpp.h"

#include "atomutils.h"

PVA_FF_H263SpecficAtom::PVA_FF_H263SpecficAtom()
        : PVA_FF_Atom(FourCharConstToUint32('d', '2', '6', '3'))
{
    _VendorCode = PACKETVIDEO_FOURCC;
    _encoder_version = 1;
    _codec_level = 10;
    _codec_profile = 0;
    _max_width = 176;
    _max_height = 144;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_H263DecBitrateAtom, (), _ph263_decbitrateatom);

    recomputeSize();
}

bool
PVA_FF_H263SpecficAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
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

    if (!PVA_FF_AtomUtils::render8(fp, _codec_level))
    {
        return false;
    }
    rendered += 1;

    if (!PVA_FF_AtomUtils::render8(fp, _codec_profile))
    {
        return false;
    }
    rendered += 1;

    if (!_ph263_decbitrateatom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _ph263_decbitrateatom->getSize();


    return true;
}

void
PVA_FF_H263SpecficAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 7; // FOR DECODER SPECIFIC STRUCT
    size += _ph263_decbitrateatom->getSize();
    _size = size;

    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}




PVA_FF_H263DecBitrateAtom::PVA_FF_H263DecBitrateAtom()
        : PVA_FF_Atom(FourCharConstToUint32('b', 'i', 't', 'r'))
{
    _avg_bitrate = 8000;
    _max_bitrate = 8000;
    recomputeSize();
}

bool
PVA_FF_H263DecBitrateAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // Render decoder bitrate info payload
    if (!PVA_FF_AtomUtils::render32(fp, _avg_bitrate))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _max_bitrate))
    {
        return false;
    }
    rendered += 4;

    return true;
}
void
PVA_FF_H263DecBitrateAtom::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 8;

    _size = size;

    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
