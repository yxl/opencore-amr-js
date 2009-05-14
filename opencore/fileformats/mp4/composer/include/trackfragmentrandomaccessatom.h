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
#ifndef __TrackFragmentRandomAccessAtom_H__
#define __TrackFragmentRandomAccessAtom_H__

#include "fullatom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#define	TFRA_LENGTH_SIZE	0x15	// all fields are of 2 bytes

struct PVA_FF_RandomAccessEntry
{
    uint32	time;
    uint32	moofOffset;
    uint16	trafNumber;
    uint16	trunNumber;
    uint16	sampleNumber;
};

class PVA_FF_TfraAtom : public PVA_FF_FullAtom
{
    public:
        PVA_FF_TfraAtom(uint32 trackId);

        virtual ~PVA_FF_TfraAtom();

        void	addSampleEntry(uint32 ts, uint32 moofOffset,
                            uint32 trafNumber, uint32 trunNumber,
                            uint32 sampleNumber);

        uint32	getTrackId();

        void	updateMoofOffset(uint32 offset);

        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP* fp);

    private:

        uint32														_trackId;

        // initialied for 2 bytes size in constructor
        uint32														_reserved;

        uint32														_entryCount;

        uint32														_moofUpdateSample;	// contains sample number from
        // where moof offset is to be updated
        // when current moof is rendered
        Oscl_Vector<PVA_FF_RandomAccessEntry, OsclMemAllocator>		*_pSampleEntries;
        virtual void recomputeSize();
};

#endif
