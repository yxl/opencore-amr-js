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
/*                           MPEG-4 SampleSizeAtom Class                         */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SampleSizeAtom Class contains the sample count and a table giving the
    size of each sample.
*/


#ifndef SAMPLESIZEATOM_H_INCLUDED
#define SAMPLESIZEATOM_H_INCLUDED

#define PV_ERROR -1

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class SampleSizeAtom : public FullAtom
{

    public:
        SampleSizeAtom(MP4_FF_FILE *fp, uint32 mediaType,
                       uint32 size, uint32 type,
                       OSCL_wString& filename, uint32 parsingMode = 0);
        virtual ~SampleSizeAtom();

        // Member gets and sets
        int32 getDefaultSampleSize() const
        {
            return _sampleSize;
        }
        uint32 getSampleCount() const
        {
            return _sampleCount;
        }

        int32 getSampleSizeAt(uint32 index);

        int32 getMaxSampleSize()
        {
            return _maxSampleSize;
        }

        bool _SkipOldEntry;

    private:
        bool ParseEntryUnit(uint32 sample_cnt);

        int32 _sampleSize;
        uint32 _sampleCount;

        int32 *_psampleSizeVec;

        int32 _mediaType;

        int32 _maxSampleSize;
        MP4_FF_FILE *_fileptr;
        uint32	_parsed_entry_cnt;

        MP4_FF_FILE *_curr_fptr;
        uint32 *_stbl_fptr_vec;
        uint32 _stbl_buff_size;
        uint32 _curr_entry_point;
        uint32 _curr_buff_number;
        uint32 _next_buff_number;
        uint32 _parsing_mode;
};

#endif // SAMPLESIZEATOM_H_INCLUDED


