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
/*                             MPEG-4 ESDAtom Class                              */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ESDAtom Class provides the offset between decoding
    time and composition time.
*/


#define IMPLEMENT_ESDAtom

#include "esdatom.h"
#include "atomdefs.h"
#include "atomutils.h"


ESDAtom::ESDAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pdescriptor = NULL;

    if (_success)
    {
        _pparent = NULL;

        uint8 descr_tag = AtomUtils::peekNextByte(fp);

        if (descr_tag == ES_DESCRIPTOR_TAG)
        {
            PV_MP4_FF_NEW(fp->auditCB, ESDescriptor, (fp), _pdescriptor);

            if (!_pdescriptor->MP4Success())
            {
                _success = false;
                _mp4ErrorCode = _pdescriptor->GetMP4Error();
            }
            else
            {
                _pdescriptor->setParent(this);
            }
        }
        else
        {
            _success = false;
            _mp4ErrorCode = READ_ESD_ATOM_FAILED;
        }
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_ESD_ATOM_FAILED;
    }
}

// Destructor
ESDAtom::~ESDAtom()
{
    if (_pdescriptor != NULL)
    {
        // Cleanup the ESDescriptor
        PV_MP4_FF_DELETE(NULL, ESDescriptor, _pdescriptor);
    }
}


