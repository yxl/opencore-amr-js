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
#define IMPLEMENT_TextSampleModifiers

#include "textsamplemodifiers.h"
#include "atomutils.h"
#include "atomdefs.h"

typedef Oscl_Vector<TimedTextModifiers*, OsclMemAllocator> timedTextModifiersVecType;
typedef Oscl_Vector<StyleRecord*, OsclMemAllocator> styleRecordVecType;

TextStyleBox:: TextStyleBox(uint8  *buf) : Atom(buf)
{
    _pparent = NULL;

    _pStyleRecordVec = NULL;

    if (_success)
    {

        PV_MP4_FF_NEW(fp->auditCB, styleRecordVecType, (), _pStyleRecordVec);

        if (!AtomUtils::read16(buf, _entryCount))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        for (uint16 i = 0; i < _entryCount; i++)
        {
            StyleRecord *pStyleRecord	= NULL;
            PV_MP4_FF_NEW(fp->auditCB, StyleRecord, (buf), pStyleRecord);
            if (!pStyleRecord->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = pStyleRecord->GetMP4Error();
                return;
            }

            _pStyleRecordVec->push_back(pStyleRecord);
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

// Destructor
TextStyleBox::~TextStyleBox()
{
    if (_pStyleRecordVec != NULL)
    {
        for (uint32 i = 0; i < _pStyleRecordVec->size(); i++)
        {
            PV_MP4_FF_DELETE(NULL, StyleRecord, (*_pStyleRecordVec)[i]);
        }
        PV_MP4_FF_TEMPLATED_DELETE(NULL, styleRecordVecType, Oscl_Vector, _pStyleRecordVec);
    }
}

TextHighlightBox::TextHighlightBox(uint8  *buf) : Atom(buf)
{
    _startCharOffset = 0;
    _endCharOffset   = 0;

    if (_success)
    {
        if (!AtomUtils::read16(buf, _startCharOffset))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        if (!AtomUtils::read16(buf, _endCharOffset))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

TextHilightColorBox::TextHilightColorBox(uint8  *buf) : Atom(buf)
{
    _pHighlightColorRGBA = NULL;

    if (_success)
    {
        PV_MP4_FF_ARRAY_NEW(NULL, uint8, (4), _pHighlightColorRGBA);

        if (!AtomUtils::readByteData(buf, 4, _pHighlightColorRGBA))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

TextHilightColorBox::~TextHilightColorBox()
{
    if (_pHighlightColorRGBA != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pHighlightColorRGBA);
    }
}

TextKaraokeBox::TextKaraokeBox(uint8  *buf) : Atom(buf)
{
    _pHighLightEndTimeVec = NULL;
    _pStartCharOffsetVec  = NULL;
    _pEndCharOffsetVec	  = NULL;

    if (_success)
    {
        if (!AtomUtils::read32(buf, _highLightStartTime))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        if (!AtomUtils::read16(buf, _entryCount))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        uint32 endTime;
        uint16 startOffset;
        uint16 endOffset;

        if (_entryCount > 0)
        {

            PV_MP4_FF_ARRAY_NEW(NULL, uint32, (_entryCount), _pHighLightEndTimeVec);
            PV_MP4_FF_ARRAY_NEW(NULL, uint16, (_entryCount), _pStartCharOffsetVec);
            PV_MP4_FF_ARRAY_NEW(NULL, uint16, (_entryCount), _pEndCharOffsetVec);

            for (uint16 i = 0; i < _entryCount; i++)
            {
                if (!AtomUtils::read32(buf, endTime))
                {
                    _success = false;
                    _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
                    return;
                }

                if (!AtomUtils::read16(buf, startOffset))
                {
                    _success = false;
                    _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
                    return;
                }

                if (!AtomUtils::read16(buf, endOffset))
                {
                    _success = false;
                    _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
                    return;
                }

                _pHighLightEndTimeVec[i] = endTime;
                _pStartCharOffsetVec[i]  = startOffset;
                _pEndCharOffsetVec[i]  = endOffset;

            }
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

TextKaraokeBox::~TextKaraokeBox()
{
    if (_pHighLightEndTimeVec != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pHighLightEndTimeVec);
    }

    if (_pStartCharOffsetVec != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pStartCharOffsetVec);
    }

    if (_pEndCharOffsetVec != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pEndCharOffsetVec);
    }

}

TextScrollDelay::TextScrollDelay(uint8  *buf) : Atom(buf)
{
    if (_success)
    {
        if (!AtomUtils::read32(buf, _scrollDelay))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

TextHyperTextBox::TextHyperTextBox(uint8  *buf) : Atom(buf)
{
    _startCharOffset = 0;
    _endCharOffset = 0;
    _urlLength = 0;
    _pURL = NULL;
    _altLength = 0;
    _pAltString = NULL;

    if (_success)
    {
        if (!AtomUtils::read16(buf, _startCharOffset))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        if (!AtomUtils::read16(buf, _endCharOffset))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        if (!AtomUtils::read8(buf, _urlLength))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        PV_MP4_FF_ARRAY_NEW(NULL, uint8, (_urlLength), _pURL);


        if (!AtomUtils::readByteData(buf, _urlLength, _pURL))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        if (!AtomUtils::read8(buf, _altLength))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        PV_MP4_FF_ARRAY_NEW(NULL, uint8, (_altLength), _pAltString);

        if (!AtomUtils::readByteData(buf, _altLength, _pAltString))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

TextHyperTextBox::~TextHyperTextBox()
{
    if (_pURL != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pURL);
    }

    if (_pAltString != NULL)
    {
        PV_MP4_ARRAY_DELETE(NULL, _pAltString);
    }
}

TextBoxBox::TextBoxBox(uint8  *buf) : Atom(buf)
{
    _pBoxRecord = NULL;

    if (_success)
    {
        PV_MP4_FF_NEW(fp->auditCB, BoxRecord, (buf), _pBoxRecord);

        if (!_pBoxRecord->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pBoxRecord->GetMP4Error();
            return;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}




TextBoxBox::~TextBoxBox()
{
    if (_pBoxRecord != NULL)
    {
        PV_MP4_FF_DELETE(NULL, BoxRecord, _pBoxRecord);
    }
}

BlinkBox::BlinkBox(uint8  *buf) : Atom(buf)
{
    if (_success)
    {
        if (!AtomUtils::read16(buf, _startCharOffset))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }

        if (!AtomUtils::read16(buf, _endCharOffset))
        {
            _success = false;
            _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
            return;
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}
#define SUPPRESS__NOT_YET_IMPLEMENTED

TextSampleModifiers*
TextSampleModifiers::parseTextModifiers(uint8* buf, uint32 size)
{
    TextSampleModifiers *pTextSampleModifiers = NULL;
#ifndef SUPPRESS__NOT_YET_IMPLEMENTED
    PV_MP4_FF_NEW(fp->auditCB, TextSampleModifiers, (buf, size), pTextSampleModifiers);

    if (!pTextSampleModifiers->MP4Success())
    {
        return NULL;
    }
#else
    OSCL_UNUSED_ARG(buf);
    OSCL_UNUSED_ARG(size);
#endif
    return (pTextSampleModifiers);
}

#ifndef SUPPRESS__NOT_YET_IMPLEMENTED

TextSampleModifiers:: TextSampleModifiers(uint8  *buf, uint32 size)
{
    TimedTextModifiers *timedTextModifier = NULL;

    timedTextModifier->value->_pBlinkBox			= NULL;
    timedTextModifier->value->_pTextBoxBox			= NULL;
    timedTextModifier->value->_pTextHighlightBox	= NULL;
    timedTextModifier->value->_pTextHilightColorBox	= NULL;
    timedTextModifier->value->_pTextHyperTextBox	= NULL;
    timedTextModifier->value->_pTextKaraokeBox		= NULL;
    timedTextModifier->value->_pTextScrollDelay		= NULL;
    timedTextModifier->value->_pTextStyleBox		= NULL;

    _success = true;

    PV_MP4_FF_NEW(fp->auditCB, timedTextModifiersVecType, (), _pTimedTextModifiersVec);

    if (_success)
    {
        uint32 count    = 0;
        uint32 atomType = AtomUtils::getNextAtomType(buf);

        while (((atomType == TEXT_STYLE_BOX) ||
                (atomType == TEXT_HIGHLIGHT_BOX) ||
                (atomType == TEXT_HILIGHT_COLOR_BOX) ||
                (atomType == TEXT_KARAOKE_BOX) ||
                (atomType == TEXT_SCROLL_DELAY_BOX) ||
                (atomType == TEXT_HYPER_TEXT_BOX) ||
                (atomType == TEXT_OVER_RIDE_BOX) ||
                (atomType == TEXT_BLINK_BOX)) &&
                (count < size))
        {
            if (atomType == TEXT_STYLE_BOX)
            {
                TextStyleBox *pStyleBox = NULL;
                PV_MP4_FF_NEW(fp->auditCB, TextStyleBox, (buf), pStyleBox);

                if (!pStyleBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = pStyleBox->GetMP4Error();
                    PV_MP4_FF_DELETE(NULL, TextStyleBox, pStyleBox);
                    pStyleBox = NULL;
                    return;
                }

                timedTextModifier->value->_pTextStyleBox = pStyleBox;
                timedTextModifier->type = TEXT_STYLE;

                _pTimedTextModifiersVec->push_back(timedTextModifier);

                count += pStyleBox->getSize();
                buf   += pStyleBox->getSize();
            }
            else if (atomType == TEXT_HIGHLIGHT_BOX)
            {
                TextHighlightBox *pHighlightBox = NULL;
                PV_MP4_FF_NEW(fp->auditCB, TextHighlightBox, (buf), pHighlightBox);

                if (!pHighlightBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = pHighlightBox->GetMP4Error();
                    PV_MP4_FF_DELETE(NULL, TextHighlightBox, pHighlightBox);
                    pHighlightBox = NULL;
                    return;
                }

                timedTextModifier->value->_pTextHighlightBox = pHighlightBox;
                timedTextModifier->type = TEXT_HIGHLIGHT;
                _pTimedTextModifiersVec->push_back(timedTextModifier);

                count += pHighlightBox->getSize();
                buf   += pHighlightBox->getSize();
            }
            else if (atomType == TEXT_HILIGHT_COLOR_BOX)
            {
                if (timedTextModifier->value->_pTextHilightColorBox == NULL)
                {
                    TextHilightColorBox *textHilightColorBox = NULL;
                    PV_MP4_FF_NEW(fp->auditCB, TextHilightColorBox, (buf), textHilightColorBox);

                    if (!textHilightColorBox->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = textHilightColorBox->GetMP4Error();
                        PV_MP4_FF_DELETE(NULL, TextHilightColorBox, textHilightColorBox);
                        textHilightColorBox = NULL;
                        return;
                    }

                    timedTextModifier->value->_pTextHilightColorBox = textHilightColorBox;
                    timedTextModifier->type = TEXT_WRAP;
                    _pTimedTextModifiersVec->push_back(timedTextModifier);

                    count += textHilightColorBox->getSize();
                    buf   += textHilightColorBox->getSize();
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
                    return;
                }
            }
            else if (atomType == TEXT_KARAOKE_BOX)
            {
                TextKaraokeBox *pKaraokeBox = NULL;
                PV_MP4_FF_NEW(fp->auditCB, TextKaraokeBox, (buf), pKaraokeBox);

                if (!pKaraokeBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = pKaraokeBox->GetMP4Error();
                    PV_MP4_FF_DELETE(NULL, TextKaraokeBox, pKaraokeBox);
                    pKaraokeBox = NULL;
                    return;
                }

                timedTextModifier->value->_pTextKaraokeBox = pKaraokeBox;
                timedTextModifier->type = TEXT_KARAOKE;
                _pTimedTextModifiersVec->push_back(timedTextModifier);

                count += pKaraokeBox->getSize();
                buf   += pKaraokeBox->getSize();
            }
            else if (atomType == TEXT_SCROLL_DELAY_BOX)
            {
                if (timedTextModifier->value->_pTextScrollDelay == NULL)
                {
                    TextScrollDelay *textScrollDelay = NULL;
                    PV_MP4_FF_NEW(fp->auditCB, TextScrollDelay, (buf), textScrollDelay);

                    if (!textScrollDelay->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = textScrollDelay->GetMP4Error();
                        PV_MP4_FF_DELETE(NULL, TextScrollDelay, textScrollDelay);
                        textScrollDelay = NULL;
                        return;
                    }
                    timedTextModifier->value->_pTextScrollDelay = textScrollDelay;
                    timedTextModifier->type = TEXT_SCROLL_DELAY;
                    _pTimedTextModifiersVec->push_back(timedTextModifier);

                    count += textScrollDelay->getSize();
                    buf   += textScrollDelay->getSize();
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
                    return;
                }
            }
            else if (atomType == TEXT_HYPER_TEXT_BOX)
            {
                TextHyperTextBox *pHyperTextBox = NULL;
                PV_MP4_FF_NEW(fp->auditCB, TextHyperTextBox, (buf), pHyperTextBox);

                if (!pHyperTextBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = pHyperTextBox->GetMP4Error();
                    PV_MP4_FF_DELETE(NULL, TextHyperTextBox, pHyperTextBox);
                    pHyperTextBox = NULL;
                    return;
                }
                timedTextModifier->value->_pTextHyperTextBox = pHyperTextBox;
                timedTextModifier->type = TEXT_HYPERTEXT;
                _pTimedTextModifiersVec->push_back(timedTextModifier);

                count += pHyperTextBox->getSize();
                buf   += pHyperTextBox->getSize();
            }
            else if (atomType == TEXT_OVER_RIDE_BOX)
            {
                if (timedTextModifier->value->_pTextBoxBox == NULL)
                {
                    TextBoxBox *textBoxBox = NULL;
                    PV_MP4_FF_NEW(fp->auditCB, TextBoxBox, (buf), textBoxBox);

                    if (!textBoxBox->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = textBoxBox->GetMP4Error();
                        PV_MP4_FF_DELETE(NULL, TextBoxBox, textBoxBox);
                        textBoxBox = NULL;
                        return;
                    }
                    timedTextModifier->value->_pTextBoxBox = textBoxBox;
                    timedTextModifier->type = TEXT_BOX;
                    _pTimedTextModifiersVec->push_back(timedTextModifier);

                    count += textBoxBox->getSize();
                    buf   += textBoxBox->getSize();
                }
                else
                {
                    _success = false;
                    _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
                    return;
                }
            }
            else if (atomType == TEXT_BLINK_BOX)
            {
                BlinkBox *pBlinkBox = NULL;
                PV_MP4_FF_NEW(fp->auditCB, BlinkBox, (buf), pBlinkBox);

                if (!pBlinkBox->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = pBlinkBox->GetMP4Error();
                    PV_MP4_FF_DELETE(NULL, BlinkBox, pBlinkBox);
                    pBlinkBox = NULL;
                    return;
                }
                timedTextModifier->value->_pBlinkBox = pBlinkBox;
                timedTextModifier->type = TEXT_BLINK;
                _pTimedTextModifiersVec->push_back(timedTextModifier);

                count += pBlinkBox->getSize();
                buf   += pBlinkBox->getSize();
            }
            atomType = AtomUtils::getNextAtomType(buf);
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_TEXT_SAMPLE_MODIFIERS_FAILED;
        return;
    }
}

TextSampleModifiers::~TextSampleModifiers()
{
    if (_pTimedTextModifiersVec != NULL)
    {
        for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size(); idx++)
        {
            PV_MP4_FF_DELETE(NULL, TimedTextModifiers, (*_pTimedTextModifiersVec)[idx]);
        }
    }
    PV_MP4_FF_TEMPLATED_DELETE(NULL, timedTextModifiersVecType, Oscl_Vector, _pTimedTextModifiersVec);
    _pTimedTextModifiersVec = NULL;
}


int32 geTextSampleModifiers::tNumTextStyleBoxes()
{
    int32 styleboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_STYLE)
            styleboxcount++;
    }
    return (styleboxcount);
}

TextStyleBox* TextSampleModifiers::getTextStyleBoxAt(int32 index)
{
    uint32 styleboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_STYLE)
            styleboxcount++;
        if (styleboxcount == (uint32)index)
            return (*_pTimedTextModifiersVec)[index]->value->_pTextStyleBox;
    }
    if ((uint32)index > styleboxcount)
        return NULL;
}

int32 TextSampleModifiers::getNumTextHighlightBoxes()
{
    int32 texthighlightboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_HIGHLIGHT)
            texthighlightboxcount++;
    }
    return (texthighlightboxcount);
}

TextHighlightBox* TextSampleModifiers::getTextHighlightBoxAt(int32 index)
{
    uint32 texthighlightboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_HIGHLIGHT)
            texthighlightboxcount++;
        if (texthighlightboxcount == (uint32)index)
            return (*_pTimedTextModifiersVec)[index]->value->_pTextHighlightBox;
    }
    if ((uint32)index > texthighlightboxcount)
        return NULL;
}

int32 TextSampleModifiers::getNumTextKaraokeBoxes()
{
    int32 textkoraokboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_KARAOKE)
            textkoraokboxcount ++;
    }
    return (textkoraokboxcount);
}

TextKaraokeBox* TextSampleModifiers::getTextKaraokeBoxAt(int32 index)
{
    uint32 textkoraokboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_KARAOKE)
            textkoraokboxcount++;
        if (textkoraokboxcount == (uint32)index)
            return (*_pTimedTextModifiersVec)[index]->value->_pTextKaraokeBox;
    }
    if ((uint32)index > textkoraokboxcount)
        return NULL;
}

int32 TextSampleModifiers::getNumTextHyperTextBoxes()
{
    int32 texthypertextboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_HYPERTEXT)
            texthypertextboxcount ++;
    }
    return (texthypertextboxcount);
}

TextHyperTextBox* TextSampleModifiers::getTextHyperTextBoxAt(int32 index)
{
    uint32 texthypertextboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_HYPERTEXT)
            texthypertextboxcount++;
        if (texthypertextboxcount == (uint32)index)
            return (*_pTimedTextModifiersVec)[index]->value->_pTextHyperTextBox;
    }
    if ((uint32)index > texthypertextboxcount)
        return NULL;
}


int32 TextSampleModifiers::getNumTextBlinkBoxes()
{
    int32 textblinkboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_BLINK)
            textblinkboxcount ++;
    }
    return (textblinkboxcount);
}


BlinkBox* TextSampleModifiers::getTextBlinkBoxAt(int32 index)
{
    uint32 textblinkboxcount = 0;
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_BLINK)
            textblinkboxcount++;
        if (textblinkboxcount == (uint32)index)
            return (*_pTimedTextModifiersVec)[index]->value->_pBlinkBox;
    }
    if ((uint32)index > textblinkboxcount)
        return NULL;
}

TextHilightColorBox* TextSampleModifiers::getTextHilightColorBox()
{
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_WRAP)
        {
            return (*_pTimedTextModifiersVec)[idx]->value->_pTextHilightColorBox;
        }
    }
    return NULL;
}

TextScrollDelay* TextSampleModifiers::getTextScrollDelayBox();
{
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_SCROLL_DELAY)
        {
            return (*_pTimedTextModifiersVec)[idx]->value->_pTextScrollDelay;
        }
    }
    return NULL;
}


TextBoxBox* TextSampleModifiers::getTextOverideBox()
{
    for (uint32 idx = 0; idx < _pTimedTextModifiersVec->size();idx++)
    {
        if ((*_pTimedTextModifiersVec)[idx]->type == TEXT_BOX)
        {
            return (*_pTimedTextModifiersVec)[idx]->value->_pTextBoxBox;
        }
    }
    return NULL;

}


#endif // SUPPRESS__NOT_YET_IMPLEMENTED

