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
#ifndef __MovieExtendsAtom_H__
#define __MovieExtendsAtom_H__

#include "atom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#include "movieextendsheaderatom.h"
#include "trackextendsatom.h"


class PVA_FF_MovieExtendsAtom : public PVA_FF_Atom
{

    public:

        PVA_FF_MovieExtendsAtom(); 				// Constructor
        virtual ~PVA_FF_MovieExtendsAtom();		// destructor


        // Member gets and sets

        // add and retrieve Trex atom for track ID
        PVA_FF_TrackExtendsAtom* getTrexAtom(uint32 trackId);
        void addTrexAtom(PVA_FF_TrackExtendsAtom *pTrexAtom);

        //get movie extend header (to update fragment duration)
        PVA_FF_MovieExtendsHeaderAtom* getMehdAtom();
        void	setMovieFragmentDuration(uint32 duration);
        void	updateMovieFragmentDuration(uint32 ts);
        void	writeMovieFragmentDuration(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);
        uint32	getDuration();

        // Rendering the PVA_FF_MovieExtendsAtom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

    private:

        // child atoms
        PVA_FF_MovieExtendsHeaderAtom*								_pMehdAtom;
        Oscl_Vector <PVA_FF_TrackExtendsAtom*, OsclMemAllocator>*	_pTrexAtomVec;

        // functions needed by composer to update and write atoms into file
        virtual void recomputeSize();
};

#endif

