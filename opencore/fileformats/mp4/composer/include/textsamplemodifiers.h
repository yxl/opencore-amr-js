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


#ifndef TEXTSAMPLEMODIFIERS_H
#define TEXTSAMPLEMODIFIERS_H


#include "oscl_file_io.h"
#include "atom.h"
#include "atomutils.h"
#include "a_atomdefs.h"
#include "boxrecord.h"
#include "stylerecord.h"

//This class is specific for the Timed text fileformat.
//this class handle and stores the modifier information in the related atoms.
class PVA_FF_TextStyleBox : public PVA_FF_Atom
{

    public:
        PVA_FF_TextStyleBox();  // Default constructor
        virtual ~PVA_FF_TextStyleBox();  // Destructor

        void setNumStyleRecordEntries(uint16 EntryCount);//entrycount
        void setStyleRecord(uint16 StartChar, uint16 EndChar, uint16 FontID, uint8 FontSizeFlag, uint8 FontSize, uint8* Trgba);

        PVA_FF_StyleRecord* getStyleRecordAt(int32 index)
        {
            if ((_pStyleRecordVec->size() == 0) ||
                    ((uint32)index >= (_pStyleRecordVec->size())))
            {
                return NULL;
            }

            return (*_pStyleRecordVec)[index];
        }
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint16  _entryCount;
        Oscl_Vector<PVA_FF_StyleRecord *, OsclMemAllocator> *_pStyleRecordVec;
};


class PVA_FF_TextHighlightBox : public PVA_FF_Atom
{

    public:
        PVA_FF_TextHighlightBox();

        virtual ~PVA_FF_TextHighlightBox() {};

        uint16 setStartCharOffset(uint16 startoffset)
        {
            if (_startCharOffset == 0)
            {
                _startCharOffset = startoffset;
            }
        }

        uint16 setEndCharOffset(uint16 endoffset)
        {
            if (_endCharOffset == 0)
            {
                _endCharOffset = endoffset;
            }
        }
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint16                  _startCharOffset;
        uint16                  _endCharOffset;
};

class PVA_FF_TextHilightColorBox : public PVA_FF_Atom
{
    public:
        PVA_FF_TextHilightColorBox();

        virtual ~PVA_FF_TextHilightColorBox();

        uint8 *setHighLightColorRGBA(uint8* Hrgba)
        {
            if (_pHighlightColorRGBA == NULL)
            {
                _pHighlightColorRGBA[0] = Hrgba[0];
                _pHighlightColorRGBA[0] = Hrgba[1];
                _pHighlightColorRGBA[0] = Hrgba[2];
                _pHighlightColorRGBA[0] = Hrgba[3];
            }
        }
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint8   *_pHighlightColorRGBA;
};

class PVA_FF_TextKaraokeBox : public PVA_FF_Atom
{
    public:
        PVA_FF_TextKaraokeBox();

        virtual ~PVA_FF_TextKaraokeBox();

        void setHighLightStartTime(uint16 HilightStartTime)
        {
            if (_highLightStartTime == 0)
            {
                _highLightStartTime = HilightStartTime;
            }
        }

        void setNumKaraokeEntries(uint16 EntryCount)
        {
            if (_entryCount == 0)
            {
                _entryCount = EntryCount;
            }
        }

        void setKaraokeVecs(uint32* endtime, uint16* startoffset, uint16* endoffset);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint32  _highLightStartTime;
        uint16  _entryCount;

        uint32  *_pHighLightEndTimeVec;
        uint16  *_pStartCharOffsetVec;
        uint16  *_pEndCharOffsetVec;

};

class PVA_FF_TextScrollDelay : public PVA_FF_Atom
{
    public:
        PVA_FF_TextScrollDelay();
        virtual ~PVA_FF_TextScrollDelay() {};
        void setScrollDelay(uint32 ScrollDelay);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint32  _scrollDelay;
};


class PVA_FF_TextHyperTextBox : public PVA_FF_Atom
{
    public:
        PVA_FF_TextHyperTextBox();
        virtual ~PVA_FF_TextHyperTextBox();
        void setStartCharOffset(uint16 StartCharOffset);
        void setEndCharOffset(uint16 EndOffset);
        void setUrlLength(uint8 UrlLength);
        void setURL(uint8* URL);
        void setAltStringLength(uint8 AltLength);
        void setAltString(uint8* AltString);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint16  _startCharOffset;
        uint16  _endCharOffset;
        uint8   _urlLength;
        uint8   *_pURL;
        uint8   _altLength;
        uint8   *_pAltString;
};

class PVA_FF_TextBoxBox : public PVA_FF_Atom
{
    public:
        PVA_FF_TextBoxBox();
        virtual ~PVA_FF_TextBoxBox();
        PVA_FF_BoxRecord* setBoxRecord(int16 Top, int16 Bottom, int16 Left, int16 Right)
        {
            PVA_FF_BoxRecord *rec = NULL;
            PV_MP4_FF_NEW(fp->auditCB, PVA_FF_BoxRecord, (Top, Bottom, Left, Right), rec);
            _pBoxRecord = rec;
        };

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        PVA_FF_BoxRecord*  _pBoxRecord;
};

class PVA_FF_BlinkBox : public PVA_FF_Atom
{
    public:
        PVA_FF_BlinkBox();
        virtual ~PVA_FF_BlinkBox() {};
        void setStartCharOffset(uint16 StartCharOffset)
        {
            if (_pstartCharOffset == 0)
            {
                _pstartCharOffset = StartCharOffset;
            }
        };
        void setEndCharOffset(uint16 EndCharOffset)
        {
            if (_pendCharOffset == 0)
            {
                _pendCharOffset = EndCharOffset;
            }
        };
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint16  _pstartCharOffset;
        uint16  _pendCharOffset;
};

#endif
