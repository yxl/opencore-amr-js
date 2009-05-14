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
/*                   MPEG-4 MediaInformationHeaderAtom Class                     */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaInformationHeaderAtom Class is an abstract base class for the
    atoms VideoMediaHeaderAton, AudioMediaHeaderAtom, HintMediaHeaderAtom,
    and Mpeg4MediaHeaderAtom.  This class onbly contains one pure virtual method
    getMediaHeaderType() that will return what type of mediaInformationHeader
    this atom is.
*/


#define IMPLEMENT_MediaInformationAtomHeader

#include "mediainformationheaderatom.h"
#include "atomdefs.h"

// Stream-in ctor
MediaInformationHeaderAtom::MediaInformationHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    if (!_success)
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_MEDIA_INFORMATION_HEADER_ATOM_FAILED;
    }
}

// Destructor
MediaInformationHeaderAtom::~MediaInformationHeaderAtom()
{

}

