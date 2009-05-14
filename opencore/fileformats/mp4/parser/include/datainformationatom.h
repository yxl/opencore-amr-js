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
    This DataInformationAtom Class contains objects that declare the location
    of the media information within the stream.
*/


#ifndef DATAINFORMATIONATOM_H_INCLUDED
#define DATAINFORMATIONATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif
#ifndef DATAREFERENCEATOM_H_INCLUDED
#include "datareferenceatom.h"
#endif

class DataInformationAtom : public Atom
{

    public:
        DataInformationAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Stream-in ctor
        virtual ~DataInformationAtom();

        // Member gets and sets
        const DataReferenceAtom &getDataReferenceAtom() const
        {
            return *_pdataReferenceAtom;
        }
        DataReferenceAtom &getMutableDataReferenceAtom()
        {
            return *_pdataReferenceAtom;
        }

    private:

        DataReferenceAtom *_pdataReferenceAtom;

};

#endif // DATAINFORMATIONATOM_H_INCLUDED

