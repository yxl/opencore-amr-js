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
    This PVA_FF_DataEntryUrlAtom Class contains a table of data references which declare
    the location of the media data used within the MPEG-4 presentation.
*/

#ifndef __DataEntryUrlAtom_H__
#define __DataEntryUrlAtom_H__

#include "dataentryatom.h"

class PVA_FF_DataEntryUrlAtom : public PVA_FF_DataEntryAtom
{

    public:
        PVA_FF_DataEntryUrlAtom(uint32 selfContained); // Constructor
        virtual ~PVA_FF_DataEntryUrlAtom();

        // Member gets and sets
        PVA_FF_UTF8_STRING_PARAM getLocation() const
        {
            return _location;
        }
        void setLocation(PVA_FF_UTF8_STRING_PARAM location)
        {
            _location = location;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        virtual void recomputeSize();

        PVA_FF_UTF8_HEAP_STRING _location;

};


#endif

