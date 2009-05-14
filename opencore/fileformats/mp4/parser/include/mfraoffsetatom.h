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
/*                            MPEG-4 Movie Fragment Random Access Offset Atom Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
*/

#ifndef MOVIEFRAGMENTRANDOMACCESSOFFSETATOM_H_INCLUDED
#define MOVIEFRAGMENTRANDOMACCESSOFFSETATOM_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif


class MfraOffsetAtom : public FullAtom
{

    public:
        MfraOffsetAtom(MP4_FF_FILE *fp,
                       uint32 size,
                       uint32 type);

        virtual ~MfraOffsetAtom();

        uint32 getSizeStoredInmfro()
        {
            return _size;
        }
    private:
        uint32 _size;
        PVLogger *iLogger, *iStateVarLogger, *iParsedDataLogger;
};

#endif
