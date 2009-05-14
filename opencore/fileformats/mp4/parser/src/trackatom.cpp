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
/*                            MPEG-4 TrackAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackAtom Class is the container for a single track in the MPEG-4
    presentation.
*/

#define IMPLEMENT_TrackAtom

#include "movieatom.h"
#include "trackatom.h"
#include "atomdefs.h"
#include "atomutils.h"

OSCL_EXPORT_REF  uint8 TrackAtom::getObjectTypeIndication()
{
    if (_pmediaAtom != NULL)
    {
        return _pmediaAtom->getObjectTypeIndication();
    }
    else
    {
        return 0xFF;
    }
} // Based on OTI value

// Constructor
TrackAtom::TrackAtom(MP4_FF_FILE *fp,
                     OSCL_wString& filename,
                     uint32 size,
                     uint32 type,
                     bool oPVContent,
                     bool oPVContentDownloadable,
                     uint32 parsingMode)
        : Atom(fp, size, type)
{

    _ptrackHeader      = NULL;
    _ptrackReference   = NULL;
    _pmediaAtom        = NULL;
    _puserdataatom     = NULL;
    _pEditAtom         = NULL;

    if (_success)
    {
        // Generalized so can read in atoms in ANY ORDER!
        _pparent = NULL;
        _ptrackReference = NULL;

        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        int32 count = _size - DEFAULT_ATOM_SIZE;

        while (((atomType == TRACK_REFERENCE_ATOM) ||
                (atomType == TRACK_HEADER_ATOM) ||
                (atomType == MEDIA_ATOM) ||
                (atomType == USER_DATA_ATOM) ||
                (atomType == EDIT_ATOM) ||
                (atomType == FREE_SPACE_ATOM) ||

                (atomType == UUID_ATOM) ||
                (atomType == UNKNOWN_ATOM))
                && (count > 0))
        {
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == EDIT_ATOM)
            {
                PV_MP4_FF_NEW(fp->auditCB, EditAtom, (fp, atomSize, atomType), _pEditAtom);

                if (!_pEditAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pEditAtom->GetMP4Error();
                    break;
                }

                count -= _pEditAtom->getSize();
            }
            else if ((atomType == FREE_SPACE_ATOM)
                     || (atomType == UUID_ATOM)
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
            else if (atomType == USER_DATA_ATOM)
            {
                if (oPVContent)
                {
                    if (_puserdataatom == NULL)
                    {
                        PV_MP4_FF_NEW(fp->auditCB, UserDataAtom, (fp, atomSize, atomType), _puserdataatom);
                        if (!_puserdataatom->MP4Success())
                        {
                            _success = false;
                            _mp4ErrorCode = _puserdataatom->GetMP4Error();
                            break;
                        }
                        count -= _puserdataatom->getSize();
                    }
                    else
                    {
                        _success = false;
                        _mp4ErrorCode = READ_USER_DATA_ATOM_FAILED;
                        break;
                    }
                }
                else
                {
                    //Skip third party user data atom
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
            }
            else if (atomType == TRACK_REFERENCE_ATOM)
            {
                // tref
                if (_ptrackReference != NULL)
                {
                    _success = false;
                    _mp4ErrorCode = DUPLICATE_TRACK_REFERENCE_ATOMS;
                    break;
                }
                PV_MP4_FF_NEW(fp->auditCB, TrackReferenceAtom, (fp, atomSize, atomType), _ptrackReference);
                if (!_ptrackReference->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _ptrackReference->GetMP4Error();
                    break;
                }
                _ptrackReference->setParent(this);
                count -= _ptrackReference->getSize();
            }
            else if (atomType == TRACK_HEADER_ATOM)
            {
                // tkhd
                if (_ptrackHeader != NULL)
                {
                    _success = false;
                    _mp4ErrorCode = DUPLICATE_TRACK_HEADER_ATOMS;
                    break;
                }
                PV_MP4_FF_NEW(fp->auditCB, TrackHeaderAtom, (fp, atomSize, atomType), _ptrackHeader);

                if (!_ptrackHeader->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _ptrackHeader->GetMP4Error();
                    break;
                }
                _ptrackHeader->setParent(this);
                count -= _ptrackHeader->getSize();
            }
            else if (atomType == MEDIA_ATOM)
            {	// mdia
                if (_pmediaAtom != NULL)
                {
                    _success = false;
                    _mp4ErrorCode = DUPLICATE_MEDIA_ATOMS;
                    break;
                }
                PV_MP4_FF_NEW(fp->auditCB, MediaAtom, (fp, filename, atomSize, atomType, oPVContentDownloadable, parsingMode), _pmediaAtom);

                if (!_pmediaAtom->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaAtom->GetMP4Error();
                    break;
                }
                _pmediaAtom->setParent(this);
                count -= _pmediaAtom->getSize();
            }
        }

        if (_success)
        {
            if (NULL == _ptrackHeader)
            {
                _success = false;
                _mp4ErrorCode = NO_TRACK_HEADER_ATOM_PRESENT;
                return;
            }

            if (NULL == _pmediaAtom)
            {
                _success = false;
                _mp4ErrorCode = NO_MEDIA_ATOM_PRESENT;
                return;
            }
            _pMediaType =
                AtomUtils::getMediaTypeFromHandlerType(_pmediaAtom->getMediaHandlerType());
        }
    }
    else
    {
        _mp4ErrorCode = READ_TRACK_ATOM_FAILED;
    }
}

int32 TrackAtom::NEWsetTrackTSOffset(uint32 ts)
{
    if (_pmediaAtom != NULL)
    {
        if (_pEditAtom != NULL)
        {
            uint32 initTimeOffset = (uint32)(_pEditAtom->getInitialTimeOffset());

            float fTS = (float)_pmediaAtom->getMediaTimescale();
            fTS *= (float)initTimeOffset;
            fTS /= (float)ts;
            _pmediaAtom->setTrackTSOffset((uint32)(fTS));
        }
    }
    return EVERYTHING_FINE;
}

int32 TrackAtom::getTrackTSOffset(uint32& aTSOffset, uint32 aMovieTimeScale)
{
    aTSOffset = 0;
    if (_pmediaAtom != NULL)
    {
        if (_pEditAtom != NULL)
        {
            uint32 initTimeOffset = (uint32)(_pEditAtom->getInitialTimeOffset());

            float fTS = (float)_pmediaAtom->getMediaTimescale();
            fTS *= (float)initTimeOffset;
            fTS /= (float)aMovieTimeScale;
            aTSOffset = (uint32)(fTS);
        }
    }
    return EVERYTHING_FINE;
}

// Destructor
TrackAtom::~TrackAtom()
{
    if (_ptrackHeader != NULL)
        PV_MP4_FF_DELETE(NULL, TrackHeaderAtom, _ptrackHeader);

    if (_pmediaAtom != NULL)
        PV_MP4_FF_DELETE(NULL, MediaAtom, _pmediaAtom);

    if (_puserdataatom != NULL)
        PV_MP4_FF_DELETE(NULL, UserDataAtom, _puserdataatom);

    if (_ptrackReference != NULL)
        PV_MP4_FF_DELETE(NULL, TrackReferenceAtom, _ptrackReference);

    if (_pEditAtom != NULL)
        PV_MP4_FF_DELETE(NULL, EditAtom, _pEditAtom);
}






