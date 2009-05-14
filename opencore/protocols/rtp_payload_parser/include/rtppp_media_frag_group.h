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
///////////////////////////////////////////////////////////////////////////////
//
// rtppp_media_frag_group.cpp
//
// Overloaded version of PVMFMediaFragGroupCombinedAlloc that allows
// notification on a given number of free chunks, opposed to only one.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef RTPPP_MEDIA_FRAG_GROUP_H
#define RTPPP_MEDIA_FRAG_GROUP_H

#include "pvmf_media_frag_group.h"

///////////////////////////////////////////////////////////////////////////////
//
// class RTPPPMediaFragGroupCombinedAlloc
//
///////////////////////////////////////////////////////////////////////////////

template<class Alloc>
class RTPPPMediaFragGroupCombinedAlloc: public PVMFMediaFragGroupCombinedAlloc<Alloc>
{
    public:

        // constructor
        RTPPPMediaFragGroupCombinedAlloc<Alloc>(
            uint in_capacity,
            uint in_fragment_capacity,
            Oscl_DefAlloc* opt_gen_alloc = 0)
                : PVMFMediaFragGroupCombinedAlloc<Alloc>(
                    in_capacity, in_fragment_capacity, opt_gen_alloc)
        {
            iGroupSize = 1;
        };

        // notification style of the base class
        void notifyfreechunkavailable(OsclMemPoolFixedChunkAllocatorObserver& obs,
                                      OsclAny* aContextData = NULL);

        // notification of multiple frag groups
        void notifyfreechunkgroupavailable(OsclMemPoolFixedChunkAllocatorObserver& obs,
                                           int groupSize, OsclAny* aContextData = NULL);

        // frag group free
        void destruct_and_dealloc(OsclAny* ptr);

        // check if number messages asked for is available
        bool IsMsgAvailable(int count)
        {
            return count <= this->num_available_mfgs;
        }

        //number of free msgs
        unsigned getNumAvailableMsgs()
        {
            return this->num_available_mfgs;
        }

    private:

        // number of free frags for when we will notify
        int iGroupSize;
};

///////////////////////////////////////////////////////////////////////////////
//
// method RTPPPMediaFragGroupCombinedAlloc::notifyfreechunkavailable
//
///////////////////////////////////////////////////////////////////////////////

template<class Alloc>
void RTPPPMediaFragGroupCombinedAlloc<Alloc>::notifyfreechunkavailable(
    OsclMemPoolFixedChunkAllocatorObserver& obs,
    OsclAny* aContextData)
{
    notifyfreechunkgroupavailable(obs, 1, aContextData);
}

///////////////////////////////////////////////////////////////////////////////
//
// method RTPPPMediaFragGroupCombinedAlloc::notifyfreechunkgroupavailable
//
///////////////////////////////////////////////////////////////////////////////

template<class Alloc>
void RTPPPMediaFragGroupCombinedAlloc<Alloc>::notifyfreechunkgroupavailable(
    OsclMemPoolFixedChunkAllocatorObserver& obs, int groupSize,
    OsclAny* aContextData)
{
    this->iGroupSize = groupSize;
    this->PVMFMediaFragGroupCombinedAlloc<Alloc>::notifyfreechunkavailable(obs, aContextData);
}

///////////////////////////////////////////////////////////////////////////////
//
// method RTPPPMediaFragGroupCombinedAlloc::destruct_and_dealloc
//
///////////////////////////////////////////////////////////////////////////////

template<class Alloc>
void RTPPPMediaFragGroupCombinedAlloc<Alloc>::destruct_and_dealloc(OsclAny* ptr)
{
    uint8* my_ptr = (uint8*)ptr;
    uint aligned_refcnt_size = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    PVMFMediaDataImpl* media_data_ptr = (PVMFMediaDataImpl*)(my_ptr + aligned_refcnt_size);
    media_data_ptr->clearMediaFragments();

    if (! this->iDestroy)
    {
        OsclRefCounter* my_refcnt = OSCL_PLACEMENT_NEW(my_ptr, OsclRefCounterDA(my_ptr, this));
        OsclSharedPtr<PVMFMediaDataImpl> shared_media_data(media_data_ptr, my_refcnt);
        this->append(shared_media_data);

        // if we've reached the quoata of empties, perform the notify
        if ((this->num_available_mfgs >= this->iGroupSize) && (this->iObserver != NULL))
        {
            this->iObserver->freechunkavailable(this->iNextAvailableContextData);
            this->iObserver = NULL;
        }
    }
    else
    {
        media_data_ptr->~PVMFMediaDataImpl();
        this->gen_alloc->deallocate(ptr);
    }
}


#endif // RTPPP_MEDIA_FRAG_GROUP_H
