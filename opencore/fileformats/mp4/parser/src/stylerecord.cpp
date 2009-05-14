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
#include "stylerecord.h"
#include "atomutils.h"
#include "isucceedfail.h"

StyleRecord:: StyleRecord(MP4_FF_FILE *fp)
{
    _success = true;
    _mp4ErrorCode = EVERYTHING_FINE;

    PV_MP4_FF_ARRAY_NEW(NULL, uint8, (4), _pRGBA);

    if (!AtomUtils::read16(fp, _startChar))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read16(fp, _endChar))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read16(fp, _fontID))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read8(fp, _fontStyleFlags))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read8(fp, _fontSize))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::readByteData(fp, 4, _pRGBA))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }
}

StyleRecord:: StyleRecord(uint8 *&buf)
{
    _success = true;
    _mp4ErrorCode = EVERYTHING_FINE;

    PV_MP4_FF_ARRAY_NEW(NULL, uint8, (4), _pRGBA);

    if (!AtomUtils::read16(buf, _startChar))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read16(buf, _endChar))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read16(buf, _fontID))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read8(buf, _fontStyleFlags))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::read8(buf, _fontSize))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }

    if (!AtomUtils::readByteData(buf, 4, _pRGBA))
    {
        _success = false;
        _mp4ErrorCode = READ_FAILED;
    }
}
StyleRecord::~StyleRecord()
{
    PV_MP4_ARRAY_DELETE(NULL, _pRGBA);
}
