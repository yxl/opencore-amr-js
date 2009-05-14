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
    This PVA_FF_ShadowSyncSampleAtom Class provides an optional set of synch samples
    when seeking or for similar purposes.
*/


#ifndef __ShadowSyncSampleAtom_H__
#define __ShadowSyncSampleAtom_H__

#include "fullatom.h"


class PVA_FF_ShadowSyncSampleAtom : public PVA_FF_FullAtom
{

    public:
        PVA_FF_ShadowSyncSampleAtom(uint8 version, uint32 flags); // Constructor
        PVA_FF_ShadowSyncSampleAtom(PVA_FF_ShadowSyncSampleAtom atom); // Copy Constructor
        virtual ~PVA_FF_ShadowSyncSampleAtom();

        // Member gets and sets
        uint32 getEntryCount()
        {
            return _entryCount;
        }

        // Adding to and getting the shadow and sync sample number values
        void addShadowSampleNumber(uint32 sample);
        Oscl_Vector<uint32, OsclMemAllocator>* getShadowSampleNumbers()
        {
            return _pshadowSampleNumbers;
        }
        uint32 getShadowSampleNumberAt(int index);

        void addSyncSampleNumber(uint32 sample);
        Oscl_Vector<uint32, OsclMemAllocator>* getSyncSampleNumbers()
        {
            return _psyncSampleNumbers;
        }
        uint32 getSyncSampleNUmberAt(int index);

        // Rendering the PVA_FF_Atom in proper format (bitlengths, etc.) to an ostream
        virtual bool renderToFileStream(MP4_AUTHOR_FF_FILE_IO_WRAP *fp);

    private:
        uint32 _entryCount;
        Oscl_Vector<uint32, OsclMemAllocator>* _pshadowSampleNumbers;
        Oscl_Vector<uint32, OsclMemAllocator>* _psyncSampleNumbers;

};


#endif

