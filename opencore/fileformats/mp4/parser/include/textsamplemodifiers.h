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
#ifndef __TextSampleModifiers_H__
#define __TextSampleModifiers_H__

#include "oscl_file_io.h"
#include "atom.h"
#include "atomutils.h"
#include "atomdefs.h"
#include "boxrecord.h"
#include "stylerecord.h"


class TextStyleBox : public Atom
{

    public:
        TextStyleBox(uint8  *buf);  // Default constructor
        virtual ~TextStyleBox();  // Destructor

        int32 getNumStyleRecordEntries()
        {
            return ((int32)(_entryCount));
        }

        StyleRecord* getStyleRecordAt(int32 index)
        {
            if ((_pStyleRecordVec->size() == 0) ||
                    ((uint32)index >= (_pStyleRecordVec->size())))
            {
                return NULL;
            }

            return (*_pStyleRecordVec)[index];
        }

    private:
        uint16                   _entryCount;
        Oscl_Vector<StyleRecord *, OsclMemAllocator> *_pStyleRecordVec;
};

class TextHighlightBox : public Atom
{

    public:
        TextHighlightBox(uint8  *buf);

        virtual ~TextHighlightBox() {};

        uint16 getStartCharOffset()
        {
            return (_startCharOffset);
        }

        uint16 getEndCharOffset()
        {
            return (_endCharOffset);
        }

    private:
        uint16                  _startCharOffset;
        uint16                  _endCharOffset;
};

class TextHilightColorBox : public Atom
{
    public:
        TextHilightColorBox(uint8  *buf);

        virtual ~TextHilightColorBox();

        uint8 *getHighLightColorRGBA()
        {
            return (_pHighlightColorRGBA);
        }

    private:
        uint8   *_pHighlightColorRGBA;
};


class TextKaraokeBox : public Atom
{
    public:
        TextKaraokeBox(uint8  *buf);

        virtual ~TextKaraokeBox();

        uint32 getHighLightStartTime()
        {
            return (_highLightStartTime);
        }

        uint16 getNumKaraokeEntries()
        {
            return (_entryCount);
        }

        uint32 *getHighLightEndTimeVec()
        {
            return (_pHighLightEndTimeVec);
        }

        uint16 *getStartCharOffsetVec()
        {
            return (_pStartCharOffsetVec);
        }

        uint16 *getEndCharOffsetVec()
        {
            return (_pEndCharOffsetVec);
        }

    private:
        uint32  _highLightStartTime;
        uint16  _entryCount;

        uint32  *_pHighLightEndTimeVec;
        uint16  *_pStartCharOffsetVec;
        uint16  *_pEndCharOffsetVec;

};

class TextScrollDelay : public Atom
{
    public:
        TextScrollDelay(uint8  *buf);

        virtual ~TextScrollDelay() {};

        uint32 getScrollDelay()
        {
            return (_scrollDelay);
        }

    private:
        uint32  _scrollDelay;
};

class TextHyperTextBox : public Atom
{
    public:
        TextHyperTextBox(uint8  *buf);

        virtual ~TextHyperTextBox();

        uint16 getStartCharOffset()
        {
            return (_startCharOffset);
        }

        uint16 getEndCharOffset()
        {
            return (_endCharOffset);
        }

        uint8 getUrlLength()
        {
            return (_urlLength);
        }

        uint8* getURL()
        {
            return (_pURL);
        }

        uint8 getAltStringLength()
        {
            return (_altLength);
        }

        uint8* getAltString()
        {
            return (_pAltString);
        }

    private:
        uint16  _startCharOffset;
        uint16  _endCharOffset;
        uint8   _urlLength;
        uint8   *_pURL;
        uint8   _altLength;
        uint8   *_pAltString;
};


class TextBoxBox : public Atom
{
    public:
        TextBoxBox(uint8  *buf);

        virtual ~TextBoxBox();

        BoxRecord* getBoxRecord()
        {
            return (_pBoxRecord);
        }

    private:
        BoxRecord*  _pBoxRecord;
};

class BlinkBox : public Atom
{
    public:
        BlinkBox(uint8  *buf);

        virtual ~BlinkBox() {};

        uint16 getStartCharOffset()
        {
            return (_startCharOffset);
        }
        uint16 getEndCharOffset()
        {
            return (_endCharOffset);
        }

    private:
        uint16  _startCharOffset;
        uint16  _endCharOffset;
};

enum TimedTextModifierType
{
    TEXT_STYLE,
    TEXT_HIGHLIGHT,
    TEXT_KARAOKE,
    TEXT_HYPERTEXT,
    TEXT_BLINK,
    TEXT_WRAP,
    TEXT_SCROLL_DELAY,
    TEXT_BOX
};

union TimedTextModUnion
{
    TextStyleBox		*_pTextStyleBox;
    TextHighlightBox	*_pTextHighlightBox;
    TextKaraokeBox		*_pTextKaraokeBox;
    TextHyperTextBox	*_pTextHyperTextBox;
    BlinkBox			*_pBlinkBox;
    TextHilightColorBox *_pTextHilightColorBox;
    TextScrollDelay     *_pTextScrollDelay;
    TextBoxBox          *_pTextBoxBox;

};

struct TimedTextModifiers
{

    TimedTextModifierType type;
    TimedTextModUnion *value;
};

class TextSampleModifiers : public ISucceedFail
{
    public:
        static TextSampleModifiers* parseTextModifiers(uint8* buf, uint32 size);

        TextSampleModifiers(uint8* buf, uint32 size);
        virtual ~TextSampleModifiers();

        int32 getNumTextStyleBoxes();
        TextStyleBox* getTextStyleBoxAt(int32 index);
        int32 getNumTextHighlightBoxes();
        TextHighlightBox* getTextHighlightBoxAt(int32 index);
        int32 getNumTextKaraokeBoxes();
        TextKaraokeBox* getTextKaraokeBoxAt(int32 index);
        int32 getNumTextHyperTextBoxes();
        TextHyperTextBox* getTextHyperTextBoxAt(int32 index);
        int32 getNumTextBlinkBoxes();
        BlinkBox* getTextBlinkBoxAt(int32 index);
        TextHilightColorBox* getTextHilightColorBox();
        TextScrollDelay* getTextScrollDelayBox();
        TextBoxBox* getTextOverideBox();
        bool PopulateTextSampleModifierParams()
        {
            return 0;
        };

    private:

        Oscl_Vector<TimedTextModifiers* , OsclMemAllocator> *_pTimedTextModifiersVec;




};

#endif
