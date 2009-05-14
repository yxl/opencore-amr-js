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
/*                        MPEG-4 TrackReferenceAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This TrackReferenceAtom Class provides a reference from the containing stream
    to another stream in the MPEG-4 presentation.
*/


#ifndef TRACKREFERENCEATOM_H_INCLUDED
#define TRACKREFERENCEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef TRACKREFERENCETYPEATOM_H_INCLUDED
#include "trackreferencetypeatom.h"
#endif

class TrackReferenceAtom : public Atom
{

    public:
        TrackReferenceAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~TrackReferenceAtom(); // Destructor

        // Member get method
        const TrackReferenceTypeAtom &getTrackReferenceTypeAtom() const
        {
            return *_ptrackReferenceTypeAtom;
        }

        // Assuming a hint track can reference only ONE track
        uint32 getTrackReference() const
        {
            if (_ptrackReferenceTypeAtom != NULL)
            {
                return _ptrackReferenceTypeAtom->getTrackReferenceAt(0);
            }
            else
            {
                return 0;
            }
        }

    private:
        TrackReferenceTypeAtom *_ptrackReferenceTypeAtom;

};

#endif  // TRACKREFERENCEATOM_H_INCLUDED



