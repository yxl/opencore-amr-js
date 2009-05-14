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
#ifndef __AVCSampleDependencyType_H__
#define __AVCSampleDependencyType_H__

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include "fullatom.h"


class AVCSampleDependencyType: public FullAtom
{
    public:

        AVCSampleDependencyType(MP4_FF_FILE *fp,
                                uint32 size, uint32 type, uint32 sample_count);
        virtual ~AVCSampleDependencyType();

    private:
        uint8 *_reserved;
        uint8 *_sample_depends_on;
        uint8 *_sample_is_depended_on;
        uint8 _version;
        uint32 flags;
};
#endif
