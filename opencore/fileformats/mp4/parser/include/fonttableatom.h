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
#ifndef FONTTABLEATOM_H_INCLUDED
#define FONTTABLEATOM_H_INCLUDED

#ifndef FONTRECORD_H_INCLUDED
#include "fontrecord.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif


#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

class FontTableAtom : public Atom
{

    public:
        FontTableAtom(MP4_FF_FILE *fp);  // Default constructor
        virtual ~FontTableAtom();  // Destructor

        uint16  getFontListSize()
        {
            return _entryCount;
        }

        FontRecord * getFontRecordAt(uint16 index);


    private:
        uint16 _entryCount;
        Oscl_Vector<FontRecord *, OsclMemAllocator> *_pFontRecordArray;
};


#endif // FONTTABLEATOM_H_INCLUDED


