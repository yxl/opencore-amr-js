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
/*
	This PVA_FF_SampleEntry Class is the base class for the HinstSampleEntry,
	PVA_FF_VisualSampleEntry, PVA_FF_AudioSampleEntry. and PVA_FF_MpegSampleEntry classes.
*/

#ifndef __FileTypeAtom_H__
#define __FileTypeAtom_H__

#include "atom.h"
#include "atomutils.h"

typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;

class PVA_FF_FileTypeAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_FileTypeAtom(); // Constructor

        virtual ~PVA_FF_FileTypeAtom()
        {
            if (_pCompatibleBrandVec != NULL)
            {
                PV_MP4_FF_TEMPLATED_DELETE(NULL, uint32VecType, Oscl_Vector, _pCompatibleBrandVec);
            }
        }

        void setMajorBrand(uint32 brand)
        {
            _Brand = brand;
        }
        void setMajorBrandVersion(uint32 version)
        {
            _Version = version;
        }
        void addCompatibleBrand(uint32 brand);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        virtual void recomputeSize();

    protected:
        uint32 _Brand;
        uint32 _Version;

        Oscl_Vector<uint32, OsclMemAllocator>* _pCompatibleBrandVec;
};

#endif
