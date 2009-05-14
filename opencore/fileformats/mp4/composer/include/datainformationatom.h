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
    This PVA_FF_DataInformationAtom Class contains objects that declare the location
    of the media information within the stream.
*/


#ifndef __DataInformationAtom_H__
#define __DataInformationAtom_H__

#include "atom.h"
#include "datareferenceatom.h"

class PVA_FF_DataInformationAtom : public PVA_FF_Atom
{

    public:
        PVA_FF_DataInformationAtom(); // Constructor

        virtual ~PVA_FF_DataInformationAtom();

        // Member gets and sets
        const PVA_FF_DataReferenceAtom &getDataReferenceAtom() const
        {
            return *_pdataReferenceAtom;
        }
        PVA_FF_DataReferenceAtom &getMutableDataReferenceAtom()
        {
            return *_pdataReferenceAtom;
        }
        void setDataReferenceAtom(PVA_FF_DataReferenceAtom *dra)
        {
            _pdataReferenceAtom = dra;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        virtual void recomputeSize();

        PVA_FF_DataReferenceAtom *_pdataReferenceAtom;

};



#endif

