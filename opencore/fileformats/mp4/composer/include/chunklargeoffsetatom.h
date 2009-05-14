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
    This PVA_FF_ChunkLargeOffsetAtom Class gives the index of each chunk into the
    containing file.
*/

#ifndef __ChunkLargeOffsetAtom_H__
#define __ChunkLargeOffsetAtom_H__

#include "fullatom.h"

class PVA_FF_ChunkLargeOffsetAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_ChunkLargeOffsetAtom(uint8 version, uint32 flags); // Constructor
        PVA_FF_ChunkLargeOffsetAtom(PVA_FF_ChunkLargeOffsetAtom atom); // Copy Constructor
        virtual ~PVA_FF_ChunkLargeOffsetAtom();

        // Member gets and sets
        uint32 getEntryCount()
        {
            return _entryCount;
        }
        void setEntryCount(uint32 count)
        {
            _entryCount = count;
        }

        // Adding to and getting first chunk offset values
        void addChunkOffset(uint64 offset);
        vector<uint64>* getChunkOffsets()
        {
            return _pchunkOffsets;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        uint32 _entryCount;
        vector<uint64>* _pchunkOffsets;

};


#endif

