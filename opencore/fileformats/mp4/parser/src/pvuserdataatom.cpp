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
/*                          MPEG-4 UserDataAtom Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This UserDataAtom Class is a container atom for informative user-data.
*/


#define __IMPLEMENT_PVUserDataAtom__

#include "pvuserdataatom.h"
#include "atomdefs.h"




// Stream-in Constructor
PVUserDataAtom::PVUserDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _success = true;

    uint32 _count = getDefaultSize();

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _version))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_version.get_size() * 2 + 2);

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _title))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_title.get_size() * 2 + 2);

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _author))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_author.get_size() * 2 + 2);

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _copyright))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_copyright.get_size() * 2 + 2);

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _description))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_description.get_size() * 2 + 2);

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _rating))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_rating.get_size() * 2 + 2);

    if (!AtomUtils::readNullTerminatedUnicodeString(fp, _creationDate))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
        return;
    }
    _count += (_creationDate.get_size() * 2 + 2);

    while (_count < _size)
    {
        uint8 data;
        if (!AtomUtils::read8(fp, data))
        {
            _success = false;
            _mp4ErrorCode = READ_PV_USER_DATA_ATOM_FAILED;
            return;
        }
        _count++;
    }
}

// Copy constructor
PVUserDataAtom::PVUserDataAtom(PVUserDataAtom &atom)
        : Atom(PVUSER_DATA_ATOM)
{
    MP4FFParserOriginalCharEnc charType;
    _version = atom.getPVVersion();
    _title = atom.getPVTitle(charType);
    _author = atom.getPVAuthor(charType);
    _copyright = atom.getPVCopyright(charType);
    _description = atom.getPVDescription(charType);
    _rating = atom.getPVRating(charType);
}

// Destructor
PVUserDataAtom::~PVUserDataAtom()
{

}

PVContentTypeAtom::PVContentTypeAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _success = true;

    //Initialize to an undefined value - all content with this atom, ought to contain
    //a valid content type
    _contentType = 0xFFFFFFFF;

    uint32 _count = getDefaultSize();

    if (!AtomUtils::read32(fp, _contentType))
    {
        _success = false;
        _mp4ErrorCode = READ_PV_CONTENT_TYPE_ATOM_FAILED;
        return;
    }
    _count += sizeof(_contentType);

    while (_count < _size)
    {
        uint8 data;
        if (!AtomUtils::read8(fp, data))
        {
            _success = false;
            _mp4ErrorCode = READ_PV_CONTENT_TYPE_ATOM_FAILED;
            return;
        }
        _count++;
    }
}
