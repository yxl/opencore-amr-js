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

/* PASP Box is used to contain the Horizontal and vertical spacing */

#ifndef PASPBOX_H_INCLUDED
#define PASPBOX_H_INCLUDED



#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"

class PASPBox : public Atom
{
    public:
        PASPBox(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~PASPBox() {};

        uint32 getHspacing()
        {
            return _hSpacing;
        }
        uint32 getVspacing()
        {
            return _vSpacing;
        }


    private:
        uint32 _hSpacing;
        uint32 _vSpacing;

};


#endif //End of PASPBOX_H_INCLUDED

