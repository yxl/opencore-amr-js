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
/*                            MPEG-4 EditListAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This EditListAtom Class contains an explicit timeline map.
*/


#ifndef EDITLISTATOM_H_INCLUDED
#define EDITLISTATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif


#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif


class EditListAtom : public FullAtom
{

    public:
        EditListAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Constructor
        virtual ~EditListAtom();
        uint32 getInitialTimeOffset();

        uint32 getEntryCount()
        {
            return _entryCount;
        }

    private:
        Oscl_Vector<uint32, OsclMemAllocator> *_psegmentDurations;
        Oscl_Vector<int32, OsclMemAllocator> *_pmediaTimes;
        Oscl_Vector<uint16, OsclMemAllocator> *_pmediaRates;
        Oscl_Vector<uint16, OsclMemAllocator> *_preserveds;

        uint32 _entryCount;
};


#endif // EDITLISTATOM_H_INCLUDED

