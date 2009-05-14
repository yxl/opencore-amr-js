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
/*                              MPEG-4 EditAtom Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/


#ifndef EDITATOM_H_INCLUDED
#define EDITATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef EDITLISTATOM_H_INCLUDED
#include "editlistatom.h"
#endif

class EditAtom : public Atom
{

    public:
        EditAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Copy Constructor
        uint32 getInitialTimeOffset();

        virtual ~EditAtom();

    private:
        Oscl_Vector<EditListAtom *, OsclMemAllocator> * _pEditListVec;

};


#endif // EDITATOM_H_INCLUDED


