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
    This PVA_FF_Mpeg4MediaHeaderAtom Class is for streams other than visual and audio.
*/


#ifndef __Mpeg4MediaHeaderAtom_H__
#define __Mpeg4MediaHeaderAtom_H__

#include "mediainformationheaderatom.h"

class PVA_FF_Mpeg4MediaHeaderAtom : public PVA_FF_MediaInformationHeaderAtom
{

    public:
        PVA_FF_Mpeg4MediaHeaderAtom(uint32 mediaType); // Constructor
        virtual ~PVA_FF_Mpeg4MediaHeaderAtom();

        virtual uint32 getMediaInformationHeaderType() const;

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        virtual void recomputeSize();

        uint32 _mediaType; // DO NOT RENDER THIS VALUE!!! (Only for internal purposes)
};


#endif

