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
#include "metadataatom.h"
#include "atomdefs.h"


MetaDataAtom::MetaDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type): Atom(fp, size, type)
{
    _success = true;

    // User ilst Data
    _pITunesILSTAtom = NULL;
    _pHdlrAtom = NULL;
    uint32 _count = _size - getDefaultSize();

    uint32 data_32_hdlr = 0;
    iLogger = PVLogger::GetLoggerObject("mp4ffparser");

    // Skip first 4 bytes.
    if (!AtomUtils::read32(fp, data_32_hdlr))
    {
        _success = false;
        _mp4ErrorCode = READ_META_DATA_FAILED;
        PVMF_MP4FFPARSER_LOGERROR((0, "ERROR =>MetaDataAtom::MetaDataAtom READ_META_DATA_FAILED   if(!AtomUtils::read32(fp,data_32_hdlr)))"));
        return;
    }
    _count -= 4;

    while (_count > 0)
    {
        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        uint32 currPtr = AtomUtils::getCurrentFilePosition(fp);
        AtomUtils::getNextAtomType(fp, atomSize, atomType);

        if ((atomType == FREE_SPACE_ATOM) || (atomType == UNKNOWN_ATOM))
        {
            //skip the atom
            if (atomSize < DEFAULT_ATOM_SIZE)
            {
                _success = false;
                _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                break;
            }
            if (_count < atomSize)
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, _count);
                _count = 0;
                return;
            }

            _count -= atomSize;
            atomSize -= DEFAULT_ATOM_SIZE;
            AtomUtils::seekFromCurrPos(fp, atomSize);
        }
        else if (atomType == HANDLER_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, HandlerAtom, (fp, atomSize, atomType), _pHdlrAtom);

            if (!_pHdlrAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomSize);
                PV_MP4_FF_DELETE(NULL, HandlerAtom, _pHdlrAtom);
                _pHdlrAtom = NULL;
                _count -= atomSize;
            }

            if (_pHdlrAtom != NULL)
            {
                if (_pHdlrAtom->getHandlerType() != ITUNES_MDIRAPPL_HDLR_PART1)
                {
                    // Skip the parsing...
                    fp->_pvfile.Seek(atomSize, Oscl_File::SEEKCUR);
                    return;
                }
                _count -= _pHdlrAtom->getSize();
            }
        }
        // Read the ilst Atom
        else if (atomType == ITUNES_ILST_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, ITunesILSTAtom, (fp, atomSize, atomType), _pITunesILSTAtom);

            if (!_pITunesILSTAtom->MP4Success())
            {
                AtomUtils::seekFromStart(fp, currPtr);
                AtomUtils::seekFromCurrPos(fp, atomSize);
                PV_MP4_FF_DELETE(NULL, ITunesILSTAtom, _pITunesILSTAtom);
                _pITunesILSTAtom = NULL;
                _count -= atomSize;
            }
            else
                _count -= _pITunesILSTAtom->getSize();
        }
    }
}

MetaDataAtom::~MetaDataAtom()
{
    if (_pHdlrAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, HandlerAtom, _pHdlrAtom);
    }
    if (_pITunesILSTAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, ITunesILSTAtom, _pITunesILSTAtom);
    }
}
