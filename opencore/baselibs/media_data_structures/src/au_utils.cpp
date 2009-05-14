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
#include "au_utils.h"
#include "access_unit_impl.h"

int skip_dropped_samples(const GAU * gauPtr, const uint32 starting_idx)
{
    uint32 sample_num = starting_idx;

    GAU * gau = const_cast<GAU *>(gauPtr);

    MediaMetaInfo * info = gau->getInfo(sample_num);

    while (info && (info->dropFlag == true))
    {
        // printf ("in skip_dropped_samples, skipping sample_num %lu with time %lu\n", sample_num, info->ts);
        sample_num++;
        info = gau->getInfo(sample_num);
    }

    if (sample_num < gauPtr->numMediaSamples)
    {
        // printf ("in skip_dropped_samples, keeping sample_num %lu with time %lu\n", sample_num, info->ts);
    }

    return sample_num;
}


/* Converts a GAU to a list of chained AccessUnits.  The AUs have to be deleted
   after they are done with.
   */
OSCL_EXPORT_REF AccessUnit*
Gau2AU(const GAU* gau,
       AUImplAllocator * au_impl_alloc,
       MediaStatusClass::status_t& media_status,
       bool ignore_sample_info)
{
    BufferFragment add_frag;

    const BufferFragment* cur_frag = NULL;
    BufferState*    cur_buf_state = NULL;
    AccessUnit* cur_au = NULL, *au_chain = NULL;
    int bytes_left_in_frag = 0;
    void* frag_ptr = NULL;
    int bytes_left_in_sample = 0;

    uint32 sample_num = 0;

    sample_num = skip_dropped_samples(gau, 0);

    if (sample_num >= gau->numMediaSamples)
        return NULL;

    media_status = MediaStatusClass::BFG_SUCCESS;

    // create the first AU
    au_chain = cur_au = OSCL_NEW(AccessUnit, (au_impl_alloc, 0, 1));
    cur_au->SetTimestamp(gau->info[sample_num].ts);

    BufferFragment tmp_frag;
    if (!ignore_sample_info)
    {
        tmp_frag.ptr = (void *) & (gau->info[sample_num].sample_info);
        tmp_frag.len = sizeof(gau->info[sample_num].sample_info);
        cur_au->AddCodecInfo(tmp_frag);
    }

    bytes_left_in_sample = gau->info[sample_num].len;

    for (int fragnum = 0; fragnum < gau->buf.num_fragments; fragnum++)
    {
        cur_frag = &gau->buf.fragments[fragnum];
        cur_buf_state = gau->buf.buf_states[fragnum];
        bytes_left_in_frag = cur_frag->len;
        frag_ptr = cur_frag->ptr;


        while (bytes_left_in_frag >= 0)
        {
            if (bytes_left_in_sample <= 0)
            {
                // close the current AU
                sample_num = skip_dropped_samples(gau, sample_num + 1);

                if (sample_num >= gau->numMediaSamples)
                    goto leave;

                cur_au->AppendNext(OSCL_NEW(AccessUnit, (au_impl_alloc, 0, 1)));
                cur_au = cur_au->GetNext();
                if (sample_num >= gau->numMediaSamples)
                    goto leave;
                cur_au->SetTimestamp(gau->info[sample_num].ts);
                if (!ignore_sample_info)
                {
                    tmp_frag.ptr = (void *) & (gau->info[sample_num].sample_info);
                    tmp_frag.len = sizeof(gau->info[sample_num].sample_info);
                    cur_au->AddCodecInfo(tmp_frag);
                }
                bytes_left_in_sample = gau->info[sample_num].len;
            }
            // add frag to au
            add_frag.len = OSCL_MIN(bytes_left_in_frag, bytes_left_in_sample);
            add_frag.ptr = frag_ptr;

            if ((media_status = cur_au->AddAUFrag(add_frag, cur_buf_state)) != MediaStatusClass::BFG_SUCCESS)
            {
                goto leave;
            }


            // update frag info
            bytes_left_in_frag -= add_frag.len;
            frag_ptr = ((uint8*)frag_ptr) + add_frag.len;
            // update AU info
            bytes_left_in_sample -= add_frag.len;
        }
    }

leave:
    return au_chain;
}
