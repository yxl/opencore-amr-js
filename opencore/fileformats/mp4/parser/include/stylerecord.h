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

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

class StyleRecord
{

    public:
        StyleRecord(MP4_FF_FILE *fp);  // Default constructor
        StyleRecord(uint8 *&buf);  // Default constructor
        virtual ~StyleRecord();  // Destructor

        uint16 getStartChar()
        {
            return _startChar;
        }

        uint16 getEndChar()
        {
            return _endChar;
        }

        uint16 getFontID()
        {
            return _fontID;
        }

        uint8 getFontStyleFlags()
        {
            return _fontStyleFlags;
        }

        uint8 getfontSize()
        {
            return _fontSize;
        }

        uint8 *getTextColourRGBA()
        {
            return _pRGBA;
        }

        bool MP4Success()
        {
            return _success;
        }

        int32 GetMP4Error()
        {
            return _mp4ErrorCode;
        }

    private:
        uint16 _startChar;
        uint16 _endChar;
        uint16 _fontID;
        uint8  _fontStyleFlags;
        uint8  _fontSize;
        uint8 *_pRGBA;

        bool  _success;
        int32 _mp4ErrorCode;
};

#endif // STYLERECORD_H_INCLUDED


