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
/*                   MPEG-4 MediaInformationHeaderAtom Class                     */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
	This MediaInformationHeaderAtom Class is an abstract base class for the
	atoms VideoMediaHeaderAton, AudioMediaHeaderAtom, HintMediaHeaderAtom,
	and Mpeg4MediaHeaderAtom.  This class onbly contains one pure virtual method
	getMediaHeaderType() that will return what type of mediaInformationHeader
	this atom is.
*/


#ifndef MEDIAINFORMATIONHEADERATOM_H_INCLUDED
#define MEDIAINFORMATIONHEADERATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif


class MediaInformationHeaderAtom : public FullAtom
{

    public:
        MediaInformationHeaderAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Stream-in ctor
        virtual ~MediaInformationHeaderAtom();

        // Method to get the header type
        virtual uint32 getMediaInformationHeaderType() const = 0;
};

#endif // MEDIAINFORMATIONHEADERATOM_H_INCLUDED


