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
// -*- c++ -*-
#ifndef ACCESS_UNIT_IMPL_H
#define ACCESS_UNIT_IMPL_H

#include "oscl_media_data.h"
#include "oscl_media_status.h"
#include "access_unit.h"
#include "oscl_mem.h"

class AccessUnitImplementation : public MediaData<AccessUnit, AU_MAX_FRAGMENTS, AU_IMMEDIATE_DATA>
{

    public:
        OSCL_IMPORT_REF AccessUnitImplementation(const int num_reserved_frags = 1);
        virtual ~AccessUnitImplementation();


        MediaStatusClass::status_t AddAUFrag(const BufferFragment& frag, BufferState* buffer_state,
                                             int32 location_offset);

        MediaStatusClass::status_t AddLocalAUFrag(const BufferFragment& frag, int32 location_offset);


        MediaStatusClass::status_t AddCodecInfo(BufferFragment& fragment);
        MediaStatusClass::status_t AddCodecInfo(void *ptr, int len);

        BufferFragment * GetCodecInfo()
        {
            return fragments;
        };

        int32 GetMaxMediaFrags() const
        {
            return GetMaxFrags() - num_reserved_fragments;
        };
        int32 GetNumMediaFrags() const
        {
            return num_fragments - num_reserved_fragments;
        };

        uint32 GetMediaLength() const
        {
            int reserved_len = 0;
            for (int ii = 0; ii < num_reserved_fragments; ii++)
            {
                reserved_len += fragments[ii].len;
            }
            return (length - reserved_len);
        }

        BufferState * GetBufferState(const int32 idx)
        {
            return buffer_states[idx];
        }

        void GetMediaFragment(uint32 index, BufferFragment& frag, BufferState*& buffer_state) const;

        const BufferFragment* GetMediaFragments() const
        {
            return (fragments + 1);
        }

        BufferFragment * GetMediaFragment(const uint32 idx)
        {
            uint32 tmp = idx + num_reserved_fragments;
            if (tmp > num_fragments)
            {
                return NULL;
            }
            else
            {
                return & fragments[tmp];
            }
        }

        //
        // Function match_bit_pattern_with_state()
        // returns true if matched,
        // else return false;
        //
        bool match_bit_pattern_no_state(const int idx, const int offset, const uint8 * pattern,
                                        const uint8 pattern_size_in_bits);

        //
        // Function match_bit_pattern_with_state() returns 0 if matched,
        // else it returns the number of bytes counting from the head that are yet to be matched.
        //

        int32 match_bit_pattern_with_state(const int32 idx, const int32 offset, const uint8 * pattern,
                                           const uint8 pattern_size_in_bits, const int32 state);


        //
        // Function seek() moves the current position forward (if positive) or backward (if negative) by
        // "delta_in_bytes". It returns false if error is encountered,
        // else return true and change idx, offset and ptr to the new position.
        // If boundary is reached, boundaryReached is set to true.
        //
        bool seek(int & idx, int & offset, uint8 * & ptr, bool & boundaryReached,
                  const int delta_in_bytes);


    private:

};






class DefaultAUAlloc : public AccessUnitAlloc
{
    public:
        DefaultAUAlloc(AUImplAllocator * impl_alloc)
        {
            au_impl_alloc = impl_alloc;
        };

        virtual ~DefaultAUAlloc() {}

        virtual AccessUnit* allocate(void * hint = 0, const int num_reserved_frags = 1)
        {
            return OSCL_NEW(AccessUnit, (au_impl_alloc, 0, num_reserved_frags));
        }

        virtual void deallocate(AccessUnit* au)
        {
            OSCL_DELETE(au);
        }
    private:
        AUImplAllocator * au_impl_alloc;
};

class DefaultAUImplAllocator : public AUImplAllocator
{

    public:
        virtual ~DefaultAUImplAllocator() {}
        virtual AccessUnitImplementation * allocate(void * hint, const int num_reserved_frags = 1)
        {
            return OSCL_NEW(AccessUnitImplementation, (num_reserved_frags));
        }

        virtual void deallocate(AccessUnitImplementation * au_implp)
        {
            OSCL_DELETE(au_implp);
        }
};


#endif
