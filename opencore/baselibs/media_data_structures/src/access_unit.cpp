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

#include "access_unit.h"
#include "access_unit_impl.h"
#include "oscl_mem.h"

OSCL_EXPORT_REF
AccessUnit::AccessUnit(AUImplAllocator* in_alloc, void * hint, const int num_reserved_frags)
{
    alloc = in_alloc;
    impl_hint = hint;
    if (!hint)
    {
        if (!(hint = OSCL_MALLOC(sizeof(AccessUnitImplementation))))
        {
            rep = NULL;
            return;
        }
    }
    rep = in_alloc->allocate(hint, num_reserved_frags);
}

OSCL_EXPORT_REF AccessUnit::~AccessUnit()
{
    alloc->deallocate(rep);
    if (!impl_hint && rep)
    {
        OSCL_FREE(rep);
    }
}

OSCL_EXPORT_REF
BufferFragment * AccessUnit::GetCodecInfo() const
{
    return rep->GetCodecInfo();
};

OSCL_EXPORT_REF
MediaStatusClass::status_t AccessUnit::AddAUFrag(const BufferFragment& frag, BufferState* buffer_state,
        int32 location_offset)
{
    return rep->AddAUFrag(frag, buffer_state, location_offset);
}

MediaStatusClass::status_t AccessUnit::AddLocalAUFrag(BufferFragment& fragment,
        int32 location_offset)
{
    return rep->AddLocalAUFrag(fragment, location_offset);
}

OSCL_EXPORT_REF
MediaStatusClass::status_t AccessUnit::AddCodecInfo(BufferFragment& fragment)
{
    return rep->AddCodecInfo(fragment);
}

OSCL_EXPORT_REF
MediaStatusClass::status_t AccessUnit::AddCodecInfo(void *ptr, int len)
{
    return rep->AddCodecInfo(ptr, len);
}

MediaStatusClass::status_t AccessUnit::GetLocalFragment(BufferFragment& fragment)
{
    return rep->GetLocalFragment(fragment);
}


uint32 AccessUnit::GetAvailableBufferSize() const
{
    return rep->GetAvailableBufferSize();
}

uint32 AccessUnit::GetLocalBufsize() const
{
    return rep->GetLocalBufsize();
}

OSCL_EXPORT_REF void AccessUnit::Clear()
{
    rep->Clear();
}

OSCL_EXPORT_REF MediaTimestamp AccessUnit::GetTimestamp() const
{
    return rep->GetTimestamp();
}

OSCL_EXPORT_REF void AccessUnit::SetTimestamp(const MediaTimestamp& media_timestamp)
{
    rep->SetTimestamp(media_timestamp);
}

const BufferFragment* AccessUnit::GetMediaFragments() const
{
    return rep->GetMediaFragments();
}

OSCL_EXPORT_REF
int32 AccessUnit::GetNumFrags() const
{
    return rep->GetNumMediaFrags();
}
int32 AccessUnit::GetMaxFrags() const
{
    return rep->GetMaxMediaFrags();
}

OSCL_EXPORT_REF uint32 AccessUnit::GetLength() const
{
    return rep->GetMediaLength();
}

OSCL_EXPORT_REF
BufferState * AccessUnit::GetBufferState(const uint32 idx)const
{
    return rep->GetBufferState(idx);
}

void AccessUnit::AppendNext(AccessUnit *next_ptr)
{
    rep->AppendNext(next_ptr);
}
OSCL_EXPORT_REF
void AccessUnit::GetMediaFragment(uint32 index, BufferFragment& frag, BufferState*& buffer_state) const
{
    rep->GetMediaFragment(index, frag, buffer_state);
}


OSCL_EXPORT_REF
bool AccessUnit::IsLocalData(const OsclMemoryFragment& frag) const
{
    return rep->IsLocalData(frag);
}

OSCL_EXPORT_REF
AccessUnit* AccessUnit::GetNext() const
{
    return rep->GetNext();
}

OSCL_EXPORT_REF
bool AccessUnit::seek(int & idx, int & offset, uint8 * & ptr, bool & boundaryReached,
                      const int delta_in_bytes) const
{
    return rep->seek(idx, offset, ptr, boundaryReached, delta_in_bytes);
}

OSCL_EXPORT_REF
bool AccessUnit::match_bit_pattern_no_state(const int idx, const int offset, const uint8 * pattern,
        const uint8 pattern_size_in_bits) const
{
    return rep->match_bit_pattern_no_state(idx, offset, pattern, pattern_size_in_bits);
}

OSCL_EXPORT_REF
int32 AccessUnit::match_bit_pattern_with_state(const int32 idx, const int32 offset, const uint8 * pattern,
        const uint8 pattern_size_in_bits, const int32 state) const
{
    return rep->match_bit_pattern_with_state(idx, offset, pattern, pattern_size_in_bits, state);
}
/*
int32 AccessUnit::GetNumMediaFrags() const {
  return rep->GetNumMediaFrags();
}
*/
OSCL_EXPORT_REF
BufferFragment * AccessUnit::GetMediaFragment(const uint32 idx) const
{
    return rep->GetMediaFragment(idx);
}


/*
void AccessUnit::reset() {
  return rep->reset();
}
*/
