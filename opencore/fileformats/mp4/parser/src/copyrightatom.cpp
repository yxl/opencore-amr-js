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
#include "copyrightatom.h"
#include "atomutils.h"
#include "atomdefs.h"

CopyRightAtom::CopyRightAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    uint32 count = getDefaultSize();

    if (_success)
    {
        if (!AtomUtils::read16(fp, _language_code))
        {
            _success = false;
            _mp4ErrorCode = READ_COPYRIGHT_ATOM_FAILED;
        }

        count += 2;

        if (count <= _size)
        {
            uint32 delta = (_size - count);
            MP4FFParserOriginalCharEnc CharType;

            if (delta > 0)
            {
                if (!AtomUtils::readString(fp, delta, CharType , _copyRightNotice))
                {
                    //error
                    _success = false;
                    _mp4ErrorCode = READ_COPYRIGHT_ATOM_FAILED;
                    return;
                }
                count += delta;
            }
            else
            {
                //error
                _success = false;
                _mp4ErrorCode = READ_COPYRIGHT_ATOM_FAILED;
                return;
            }

            if (count < _size)
            {
                AtomUtils::seekFromCurrPos(fp, (_size - count));
                count = _size;
            }
        }
        else if (count > _size)
        {
            _success = false;
            _mp4ErrorCode = READ_USER_DATA_ATOM_FAILED;
            return;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_COPYRIGHT_ATOM_FAILED;
    }
}


// Destructor
CopyRightAtom::~CopyRightAtom()
{
}

