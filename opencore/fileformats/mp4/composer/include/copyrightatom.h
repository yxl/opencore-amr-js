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
    This PVA_FF_CopyrightAtom Class contains the user-defined copyright notice.
*/

#ifndef __CopyrightAtom_H__
#define __CopyrightAtom_H__

#include "fullatom.h"

class PVA_FF_CopyrightAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_CopyrightAtom(uint8 version, uint32 flags); // Constructor

        virtual ~PVA_FF_CopyrightAtom();

        // Member gets and sets
        uint16 getLanguage()
        {
            return _language;
        }
        void setLanguage(uint16 language)
        {
            _language = language;
        }

        OSCL_wString getNotice()
        {
            return _notice;
        }
        void setNotice(OSCL_wString notice)
        {
            _notice = notice;
        }


        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        uint16 _language; // (15bits) padded with a single bit when rendering
        OSCL_wString _notice;

};


#endif

