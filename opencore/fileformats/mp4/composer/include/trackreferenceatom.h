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
    This PVA_FF_TrackReferenceAtom Class provides a reference from the containing stream
    to another stream in the MPEG-4 presentation.
*/


#ifndef __TrackReferenceAtom_H__
#define __TrackReferenceAtom_H__

#include "atom.h"
#include "trackreferencetypeatom.h"


class PVA_FF_TrackReferenceAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_TrackReferenceAtom(uint32 refType); // Constructor
        virtual ~PVA_FF_TrackReferenceAtom(); // Destructor

        // Member get method
        PVA_FF_TrackReferenceTypeAtom &getTrackReferenceTypeAtom()
        {
            return *_ptrackReferenceTypeAtom;
        }
        PVA_FF_TrackReferenceTypeAtom *getTrackReferenceTypeAtomPtr()
        {
            return _ptrackReferenceTypeAtom;
        }

        // Add track reference - return index of reference in table - 1-based
        int32 addTrackReference(uint32 ref)
        {
            return _ptrackReferenceTypeAtom->addTrackReference(ref);
        }

        // Assuming a hint track can reference only ONE track
        uint32 getTrackReference() const
        {
            return _ptrackReferenceTypeAtom->getTrackReferenceAt(0);
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        virtual void recomputeSize();

        PVA_FF_TrackReferenceTypeAtom *_ptrackReferenceTypeAtom;

};



#endif

