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
/*                            MPEG-4 TrackReferenceAtom Class                    */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/


#define IMPLEMENT_TrackReferenceAtom

#include "trackreferenceatom.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in Constructor
TrackReferenceAtom::TrackReferenceAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    _ptrackReferenceTypeAtom = NULL;

    if (_success)
    {
        _pparent = NULL;

        uint32 count = _size - DEFAULT_ATOM_SIZE;

        while (count > 0)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            // Currently we support only tref type "dpnd"
            if (atomType == DPND_TRACK_REFERENCE_TYPE)
            {
                if (_ptrackReferenceTypeAtom == NULL)
                {
                    PV_MP4_FF_NEW(fp->auditCB, TrackReferenceTypeAtom, (fp, atomSize, atomType), _ptrackReferenceTypeAtom);

                    if (!_ptrackReferenceTypeAtom->MP4Success())
                    {
                        _success = false;
                        _mp4ErrorCode = _ptrackReferenceTypeAtom->GetMP4Error();
                        return;
                    }
                    else
                    {
                        _ptrackReferenceTypeAtom->setParent(this);
                        count -= _ptrackReferenceTypeAtom->getSize();
                    }
                }
                else
                {
                    // Multiple "dpnd" - skip
                    count -= atomSize;
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            else
            {
                count -= atomSize;
                atomSize -= DEFAULT_ATOM_SIZE;
                AtomUtils::seekFromCurrPos(fp, atomSize);
            }
        }
    }
    else
    {
        _mp4ErrorCode = READ_TRACK_REFERENCE_ATOM_FAILED;
    }
}

// Destructor
TrackReferenceAtom::~TrackReferenceAtom()
{
    if (_ptrackReferenceTypeAtom != NULL)
    {
        PV_MP4_FF_DELETE(NULL, TrackReferenceTypeAtom, _ptrackReferenceTypeAtom);
    }
}


