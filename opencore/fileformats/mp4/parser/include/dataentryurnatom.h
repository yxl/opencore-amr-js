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
/*                        MPEG-4 DataEntryUrnAtom Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This DataEntryUrnAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/


#ifndef DATAENTRYURNATOM_H_INCLUDED
#define DATAENTRYURNATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef DATAENTRYATOM_H_INCLUDED
#include "dataentryatom.h"
#endif

class DataEntryUrnAtom : public DataEntryAtom
{

    public:
        DataEntryUrnAtom(uint8 version, uint32 flags); // Constructor
        DataEntryUrnAtom(DataEntryUrnAtom atom); // Copy constructor
        virtual ~DataEntryUrnAtom();

        // Member gets and sets
        OSCL_wString& getName()
        {
            return _name;
        }
        OSCL_wString& getLocation()
        {
            return _location;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _name;
        OSCL_wHeapString<OsclMemAllocator> _location;

};

#endif // DATAENTRYURNATOM_H_INCLUDED


