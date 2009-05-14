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
#ifndef __MovieFragmentRandomAccessAtom_H__
#define __MovieFragmentRandomAccessAtom_H__

#include "atom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#include "trackfragmentrandomaccessatom.h"
#include "moviefragmentrandomaccessoffsetatom.h"


class PVA_FF_MovieFragmentRandomAccessAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_MovieFragmentRandomAccessAtom();

        virtual ~PVA_FF_MovieFragmentRandomAccessAtom();

        void	addTrackFragmentRandomAccessAtom(uint32 trackId);

        PVA_FF_TfraAtom*	getTfraAtom(uint32 trackId);

        void	addSampleEntry(uint32 trackId, uint32 time, uint32 moofOffset,
                            uint32 trafNumber, uint32 trunNumber,
                            uint32 sampleNumber);

        void	updateMoofOffset(uint32 offset);

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

    private:
        PVA_FF_MfroAtom										*_pMfroAtom;
        Oscl_Vector <PVA_FF_TfraAtom*, OsclMemAllocator>	*_pTfraList;

        virtual void recomputeSize();
};

#endif
