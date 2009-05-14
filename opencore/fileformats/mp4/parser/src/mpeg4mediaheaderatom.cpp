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
/*                      MPEG-4 Mpeg4MediaHeaderAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This Mpeg4MediaHeaderAtom Class is for streams other than visual and audio.
*/

#define IMPLEMENT_Mpeg4MediaHeaderAtom

#include "mpeg4mediaheaderatom.h"
#include "atomdefs.h"
#include "atomutils.h"

// Stream-in ctor
Mpeg4MediaHeaderAtom::Mpeg4MediaHeaderAtom(MP4_FF_FILE *fp,
        uint32 mediaType,
        uint32 size,
        uint32 type)
        : MediaInformationHeaderAtom(fp, size, type)
{
    if (_success)
    {
        _mediaType = mediaType;
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_MPEG4_MEDIA_HEADER_ATOM_FAILED;
    }
}

// Destructor
Mpeg4MediaHeaderAtom::~Mpeg4MediaHeaderAtom()
{
    // Empty
}

uint32
Mpeg4MediaHeaderAtom::getMediaInformationHeaderType() const
{
    return _mediaType;
}
