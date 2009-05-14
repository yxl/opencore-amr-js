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
    This PVA_FF_ExpandableBaseClass Class is the base class for all Descriptors that
    allows the encoding the size of the class in bytes with a VARIABLE NUMBER OF BYTES
*/


#ifndef __ExpandableBaseClass_H__
#define __ExpandableBaseClass_H__

#ifndef OSCL_TYPE_H_INCLUDED
#include "oscl_types.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef PARENTABLE_H_INCLUDED
#include "parentable.h"
#endif

#ifndef RENDERABLE_H_INCLUDED
#include "renderable.h"
#endif

//using namespace std;


class PVA_FF_ExpandableBaseClass : public PVA_FF_Parentable, public PVA_FF_Renderable
{

    public:
        PVA_FF_ExpandableBaseClass(); // Constructor
        virtual ~PVA_FF_ExpandableBaseClass(); // Destructor

        virtual void recomputeSize() {} // To implement PVA_FF_Parentable interface

        // For PVA_FF_Renderable interface
        virtual uint32 getSize() const
        {
            return getSizeOfDescriptorObject();
        }

        virtual uint32 getSizeOfClass() const
        {
            return _sizeOfClass;
        }
        uint32  getSizeOfSizeField() const
        {
            return _sizeOfSizeField;
        }
        uint32  getSizeOfDescriptorObject() const
        {
            return _sizeOfSizeField + _sizeOfClass + 1;
        }

        uint8 getTag() const
        {
            return _tag;
        }

    protected:
        uint32 _sizeOfSizeField;
        uint32 _sizeOfClass; // Now JUST THE SIZE OF THE DESCRIPTOR INFO!

        uint8 _tag;
        int32 renderSizeOfClassToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp) const;

};



#endif

