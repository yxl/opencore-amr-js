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
/*                       MPEG-4 DataInformationAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DataInformationAtom Class contains objects that declare the location
    of the media information within the stream.
*/


#define IMPLEMENT_DataInformationAtom

#include "datainformationatom.h"
#include "atomdefs.h"
#include "atomutils.h"


// Stream-in ctor
DataInformationAtom::DataInformationAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _pdataReferenceAtom = NULL;

    if (_success)
    {
        _pparent = NULL;
        _success = true;

        uint32 atomType = UNKNOWN_ATOM;
        uint32 atomSize = 0;

        AtomUtils::getNextAtomType(fp, atomSize, atomType);

        if (atomType == DATA_REFERENCE_ATOM)
        {
            PV_MP4_FF_NEW(fp->auditCB, DataReferenceAtom, (fp, atomSize, atomType), _pdataReferenceAtom);
            if (!_pdataReferenceAtom->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = _pdataReferenceAtom->GetMP4Error();
            }
            else
            {
                _pdataReferenceAtom->setParent(this);
            }
        }
        else if ((atomType == UNKNOWN_ATOM) ||
                 (atomType == UUID_ATOM))
        {
            if (atomSize < DEFAULT_ATOM_SIZE)
            {
                _success = false;
                _mp4ErrorCode = ZERO_OR_NEGATIVE_ATOM_SIZE;
                return;
            }
            if (atomSize > _size)
            {
                _success = false;
                _mp4ErrorCode = READ_FAILED;
                return;
            }
            AtomUtils::seekFromCurrPos(fp, (atomSize - DEFAULT_ATOM_SIZE));
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_DATA_INFORMATION_ATOM_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_DATA_INFORMATION_ATOM_FAILED;
    }
}



DataInformationAtom::~DataInformationAtom()
{
    if (_pdataReferenceAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, DataReferenceAtom, _pdataReferenceAtom);
    }
}




