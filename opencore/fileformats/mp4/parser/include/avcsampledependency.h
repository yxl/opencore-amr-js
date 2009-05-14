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
#ifndef __AVCSampleDependency_H__
#define __AVCSampleDependency_H__

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"
#include "fullatom.h"


class AVCSampleDependency : public FullAtom
{

    public:

        AVCSampleDependency(MP4_FF_FILE *fp, uint32 size, uint32 type, uint32 sample_count);

        virtual ~AVCSampleDependency();

        uint16 getDependencyCount(uint32 index)
        {
            return _dependencycount[index];
        }

        uint16 getRelativeSampleNumber(uint32 index)
        {

            return _relativesamplenumber[index];
        }

    private:
        uint16 *_dependencycount;
        uint16 *_relativesamplenumber;
};
#endif
