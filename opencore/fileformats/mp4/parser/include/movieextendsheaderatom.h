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
/*                            MPEG-4 Movie Extends Header Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#ifndef MOVIEEXTENDSHEADERATOM_H_INCLUDED
#define MOVIEEXTENDSHEADERATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class MovieExtendsHeaderAtom : public FullAtom
{

    public:
        MovieExtendsHeaderAtom(MP4_FF_FILE *fp,
                               uint32 size,
                               uint32 type);
        virtual ~MovieExtendsHeaderAtom();

        uint64 getFragmentDuration()
        {
            if (_version == 1)
            {
                return _fragmentDuration64;
            }
            else
                return _fragmentDuration32;
        }

    private:
        uint32 _fragmentDuration32;
        uint64 _fragmentDuration64;
        uint32 _version;
        uint32 _flags;
};

#endif
