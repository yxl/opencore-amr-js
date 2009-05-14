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
    This PVA_FF_MovieHeaderAtom Class defines the overall media-independent information
    relevant to the MPEG-4 presentation as a whole.
*/


#ifndef __MovieHeaderAtom_H__
#define __MovieHeaderAtom_H__

#include "fullatom.h"
#include "pv_mp4ffcomposer_config.h"

class PVA_FF_MovieHeaderAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_MovieHeaderAtom(uint8 version, uint32 flags, uint32 fileAuthoringFlags); // Constructor
        virtual ~PVA_FF_MovieHeaderAtom();

        // Creation Time gets and sets - may not need to have the set method public!
        void setCreationTime(uint32 ct)
        {
            _creationTime = ct;
        }
        uint32 getCreationTime() const
        {
            return _creationTime;
        }

        // Modification Time gets and sets - may not need to have the set method public!
        void setModificationTime(uint32 mt)
        {
            _modificationTime = mt;
        }
        uint32 getModificationTime() const
        {
            return _modificationTime;
        }

        // Time Scale gets and sets
        void setTimeScale(uint32 ts)
        {
            _timeScale = ts;
        }
        uint32 getTimeScale() const
        {
            return _timeScale;
        }

        // Duration gets and sets
        void setDuration(uint32 d)
        {
            if (d > _duration)
            {
                _duration = d;
            }
        }

        uint32 getDuration() const
        {
            return _duration;
        }
        uint32 getLastTS()	const
        {
            return _prevTS;
        }

        void addSample(uint32 ts); // Informs that another sample was added at time ts

        // TrackID
        uint32 getNextTrackID() const;
        uint32 findNextTrackID();

        virtual void recomputeSize();

        // Sets the time value to the current time in seconds since 1970 - Need to FIX to 1904
        void setTime(uint32 &val);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream.
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:

        uint32 _creationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits (version 0) for now
        uint32 _modificationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint32 _timeScale; // 4 (32bits)
        uint32 _duration; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now

        uint32 _nextTrackID; // 4 (32 bits)

        uint32 _prevTS;
        uint32 _deltaTS;

};



#endif

