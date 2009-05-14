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


#ifndef __FreeSpaceAtom_H__
#define __FreeSpaceAtom_H__

#include "atom.h"


class PVA_FF_FreeSpaceAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_FreeSpaceAtom(); // Constructor
        PVA_FF_FreeSpaceAtom(PVA_FF_FreeSpaceAtom atom); // Copy Constructor
        virtual ~PVA_FF_FreeSpaceAtom();

        // Adding empty data
        void addData(); // Data doesn't matter so no need to pass in any value
        Oscl_Vector<uint8, OsclMemAllocator>* getData()
        {
            return _pdata;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        Oscl_Vector<uint8, OsclMemAllocator>* _pdata;

};


#endif

