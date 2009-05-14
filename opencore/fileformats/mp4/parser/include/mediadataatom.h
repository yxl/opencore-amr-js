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
/*                          MPEG-4 MediaDataAtom Class                           */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This MediaDataAtom Class contains the media data.
*/


#ifndef MEDIADATAATOM_H_INCLUDED
#define MEDIADATAATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef RENDERABLE_H_INCLUDED
#include "renderable.h"
#endif

#ifndef TRACKATOM_H_INCLUDED
#include "trackatom.h"
#endif

class MediaDataAtom : public Atom
{

    public:
        MediaDataAtom(MP4_FF_FILE *fp, OSCL_wString& filename);
        virtual ~MediaDataAtom();

    private:
        int32 _type;
};

#endif // MEDIADATAATOM_H_INCLUDED


