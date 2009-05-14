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
#include "editatom.h"
#include "atomutils.h"
#include "atomdefs.h"

typedef Oscl_Vector<EditListAtom*, OsclMemAllocator> editListAtomVecType;

//WMF
// Stream-in Constructor
EditAtom::EditAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _pEditListVec = NULL;

    if (_success)
    {
        int32 dataLength;

        PV_MP4_FF_NEW(fp->auditCB, editListAtomVecType, (), _pEditListVec);

        //multiple editlist atom within one edit atom
        dataLength = getSize() - 8;
        while (dataLength > 0)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == EDIT_LIST_ATOM)
            {
                EditListAtom *_pEditList = NULL;

                PV_MP4_FF_NEW(fp->auditCB, EditListAtom, (fp, atomSize, atomType), _pEditList);

                if (!_pEditList->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pEditList->GetMP4Error();
                    PV_MP4_FF_DELETE(NULL, EditListAtom, _pEditList);
                    break;
                }
                (*_pEditListVec).push_back(_pEditList);
                dataLength -= _pEditList->getSize();
            }
            else
            {
                _success = false;
                _mp4ErrorCode = READ_UNKNOWN_ATOM;
                break;
            }

        }
    }
}

uint32 EditAtom::getInitialTimeOffset()
{
    if (_pEditListVec == NULL)
    {
        return 0;
    }

    //only assume one edit list atom in edit atom, so get element 0
    if (_pEditListVec->size() != 0)
    {
        return (*_pEditListVec)[0]->getInitialTimeOffset();
    }
    else
    {
        return 0;
    }
}

// Destructor
EditAtom::~EditAtom()
{
    uint32 i;

    if (_pEditListVec != NULL)
    {
        for (i = 0; i < _pEditListVec->size(); i++)
            PV_MP4_FF_DELETE(NULL, EditListAtom, (*_pEditListVec)[i]);

        PV_MP4_FF_TEMPLATED_DELETE(NULL, editListAtomVecType, Oscl_Vector, _pEditListVec);
    }
}
