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
    The contents of the PVA_FF_FreeSpaceAtom Class may be ignored, or the contents deleted
    without affecting the presentation.
*/


#ifndef __EditAtom_H__
#define __EditAtom_H__

#include "atom.h"
#include "editlistatom.h"

class PVA_FF_EditAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_EditAtom(); // Constructor
        PVA_FF_EditAtom(const PVA_FF_EditAtom &atom); // Copy Constructor
        virtual ~PVA_FF_EditAtom();

        // Member get method
        const PVA_FF_EditListAtom *getEditListAtom()
        {
            return _peditList;
        }


        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        void addEmptyEdit(uint32 duration);
        void addEditEntry(uint32 duration, int32 time, uint16 rate);

        virtual void recomputeSize();

    private:
        PVA_FF_EditListAtom *_peditList;
        uint32       _pMediaTime;

};


#endif

