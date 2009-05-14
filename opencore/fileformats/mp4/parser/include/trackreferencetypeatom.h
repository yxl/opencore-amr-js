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
/*                     MPEG-4 TrackReferenceTypeAtom Class                       */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackReferenceTypeAtom Class provides a reference from the containing stream
    to another stream in the MPEG-4 presentation.
*/


#ifndef TRACKREFERENCETYPEATOM_H_INCLUDED
#define TRACKREFERENCETYPEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif


class TrackReferenceTypeAtom : public Atom
{

    public:
        TrackReferenceTypeAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~TrackReferenceTypeAtom(); // Destructor

        // Get track reference
        uint32 getTrackReferenceAt(int32 index) const;

    private:
        Oscl_Vector<uint32, OsclMemAllocator> *_trackIDs;
};

#endif // TRACKREFERENCETYPEATOM_H_INCLUDED


