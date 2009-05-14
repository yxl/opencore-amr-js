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
#ifndef BOXRECORD_H_INCLUDED
#define BOXRECORD_H_INCLUDED

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

class BoxRecord
{

    public:
        BoxRecord(MP4_FF_FILE *fp);  // Default constructor
        BoxRecord(uint8 *&buf);  // Default constructor
        virtual ~BoxRecord() {};  // Destructor

        int16 getBoxTop()
        {
            return _top;
        }

        int16 getBoxLeft()
        {
            return _left;
        }

        int16 getBoxBottom()
        {
            return _bottom;
        }

        int16 getBoxRight()
        {
            return _right;
        }

        bool  MP4Success()
        {
            return _success;
        }

        int32 GetMP4Error()
        {
            return _mp4ErrorCode;
        }

    private:
        int16 _top;
        int16 _left;
        int16 _bottom;
        int16 _right;

        bool  _success;
        int32 _mp4ErrorCode;
};

#endif // BOXRECORD_H_INCLUDED

