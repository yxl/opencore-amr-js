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
    This PVA_FF_TrackHeaderAtom Class specifies the characteristics of a single MPEG-4
    track.
*/


#ifndef __TrackHeaderAtom_H__
#define __TrackHeaderAtom_H__

#include "fullatom.h"
#include "pv_mp4ffcomposer_config.h"

class PVA_FF_TrackHeaderAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_TrackHeaderAtom(int32 type, uint32 trackID, uint8 version, uint32 flags, uint32 fileAuthoringFlags); // Constructor
        virtual ~PVA_FF_TrackHeaderAtom();
        void init(int32 type);

        void addSample(uint32 ts);
        void updateLastTSEntry(uint32 ts);

        // Media type gets adn sets
        int32 getTrackMediaType() const
        {
            return _mediaType;
        }
        void setTrackMediaType(int32 type)
        {
            _mediaType = type;
        }

        // Creation Time gets and sets
        void setCreationTime(uint32 ct)
        {
            _creationTime = ct;
        }
        uint32 getCreationTime() const
        {
            return _creationTime;
        }

        // Modification Time gets and sets
        void setModificationTime(uint32 mt)
        {
            _modificationTime = mt;
        }
        uint32 getModificationTime() const
        {
            return _modificationTime;
        }

        // Time Scale gets and sets
        void setTrackID(uint32 id)
        {
            _trackID = id;
        }
        uint32 getTrackID() const
        {
            return _trackID;
        }

        // Duration gets and sets
        void setDuration(uint32 d)
        {
            _duration = d;
        }
        uint32 getDuration() const
        {
            return (_duration + _deltaTS);
        }

        // TimeScale gets and sets - in terms of the MOVIE timescale
        void setTimeScale(uint32 ts)
        {
            _timeScale = ts;
        }
        uint32 getTimeScale() const
        {
            return _timeScale;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream.
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);
        void setVideoWidthHeight(int16 width, int16 height);

    private:
        virtual void recomputeSize();
        int32 _mediaType;


        uint32 _creationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint32 _modificationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint32 _trackID; // 4 (32bits)

        uint32 _reserved1; // = 0;

        uint32 _duration; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now

        // Static reserved constants
        uint32 _reserved2[3]; // = { 0, 0, 0 };
        uint16 _reserved3; // = 0; // 0x0100 if audio track
        uint16 _reserved4; // = 0;
        uint32 _reserved5[9]; // = { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
        uint32 _reserved6; // = 0; // 0x01400000 if visual track
        uint32 _reserved7; // = 0; // 0x00f00000 if visual track

        // private member NOT to be rendered - only for internal use
        uint32 _timeScale;

        uint32 _prevTS;
        uint32 _deltaTS;
        uint16 _height;
        uint16 _width;

};



#endif

