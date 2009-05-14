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
#include "fontrecord.h"
#include "atomutils.h"
#include "isucceedfail.h"

FontRecord::FontRecord(MP4_FF_FILE *fp)
{
    _success = true;
    uint8 data;

    AtomUtils::read16(fp, _fontID);
    AtomUtils::read8(fp, data);

    _fontLength = (int8)data;

    _pFontName = NULL;

    if (_fontLength >  0)
    {
        _pFontName = (uint8 *)(oscl_malloc(_fontLength * sizeof(char)));

        if (_pFontName == NULL)
        {
            _success = false;
            _mp4ErrorCode = READ_FONT_RECORD_FAILED;
            return;
        }

        if (!AtomUtils::readByteData(fp, _fontLength,
                                     _pFontName))
        {
            _success = false;
            _mp4ErrorCode = READ_FONT_RECORD_FAILED;
        }
    }

}

FontRecord::~FontRecord()
{
    if (_pFontName != NULL)
    {
        oscl_free(_pFontName);
    }
}

