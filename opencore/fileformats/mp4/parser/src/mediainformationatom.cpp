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
/*                       MPEG-4 MediaInformationAtom Class                       */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaInformationHeaderAtom Class is the base class of the other
    MediaInformationHeaderAtoms, VideoMediaHeaderAtom VIDEO_MEDIA_HEADER_ATOM, SoundMediaHeaderAtom
    SOUND_MEDIA_HEADER_ATOM, HintMediaHeaderAtom HINT_MEDIA_HEADER_ATOM, and Mpeg4MediaHeaderAtom MPEG4_MEDIA_HEADER_ATOM
*/


#define IMPLEMENT_MediaInformationAtom_H__

#include "mediainformationatom.h"
#include "atomutils.h"
#include "atomdefs.h"

#include "videomediaheaderatom.h"
#include "soundmediaheaderatom.h"
#include "hintmediaheaderatom.h"
#include "mpeg4mediaheaderatom.h"


// Stream-in ctor
MediaInformationAtom::MediaInformationAtom(MP4_FF_FILE *fp,
        uint32 mediaType,
        OSCL_wString& filename,
        bool oPVContentDownloadable,
        uint32 parsingMode)
        : Atom(fp)
{
    _pmediaInformationHeader = NULL;
    _pdataInformationAtom    = NULL;
    _psampleTableAtom        = NULL;

    if (_success)
    {
        _pparent = NULL;

        // Need to allow reading in in ANY ORDER!
        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        int32 count = _size - DEFAULT_ATOM_SIZE;

        // Read in the MediaInformationHeaderAtom (either Video, Sound, Hint, or Mpeg4)
        // and Data Information Atom and Sample Table Atom in ANY ORDER!!!!
        while (count > 0)
        {
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == SOUND_MEDIA_HEADER_ATOM)
            { //smhd
                // Read in SoundMediaHeaderAtom

                PV_MP4_FF_NEW(fp->auditCB, SoundMediaHeaderAtom, (fp, atomSize, atomType), _pmediaInformationHeader);

                if (!_pmediaInformationHeader->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaInformationHeader->GetMP4Error();
                    break;
                }
                count -= _pmediaInformationHeader->getSize();
            }
            else if (atomType == VIDEO_MEDIA_HEADER_ATOM)
            {
                // Read in VideoMediaHeaderAtom
                PV_MP4_FF_NEW(fp->auditCB, VideoMediaHeaderAtom, (fp, atomSize, atomType), _pmediaInformationHeader);

                if (!_pmediaInformationHeader->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaInformationHeader->GetMP4Error();
                    break;
                }
                count -= _pmediaInformationHeader->getSize();
            }
            else if ((atomType == UUID_ATOM)
                     || (atomType == UNKNOWN_ATOM))
            {
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    _success = false;
                    _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                    break;
                }
                if (count < (int32)atomSize)
                {
                    _success = false;
                    _mp4ErrorCode = READ_FAILED;
                    break;
                }
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
            else if (atomType == HINT_MEDIA_HEADER_ATOM)
            {
                // Read in HintMediaHeaderAtom

                PV_MP4_FF_NEW(fp->auditCB, HintMediaHeaderAtom,
                              (fp, atomSize, atomType),
                              _pmediaInformationHeader);

                if (!_pmediaInformationHeader->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaInformationHeader->GetMP4Error();
                    break;
                }
                count -= _pmediaInformationHeader->getSize();
            }
            else if (atomType == MPEG4_MEDIA_HEADER_ATOM)
            {
                //nmhd
                // Read in generic Mpeg4MediaHeaderAtom

                PV_MP4_FF_NEW(fp->auditCB, Mpeg4MediaHeaderAtom,
                              (fp, mediaType, atomSize, atomType),
                              _pmediaInformationHeader);

                if (!_pmediaInformationHeader->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaInformationHeader->GetMP4Error();
                    break;
                }
                count -= _pmediaInformationHeader->getSize();
            }
            else if (atomType == DATA_INFORMATION_ATOM)
            {
                //dinf
                // Read in the DataInformationAtom
                PV_MP4_FF_NEW(fp->auditCB, DataInformationAtom,
                              (fp, atomSize, atomType),
                              _pdataInformationAtom);

                if (!_pdataInformationAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pdataInformationAtom->GetMP4Error();
                    break;
                }
                _pdataInformationAtom->setParent(this);
                count -= _pdataInformationAtom->getSize();
            }
            else if (atomType == SAMPLE_TABLE_ATOM)
            {
                //stbl
                // Read in the SampleTableAtom
                PV_MP4_FF_NEW(fp->auditCB, SampleTableAtom,
                              (fp, mediaType, filename, atomSize,
                               atomType, oPVContentDownloadable,
                               parsingMode), _psampleTableAtom);

                if (!_psampleTableAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _psampleTableAtom->GetMP4Error();
                    break;
                }
                _psampleTableAtom->setParent(this);
                count -= _psampleTableAtom->getSize();
            }
            else
            {
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
        }
        if (_pmediaInformationHeader != NULL)
            _pmediaInformationHeader->setParent(this);

        if ((_pmediaInformationHeader == NULL) ||
                (_pdataInformationAtom == NULL)    ||
                (_psampleTableAtom == NULL))
        {
            _success = false;
            _mp4ErrorCode = READ_MEDIA_INFORMATION_ATOM_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_MEDIA_INFORMATION_ATOM_FAILED;
    }
}

// Destructor
MediaInformationAtom::~MediaInformationAtom()
{
    if (_psampleTableAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, SampleTableAtom, _psampleTableAtom);
    }
    if (_pdataInformationAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, DataInformationAtom, _pdataInformationAtom);
    }
    if (_pmediaInformationHeader != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MediaInformationHeaderAtom, _pmediaInformationHeader);
    }
}
