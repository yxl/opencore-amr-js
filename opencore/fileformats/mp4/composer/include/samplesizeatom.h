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
    This PVA_FF_SampleSizeAtom Class contains the sample count and a table giving the
    size of each sample.
*/


#ifndef __SampleSizeAtom_H__
#define __SampleSizeAtom_H__

#define PV_ERROR -1

#include "fullatom.h"


class PVA_FF_SampleSizeAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_SampleSizeAtom(uint32 mediaType); // Constructor
        virtual ~PVA_FF_SampleSizeAtom();

        // Member gets and sets
        uint32 getDefaultSampleSize() const
        {
            return _sampleSize;
        }
        void setDefaultSampleSize(uint32 size)
        {
            _sampleSize = size;
        }

        void nextSample(uint32 size);
        uint32 getSampleCount() const
        {
            return _sampleCount;
        }

        Oscl_Vector<uint32, OsclMemAllocator>& getSampleSizeVec() const
        {
            return *_psampleSizeVec;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        bool reAuthorFirstSample(uint32 size);

    private:
        void addNextSampleSize(uint32 size);
        virtual void recomputeSize();

        uint32 _sampleSize;
        uint32 _sampleCount;
        Oscl_Vector<uint32, OsclMemAllocator>* _psampleSizeVec;

        int _mediaType;
};


#endif

