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
/*                           MPEG-4 FreeSpaceAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    The contents of the FreeSpaceAtom Class may be ignored, or the contents deleted
    without affecting the presentation.
*/


#ifndef FREESPACEATOM_H_INCLUDED
#define FREESPACEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

class FreeSpaceAtom : public Atom
{

    public:
        FreeSpaceAtom(); // Constructor
        FreeSpaceAtom(FreeSpaceAtom atom); // Copy Constructor
        virtual ~FreeSpaceAtom();

        // Adding empty data
        void addData(); // Data doesn't matter so no need to pass in any value
        vector<uint8>* getData()
        {
            return _pdata;
        }

        // Rendering the Atom in proper format (bitlengths, etc.) to an ostream
        virtual void renderToFileStream(ofstream &os);
        // Reading in the Atom components from an input stream
        virtual void readFromFileStream(ifstream &is);

    private:
        vector<uint8>* _pdata;
};

#endif // FREESPACEATOM_H_INCLUDED


