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
/*                        MPEG-4 ObjectDescriptorAtom Class                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ObjectDescriptorAtom Class contains an ObjectDescriptor or an
    InitialObjectDescriptor.
*/


#define IMPLEMENT_ObjectDescriptorAtom

#include "objectdescriptoratom.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in Constructor
ObjectDescriptorAtom::ObjectDescriptorAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : FullAtom(fp, size, type)
{
    _pOD = NULL;

    if (_success)
    {
        _pparent = NULL;

        PV_MP4_FF_NEW(fp->auditCB, InitialObjectDescriptor, (fp), _pOD);

        if (!_pOD->MP4Success())
        {
            _success = false;
            _mp4ErrorCode = _pOD->GetMP4Error();
        }

        if (_success)
            _pOD->setParent(this);
    }
    else
    {
        if (_mp4ErrorCode != ATOM_VERSION_NOT_SUPPORTED)
            _mp4ErrorCode = READ_OBJECT_DESCRIPTOR_ATOM_FAILED;
    }

}

// Destructor
ObjectDescriptorAtom::~ObjectDescriptorAtom()
{
    // Cleanup InitialObjectDescriptor
    if (_pOD != NULL)
    {
        PV_MP4_FF_DELETE(NULL, InitialObjectDescriptor, _pOD);
    }
}


