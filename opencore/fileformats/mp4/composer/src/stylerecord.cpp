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
#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include"stylerecord.h"
#include"a_atomdefs.h"

PVA_FF_StyleRecord::PVA_FF_StyleRecord() : PVA_FF_Atom(FourCharConstToUint32('t', 't', 's', 's'))
{
    _startChar = 0;
    _endChar = 0;
    _fontID = 0;
    _fontStyleFlags = 0;
    _fontSize = 0;
    _pRGBA = NULL;
    recomputeSize();
}

PVA_FF_StyleRecord::PVA_FF_StyleRecord(uint16 StartChar, uint16 EndChar, uint16 FontID, uint8 FontSizeFlag, uint8 FontSize, uint8* Trgba)
        : PVA_FF_Atom(FourCharConstToUint32('t', 't', 's', 's'))
{
    _endChar = 0;
    _fontID = 0;
    _fontStyleFlags = 0;
    _fontSize = 0;
    _pRGBA = NULL;
    setStartChar(StartChar);
    setEndChar(EndChar);
    setFontID(FontID);
    setFontStyleFlags(FontSizeFlag);
    setFontSize(FontSize);
    setTextColourRGBA(Trgba);

    recomputeSize();
}
void PVA_FF_StyleRecord::setStartChar(uint16 startchar)
{
    if (_startChar == 0)
    {
        _startChar = startchar;
    }
}

void PVA_FF_StyleRecord::setEndChar(uint16 endchar)
{
    if (_endChar == 0)
    {
        _endChar = endchar;
    }
}

void PVA_FF_StyleRecord::setFontID(uint16 FontID)
{
    if (_fontID == 0)
    {
        _fontID = FontID;
    }
}

void PVA_FF_StyleRecord::setFontStyleFlags(uint8 flag)
{
    if (_fontStyleFlags == 0)
    {
        _fontStyleFlags = flag;
    }
}

void PVA_FF_StyleRecord::setFontSize(uint8 FontSize)
{
    if (_fontSize == 0)
    {
        _fontSize = FontSize;
    }
}

void PVA_FF_StyleRecord::setTextColourRGBA(uint8* RGBA)
{
    if (_pRGBA == NULL)
    {
        _pRGBA = (uint8 *)(OSCL_MALLOC(4 * sizeof(uint8)));
        _pRGBA[0] = RGBA[0];
        _pRGBA[1] = RGBA[1];
        _pRGBA[2] = RGBA[2];
        _pRGBA[3] = RGBA[3];
    }

}


bool PVA_FF_StyleRecord::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!PVA_FF_AtomUtils::render16(fp, _startChar))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render16(fp, _endChar))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render16(fp, _fontID))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render8(fp, _fontStyleFlags))
    {
        return false;
    }
    rendered += 1;
    if (!PVA_FF_AtomUtils::render8(fp, _fontSize))
    {
        return false;
    }
    rendered += 1;
    if (!PVA_FF_AtomUtils::renderByteData(fp, 4, (uint8 *)_pRGBA))
    {
        return false;
    }
    rendered  += 4;


    return true;
}

void PVA_FF_StyleRecord::recomputeSize()
{
    int32 size = 0;

    size += 2;
    size += 2;
    size += 2;
    size += 1;
    size += 1;
    size += 4;

    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

uint32 PVA_FF_StyleRecord::getSize()
{
    recomputeSize();
    return (_size);
}

// Destructor
PVA_FF_StyleRecord::~PVA_FF_StyleRecord()
{
    if (_pRGBA)
    {
        OSCL_FREE(_pRGBA);
        _pRGBA = NULL;
    }
}
