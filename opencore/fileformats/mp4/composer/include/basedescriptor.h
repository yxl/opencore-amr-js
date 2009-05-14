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
    This PVA_FF_BaseDescriptor Class
*/


#ifndef __BaseDescriptor_H__
#define __BaseDescriptor_H__

#include "oscl_types.h"

#include "expandablebaseclass.h"

const uint32 DEFAULT_DESCRIPTOR_SIZE = 1; // 8 bits for the tag ONLY
// _sizeOfClass is computed explicitly elsewhere!

class PVA_FF_BaseDescriptor : public PVA_FF_ExpandableBaseClass
{

    public:
        PVA_FF_BaseDescriptor(uint8 tag); // Constructor
        virtual ~PVA_FF_BaseDescriptor();

        // Rendering only the members of the PVA_FF_BaseDescriptor class
        int renderBaseDescriptorMembers(MP4_AUTHOR_FF_FILE_IO_WRAP *fp) const;

        virtual void recomputeSize() = 0; // Should get overridden
        uint32 getSize() const
        {
            return _sizeOfClass;
        }
        uint32 getDefaultDescriptorSize() const
        {
            return DEFAULT_DESCRIPTOR_SIZE;
        }

    private:
        PVA_FF_BaseDescriptor() {} // Disabling public default constructor

};



#endif

