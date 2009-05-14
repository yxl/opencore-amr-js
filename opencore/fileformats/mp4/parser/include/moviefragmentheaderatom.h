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
#ifndef MOVIEFRAGMENTHEADERATOM_H_INCLUDED
#define MOVIEFRAGMENTHEADERATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class MovieFragmentHeaderAtom : public FullAtom
{

    public:
        MovieFragmentHeaderAtom(MP4_FF_FILE *fp,
                                uint32 size,
                                uint32 type);
        virtual ~MovieFragmentHeaderAtom();
        int32 getSequenceNumber()
        {
            return _sequence_number;
        }

    private:
        uint32 _sequence_number;
        uint32 _version;
        uint32 _flags;
};

#endif
