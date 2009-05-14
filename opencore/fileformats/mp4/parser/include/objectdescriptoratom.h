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
/*                        MPEG-4 ObjectDescriptorAtom Class                      */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ObjectDescriptorAtom Class contains an ObjectDescriptor or an
    InitialObjectDescriptor.
*/


#ifndef OBJECTDESCRIPTORATOM_H_INCLUDED
#define OBJECTDESCRIPTORATOM_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef INITIALOBJECTDESCRIPTOR_H_INCLUDED
#include "initialobjectdescriptor.h"
#endif

class ObjectDescriptorAtom : public FullAtom
{

    public:
        ObjectDescriptorAtom(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~ObjectDescriptorAtom();

        // Member gets
        const InitialObjectDescriptor &getInitialObjectDescriptor() const
        {
            return *_pOD;
        }

    private:
        InitialObjectDescriptor *_pOD;
};

#endif // OBJECTDESCRIPTORATOM_H_INCLUDED

