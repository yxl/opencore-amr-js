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
    This PVA_FF_UserDataAtom Class is a container atom for informative user-data.
*/


#ifndef __UserDataAtom_H__
#define __UserDataAtom_H__

#include "atom.h"
#include "a_isucceedfail.h"
#include "atomutils.h"


class PVA_FF_UserDataAtom : public PVA_FF_Atom, public PVA_FF_ISucceedFail
{

    public:
        PVA_FF_UserDataAtom(); 				// Constructor
        virtual ~PVA_FF_UserDataAtom();

        void addAtom(PVA_FF_Atom* atom);

        int32 getUserDataAtomVecSize()
        {
            if (_pUserDataAtomVec != NULL)
            {
                return (_pUserDataAtomVec->size());
            }
            return 0;
        }

        virtual void recomputeSize();

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);


    private:
        Oscl_Vector<PVA_FF_Atom*, OsclMemAllocator> *_pUserDataAtomVec; //Array of User Data PVA_FF_Atom Pointers

};

#endif

