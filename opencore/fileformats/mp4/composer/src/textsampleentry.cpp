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


#include "textsampleentry.h"
#include "atomutils.h"
#include "a_atomdefs.h"
#include "boxrecord.h"
#include "stylerecord.h"
#include "fonttableatom.h"
#include "textsampledescinfo.h"



// Constructor
PVA_FF_TextSampleEntry::PVA_FF_TextSampleEntry()
        : PVA_FF_SampleEntry(TEXT_SAMPLE_ENTRY)
{

    init();
    recomputeSize();


}

// Destructor
PVA_FF_TextSampleEntry::~PVA_FF_TextSampleEntry()
{
    if (_pBackgroundRGBA)
    {
        OSCL_FREE(_pBackgroundRGBA);
        _pBackgroundRGBA = NULL;
    }
    PV_MP4_FF_DELETE(NULL, PVA_FF_BoxRecord, _pBoxRecord);
    PV_MP4_FF_DELETE(NULL, PVA_FF_StyleRecord, _pStyleRecord);
    PV_MP4_FF_DELETE(NULL, PVA_FF_FontTableAtom, _pFontTableAtom);
}

void PVA_FF_TextSampleEntry::init()
{
    for (int i = 0;i < 6;i++)
    {
        _reserved[i] = 0;
    }
    _dataReferenceIndex = 1;
    _preDefined1 = 0;
    _reserved1 = 0;

    for (int j = 0;j < 3;j++)
    {
        _predefined2[j] = 0;
    }

    _reserved2 = 0;
    _preDefined2 = 0;
    _predefined3 = -1;

    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_BoxRecord, (), _pBoxRecord);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_StyleRecord, (), _pStyleRecord);
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_FontTableAtom, (), _pFontTableAtom);

}


void PVA_FF_TextSampleEntry::addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo)
{
    _displayFlags = pinfo->display_flags;
    _horzJustification = pinfo->hJust;
    _vertJustification = pinfo->vJust;

    _pBackgroundRGBA = (uint8 *)(OSCL_MALLOC(4 * sizeof(uint8)));
    _pBackgroundRGBA[0] = pinfo->bkRgba[0];
    _pBackgroundRGBA[1] = pinfo->bkRgba[1];
    _pBackgroundRGBA[2] = pinfo->bkRgba[2];
    _pBackgroundRGBA[3] = pinfo->bkRgba[3];

    _start_sample_num = pinfo->start_sample_num;
    _end_sample_num = pinfo->end_sample_num;
    _sdIndex = pinfo->sdindex;

    int16 top = pinfo->top;
    _pBoxRecord->setBoxTop(top);

    int16 left = pinfo->left;
    _pBoxRecord->setBoxLeft(left);

    int16 bottom = pinfo->bottom;
    _pBoxRecord->setBoxBottom(bottom);

    int16 right = pinfo->right;
    _pBoxRecord->setBoxRight(right);

    uint16 startchar = pinfo->startChar;
    _pStyleRecord->setStartChar(startchar);

    uint16 endchar = pinfo->endChar;
    _pStyleRecord->setEndChar(endchar);

    uint16 fontid = pinfo->font_id;
    _pStyleRecord->setFontID(fontid);

    uint8 fontsize_flag = pinfo->fontSizeFlags;
    _pStyleRecord->setFontStyleFlags(fontsize_flag);

    uint8 fontsize = pinfo->fontSize;
    _pStyleRecord->setFontSize(fontsize);

    uint8* tRGBA = pinfo->tRgba;
    _pStyleRecord->setTextColourRGBA(tRGBA);

    uint16 fontlistsize = pinfo->fontListSize;
    _pFontTableAtom->setFontListSize(fontlistsize);

    uint16 Fontlistid = pinfo->fontListID;
    uint16 FontID = pinfo->fontID;
    int8 Fontlength = pinfo->font_length;
    uint8* Fontname = pinfo->font_name;
    _pFontTableAtom->setFontRecord(Fontlistid, FontID, Fontlength, Fontname);


}


// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool PVA_FF_TextSampleEntry::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{

    int32 rendered = 0;
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    // From PVA_FF_SampleEntry base class
    for (int k = 0; k < 6; k++)
    {
        if (!PVA_FF_AtomUtils::render8(fp, _reserved[k]))
        {
            return false;
        }
        rendered += 1;
    }


    if (!PVA_FF_AtomUtils::render16(fp, _dataReferenceIndex))
    {
        return false;
    }
    rendered  += 2;
    if (!PVA_FF_AtomUtils::render32(fp, _displayFlags))
    {
        return false;
    }
    rendered  += 4;
    if (!PVA_FF_AtomUtils::render8(fp, _horzJustification))
    {
        return false;
    }
    rendered  += 1;
    if (!PVA_FF_AtomUtils::render8(fp, _vertJustification))
    {
        return false;
    }
    rendered  += 1;
    if (!PVA_FF_AtomUtils::renderByteData(fp, 4, (uint8 *)_pBackgroundRGBA))
    {
        return false;
    }
    rendered  += 4;

    if (!_pBoxRecord->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pBoxRecord->getSize();

    if (!_pStyleRecord->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pStyleRecord->getSize();

    if (!_pFontTableAtom->renderToFileStream(fp))
    {
        return false;
    }
    rendered += _pFontTableAtom->getSize();
    return true;
}

void PVA_FF_TextSampleEntry::recomputeSize()
{
    int32 size = getDefaultSize();

    size += 8; // For reserved and dataRefIndex of PVA_FF_SampleEntry base class

    size += 4;
    size += 1;
    size += 1;
    size += 4;
    size += _pBoxRecord->getSize();
    size += _pStyleRecord->getSize();
    size += _pFontTableAtom->getSize();
    _size = size;

    // Update size of parent
    if (_pparent != NULL)
    {
        _pparent->recomputeSize();
    }
}
