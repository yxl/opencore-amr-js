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
#ifndef __InterleaveBuffer_H__
#define __InterleaveBuffer_H__

#include "atom.h"
#include "a_atomdefs.h"
#include "atomutils.h"

#include "oscl_byte_order.h"
#include "oscl_bin_stream.h"
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

class PVA_FF_InterLeaveBuffer
{

    public:

        PVA_FF_InterLeaveBuffer(uint32 mediaType, uint32 codecType, uint32 trackId);

        virtual	~PVA_FF_InterLeaveBuffer();

        bool	addSampleToInterLeaveBuffer(Oscl_Vector < OsclMemoryFragment,
                                         OsclMemAllocator > & fragmentList,
                                         uint32 size, uint32 ts, uint8 flags, int32 index);

        bool	checkInterLeaveBufferSpace(uint32 size);

        Oscl_Vector<uint32, OsclMemAllocator>* getTimeStampVec();
        Oscl_Vector<uint32, OsclMemAllocator>* getSampleSizeVec();
        Oscl_Vector<uint8, OsclMemAllocator>* getFlagsVec();
        Oscl_Vector<int32, OsclMemAllocator>* getTextIndexVec();

        uint8*	resetInterLeaveBuffer(uint32 &chunkSize);
        uint32	getCurrentInterLeaveBufferSize();

        uint32	getLastChunkEndTime();
        void	setLastChunkEndTime();
        void	setLastChunkEndTime(uint32 time);
        uint32	getTrackID();
        uint32	getFirstTSEntry();
        uint32	getLastSampleTS();

    private:

        uint32	_trackId;
        uint32	_mediaType;
        uint32	_codecType;
        uint32	_lastChunkEndTime;		// use to calculate present trun duration from sample time stamp
        uint8*	_interLeaveBuffer;		// samples copied to buffer before written to file
        uint32	_maxInterLeaveBufferSize;	// max size if interleave buffer ( codec dependent)
        uint32	_currInterLeaveBufferSize;	// current occupancy in interleave buffer
        uint32	_lastInterLeaveBufferTS;	// Time stamp of last interleave buffer
        // to calculate duration of interleave buffer
        uint32	_lastSampleTS;



        // Sample parameters are stored in these vectors before writing to TRUN when
        // interleave duration is reached
        Oscl_Vector<uint32, OsclMemAllocator>* _pTimeStampVec;
        Oscl_Vector<uint32, OsclMemAllocator>* _pSampleSizeVec;
        Oscl_Vector<uint8, OsclMemAllocator>* _pSampleFlagsVec;
        Oscl_Vector<int32, OsclMemAllocator>* _pIndexVec;

};

#endif
