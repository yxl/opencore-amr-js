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
    This DataReferenceAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/


#ifndef DATAREFERENCEATOM_H_INCLUDED
#define DATAREFERENCEATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif
#ifndef DATAENTRYATOM_H_INCLUDED
#include "dataentryatom.h"
#endif
#ifndef DATAENTRYURLATOM_H_INCLUDED
#include "dataentryurlatom.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

class DataReferenceAtom : public FullAtom
{

    public:
        DataReferenceAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~DataReferenceAtom();

        // Member gets and sets
        uint32 getEntryCount() const
        {
            return _entryCount;
        }
        DataEntryAtom *getEntryAt(int32 index);

    private:

        uint32 _entryCount;
        Oscl_Vector<DataEntryUrlAtom*, OsclMemAllocator> *_pdataEntryVec;

};

#endif // DATAREFERENCEATOM_H_INCLUDED

