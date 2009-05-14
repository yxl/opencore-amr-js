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
/*                         MPEG-4 SyncSampleAtom Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This SyncSampleAtom Class provides a compact marking of the random access
    points within the stream.
*/


#ifndef SYNCSAMPLEATOM_H_INCLUDED
#define SYNCSAMPLEATOM_H_INCLUDED


#define PV_ERROR -1

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class SyncSampleAtom : public FullAtom
{

    public:
        SyncSampleAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~SyncSampleAtom(); // Destructor

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        uint32* getSampleNumberVector() const
        {
            return _psampleNumbers;
        }

        // Returns the sync sample number at vector location 'index'
        // Note that index is not a sample number, but rather a position along the vector
        int32 getSampleNumberAt(int32 index) const;

        // Returns the first sync sample that occurs at or after 'sampleNum'
        int32 getSyncSampleFollowing(uint32 SampleNum) const;
        int32 getSyncSampleBefore(uint32 SampleNum) const;

        bool IsSyncSample(uint32 sampleNum) const;

    private:
        uint32 _entryCount;
        uint32 *_psampleNumbers;

        uint32 _nextSampleNumber;
};


#endif // SYNCSAMPLEATOM_H_INCLUDED

