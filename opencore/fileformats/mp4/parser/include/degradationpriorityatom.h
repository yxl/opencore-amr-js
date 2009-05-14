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
/*                     MPEG-4 DegradationPriorityAtom Class                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DegradationPriorityAtom Class provides a compact marking of the random access
    points within the stream.
*/


#ifndef DEGRADATIONPRIORITYATOM_H_INCLUDED
#define DEGRADATIONPRIORITYATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class DegradationPriorityAtom : public FullAtom
{

    public:
        DegradationPriorityAtom(uint8 version, uint32 flags); // Constructor
        DegradationPriorityAtom(DegradationPriorityAtom atom); // Copy Constructor
        virtual ~DegradationPriorityAtom();

        // Adding to and getting the sample priority values
        void addPriority(uint16 priority);
        vector<uint16>* getPriorities()
        {
            return _ppriorities;
        }

        // Rendering the Atom in proper format (bitlengths, etc.) to an ostream
        virtual void renderToFileStream(ofstream &os);
        // Reading in the Atom components from an input stream
        virtual void readFromFileStream(ifstream &is);

    private:
        vector<uint16>* _ppriorities; // (15bit) with 1 bit padding when rendering

};


#endif // DEGRADATIONPRIORITYATOM_H_INCLUDED


