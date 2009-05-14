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

#include "buf_frag_group.h"
#include "oscl_string_utils.h"
//
// the follow routine seeks forward or backward, depending on delta_in_bytes
// It returns true if successfully seeked to the desired place or reached BufFragGroup boundary; the
// latter case the flag boundaryReached is set to ture, and ptr to the last byte if it is the
// end boundary reached, or points to first byte if the begin of the AccessUnit is reached..
// Otherwise it returns false when errors such as state corruption are detected.
//

bool seekBufFragGroup(BufferFragment * fragments,
                      const int        numFrags,
                      int & idx,
                      int & offset,
                      uint8 * & ptr,
                      bool & boundaryReached,
                      const int delta_in_bytes,
                      const int num_reserved_frags)
{
    if (idx + num_reserved_frags >= numFrags
            || idx < 0
            || num_reserved_frags < 0)
    {
        boundaryReached = true;
        return false;
    }
    int32 first_frag_size = fragments[idx+num_reserved_frags].len - offset;

    boundaryReached = false;

    BufferFragment * frag;

    if (first_frag_size < 0)
    {
        // state inconsistency
        return false;
    }

    if (delta_in_bytes > 0)   // seek forward
    {
        frag = & fragments[idx+num_reserved_frags];

        if (first_frag_size >= delta_in_bytes)
        {
            offset += delta_in_bytes;

            ptr = (uint8 *) frag->ptr + offset;
            //        ptr = (this->GetMediaFragment(idx))->ptr + offset + delta_in_bytes;
            return true;
        }
        int32 remain = delta_in_bytes - first_frag_size;
        idx++;
        while (idx + num_reserved_frags < numFrags)
        {
            frag = & fragments[idx+num_reserved_frags];
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
        // reached end of frag group

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
            frag = & fragments[idx+num_reserved_frags];
            if (NULL == frag)
                return false;
            ptr = (uint8 *) frag->ptr + offset;  // should use GetMediaFragment
            return true;
        };

        int32 remain = delta_in_bytes + offset;
        while (idx >= 0)
        {
            frag = & fragments[idx+num_reserved_frags];
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
        frag = & fragments[num_reserved_frags];
        ptr = (uint8 *) frag->ptr;
        boundaryReached = true;
        return true;
    }
}
