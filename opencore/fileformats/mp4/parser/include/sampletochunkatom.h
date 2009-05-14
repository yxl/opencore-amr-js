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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                         MPEG-4 SampleToChunkAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SampleSizeAtom Class contains the sample count and a table giving the
    size of each sample.
*/


#ifndef SAMPLETOCHUNKATOM_H_INCLUDED
#define SAMPLETOCHUNKATOM_H_INCLUDED

#define PV_ERROR -1

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class SampleToChunkAtom : public FullAtom
{

    public:
        SampleToChunkAtom(MP4_FF_FILE *fp, uint32 size, uint32 type, OSCL_wString& filename, uint32 parsingMode);
        virtual ~SampleToChunkAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        uint32 getNextChunkNumber()
        {
            return _currentChunkNumber++;
        }

        int32 getFirstChunkAt(uint32 index);
        int32 getSamplesPerChunkAt(uint32 index) ;

        uint32 getSDIndex() const;
        // Returns the chunk number for the given sample number
        uint32 getChunkNumberForSample(uint32 sampleNum);
        uint32 getChunkNumberForSampleGet(uint32 sampleNum);

        // Returns the number of the first sample in chunk number 'chunk'
        uint32 getFirstSampleNumInChunk(uint32 chunkNum) ;
        uint32 getFirstSampleNumInChunkPeek() const;

        int32 getMaxNumSamlplesPerChunk()
        {
            return _maxNumSamplesPerChunk;
        }
        void setMaxNumSamlplesPerChunk(int32 max)
        {
            _maxNumSamplesPerChunk = max;
        }
        int32 getMaxChunkDataSize()
        {
            return _maxChunkDataSize;
        }
        void setMaxChunkDataSize(int32 max)
        {
            _maxChunkDataSize = max;
        }

        uint32 getNumChunksInRunofChunks(uint32 chunk) ;
        uint32 getSamplesPerChunkCorrespondingToSample(uint32 sampleNum) ;
        uint32 getSamplesPerChunkCorrespondingToSampleGet() const;
        int32 resetStateVariables();
        int32 resetStateVariables(uint32 sampleNum);
        uint32 getFirstSampleNumInChunkGet() const;
        uint32 getChunkNumberForSamplePeek(uint32 sampleNum);
        uint32 getNumChunksInRunofChunksGet() const;
        uint32 getSDIndexPeek() const;
        uint32 getSDIndexGet() const;
        int32 resetPeekwithGet();
        uint32 getCurrPeekSampleCount()
        {
            return _currPeekSampleCount;
        }

    private:

        bool ParseEntryUnit(uint32 sample_cnt);
        void CheckAndParseEntry(uint32 i);
        uint32 _entryCount;
        uint32 *_pfirstChunkVec;
        uint32 *_psamplesPerChunkVec;
        uint32 *_psampleDescriptionIndexVec;

        int32 _mediaType;

        uint32 _currentChunkNumSamples;
        uint32 _currentChunkDataSize; // Ad up all sample sizes for the current chunk
        uint32 _currentChunkNumber;

        int32 _maxNumSamplesPerChunk;
        int32 _maxChunkDataSize;

        uint32 _Index;

        int32 _numChunksInRun;
        int32 _majorGetIndex;
        int32 _currGetChunk;
        int32 _numGetChunksInRun;
        uint32 _currGetSampleCount;
        int32 _firstGetSampleInCurrChunk;
        int32 _numGetSamplesPerChunk;
        int32 _currGetSDI;

        int32 _majorPeekIndex;
        int32 _currPeekChunk;
        int32 _numPeekChunksInRun;
        uint32 _currPeekSampleCount;
        int32 _firstPeekSampleInCurrChunk;
        int32 _numPeekSamplesPerChunk;
        int32 _currPeekSDI;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
        uint32 _parsing_mode;

        MP4_FF_FILE *_fileptr;
        uint32	_parsed_entry_cnt;
        MP4_FF_FILE *_curr_fptr;
        uint32 *_stbl_fptr_vec;
        uint32 _stbl_buff_size;
        uint32 _curr_entry_point;
        uint32 _curr_buff_number;
        uint32 _next_buff_number;



};


#endif // SAMPLETOCHUNKATOM_H_INCLUDED

