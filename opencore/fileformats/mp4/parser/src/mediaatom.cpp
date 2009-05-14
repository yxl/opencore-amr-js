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
/*                            MPEG-4 MediaAtom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaAtom Class contains all the objects that declare information
    about the media data within the stream.
*/

#define IMPLEMENT_MediaAtom_H__

#include "mediaatom.h"
#include "atomdefs.h"
#include "atomutils.h"


// Stream-in ctor
MediaAtom::MediaAtom(MP4_FF_FILE *fp,
                     OSCL_wString& filename,
                     uint32 size,
                     uint32 type,
                     bool oPVContentDownloadable,
                     uint32 parsingMode)
        : Atom(fp, size, type)
{
    _pmediaHeader      = NULL;
    _phandler          = NULL;
    _pmediaInformation = NULL;

    if (_success)
    {
        uint32 mediaType = 0;
        uint32 handlerType;

        // Still need to be able to read in these atoms in ANY ORDER!!!
        _pparent = NULL;

        int32 count = _size - DEFAULT_ATOM_SIZE;
        uint32 minf_fp = 0;

        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        // Can read in mdhd, hdlr, minf atoms in ANY order
        while (((atomType == MEDIA_HEADER_ATOM) ||
                (atomType == HANDLER_ATOM) ||
                (atomType == MEDIA_INFORMATION_ATOM) ||
                (atomType == UUID_ATOM) ||
                (atomType == UNKNOWN_ATOM)) &&
                (count > 0))
        {
            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if ((atomType == UUID_ATOM)
                    || (atomType == UNKNOWN_ATOM)
                    || (atomType == MEDIA_INFORMATION_ATOM))
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
                count    -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                if (atomType == MEDIA_INFORMATION_ATOM)
                {
                    minf_fp = AtomUtils::getCurrentFilePosition(fp);
                    minf_fp -= DEFAULT_ATOM_SIZE;
                }
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
            else if (atomType == MEDIA_HEADER_ATOM)
            {
                //"mdhd"
                PV_MP4_FF_NEW(fp->auditCB, MediaHeaderAtom, (fp, atomSize, atomType), _pmediaHeader);//"mdhd"

                // Error checking
                if (!_pmediaHeader->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaHeader->GetMP4Error();
                    return;
                }

                _pmediaHeader->setParent(this);
                count -= _pmediaHeader->getSize();
            }
            else if (atomType == HANDLER_ATOM)
            {
                //"hdlr"

                PV_MP4_FF_NEW(fp->auditCB, HandlerAtom, (fp, atomSize, atomType), _phandler);

                // Error checking
                if (!_phandler->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _phandler->GetMP4Error();
                    return;
                }


                // Need to get media and handler types from handler before can read in media information atom
                //handlerType should be correct now first one fp "vide",
                handlerType = _phandler->getHandlerType();
                //mediaType should be the same as handlerType, if that handlerType do exists
                mediaType   = AtomUtils::getMediaTypeFromHandlerType(handlerType);

                _phandler->setParent(this);
                count -= _phandler->getSize();
            }
        }

        if (minf_fp)
        {
            if ((mediaType == MEDIA_TYPE_AUDIO) ||
                    (mediaType == MEDIA_TYPE_VISUAL) ||
                    (mediaType == MEDIA_TYPE_TEXT))
            {
                int32 filePointer;
                filePointer = AtomUtils::getCurrentFilePosition(fp);

                //move file pointer to the beginning of minf atom
                AtomUtils::seekFromStart(fp, minf_fp);

                PV_MP4_FF_NEW(fp->auditCB, MediaInformationAtom, (fp, mediaType, filename,
                              oPVContentDownloadable,
                              parsingMode),
                              _pmediaInformation);

                // Error checking
                if (!_pmediaInformation->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pmediaInformation->GetMP4Error();
                    return;
                }
                _pmediaInformation->setParent(this);
                //restore file pointer back
                AtomUtils::seekFromStart(fp, filePointer);

                if ((_pmediaHeader == NULL) ||
                        (_phandler == NULL) ||
                        (_pmediaInformation == NULL))

                {
                    _success = false;
                    _mp4ErrorCode = READ_MEDIA_ATOM_FAILED;
                    return;
                }
            }
        }
    }
    else
    {
        _mp4ErrorCode = READ_MEDIA_ATOM_FAILED;
    }
}

// Destructor
MediaAtom::~MediaAtom()
{
    if (_pmediaHeader != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MediaHeaderAtom, _pmediaHeader);
    }

    if (_phandler != NULL)
    {
        PV_MP4_FF_DELETE(NULL, HandlerAtom, _phandler);
    }

    if (_pmediaInformation != NULL)
    {
        PV_MP4_FF_DELETE(NULL, MediaInformationAtom, _pmediaInformation);
    }
}


