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
    This PVA_FF_EditListAtom Class contains an explicit timeline map.
*/

#ifndef __EditListAtom_H__
#define __EditListAtom_H__

#include "fullatom.h"
#include "editlistatom.h"


class PVA_FF_EditListAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_EditListAtom(); // Constructor
        PVA_FF_EditListAtom(const PVA_FF_EditListAtom &atom); // Copy Constructor
        virtual ~PVA_FF_EditListAtom();

        // Member gets and sets
        uint32 getEntryCount()
        {
            return _entryCount;
        }

        // Adding to and getting first chunk, samples per chunk, and sample
        // description index values
        void addEditEntry(uint32 duration, int32 time, uint16 rate);

        const Oscl_Vector<uint32, OsclMemAllocator> &getSegmentDurations()
        {
            return *_psegmentDurations;
        }
        const Oscl_Vector<int32, OsclMemAllocator> &getMediaTimes()
        {
            return *_pmediaTimes;
        }
        const Oscl_Vector<uint16, OsclMemAllocator> &getMediaRates()
        {
            return *_pmediaRates;
        }

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

        virtual void recomputeSize();

    private:
        uint32 _entryCount;
        Oscl_Vector<uint32, OsclMemAllocator>* _psegmentDurations; // (32/64) - Default to 32 for now - templatize later
        Oscl_Vector<int32, OsclMemAllocator>*  _pmediaTimes; // (32/64) - Default to 32 for now - templatize later
        Oscl_Vector<uint16, OsclMemAllocator>* _pmediaRates;
        Oscl_Vector<uint16, OsclMemAllocator>* _preserveds; // All entries set to zero


};


#endif

