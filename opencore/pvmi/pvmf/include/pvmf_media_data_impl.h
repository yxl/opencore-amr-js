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
/**
 *  @file pvmf_media_data_impl.h
 *  @brief This file defines the PV Multimedia Framework (PVMF) media data
 *  class which is used as the interface to the media data container implementations
 *  which are passed between nodes along the normal media data path.
 *
 */

#ifndef PVMF_MEDIA_DATA_IMPL_H_INCLUDED
#define PVMF_MEDIA_DATA_IMPL_H_INCLUDED


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

class PVMFMediaDataImpl: public HeapBase
{


    public:
        virtual ~PVMFMediaDataImpl() {}
        OSCL_IMPORT_REF virtual uint32 getMarkerInfo() = 0;
        OSCL_IMPORT_REF virtual bool isRandomAccessPoint() = 0;
        OSCL_IMPORT_REF virtual uint32 getErrorsFlag() = 0;
        OSCL_IMPORT_REF virtual uint32 getNumFragments() = 0;
        OSCL_IMPORT_REF virtual bool getMediaFragment(uint32 index, OsclRefCounterMemFrag& memfrag) = 0;
        OSCL_IMPORT_REF virtual bool getMediaFragmentSize(uint32 index, uint32& size) = 0;
        OSCL_IMPORT_REF virtual uint32 getFilledSize() = 0;
        OSCL_IMPORT_REF virtual uint32 getCapacity() = 0;

        OSCL_IMPORT_REF virtual void setCapacity(uint32 aCapacity) = 0;
        OSCL_IMPORT_REF virtual bool setMediaFragFilledLen(uint32 index, uint32 len) = 0;
        OSCL_IMPORT_REF virtual bool setMarkerInfo(uint32 marker) = 0;
        OSCL_IMPORT_REF virtual bool setRandomAccessPoint(bool flag) = 0;
        OSCL_IMPORT_REF virtual bool setErrorsFlag(uint32 flag) = 0;

        OSCL_IMPORT_REF virtual bool appendMediaFragment(OsclRefCounterMemFrag& memfrag) = 0;
        OSCL_IMPORT_REF virtual bool clearMediaFragments() = 0;

};



#endif
