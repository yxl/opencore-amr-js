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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                        MPEG-4 TextSampleEntry Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TextSampleEntry Class is used for text streams.
*/

#define IMPLEMENT_TextSampleEntry

#include "boxrecord.h"
#include "fontrecord.h"
#include "stylerecord.h"
#include "fonttableatom.h"
#include "textsampleentry.h"
#include "atomutils.h"
#include "atomdefs.h"


OSCL_EXPORT_REF TextSampleEntry:: TextSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pparent = NULL;

    _pBackgroundRGBA = NULL;
    _pBoxRecord  	 = NULL;
    _pStyleRecord	 = NULL;
    _pFontTableAtom  = NULL;

    if (_success)
    {

        AtomUtils::read32(fp, _displayFlags);

        uint8 horz = 0;
        AtomUtils::read8(fp, horz);
        _horzJustification = (int8)horz;

        uint8 vert = 0;
        AtomUtils::read8(fp, vert);
        _vertJustification = (int8)vert;

        PV_MP4_FF_ARRAY_NEW(NULL, uint8, (4), _pBackgroundRGBA);

        AtomUtils::readByteData(fp, 4, _pBackgroundRGBA);

        PV_MP4_FF_NEW(fp->auditCB, BoxRecord, (fp), _pBoxRecord);
        if (!_pBoxRecord->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pBoxRecord->GetMP4Error();
            return;
        }

        PV_MP4_FF_NEW(fp->auditCB, StyleRecord, (fp), _pStyleRecord);
        if (!_pStyleRecord->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pStyleRecord->GetMP4Error();
            return;
        }

        PV_MP4_FF_NEW(fp->auditCB, FontTableAtom, (fp), _pFontTableAtom);
        if (!_pFontTableAtom->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pFontTableAtom->GetMP4Error();
            return;
        }
    }
}

// Destructor
OSCL_EXPORT_REF TextSampleEntry::~TextSampleEntry()
{
    if (_pBackgroundRGBA != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pBackgroundRGBA);
    }
    if (_pBoxRecord != NULL)
    {
        PV_MP4_FF_DELETE(NULL, BoxRecord, _pBoxRecord);
    }
    if (_pStyleRecord != NULL)
    {
        PV_MP4_FF_DELETE(NULL, StyleRecord, _pStyleRecord);
    }
    if (_pFontTableAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, FontTableAtom, _pFontTableAtom);
    }
}

OSCL_EXPORT_REF int16 TextSampleEntry::getBoxTop()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxTop());
    }
    else
    {
        return -1;
    }
}

OSCL_EXPORT_REF int16 TextSampleEntry::getBoxLeft()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxLeft());
    }
    else
    {
        return -1;
    }
}

OSCL_EXPORT_REF int16 TextSampleEntry::getBoxBottom()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxBottom());
    }
    else
    {
        return -1;
    }
}

OSCL_EXPORT_REF int16 TextSampleEntry::getBoxRight()
{
    if (_pBoxRecord != NULL)
    {
        return (_pBoxRecord->getBoxRight());
    }
    else
    {
        return -1;
    }
}
OSCL_EXPORT_REF uint16 TextSampleEntry::getFontID()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getFontID();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint16 TextSampleEntry::getStartChar()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getStartChar();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint16 TextSampleEntry::getEndChar()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getEndChar();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8 TextSampleEntry::getFontStyleFlags()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getFontStyleFlags();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8 TextSampleEntry::getfontSize()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord->getfontSize();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF uint8* TextSampleEntry::getTextColourRGBA()
{
    if (_pStyleRecord != NULL)
    {
        return _pStyleRecord-> getTextColourRGBA();
    }
    else
    {
        return NULL;
    }
}

OSCL_EXPORT_REF uint16 TextSampleEntry:: getFontListSize()
{
    if (_pFontTableAtom != NULL)
    {
        return _pFontTableAtom-> getFontListSize();
    }
    else
    {
        return 0;
    }
}

OSCL_EXPORT_REF FontRecord* TextSampleEntry::getFontRecordAt(uint16 index)
{
    if (_pFontTableAtom != NULL)
    {
        return _pFontTableAtom->getFontRecordAt(index);
    }
    else
    {
        return NULL;
    }
}

uint8 TextSampleEntry::getObjectTypeIndication() const
{
    return TIMED_TEXT;
}






