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
/*                       MPEG-4 ExpandableBaseClass Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This ExpandableBaseClass Class is the base class for all Descriptors that
    allows the encoding the size of the class in bytes with a VARIABLE NUMBER OF BYTES
*/


#ifndef EXPANDABLEBASECLASS_H_INCLUDED
#define EXPANDABLEBASECLASS_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PARENTABLE_H_INCLUDED
#include "parentable.h"
#endif

#ifndef RENDERABLE_H_INCLUDED
#include "renderable.h"
#endif

#ifndef ISUCCEEDFAIL_H_INCLUDED
#include "isucceedfail.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif


class ExpandableBaseClass : public Parentable, public Renderable, public ISucceedFail
{

    public:
        ExpandableBaseClass() {} // Default constructor
        ExpandableBaseClass(MP4_FF_FILE *fp, bool fixedLengthSizeField = false); // Stream-in Constructor
        virtual ~ExpandableBaseClass(); // Destructor

        virtual uint32 getSize() const
        {
            return _sizeOfClass;
        }
        uint32 getSizeOfSizeField() const
        {
            return _sizeOfSizeField;
        }
        uint8 getTag() const
        {
            return _tag;
        }

    protected:
        int32 readSizeOfClassFromFileStream(MP4_FF_FILE *fp);

        uint32 _sizeOfSizeField;
        uint32 _sizeOfClass;
        uint8 _tag;

};

#endif // EXPANDABLEBASECLASS_H_INCLUDED


