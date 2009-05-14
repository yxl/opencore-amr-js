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
    This PVA_FF_VideoMediaHeaderAtom Class contains general presentation information,
    independent of coding, about the visual media within the stream.
*/


#ifndef __VideoMediaHeaderAtom_H__
#define __VideoMediaHeaderAtom_H__

#include "mediainformationheaderatom.h"

class PVA_FF_VideoMediaHeaderAtom : public PVA_FF_MediaInformationHeaderAtom
{

    public:
        PVA_FF_VideoMediaHeaderAtom(); // Constructor
        virtual ~PVA_FF_VideoMediaHeaderAtom();

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        virtual uint32 getMediaInformationHeaderType() const;

    private:
        virtual void recomputeSize();

        // Static reserved constants
        uint32 _reserved; // = 0 // 8 (64bits)
};



#endif

