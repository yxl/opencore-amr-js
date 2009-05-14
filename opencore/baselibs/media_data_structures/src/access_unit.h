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
#ifndef ACCESS_UNIT_H
#define ACCESS_UNIT_H

#include "oscl_media_data.h"

// Constants controlling the size of the AU container
const int AU_MAX_FRAGMENTS = 30;
const int AU_IMMEDIATE_DATA = 1500;

const int DEF_AU_FRAGS = 10;
const int DEF_AU_IMMEDIATE_DATA = 100;

class AccessUnitImplementation;

typedef MemAllocator<AccessUnitImplementation> AUImplAllocator;

class AccessUnit
{
    public:

        OSCL_IMPORT_REF AccessUnit(AUImplAllocator* in_alloc, void *hint = 0, const int num_reserved_frags = 1);
        OSCL_IMPORT_REF ~AccessUnit();


        OSCL_IMPORT_REF MediaStatusClass::status_t AddAUFrag(const BufferFragment& frag, BufferState* buffer_state,
                int32 location_offset = APPEND_MEDIA_AT_END);

        OSCL_IMPORT_REF MediaStatusClass::status_t AddCodecInfo(BufferFragment& fragment);
        OSCL_IMPORT_REF MediaStatusClass::status_t AddCodecInfo(void *ptr, int len);

        OSCL_IMPORT_REF void Clear();

        // void reset();


        OSCL_IMPORT_REF BufferFragment * GetCodecInfo() const;

        MediaStatusClass::status_t GetLocalFragment(BufferFragment& fragment);

        MediaStatusClass::status_t AddLocalAUFrag(BufferFragment& fragment, int32 location_offset = APPEND_MEDIA_AT_END);

        uint32 GetAvailableBufferSize() const;
        uint32 GetLocalBufsize() const;

        OSCL_IMPORT_REF MediaTimestamp GetTimestamp() const;

        OSCL_IMPORT_REF void SetTimestamp(const MediaTimestamp& timestamp);

        OSCL_IMPORT_REF const BufferFragment* GetMediaFragments() const;

        OSCL_IMPORT_REF BufferState * GetBufferState(const uint32 idx)const;

        OSCL_IMPORT_REF void GetMediaFragment(uint32 index, BufferFragment& frag, BufferState*& buffer_state) const;
        OSCL_IMPORT_REF BufferFragment * GetMediaFragment(const uint32 idx)const;

        OSCL_IMPORT_REF int32 GetNumFrags() const;
        // int32 GetNumMediaFrags() const;
        OSCL_IMPORT_REF int32 GetMaxFrags() const;
        OSCL_IMPORT_REF uint32 GetLength() const;
        //  uint32 GetSize() const { return GetLength(); };

        //
        // The following method emphasize the fact that MediaSize does not count reserved stuff
        //
        uint32 GetMediaSize() const
        {
            return GetLength();
        };

        void AppendNext(AccessUnit *next_ptr);

        OSCL_IMPORT_REF bool IsLocalData(const OsclMemoryFragment& frag) const;

        OSCL_IMPORT_REF AccessUnit* GetNext() const;

        //
        // Function match_bit_pattern_with_state() returns 0 if matched,
        // else it returns the number of bytes counting from the head that are yet to be matched.
        //
        OSCL_IMPORT_REF
        int32 match_bit_pattern_with_state(const int32 idx, const int32 offset, const uint8 * pattern,
                                           const uint8 pattern_size_in_bits, const int32 state) const;


        //
        // Function seek() moves the current position forward (if positive) or backward (if negative) by
        // "delta_in_bytes". It returns false if error is encountered,
        // else return true and change idx, offset and ptr to the new position.
        // If boundary is reached, boundaryReached is set to true.
        //
        OSCL_IMPORT_REF bool seek(int & idx, int & offset, uint8 * & ptr, bool & boundaryReached,
                                  const int delta_in_bytes) const;

        //
        // Function match_bit_pattern_with_state()
        // returns true if matched,
        // else return false;
        //
        OSCL_IMPORT_REF bool match_bit_pattern_no_state(const int idx, const int offset, const uint8 * pattern,
                const uint8 pattern_size_in_bits) const;

    private:
        AUImplAllocator* alloc;
        AccessUnitImplementation* rep;
        void *impl_hint;

};

typedef MemAllocator<AccessUnit> AccessUnitAlloc;

#endif
