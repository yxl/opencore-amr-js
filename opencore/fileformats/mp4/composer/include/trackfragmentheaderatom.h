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
#ifndef __TrackFragmentHeaderAtom_H__
#define __TrackFragmentHeaderAtom_H__

#include "fullatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#define	FLAGS_TFHD_ATOM		0x000001	// only base data offset present


class PVA_FF_TrackFragmentHeaderAtom : public PVA_FF_FullAtom
{


    public:
        PVA_FF_TrackFragmentHeaderAtom(uint32 trackId);

        virtual ~PVA_FF_TrackFragmentHeaderAtom();

        void	setHeaderData();	// parameters will be introduced later (not in use)
        void	setBaseDataOffset(uint64 offset);
        uint32	getTrackId();
        uint32	getBaseDataOffset();

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

    private:
        uint32	_trackId;
        uint64 	_baseDataOffset;
        uint32	_sampleDescriptionIndex;
        uint32	_defaultSampleDuration;
        uint32	_defaultSampleSize;
        uint32	_defaultSampleFlags;

        virtual void recomputeSize();
};

#endif
