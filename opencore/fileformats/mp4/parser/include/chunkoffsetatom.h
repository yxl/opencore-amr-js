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
    This ChunkOffsetAtom Class gives the index of each chunk into the
    containing file.
*/


#ifndef CHUNKOFFSETATOM_H_INCLUDED
#define CHUNKOFFSETATOM_H_INCLUDED

#define PV_ERROR -1

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class ChunkOffsetAtom : public FullAtom
{

    public:
        ChunkOffsetAtom(MP4_FF_FILE *fp, uint32 size, uint32 type,
                        OSCL_wString& filename,
                        uint32 parsingMode);
        virtual ~ChunkOffsetAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        // Getting first chunk offset values
        int32 getChunkOffsetAt(int32 index) ;
        int32 getChunkClosestToOffset(uint32 offSet, int32& index);


    private:
        bool ParseEntryUnit(uint32 sample_cnt);
        uint32 _entryCount;
        uint32 *_pchunkOffsets;

        int32 _mediaType;
        uint32 _currentDataOffset;
        MP4_FF_FILE *_fileptr;
        uint32	_parsed_entry_cnt;

        MP4_FF_FILE *_curr_fptr;
        uint32 *_stbl_fptr_vec;
        uint32 _stbl_buff_size;
        uint32 _curr_entry_point;
        uint32 _curr_buff_number;
        uint32 _next_buff_number;
        uint32 _parsingMode;

};

#endif // CHUNKOFFSETATOM_H_INCLUDED

