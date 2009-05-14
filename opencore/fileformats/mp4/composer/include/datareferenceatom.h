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
    This PVA_FF_DataReferenceAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/


#ifndef __DataReferenceAtom_H__
#define __DataReferenceAtom_H__

#include "fullatom.h"
#include "dataentryatom.h"

class PVA_FF_DataReferenceAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_DataReferenceAtom(); // Constructor
        virtual ~PVA_FF_DataReferenceAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }

        // Adding atoms to and getting the vector of DataEntryAtoms
        void addDataEntryAtom(PVA_FF_DataEntryAtom *atom);

        Oscl_Vector<PVA_FF_DataEntryAtom*, OsclMemAllocator>& getDataEntryVec() const
        {
            return *_pdataEntryVec;
        }
        Oscl_Vector<PVA_FF_DataEntryAtom*, OsclMemAllocator>& getMutableDataEntryVec()
        {
            return *_pdataEntryVec;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        void init();
        virtual void recomputeSize();

        uint32 _entryCount;
        Oscl_Vector<PVA_FF_DataEntryAtom*, OsclMemAllocator> *_pdataEntryVec; // Careful with vector access and cleanup!

};


#endif

