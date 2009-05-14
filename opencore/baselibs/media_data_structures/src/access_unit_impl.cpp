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
#include "access_unit_impl.h"
#include "oscl_string_utils.h"

/* **********************************************
 *
 *  NOTE: Throughout this implementation, fragment number 0
 *  is reserved for the Codec Info.
 *
 * **********************************************/

OSCL_EXPORT_REF
AccessUnitImplementation::AccessUnitImplementation(const int num_reserved_frags)
{
    // the default constructor for BufFragGroup takes care of other things
    timestamp = 0;
    length = 0;

    num_fragments = num_reserved_fragments = num_reserved_frags;

    // reserve fragment 0 for CodecInfo
    fragments[0].ptr = NULL;
    fragments[0].len = 0;
    buffer_states[0] = NULL;

}


AccessUnitImplementation::~AccessUnitImplementation()
{
    // decrement the ref count on all buffers
    for (uint ii = 0; ii < num_fragments; ++ii)
    {
        if (buffer_states[ii])
        {
            buffer_states[ii]->decrement_refcnt();
        }
    }
}

MediaStatusClass::status_t AccessUnitImplementation::AddCodecInfo(BufferFragment& fragment)
{
    if (fragments[0].ptr != NULL)
    {
        // already codec info
        return MediaStatusClass::FIXED_FRAG_LOC_FULL;
    }
    return AddLocalFragment(fragment, (int32) 0);
}

MediaStatusClass::status_t AccessUnitImplementation::AddCodecInfo(void *ptr, int len)
{
    if (fragments[0].ptr != NULL)
    {
        // already codec info
        return MediaStatusClass::FIXED_FRAG_LOC_FULL;
    }
    BufferFragment fragment;
    fragment.ptr = ptr;
    fragment.len = len;
    return AddLocalFragment(fragment, (int32) 0);
}


MediaStatusClass::status_t AccessUnitImplementation::AddAUFrag(const BufferFragment& frag, BufferState* buffer_state,
        int32 location_offset)
{
    location_offset = (location_offset >= 0) ? location_offset + 1 : location_offset;
    return AddFragment(frag, buffer_state, location_offset);
}

MediaStatusClass::status_t AccessUnitImplementation::AddLocalAUFrag(const BufferFragment& frag, int32 location_offset)
{
    location_offset = (location_offset >= 0) ? location_offset + 1 : location_offset;
    return AddLocalFragment(frag, location_offset);
}

void AccessUnitImplementation::GetMediaFragment(uint32 index, BufferFragment& frag, BufferState*& buffer_state) const
{

    if (index + num_reserved_fragments >= num_fragments)
    {
        buffer_state = NULL;
        frag.ptr = NULL;
        frag.len = 0;
        return;
    }

    frag.ptr = fragments[index + num_reserved_fragments].ptr;
    frag.len = fragments[index + num_reserved_fragments].len;
    buffer_state = buffer_states[index + num_reserved_fragments];

}


// bit_pattern_masks assume MSB first
const uint8 bit_pattern_masks[7] =
{
    0xfe, 0xfc, 0xf8, 0xf0, 0xE0, 0xC0, 0x80
};

//
// the follow routine seeks forward or backward, depending on delta_in_bytes
// It returns true if successfully seeked to the desired place or reached MediaData boundary; the
//   latter case the flag boundaryReached is set to ture, and ptr to the last byte if it is the
// end boundary reached, or points to first byte if the begin of the AccessUnit is reached..
// Otherwise it returns false when errors such as state corruption are detected.
//
bool AccessUnitImplementation::seek(int & idx, int & offset, uint8 * & ptr, bool & boundaryReached,
                                    const int delta_in_bytes)
{
    int32 first_frag_size = this->fragments[idx+num_reserved_fragments].len - offset;

    boundaryReached = false;

    BufferFragment * frag;

    if (first_frag_size < 0)
    {
        // state inconsistency
        return false;
    }

    if (delta_in_bytes > 0)   // seek forward
    {

        frag = GetMediaFragment(idx);
        if (NULL == frag)
            return false;

        if (first_frag_size >= delta_in_bytes)
        {
            offset += delta_in_bytes;

            ptr = (uint8 *) frag->ptr + offset;
            //        ptr = (this->GetMediaFragment(idx))->ptr + offset + delta_in_bytes;
            return true;
        }
        int32 remain = delta_in_bytes - first_frag_size;
        idx++;
        while (idx < (int) this->GetNumMediaFrags())
        {
            frag = GetMediaFragment(idx);
            if (NULL == frag)
                return false;

            if ((int32) frag->len >= remain)   // found
            {
                offset = remain;
                ptr = (uint8 *) frag->ptr + remain;
                return true;
            }
            else
            {
                remain -= frag->len;
                idx++;
            }
        }
        // reached end of MediaData
        frag = GetMediaFragment(idx);
        if (NULL == frag)
            return false;
        offset = frag->len;
        ptr = (uint8 *) frag->ptr + offset - 1;
        boundaryReached = true;
        return true;
    }
    else   // delta is negative, seek backwards
    {
        if (offset >= oscl_abs(delta_in_bytes))
        {
            offset += delta_in_bytes;
            frag = GetMediaFragment(idx);
            if (NULL == frag)
                return false;
            ptr = (uint8 *) frag->ptr + offset;  // should use GetMediaFragment
            return true;
        }
        else
        {
            boundaryReached = true;
            return true;
        }

        int32 remain = delta_in_bytes + offset;
        while (idx >= 0)
        {
            frag = GetMediaFragment(idx);
            if (NULL == frag)
                return false;
            if ((int32) frag->len > oscl_abs(remain))
            {
                offset = frag->len + remain;
                ptr = (uint8 *) frag->ptr + remain;
                return true;
            }
            else
            {
                remain += frag->len;
            }
            idx--;
        }

        // We are back to the begining of the MediaData. Return (0, 0)
        idx = offset = 0;
        frag = GetMediaFragment(idx);
        if (NULL == frag)
            return false;
        ptr = (uint8 *) frag->ptr;
        boundaryReached = true;
        return true;
    }
}

// returns true if matched or error happened (such as bound reached unexpectedly);
// else return false;
bool AccessUnitImplementation::match_bit_pattern_no_state(const int idx, const int offset, const uint8 * pattern,
        const uint8 pattern_size_in_bits)
{
    uint8 num_bytes = pattern_size_in_bits / 8;
    uint8 bits_in_fraction = pattern_size_in_bits % 8;

    uint8 bit_mask = 0xff; // start with full mask

    if (bits_in_fraction > 0)
    {
        // build the bit mask for the fractional byte
        bit_mask = bit_pattern_masks[7 - bits_in_fraction];
        /*
         uint8 tmp = 1;

         for (uint8 ii=0; ii<(8-bits_in_fraction); ii++) {
           bit_mask -= tmp;
           tmp *= 2;
         }
         */
    }
    BufferFragment *   frag = GetMediaFragment(idx);
    if (NULL == frag)
        return false;

    octet * tmp_ptr = (uint8 *) frag->ptr + offset;

    // Either num_bytes = 0 or no match;
    // Now we need to figure out next state
    bool boundaryReached;

    /* the following does not work due to endianess
    uint32 tmp_marker = *(uint32 *) tmp_ptr;
    if (tmp_marker == 0x00008000 & 0xffff8000) {
     // printf("Resysnch marker found at idx = %d, offset = %d\n", idx, offset);
      //      exit(1);
      return true;
    }
    */

    int tmp_idx = idx;
    int tmp_offset = offset;
    for (uint8 ii = 0; ii < num_bytes; ii++)
    {
        if ((* tmp_ptr) != pattern[ii])
        {
            return false;
        }
        else
        {
            this->seek(tmp_idx, tmp_offset, tmp_ptr, boundaryReached, 1);
            if (boundaryReached)
            {
                return true;
            }
        }
    }

    // printf("The whole %d octets are matched, now matching %d bits in the fraction byte\n", num_bytes, bits_in_fraction);
    if (bits_in_fraction)
    {
        if (((*tmp_ptr) & bit_mask) == (pattern[num_bytes] & bit_mask))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }

} // end of method

// state indicates the num of bytes to move back to.
int32 AccessUnitImplementation::match_bit_pattern_with_state(const int32 idx, const int32 offset, const uint8 * pattern,
        const uint8 pattern_size_in_bits, const int32 state)
{
    uint8 num_bytes = pattern_size_in_bits / 8;
    uint8 bits_in_fraction = pattern_size_in_bits % 8;

    uint8 bit_mask = 0xff; // start with full mask

    if (bits_in_fraction > 0)
    {
        // build the bit mask for the fractional byte
        bit_mask = bit_pattern_masks[7 - bits_in_fraction];
        /*
         uint8 tmp = 1;

         for (uint8 ii=0; ii<(8-bits_in_fraction); ii++) {
           bit_mask -= tmp;
           tmp *= 2;
         }
         */
    }

    BufferFragment *  frag = GetMediaFragment(idx);
    if (NULL == frag)
    {
        // printf("Error in pattern_match_with_state\n");
        return 0;
    }
    if (offset >= (int) frag->len)
    {
        return 1;
    }
    octet * tmp_ptr = (uint8 *) frag->ptr + offset;

    // uint8 num_bytes_to_match = state;

    uint8 num_bits_to_match;
    if (state*8 > pattern_size_in_bits)
    {
        num_bits_to_match = pattern_size_in_bits;
    }
    else
    {
        num_bits_to_match = state * 8;
    }
    if ((num_bytes > 0) && this->match_bit_pattern_no_state(idx, offset, pattern, num_bits_to_match))
    {
        return 0;  // 0 means matched;
    }
    else
    {
        // Either num_bytes = 0 or no match;
        // Now we need to figure out next state
        for (uint8 ii = 1; ii < num_bytes; ii++)
        {
            if (this->match_bit_pattern_no_state(idx, offset, (pattern + ii), pattern_size_in_bits - ii*8))
            {
                return ii;
            }
        }

        if (bits_in_fraction && ((*tmp_ptr & bit_mask) == (pattern[num_bytes] & bit_mask)))
        {
            return num_bytes;
        }
        else
        {
            return (num_bytes + 1);
        }
    }
} // end of method





