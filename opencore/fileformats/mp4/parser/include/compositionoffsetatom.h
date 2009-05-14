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
/*                        MPEG-4 CompositionOffsetAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This atom gives difference between decoding time and composition time on each
    sample basis. This atom is optional and must be present only if Decoding time
    and Composition Time differ for any samples. As understood that Decoding time
    is always less than composition time, the offsets are termed as unsigned
    numbers such.
*/


#ifndef COMPOSITIONOFFSETATOM_H_INCLUDED
#define COMPOSITIONOFFSETATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#include "oscl_scheduler.h"

#define PV_ERROR -1

class CompositionOffsetAtom : public FullAtom,
            public OsclTimerObject
{

    public:
        CompositionOffsetAtom(MP4_FF_FILE *fp,
                              uint32 mediaType,
                              uint32 size,
                              uint32 type,
                              OSCL_wString& filename,
                              uint32 parsingMode = 0);
        virtual ~CompositionOffsetAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        uint32 getSampleCountAt(int32 index);
        int32 getSampleOffsetAt(int32 index);
        int32 getTimeOffsetForSampleNumberPeek(uint32 sampleNum);
        int32 getTimeOffsetForSampleNumber(uint32 num);
        int32 getTimeOffsetForSampleNumberGet(uint32 num);
        void setSamplesCount(uint32 SamplesCount);

        int32 resetStateVariables();
        int32 resetStateVariables(uint32 sampleNum);

        int32 resetPeekwithGet();
        uint32 getCurrPeekSampleCount()
        {
            return _currPeekSampleCount;
        }

        //Marker Table Related Functions
        int32 getTimeOffsetFromMT(uint32 samplenum, uint32 currEC, uint32 currSampleCount);
        int32 createMarkerTable();
        uint32 populateMarkerTable();
        void deleteMarkerTable();

    private:
        bool ParseEntryUnit(uint32 entry_cnt);
        void CheckAndParseEntry(uint32 i);
        // from OsclTimerObject
        void Run();
        uint32 _entryCount;

        uint32 *_psampleCountVec;
        uint32 *_psampleOffsetVec;

        //marker table related
        uint32 *MT_SampleCount;
        uint32 *MT_EntryCount;
        uint32 _iTotalNumSamplesInTrack;
        uint32 MT_Counter;
        uint32 addSampleCount;
        uint32 prevSampleCount;
        uint32 entrycountTraversed;
        uint32 MT_Table_Size;
        bool iMarkerTableCreation;
        uint32 refSample;
        uint32 MT_j;

        uint32 _mediaType;

        // For visual samples
        uint32 _currentTimestamp;

        MP4_FF_FILE *_fileptr;

        MP4_FF_FILE *_curr_fptr;
        uint32 *_stbl_fptr_vec;
        uint32 _stbl_buff_size;
        uint32 _curr_entry_point;
        uint32 _curr_buff_number;
        uint32 _next_buff_number;

        uint32	_parsed_entry_cnt;
        uint32 _currGetSampleCount;
        int32 _currGetIndex;
        int32 _currGetTimeOffset;
        uint32 _currPeekSampleCount;
        int32 _currPeekIndex;
        int32 _currPeekTimeOffset;
        uint32 _parsing_mode;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
};

#endif  // COMPOSITIONOFFSETATOM_H_INCLUDED

