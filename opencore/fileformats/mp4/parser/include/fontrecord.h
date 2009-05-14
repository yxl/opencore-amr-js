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
#ifndef FONTRECORD_H_INCLUDED
#define FONTRECORD_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDE
#include "oscl_base.h"
#endif

class MP4_FF_FILE;

class FontRecord
{

    public:
        FontRecord(MP4_FF_FILE *fp);  // Default constructor
        virtual ~FontRecord();  // Destructor

        uint16 getFontID()
        {
            return _fontID;
        }

        int8   getFontLength()
        {
            return _fontLength;
        }

        uint8* getFontName()
        {
            return _pFontName;
        }

        bool GetMP4Success()
        {
            return _success;
        }

        int32 GetMP4Error()
        {
            return _mp4ErrorCode;
        }

    private:
        uint16 _fontID;
        int8   _fontLength;
        uint8  *_pFontName;

        bool  _success;
        int32 _mp4ErrorCode;
};


#endif // FONTRECORD_H_INCLUDED

