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
/*                        MPEG-4 MediaHeaderAtom Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaHeaderAtom Class contains all the objects that declare information
    about the media data within the stream.
*/


#define IMPLEMENT_MediaHeaderAtom

#include "mediaheaderatom.h"
#include "atomutils.h"
#include "atomdefs.h"

// Stream-in ctor
MediaHeaderAtom::MediaHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    if (_success)
    {

        _pparent = NULL;
        if (getVersion() == 1)
        {
            if (!AtomUtils::read64(fp, _creationTime64))
                _success = false;
            if (!AtomUtils::read64(fp, _modificationTime64))
                _success = false;
            if (!AtomUtils::read32(fp, _timeScale))
                _success = false;
            if (!AtomUtils::read64(fp, _duration64))
                _success = false;
        }
        else
        {
            if (!AtomUtils::read32(fp, _creationTime))
                _success = false;
            if (!AtomUtils::read32(fp, _modificationTime))
                _success = false;
            if (!AtomUtils::read32(fp, _timeScale))
                _success = false;
            if (!AtomUtils::read32(fp, _duration))
                _success = false;

        }
        if (!AtomUtils::read16(fp, _language))
            _success = false;

        // Read the last two bytes of reserved
        if (!AtomUtils::read16(fp, _reserved))
            _success = false;

        if (!_success)
            _mp4ErrorCode = READ_MEDIA_HEADER_ATOM_FAILED;
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_MEDIA_HEADER_ATOM_FAILED;
    }
}


// Destructor
MediaHeaderAtom::~MediaHeaderAtom()
{
    // Empty
}
