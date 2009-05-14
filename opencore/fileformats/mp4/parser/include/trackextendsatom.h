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
/*                            MPEG-4 Track Extends Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/
#ifndef TRACKEXTENDSATOM_H_INCLUDED
#define TRACKEXTENDSATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class TrackExtendsAtom : public FullAtom
{

    public:
        TrackExtendsAtom(MP4_FF_FILE *fp,
                         uint32 size,
                         uint32 type);
        virtual ~TrackExtendsAtom();

        uint32 getDefaultSampleDescriptionIndex()
        {
            return _default_sample_description_index;
        }

        uint32 getDefaultSampleDuration()
        {
            return _default_sample_duration;
        }

        uint32 getDefaultSampleSize()
        {
            return _default_sample_size;
        }

        uint32 getDefaultSampleFlag()
        {
            return _default_sample_flag;
        }

        uint32 getTrackId()
        {
            return _trackID;
        }
    private:
        uint32 _trackID;
        uint32 _default_sample_description_index;
        uint32 _default_sample_duration;
        uint32 _default_sample_size;
        uint32 _default_sample_flag;
        uint32 _version;
        uint32 _flags;
};

#endif
