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
#ifndef __TrackFragmentRunAtom_H__
#define __TrackFragmentRunAtom_H__

#include "fullatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#define TRACK_RUN_ATOM_FLAGS	0x00000701	// flag specify that only data offset, sample duration,
// sample size and sample flags are present

#define KEY_FRAME_ENTRY				0x010000

struct PVA_FF_TrackRunSample
{
    uint32		sampleDuration;
    uint32		sampleSize;
    uint32		sampleFlags;
    uint32		sampleCompositionTimeOffset;	// not in use
};

class PVA_FF_TrackFragmentRunAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_TrackFragmentRunAtom();

        virtual ~PVA_FF_TrackFragmentRunAtom();

        void	setFlags(uint32 flags);
        void	setDataOffset(uint32 offset);
        void	addSample(uint32 size, uint32 ts, uint8 flags);
        void	updateLastTSEntry(uint32 ts);

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);
        uint32 getSampleCount() const
        {
            return _sampleCount;
        }
    private:

        uint32		_currentTimestamp;
        uint8		_firstEntry;

        uint32		_sampleCount;
        uint32		_dataOffset;
        uint32		_firstSampleFlags;
        bool		_lastTSupdated;

        Oscl_Vector<PVA_FF_TrackRunSample, OsclMemAllocator>* _psampleEntriesVec;

        virtual void recomputeSize();
};

#endif
