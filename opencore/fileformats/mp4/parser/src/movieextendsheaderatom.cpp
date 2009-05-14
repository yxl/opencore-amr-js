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
#define IMPLEMENT_MovieExtendsHeaderAtom

#include "movieextendsheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

MovieExtendsHeaderAtom ::MovieExtendsHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)

{
    OSCL_UNUSED_ARG(type);

    _fragmentDuration32 = 0;
    _fragmentDuration64 = 0;

    uint8 ver = getVersion();

    _version = ver;
    if (_success)
    {
        if (ver == 0)
        {
            if (!AtomUtils::read32(fp, _fragmentDuration32))
            {
                _success = false;
                _mp4ErrorCode = READ_MOVIE_EXTENDS_HEADER_FAILED;
                return;
            }
        }
        else if (ver == 1)
        {
            if (!AtomUtils::read64(fp, _fragmentDuration64))
            {
                _success = false;
                _mp4ErrorCode = READ_MOVIE_EXTENDS_HEADER_FAILED;
                return;
            }

        }
        else
        {
            _success = false;
            _mp4ErrorCode = NOT_SUPPORTED;
            return;

        }

    }
}
// Destructor
MovieExtendsHeaderAtom::~MovieExtendsHeaderAtom()
{
}
