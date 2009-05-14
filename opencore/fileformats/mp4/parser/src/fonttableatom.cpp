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
#include "fonttableatom.h"
#include "atomutils.h"

#define MAX_ALLOWED_FONT_RECORD_ENTRIES 255

typedef Oscl_Vector<FontRecord*, OsclMemAllocator> fontRecordVecType;

FontTableAtom::FontTableAtom(MP4_FF_FILE *fp)
        : Atom(fp)
{
    _pFontRecordArray = NULL;

    if (_success)
    {
        AtomUtils::read16(fp, _entryCount);

        int16 tmp = (int16)_entryCount;

        if ((tmp < 0) || (tmp > MAX_ALLOWED_FONT_RECORD_ENTRIES))
        {
            _success = false;
            _mp4ErrorCode = READ_FONT_TABLE_ATOM_FAILED;
            return;
        }

        PV_MP4_FF_NEW(fp->auditCB, fontRecordVecType, (), _pFontRecordArray);

        for (uint32 i = 0; i  <  _entryCount;  i++)
        {
            FontRecord *rec = NULL;
            PV_MP4_FF_NEW(fp->auditCB, FontRecord, (fp), rec);

            if (!rec-> GetMP4Success())
            {
                PV_MP4_FF_DELETE(NULL, FontRecord, rec);
                _success = false;
                _mp4ErrorCode = READ_FONT_TABLE_ATOM_FAILED;
                return;
            }

            (*_pFontRecordArray).push_back(rec);
        }
    }
    else
    {
        _mp4ErrorCode = READ_FONT_TABLE_ATOM_FAILED;
    }
}

FontRecord *
FontTableAtom::getFontRecordAt(uint16 index)
{
    if ((index) > _pFontRecordArray->size())
    {
        return NULL;
    }

    if (index < _entryCount)
    {
        return (FontRecord *)(*_pFontRecordArray)[(int32)index];
    }
    else
    {
        return NULL;
    }
}


// Destructor
FontTableAtom::~FontTableAtom()
{
    if (_pFontRecordArray != NULL)
    {
        for (uint32 i = 0; i < _entryCount; i++)
        {
            PV_MP4_FF_DELETE(NULL, FontRecord, (*_pFontRecordArray)[i]);
        }

        PV_MP4_FF_TEMPLATED_DELETE(NULL, fontRecordVecType, Oscl_Vector, _pFontRecordArray);
    }
}

