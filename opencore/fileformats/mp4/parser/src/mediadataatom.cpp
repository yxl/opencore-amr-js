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
/*                          MPEG-4 MediaDataAtom Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaDataAtom Class contains the media data.  This class can operate in
    either one of two ways - 1. it can store all it's data in memory (such as
    during the creation of ObjectDescriptor streams), or 2. it can maintain all
    it's data on disk (such as during the creation ofmedia streams - i.e. audio
    and video).

    Note that during reading in this atom from a file stream, the type is forced
    to MEDIA_DATA_ON_DISK thereby keeping all the object data in the physical
    file.
*/

#define IMPLEMENT_MediaDataAtom

#include "mediadataatom.h"

#include "atomutils.h"
#include "atomdefs.h"

// Stream-in Constructor
MediaDataAtom::MediaDataAtom(MP4_FF_FILE *fp, OSCL_wString& filename)
        : Atom(fp)
{
    OSCL_UNUSED_ARG(filename);
    if (_success)
    {

        _type = MEDIA_DATA_ON_DISK;

        // Seek past the end of the MEDIA_DATA_ATOM atom
        int32 offset = getSize() - getDefaultSize();
        int32 filePointer;
        filePointer = AtomUtils::getCurrentFilePosition(fp);

        if ((filePointer + offset) == fp->_fileSize)
        {
            // LAST MDAT IN THE FILE
            _success = true;
        }
        AtomUtils::seekFromStart(fp, (filePointer + offset));
    }
    else
    {
        _mp4ErrorCode = READ_MEDIA_DATA_ATOM_FAILED;
    }
}

// Destructor
MediaDataAtom::~MediaDataAtom()
{

}

