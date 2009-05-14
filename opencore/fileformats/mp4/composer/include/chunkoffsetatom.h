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
    This PVA_FF_ChunkOffsetAtom Class gives the index of each chunk into the
    containing file.
*/

#ifndef __ChunkOffsetAtom_H__
#define __ChunkOffsetAtom_H__

#define PV_ERROR -1

#include "fullatom.h"

class PVA_FF_ChunkOffsetAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_ChunkOffsetAtom(uint32 mediaType,
                               uint32 fileAuthoringFlags); // Constructor

        virtual ~PVA_FF_ChunkOffsetAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }
        void nextSample(uint32 size, bool isChunkStart, uint32 baseOffset = 0);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        void updateChunkEntries(uint32 fileOffset);

        void reAuthorFirstChunkOffset(uint32 offset);

    private:
        // Adding first chunk offset values
        void addChunkOffset(uint32 offset);

        virtual void recomputeSize();

        uint32 _entryCount;
        Oscl_Vector<uint32, OsclMemAllocator>* _pchunkOffsets;
        int _mediaType;
        uint32 _currentDataOffset;

        bool _modified; // Flag used to check whether or not the atom was modified since the
        // last render to file.

        bool _oInterLeaveMode;
};


#endif

