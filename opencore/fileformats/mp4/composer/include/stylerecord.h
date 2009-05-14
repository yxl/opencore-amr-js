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
#ifndef STYLERECORD_H_INCLUDED
#define STYLERECORD_H_INCLUDED

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"

class PVA_FF_StyleRecord : public PVA_FF_Atom
{

    public:
        PVA_FF_StyleRecord();  // Default constructor
        PVA_FF_StyleRecord(uint16 StartChar, uint16 EndChar, uint16 FontID, uint8 FontSizeFlag, uint8 FontSize, uint8* Trgba); // Default constructor
        virtual ~PVA_FF_StyleRecord();  // Destructor

        void setStartChar(uint16);
        void setEndChar(uint16);
        void setFontID(uint16);
        void setFontStyleFlags(uint8);
        void setFontSize(uint8);
        void setTextColourRGBA(uint8*);
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        virtual void recomputeSize();
        virtual uint32 getSize();

    private:
        uint8* _sample;
        uint16 _startChar;
        uint16 _endChar;
        uint16 _fontID;
        uint8  _fontStyleFlags;
        uint8  _fontSize;
        uint8 *_pRGBA;

};

#endif // STYLERECORD_H_INCLUDED

