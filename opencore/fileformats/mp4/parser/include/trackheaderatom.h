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
/*                         MPEG-4 TrackHeaderAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackHeaderAtom Class specifies the characteristics of a single MPEG-4
    track.
*/


#ifndef TRACKHEADERATOM_H_INCLUDED
#define TRACKHEADERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class TrackHeaderAtom : public FullAtom
{

    public:
        TrackHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~TrackHeaderAtom();

        int32 getTrackMediaType() const
        {
            return _mediaType;
        }

        uint64 getCreationTime() const
        {
            if (getVersion() == 1)
                return _creationTime64;
            else
                return _creationTime;
        }
        uint64 getModificationTime() const
        {
            if (getVersion() == 1)
                return _modificationTime64;
            else
                return _modificationTime;
        }

        uint32 getTrackID() const
        {
            return _trackID;
        }

        uint64 getDuration() const
        {
            if (getVersion() == 1)
                return _duration64;
            else
                return _duration;
        }

        int16 getLayer()
        {
            return _layer;
        }
        uint16 getAlternateGroup()
        {
            return _alternateGroup;
        }
        int32 getTextTrackWidth()
        {
            return _width;
        }
        int32 getTextTrackHeight()
        {
            return _height;
        }
        int32 getTextTrackXOffset()
        {
            return _tx;
        }
        int32 getTextTrackYOffset()
        {
            return _ty;
        }

    private:
        int32 _mediaType;

        uint32 _creationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _creationTime64;

        uint32 _modificationTime; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _modificationTime64;

        uint32 _trackID; // 4 (32bits)
        uint32 _reserved1; // = 0;

        uint32 _duration; // 4/8 (32/64bits) -- Will templatize later - using 32bits for now
        uint64 _duration64;

        // Static reserved constants
        uint32 _reserved2[3]; // = { 0, 0, 0 };
        uint16 _reserved3; // = 0; // 0x0100 if audio track
        uint16 _reserved4; // = 0;
        uint32 _reserved5[9]; // = { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 };
        uint32 _reserved6; // = 0; // 0x01400000 if visual track
        uint32 _reserved7; // = 0; // 0x00f00000 if visual track

        // private member NOT to be rendered - only for internal use
        uint32 _timeScale;

        int16  _layer;
        uint16 _alternateGroup;
        int32 _width;
        int32 _height;
        int32  _tx;
        int32  _ty;
};

#endif // TRACKHEADERATOM_H_INCLUDED

