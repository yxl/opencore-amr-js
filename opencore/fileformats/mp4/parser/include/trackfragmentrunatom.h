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
/*                            MPEG-4 Track Fragment Run Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#ifndef TRACKFRAGMENTRUNATOM_H_INCLUDED
#define TRACKFRAGMENTRUNATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

class TFrunSampleTable
{
    public:
        TFrunSampleTable(MP4_FF_FILE *fp , uint32 tr_flag, uint32 base_data_offset,
                         uint64 sampleTS);

        void setDefaultDuration(uint32 sampleTS, uint32 default_duration)
        {
            _sample_timestamp = sampleTS;
            _sample_duration = default_duration;
        }
        void setDefaultSampleSize(uint32 sampleoffset, uint32 default_samplesize)
        {
            _sample_offset = sampleoffset;
            _sample_size = default_samplesize;
        }

        ~TFrunSampleTable() {};

        uint32 _sample_duration;
        uint32 _sample_size;
        uint32 _sample_flags;
        uint32 _sample_composition_time_offset;
        uint32 _sample_offset;
        uint64 _sample_timestamp;
    private:

};

class TrackFragmentRunAtom : public FullAtom
{

    public:
        TrackFragmentRunAtom(MP4_FF_FILE *fp,
                             uint32 size,
                             uint32 type,
                             uint32 base_data_offset,
                             uint32 &currentTrunOffset,
                             uint32 &offset,
                             uint64 trackDuration,
                             bool bdo_present,
                             bool &trunParsingCompleted,
                             uint32 &countOfTrunsParsed);

        virtual ~TrackFragmentRunAtom();

        void ParseTrunAtom(MP4_FF_FILE *fp,
                           uint32 &offset,
                           bool &trunParsingCompleted,
                           uint32 &countOfTrunsParsed);

        uint64 getDataOffset()
        {
            return _data_offset;
        }
        uint32 getSampleCount()
        {
            return _sample_count;
        }

        Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>* getSampleTable()
        {
            return _pTFrunSampleTable;
        }
        uint32 _sample_offset;
        uint64 _sampleTimeStamp;
        void setDefaultDuration(uint32 default_duration);
        void setDefaultSampleSize(uint32 default_samplesize, uint32& sigmaSampleSize);
        void setSampleDurationAndTimeStampFromSampleNum(uint32 startSampleNum, uint32 startDuration, uint32 default_duration);

    private:
        uint32 tr_flag;
        uint32 _sample_count;
        uint32 _data_offset;
        uint32 _first_sample_flags;
        Oscl_Vector<TFrunSampleTable*, OsclMemAllocator>  *_pTFrunSampleTable;
        uint32 _version;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
        uint32 _trun_start_offset;
        uint32 _samplesToBeParsed;
        uint32 _partialTrunOffset;

};

#endif
