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
    This BaseDescriptor Class
*/


#ifndef BASEDESCRIPTOR_H_INCLUDED
#define BASEDESCRIPTOR_H_INCLUDED

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef EXPANDABLEBASECLASS_H_INCLUDED
#include "expandablebaseclass.h"
#endif

const uint32 DEFAULT_DESCRIPTOR_SIZE = 1; // 8 bits for the tag ONLY
// _sizeOfClass is computed explicitly elsewhere!

class BaseDescriptor : public ExpandableBaseClass
{

    public:
        BaseDescriptor(uint8 tag)
        {
            _tag = tag;    // DELETE THIS CTOR
        }
        BaseDescriptor(MP4_FF_FILE *fp, bool o3GPPTrack = false); // Stream-in Constructor
        virtual ~BaseDescriptor();

        uint32 getSize() const
        {
            return _sizeOfClass;
        }
        uint32 getDefaultDescriptorSize() const
        {
            return DEFAULT_DESCRIPTOR_SIZE;
        }

};

#endif // BASEDESCRIPTOR_H_INCLUDED

