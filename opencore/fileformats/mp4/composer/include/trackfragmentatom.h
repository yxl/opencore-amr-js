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
#ifndef __TrackFragmentAtom_H__
#define __TrackFragmentAtom_H__

#include "atom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#include "trackfragmentheaderatom.h"
#include "trackfragmentrunatom.h"

#include "oscl_byte_order.h"
#include "oscl_bin_stream.h"
typedef Oscl_Vector<uint32, OsclMemAllocator> uint32VecType;
typedef Oscl_Vector<uint8, OsclMemAllocator> uint8VecType;


class PVA_FF_TrackFragmentAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_TrackFragmentAtom(uint32 mediaType, uint32 codecType, uint32 trackId,
                                 uint32 interleaveDuration, uint32 timescale);

        virtual	~PVA_FF_TrackFragmentAtom();

        PVA_FF_TrackFragmentRunAtom*	addTrackRun();

        void addSampleToFragment(uint32 size, uint32 ts, uint8 flags,
                                 uint32 baseOffset, bool otrunStart);

        uint32	getTrackId();

        uint32	getFragmentDuration();

        uint32	getTrunNumber();

        void	setTrackFragmentHeaderData(uint32 flags);

        void	setTrackFragmentBaseDataOffset(uint64 offset);

        void	updateLastTSEntry(uint32 ts);

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

        uint32 getSampleCount() const
        {
            uint32 lSampleCount = 0;
            for (uint32 ii = 0; ii < _pTrunList->size(); ii++)
            {
                lSampleCount +=	((*_pTrunList)[ii])->getSampleCount();
            }
            return lSampleCount;
        }

    private:

        PVA_FF_TrackFragmentHeaderAtom	*_pTfhdAtom;
        Oscl_Vector <PVA_FF_TrackFragmentRunAtom*, OsclMemAllocator> *_pTrunList;		// list of TRUNs in this fragment

        int32 _codecType;
        int32 _mediaType;

        uint32	_timescale;
        bool	_firstEntry;
        uint32	_prevTS;
        uint32	_interleaveDuration;	// determines duration of each TRUN
        uint32	_fragmentDuration;		// determines the duration of samples in this fragment
        bool	_ofirstTrun;

        virtual void recomputeSize();

};

#endif
