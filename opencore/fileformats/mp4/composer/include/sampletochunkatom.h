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
    This PVA_FF_SampleSizeAtom Class contains the sample count and a table giving the
    size of each sample.
*/


#ifndef __SampleToChunkAtom_H__
#define __SampleToChunkAtom_H__

#define PV_ERROR -1

#include "fullatom.h"


class PVA_FF_SampleToChunkAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_SampleToChunkAtom(uint32 mediaType,
                                 uint32 fileAuthoringFlags); // Constructor
        virtual ~PVA_FF_SampleToChunkAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        // Returns whether or not the new sample is the start of a new chunk
        bool nextSample(int32 index, uint32 size = 0, bool oChunkStart = false);

        uint32 getNextChunkNumber()
        {
            return _currentChunkNumber++;
        }

        int32 getSampleDescriptionIndexAt(uint32 index);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        // Adding to and getting first chunk, samples per chunk, and sample
        // description index values
        void addChunkEntry(uint32 chunk, uint32 samples, uint32 index);
        void replaceLastChunkEntry(uint32 chunk, uint32 samples, uint32 index);
        bool isNewChunk(uint32 size, int32 index);

        virtual void recomputeSize();

        uint32 _entryCount;
        Oscl_Vector<uint32, OsclMemAllocator>* _pfirstChunkVec;
        Oscl_Vector<uint32, OsclMemAllocator>* _psamplesPerChunkVec;
        Oscl_Vector<uint32, OsclMemAllocator>* _psampleDescriptionIndexVec;

        int32 _mediaType;

        uint32 _currentChunkNumSamples;
        uint32 _currentChunkDataSize; // Ad up all sample sizes for the current chunk
        uint32 _currentChunkNumber;

        uint32 _maxNumSamplesPerChunk;
        uint32 _maxChunkDataSize;
        int32 _currIndex;

        static const int32 DEFAULT_MAX_NUM_SAMPLES_PER_CHUNK;
        static const int32 DEFAULT_MAX_CHUNK_DATA_SIZE;

        bool _oInterLeaveMode;
};


#endif

