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
#include"fontrecord.h"
#include"a_atomdefs.h"



PVA_FF_FontRecord::PVA_FF_FontRecord(uint16 FontListID, uint16 FontID, int8 FontLength, uint8* FontName)
        : PVA_FF_Atom(FourCharConstToUint32('t', 't', 's', 'f'))
{
    OSCL_UNUSED_ARG(FontListID);
    _fontID = 0;
    _fontLength = 0;
    _pFontName = NULL;

    if (_fontID == 0)
    {
        _fontID = FontID;
    }
    if (_fontLength == 0)
    {
        _fontLength = FontLength;
    }

    if (_fontLength >  0)
    {
        _pFontName = (uint8 *)(OSCL_MALLOC(_fontLength * sizeof(char)));

        for (uint32 i = 0; i < (uint32)_fontLength; i++)
        {
            _pFontName[i] = FontName[i];
        }
    }

    recomputeSize();

}


bool PVA_FF_FontRecord::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;

    if (!PVA_FF_AtomUtils::render16(fp, _fontID))
    {
        return false;
    }
    rendered += 2;
    if (!PVA_FF_AtomUtils::render8(fp, _fontLength))
    {
        return false;
    }
    rendered += 1;
    if (!PVA_FF_AtomUtils::renderByteData(fp, _fontLength, _pFontName))
    {
        return false;
    }
    rendered  += _fontLength;


    return true;
}

void PVA_FF_FontRecord::recomputeSize()
{
    int32 size = 0;

    size += 2;
    size += 1;
    size += _fontLength;

    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

uint32 PVA_FF_FontRecord::getSize()
{
    recomputeSize();
    return (_size);
}

// Destructor
PVA_FF_FontRecord::~PVA_FF_FontRecord()
{
    if (_pFontName)
    {
        OSCL_FREE(_pFontName);
        _pFontName = NULL;
    }
}
