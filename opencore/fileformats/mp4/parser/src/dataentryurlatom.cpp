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
/*                        MPEG-4 DataEntryUrlAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DataEntryUrlAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/


#define IMPLEMENT_DataEntryUrlAtom_H__

#include "dataentryurlatom.h"
#include "atomdefs.h"
#include "atomutils.h"

// Stream-in ctor
DataEntryUrlAtom::DataEntryUrlAtom(MP4_FF_FILE *fp)
        : DataEntryAtom(fp)
{
    if (_success)
    {
        _pparent = NULL;

        if (getFlags() != 1)
        {
            // media NOT self contained
            if (!AtomUtils::readNullTerminatedString(fp, _location))
            {
                _success = false;
                _mp4ErrorCode = READ_DATA_ENTRY_URL_ATOM_FAILED;
            }

            //We DO NOT SUPPORT NON-SELF CONTAINED files.
            //So flag ERROR, and return

            _success = false;
            _mp4ErrorCode = MEDIA_DATA_NOT_SELF_CONTAINED;
        }
        else
        {
            uint32 count = getSize();;
            count -= DEFAULT_ATOM_SIZE;
            count -= 4;
            if (count > 0)
                AtomUtils::seekFromCurrPos(fp, count);
        }
    }
    else
    {
        _mp4ErrorCode = READ_DATA_ENTRY_URL_ATOM_FAILED;
    }

}

// Destructor
DataEntryUrlAtom::~DataEntryUrlAtom()
{
    // Empty
}

