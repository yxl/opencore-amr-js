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
#ifndef __AVCSampleToGroup_H__
#define __AVCSampleToGroup_H__

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include "fullatom.h"


class AVCSampleToGroup: public FullAtom
{
    public:

        AVCSampleToGroup(MP4_FF_FILE *fp,
                         uint32 size, uint32 type);
        virtual ~AVCSampleToGroup();
        uint32 getGroupingType()
        {
            return _grouping_type;
        }
        uint32 getGroupDescriptionIndex(uint32 index)
        {
            return _group_description_indexVec[index];
        }

    private:
        uint32 _grouping_type;
        uint32 _entry_count;
        uint32 * _samplecountVec;
        uint32 * _group_description_indexVec;
};
#endif
