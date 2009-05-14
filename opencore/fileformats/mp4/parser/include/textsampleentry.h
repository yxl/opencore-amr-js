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
/*                        MPEG-4 TextSampleEntry Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TextSampleEntry Class is used for text streams.
*/


#ifndef TEXTSAMPLEENTRY_H_INCLUDED
#define TEXTSAMPLEENTRY_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef SAMPLEENTRY_H_INCLUDED
#include "sampleentry.h"
#endif

#define MAX_TEXT_SAMPLE_SIZE  2048

class BoxRecord;
class FontRecord;
class StyleRecord;
class FontTableAtom;

class TextSampleEntry : public SampleEntry
{

    public:
        OSCL_IMPORT_REF TextSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type);
        OSCL_IMPORT_REF virtual ~TextSampleEntry();  // Destructor

        uint32 getDisplayFlags()
        {
            return _displayFlags;
        }

        int8 getHorzJustification()
        {
            return _horzJustification;
        }

        int8 getVertJustification()
        {
            return _vertJustification;
        }

        uint8 *getBackgroundColourRGBA()
        {
            return _pBackgroundRGBA;
        }

        virtual uint32 getMaxBufferSizeDB() const
        {
            return MAX_TEXT_SAMPLE_SIZE;
        }

        OSCL_IMPORT_REF int16 getBoxTop();
        OSCL_IMPORT_REF int16 getBoxLeft();
        OSCL_IMPORT_REF int16 getBoxBottom();
        OSCL_IMPORT_REF int16 getBoxRight();

        OSCL_IMPORT_REF uint16 getStartChar() ;
        OSCL_IMPORT_REF uint16 getEndChar() ;
        OSCL_IMPORT_REF uint16 getFontID();
        OSCL_IMPORT_REF uint8 getFontStyleFlags() ;
        OSCL_IMPORT_REF uint8 getfontSize();
        OSCL_IMPORT_REF uint8 *getTextColourRGBA();
        OSCL_IMPORT_REF uint16  getFontListSize();
        OSCL_IMPORT_REF FontRecord   *getFontRecordAt(uint16 index) ;

        virtual uint8 getObjectTypeIndication() const;

    private:
        uint32 	        _displayFlags;
        int8  	        _horzJustification;
        int8  	        _vertJustification;
        uint8 		    *_pBackgroundRGBA;
        BoxRecord 	    *_pBoxRecord;
        StyleRecord 	*_pStyleRecord;
        FontTableAtom	*_pFontTableAtom;
};


#endif  // TEXTSAMPLEENTRY_H_INCLUDED


