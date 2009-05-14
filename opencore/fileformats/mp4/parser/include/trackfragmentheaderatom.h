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
/*                            MPEG-4 Track Fragment Header Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#ifndef TRACKFRAGMENTHEADERATOM_H_INCLUDED
#define TRACKFRAGMENTHEADERATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class TrackFragmentHeaderAtom : public FullAtom
{

    public:
        TrackFragmentHeaderAtom(MP4_FF_FILE *fp,
                                uint32 size,
                                uint32 type);
        virtual ~TrackFragmentHeaderAtom();

        uint64 getBaseDataOffset()
        {
            return _base_data_offset;
        }
        uint32 getSampleDescriptionIndex()
        {
            return _sample_description_index;
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
        uint32 tf_flag;
        uint32 _trackID;
        uint64 _base_data_offset;
        uint32 _sample_description_index;
        uint32 _default_sample_duration;
        uint32 _default_sample_size;
        uint32 _default_sample_flag;
        uint32 _version;
};

#endif
