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
    This CopyrightAtom Class contains the user-defined copyright notice.
*/


#ifndef COPYRIGHTATOM_H_INCLUDED
#define COPYRIGHTATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

class CopyRightAtom : public FullAtom
{

    public:
        CopyRightAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Constructor
        virtual ~CopyRightAtom();

        // Member gets and sets
        uint16 getLanguageCode()
        {
            return _language_code;
        }

        OSCL_wString& getCopyRightString()
        {
            return _copyRightNotice;
        }

    private:
        uint16 	     _language_code; // (15bits) padded with a single bit when rendering
        OSCL_wHeapString<OsclMemAllocator>  _copyRightNotice;
};


#endif // COPYRIGHTATOM_H_INCLUDED

