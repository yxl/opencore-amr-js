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
#ifndef FILETYPEATOM_H_INCLUDED
#define FILETYPEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#define  MINIMUM_SIZE_REQUIRED_TO_READ_MAJOR_BRAND 12
#define  MINIMUM_SIZE_REQUIRED_TO_READ_MINOR_VERSION 16

class FileTypeAtom : public Atom
{

    public:
        OSCL_IMPORT_REF FileTypeAtom(MP4_FF_FILE *fp, uint32 atomSize, uint32 atomType); // stream-in constructor
        OSCL_IMPORT_REF virtual ~FileTypeAtom();

        OSCL_IMPORT_REF uint32 getMajorBrand()
        {
            return _pMajorBrand;
        }
        uint32 getMajorBrandVersion()
        {
            return _pVersion;
        }
        OSCL_IMPORT_REF Oscl_Vector<uint32, OsclMemAllocator> *getCompatibleBrand()
        {
            return _pCompatible_brand;
        }

    private:
        uint32 	_pMajorBrand;
        uint32	_pVersion;
        Oscl_Vector<uint32, OsclMemAllocator> *_pCompatible_brand;
};


#endif // FILETYPEATOM_H_INCLUDED

