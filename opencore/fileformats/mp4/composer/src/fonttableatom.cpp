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
#include "fontrecord.h"
#include "fonttableatom.h"
#include"a_atomdefs.h"

#define MAX_ALLOWED_FONT_RECORD_ENTRIES 255

typedef Oscl_Vector<PVA_FF_FontRecord*, OsclMemAllocator> fontRecordVecType;

PVA_FF_FontTableAtom::PVA_FF_FontTableAtom() : PVA_FF_Atom(FourCharConstToUint32('f', 't', 'a', 'b'))
{
    _fontlistsize = 0;
    _pFontRecordArray = NULL;
    PV_MP4_FF_NEW(fp->auditCB, fontRecordVecType, (), _pFontRecordArray);
    recomputeSize();
}

void PVA_FF_FontTableAtom::setFontListSize(uint16 FontListSize)
{
    if (_fontlistsize == 0)
    {
        _fontlistsize = FontListSize;
    }

}

void PVA_FF_FontTableAtom::setFontRecord(uint16 FontListID, uint16 FontId, int8 FontLength, uint8* FontName)
{

    int16 tmp = (int16)_fontlistsize;
    if ((tmp < 0) || (tmp > MAX_ALLOWED_FONT_RECORD_ENTRIES))
    {
        return;
    }
    for (uint32 i = 0; i  <  _fontlistsize;  i++)
    {
        PVA_FF_FontRecord *rec = NULL;
        PV_MP4_FF_NEW(fp->auditCB, PVA_FF_FontRecord, (FontListID, FontId, FontLength, FontName), rec);

        (*_pFontRecordArray).push_back(rec);
    }

}


bool PVA_FF_FontTableAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0;
    uint32 length;

    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render16(fp, _fontlistsize))
    {
        return false;
    }
    rendered += 2;

    for (uint32 i = 0;i < _pFontRecordArray->size();i++)
    {
        length = (*_pFontRecordArray)[i]->getSize();
        if (!((*_pFontRecordArray)[i]->renderToFileStream(fp)))
        {
            return false;
        }

        rendered  += length;
    }


    return true;
}

void PVA_FF_FontTableAtom::recomputeSize()
{
    uint32 length;
    int32 size = getDefaultSize();

    size += 2;
    for (uint32 i = 0;i < _pFontRecordArray->size();i++)
    {
        length = (*_pFontRecordArray)[i]->getSize();
        size += length;
    }


    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}

uint32 PVA_FF_FontTableAtom::getSize()
{
    recomputeSize();
    return (_size);
}


// Destructor
PVA_FF_FontTableAtom::~PVA_FF_FontTableAtom()
{

    if (_pFontRecordArray != NULL)
    {
        for (uint32 i = 0; i < _pFontRecordArray->size(); i++)
        {
            delete((*_pFontRecordArray)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, fontRecordVecType, Oscl_Vector, _pFontRecordArray);
    }

}
