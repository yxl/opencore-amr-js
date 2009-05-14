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
/*
    This PVA_FF_HandlerAtom Class declares the nature of the media in the stream.
*/

#define IMPLEMENT_HandlerAtom

#include "handleratom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

// Constructor
PVA_FF_HandlerAtom::PVA_FF_HandlerAtom(int32 handlerType, uint8 version, uint32 flags)
        : PVA_FF_FullAtom(HANDLER_ATOM, version, flags)
{
    init(handlerType);
    recomputeSize();
}

// Destructor
PVA_FF_HandlerAtom::~PVA_FF_HandlerAtom()
{
    // Empty
}

void
PVA_FF_HandlerAtom::init(int32 handlerType)
{
    switch (handlerType)
    {
        case MEDIA_TYPE_AUDIO:
            _handlerType = MEDIA_TYPE_AUDIO;
            _name = "SoundHandler";
            break;
        case MEDIA_TYPE_TEXT:
            _handlerType = MEDIA_TYPE_TEXT;
            _name = "TextHandler";
            break;
        case MEDIA_TYPE_VISUAL:
            _handlerType = MEDIA_TYPE_VISUAL;
            _name = "VideoHandler";
            break;
        default:
            _handlerType = UNKNOWN_HANDLER; // Error condition!
            _name = "UNKNOWNHandler";
            break;
    }

    reserved1 = 0;
}

void
PVA_FF_HandlerAtom::recomputeSize()
{
    int32 size = getDefaultSize();
    size += 4; // For 4 bytes of missed 0
    size += 4; // For handlerType
    size += 12; // For (8)[12] reserved
    size += _name.get_size() + 1; // +1 for null termination

    _size = size;

}

// Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
bool
PVA_FF_HandlerAtom::renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp)
{
    int32 rendered = 0; // Keep track of number of bytes rendered

    // Render PVA_FF_Atom type and size
    if (!renderAtomBaseMembers(fp))
    {
        return false;
    }
    rendered += getDefaultSize();

    if (!PVA_FF_AtomUtils::render32(fp, 0))
    {
        return false;
    }
    rendered += 4;

    if (!PVA_FF_AtomUtils::render32(fp, _handlerType))
    {
        return false;
    }
    rendered += 4;

    uint8 reserved = 0;
    for (int32 i = 0; i < 12; i++)
    {
        if (!PVA_FF_AtomUtils::render8(fp, reserved))
        {
            return false;
        }
    }
    rendered += 12;

    if (!PVA_FF_AtomUtils::renderNullTerminatedString(fp, _name))
    {
        return false;
    }
    rendered += _name.get_size() + 1;

    return true;
}

