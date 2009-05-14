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
#include "filetypeatom.h"
#include "atomutils.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

OSCL_EXPORT_REF FileTypeAtom::FileTypeAtom(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    uint32 i, brandName, atomSize;

    _pCompatible_brand = NULL;
    _pMajorBrand = 0;
    _pVersion = 0;
    if (_success)
    {
        PV_MP4_FF_NEW(fp->auditCB, uint32VecType, (), _pCompatible_brand);


        // ftype size is only 8. In that case we should not read beyond the atom boundry blindly
        atomSize = getSize();
        int32 count = atomSize;
        count -= DEFAULT_ATOM_SIZE;

        if (atomSize >= MINIMUM_SIZE_REQUIRED_TO_READ_MAJOR_BRAND)
        {
            AtomUtils::read32(fp, _pMajorBrand);
            count -= 4;
        }

        // Read only when it is inside the atom size specified. dont just assume that it is present
        if (atomSize >= MINIMUM_SIZE_REQUIRED_TO_READ_MINOR_VERSION)
        {
            AtomUtils::read32(fp, _pVersion);
            count -= 4;
        }

        //subtract 16 bytes, 4 from tag and 4 from size 4 from Brand and 4 from Version, now I assume
        //the size if the total atom

        if (atomSize >= 16)
        {
            for (i = 0; i < atomSize - 16; i += 4)
            {
                if (count >= 4)
                {
                    AtomUtils::read32(fp, brandName);
                    (*_pCompatible_brand).push_back(brandName);
                    count -= 4;
                }
            }
        }
        if (count > 0)
        {
            AtomUtils::seekFromCurrPos(fp, count);
        }
    }
    else
    {
        _success = false;
        _mp4ErrorCode = READ_FILE_TYPE_ATOM_FAILED;
    }
}


// Destructor
OSCL_EXPORT_REF FileTypeAtom::~FileTypeAtom()
{
    if (_pCompatible_brand != NULL)
        PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pCompatible_brand);
}

