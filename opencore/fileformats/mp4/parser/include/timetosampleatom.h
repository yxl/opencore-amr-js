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
/*                        MPEG-4 TimeToSampleAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TimeToSampleAtom Class contains a compact version of a table that allows
    indexing from decoding to sample number.
*/


#ifndef TIMETOSAMPLEATOM_H_INCLUDED
#define TIMETOSAMPLEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#define PV_ERROR -1

class TimeToSampleAtom : public FullAtom
{

    public:
        TimeToSampleAtom(MP4_FF_FILE *fp,
                         uint32 mediaType,
                         uint32 size,
                         uint32 type,
                         OSCL_wString& filename,
                         uint32 parsingMode = 0);
        virtual ~TimeToSampleAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        uint32 getSampleCountAt(int32 index);
        int32 getSampleDeltaAt(int32 index);
        int32 getSampleNumberFromTimestamp(uint32 ts,
                                           bool oAlwaysRetSampleCount = false);
        int32 getTimeDeltaForSampleNumber(uint32 num);
        int32 getTimestampForSampleNumber(uint32 num);

        int32 getTimeDeltaForSampleNumberPeek(uint32 num);
        int32 getTimeDeltaForSampleNumberGet(uint32 num);

        int32 resetStateVariables();
        int32 resetStateVariables(uint32 sampleNum);

        int32 resetPeekwithGet();
        uint32 getCurrPeekSampleCount()
        {
            return _currPeekSampleCount;
        }



    private:
        bool ParseEntryUnit(uint32 entry_cnt);
        void CheckAndParseEntry(uint32 i);
        uint32 _entryCount;

        uint32 *_psampleCountVec;
        uint32 *_psampleDeltaVec;

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
        int32 _currGetTimeDelta;
        uint32 _currPeekSampleCount;
        int32 _currPeekIndex;
        int32 _currPeekTimeDelta;
        uint32 _parsing_mode;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;

};

#endif  // TIMETOSAMPLEATOM_H_INCLUDED

