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
/*                           MPEG-4 Renderable Class                             */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This Renderable Class is the base class for ALL other components of the MP4
    file format - including Atoms, Descriptors, and Commands.  This class provides
    the renderToFileStream(...) and readFromFileStream(...) pure virtual methods.
*/


#ifndef RENDERABLE_H_INCLUDED
#define RENDERABLE_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

class Renderable
{

    public:
        virtual ~Renderable() {}

        virtual uint32 getSize() const = 0;
};

#endif // RENDERABLE_H_INCLUDED


