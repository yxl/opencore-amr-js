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
/*                          MPEG-4 HandlerAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This HandlerAtom Class declares the nature of the media in the stream.
*/


#define IMPLEMENT_HandlerAtom

#include "handleratom.h"
#include "atomdefs.h"
#include "atomutils.h"

// Stream-in ctor
HandlerAtom::HandlerAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    if (_success)
    {
        _pparent = NULL;

        uint32 junk;

        uint32  count = _size - DEFAULT_FULL_ATOM_SIZE;

        // Reading in 4 bytes of missed 0
        if (!AtomUtils::read32(fp, junk))
        {
            _success = false;
            _mp4ErrorCode = READ_HANDLER_ATOM_FAILED;
            return;
        }
        count -= 4;

        if (!AtomUtils::read32(fp, _handlerType))
        {
            _success = false;
            _mp4ErrorCode = READ_HANDLER_ATOM_FAILED;
            return;
        }
        count -= 4;

        //Skip rest of the handler atom - it has no info of value to us
        AtomUtils::seekFromCurrPos(fp, count);
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_HANDLER_ATOM_FAILED;
    }

}

// Destructor
HandlerAtom::~HandlerAtom()
{
    // Empty
}


