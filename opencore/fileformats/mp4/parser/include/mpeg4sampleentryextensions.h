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
#ifndef __MPEG4SampleEntryExtensions_H__
#define __MPEG4SampleEntryExtensions_H__

#include "atom.h"
#include "atomutils.h"
#include "oscl_file_io.h"

class MPEG4BitRateBox : public Atom
{
    public:
        MPEG4BitRateBox(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~MPEG4BitRateBox() {};

        uint32 getBufferSizeDB()
        {
            return _bufferSizeDB;
        }
        uint32 getMaxBitRate()
        {
            return _maxBitRate;
        }
        uint32 getAvgBitRate()
        {
            return _avgBitRate;
        }

    private:
        uint32 _bufferSizeDB;
        uint32 _maxBitRate;
        uint32 _avgBitRate;
};

#endif
