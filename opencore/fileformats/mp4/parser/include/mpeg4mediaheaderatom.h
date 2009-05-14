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
/*                      MPEG-4 Mpeg4MediaHeaderAtom Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This Mpeg4MediaHeaderAtom Class is for streams other than visual and audio.
*/


#ifndef MPEG4MEDIAHEADERATOM_H_INCLUDED
#define MPEG4MEDIAHEADERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef MEDIAINFORMATIONHEADERATOM_H_INCLUDED
#include "mediainformationheaderatom.h"
#endif

class Mpeg4MediaHeaderAtom : public MediaInformationHeaderAtom
{

    public:
        Mpeg4MediaHeaderAtom(MP4_FF_FILE *fp,
                             uint32 mediaType,
                             uint32 size,
                             uint32 type);

        virtual ~Mpeg4MediaHeaderAtom();

        virtual uint32 getMediaInformationHeaderType() const;


    private:
        uint32 _mediaType; // DO NOT RENDER THIS VALUE!!! (Only for internal purposes)
};


#endif  // MPEG4MEDIAHEADERATOM_H_INCLUDED

