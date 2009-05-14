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
#ifndef TEXTSAMPLEENTRY_H_INCLUDED
#define TEXTSAMPLEENTRY_H_INCLUDED


#include "atom.h"
#include "oscl_file_io.h"
#include "decoderspecificinfo.h"
#include "sampleentry.h"


class PVA_FF_BoxRecord;
class PVA_FF_FontRecord;
class PVA_FF_StyleRecord;
class PVA_FF_FontTableAtom;


//This class is specific for the Timed text fileformat.
//it handles and stores the configuraion information in the related atoms.

class PVA_FF_TextSampleEntry : public PVA_FF_SampleEntry
{

    public:
        OSCL_IMPORT_REF	 PVA_FF_TextSampleEntry();
        OSCL_IMPORT_REF	 virtual ~PVA_FF_TextSampleEntry();  // Destructor

        OSCL_IMPORT_REF void setBoxTop(int16 top);
        OSCL_IMPORT_REF void setBoxLeft(int16 left);
        OSCL_IMPORT_REF void setBoxBottom(int16 bottom);
        OSCL_IMPORT_REF void setBoxRight(int16 right);

        OSCL_IMPORT_REF void setStartChar(uint16 startchar) ;
        OSCL_IMPORT_REF void setEndChar(uint16 endchar) ;
        OSCL_IMPORT_REF void setFontID(uint16 fontID);
        OSCL_IMPORT_REF void setFontStyleFlags(uint8 flag) ;
        OSCL_IMPORT_REF void setfontSize(uint8 fontsize);
        OSCL_IMPORT_REF void setTextColourRGBA(uint8* RGBA);
        OSCL_IMPORT_REF void setFontListSize(uint16 fontlistsize);
        OSCL_IMPORT_REF void   *setFontRecord(uint16 fontlistID, uint16 fontID, int8 fontLength, uint8* fontName);

        void setSample(void* sample, uint32 size);
        void addTextDecoderSpecificInfo(PVA_FF_TextSampleDescInfo *pinfo);
        virtual void recomputeSize();
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
    private:
        void init();

        // Reserved constants
        uint8  _reserved[6];
        uint16 _dataReferenceIndex;
        int16  _preDefined1;
        int16  _reserved1;
        int32  _predefined2[3];
        int32  _reserved2;
        int16  _preDefined2;
        int16  _predefined3;

        uint32			_start_sample_num;
        uint32			_end_sample_num;
        int32			_sdIndex;
        uint32 	        _displayFlags;
        int8  	        _horzJustification;
        int8  	        _vertJustification;
        uint8 		    *_pBackgroundRGBA;
        PVA_FF_BoxRecord 	    *_pBoxRecord;
        PVA_FF_StyleRecord 		*_pStyleRecord;
        PVA_FF_FontTableAtom	*_pFontTableAtom;
};


#endif  // TEXTSAMPLEENTRY_H_INCLUDED
