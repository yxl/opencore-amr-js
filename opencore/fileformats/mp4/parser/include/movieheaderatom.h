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
/*                         MPEG-4 MovieHeaderAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MovieHeaderAtom Class defines the overall media-independent information
    relevant to the MPEG-4 presentation as a whole.
*/


#ifndef MOVIEHEADERATOM_H_INCLUDED
#define MOVIEHEADERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class MovieHeaderAtom : public FullAtom
{

    public:
        MovieHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~MovieHeaderAtom();

        // Creation Time gets  - may not need to have the set method public!
        uint64 getCreationTime() const
        {
            if (getVersion() == 1)
                return _creationTime64;
            else
                return _creationTime;

        }

        // Modification Time gets - may not need to have the set method public!
        uint64 getModificationTime() const
        {
            if (getVersion() == 1)
                return _modificationTime64;
            else
                return _modificationTime;
        }

        // Time Scale gets
        uint32 getTimeScale() const
        {
            return _timeScale;
        }

        // Duration gets
        uint64 getDuration() const
        {
            if (getVersion() == 1)
                return _duration64;
            else
                return _duration;
        }

        // NextTrackID gets
        uint32 getNextTrackID() const
        {
            return _nextTrackID;
        }

        OSCL_wHeapString<OsclMemAllocator> getCreationDate()
        {
            return (convertTimeToDate(_creationTime));
        }

    private:

        OSCL_wHeapString<OsclMemAllocator> convertTimeToDate(uint32 time);

        uint32 _creationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits (version 0) for now
        uint64 _creationTime64;

        uint32 _modificationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _modificationTime64;

        uint32 _timeScale; // 4 (32bits)

        uint32 _duration; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _duration64;

        uint32 _nextTrackID; // 4 (32 bits)

};

#endif  // MOVIEHEADERATOM_H_INCLUDED


