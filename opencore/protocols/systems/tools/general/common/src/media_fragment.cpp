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

#include "media_fragment.h"


OSCL_EXPORT_REF MediaFragment::MediaFragment(const MediaFragment& that, OsclAny* ptr, int32 len):
        _fragment(that._fragment), _state(that._state)
{
    if (_state)
        _state->increment_refcnt();
    if (ptr && (len > 0))
    {
        if ((ptr >= _fragment.ptr && ptr < ((uint8*)_fragment.ptr + _fragment.len)) &&
                (((uint8*)ptr + len) <= ((uint8*)_fragment.ptr + _fragment.len)))
        {
            _fragment.ptr = ptr;
            _fragment.len = len;
        }
    }
}

// Default fragment allocator.  Just malloc's and free's memory as needed.
OSCL_EXPORT_REF MediaFragment* DefFragmentAllocator::allocate_fragment(uint32 sz)
{
    BufferFragment frag;
    BufferState* state = NULL;

    frag.ptr = OSCL_DEFAULT_MALLOC(sz);
    frag.len = sz;
    state = OSCL_NEW(BufferState, (&buffer_state_listener, frag.ptr));
    return OSCL_NEW(MediaFragment, (frag, state));
}

OSCL_EXPORT_REF MediaFragment* DefFragmentAllocator::allocate_fragment(MediaFragment* fragment,
        OsclAny* ptr, uint32 len)
{
    return OSCL_NEW(MediaFragment, (*fragment, ptr, len));
}


OSCL_EXPORT_REF int32 DefFragmentAllocator::deallocate_fragment(MediaFragment* fragment)
{
    OSCL_DELETE(fragment);
    return 1;
}



// Buffer pool manager.  Frees the buffer back to the pool.
OSCL_EXPORT_REF OsclAny BufferPoolMgr::BufferReleased(OsclAny* ptr, BufferState* state)
{
    OSCL_UNUSED_ARG(state);

    BufferStateEntry *entry = (BufferStateEntry *) ptr;
    pBufferPool->free_buffer_to_pool(entry);
}

// Buffer fragment pool.  Memory is pre-allocated at constructor time.
OSCL_EXPORT_REF PoolFragmentAllocator::PoolFragmentAllocator(int32 max_frags, int32 max_frag_size)
{
    buffer_state_listener = OSCL_NEW(BufferPoolMgr, (this));

    num_frags = max_frags;
    frag_size = max_frag_size;
    entire_list = new BufferStateEntry[max_frags];
    free_list = entire_list;
    buffer = (uint8 *) OSCL_DEFAULT_MALLOC((max_frags * max_frag_size));
    oscl_memset(buffer, 0, (max_frags * max_frag_size));

    int32 i;
    for (i = 0; i < max_frags; i++)
    {
        entire_list[i].buf_state.bind(&entire_list[i], buffer_state_listener);
        entire_list[i].frag.ptr = buffer + (i * max_frag_size);
        entire_list[i].frag.len = max_frag_size;
        if (i > 0)
        {
            entire_list[i-1].next_entry = &entire_list[i];
        }
    }
    entire_list[i-1].next_entry = NULL;
}

OSCL_EXPORT_REF PoolFragmentAllocator::~PoolFragmentAllocator()
{
    for (int32 i = 0; i < num_frags; i++)
    {
        if (entire_list[i].buf_state.get_refcount() > 0)
        {
            OSCL_ASSERT(0);
        }
    }

    if (buffer) OSCL_DEFAULT_FREE(buffer);
    if (entire_list) delete[] entire_list;
    if (buffer_state_listener) OSCL_DELETE(buffer_state_listener);
}

OSCL_EXPORT_REF MediaFragment* PoolFragmentAllocator::allocate_fragment(uint32 sz)
{
    BufferStateEntry *entry;

    if (free_list && ((int32) sz <= frag_size))
    {
        entry = free_list;
        free_list = free_list->next_entry;

        entry->next_entry = NULL;
        entry->frag.len = sz;
        entry->media_frag.Set(entry->frag, &entry->buf_state);

        return &entry->media_frag;
    }
    else
    {
        return NULL;
    }
}

OSCL_EXPORT_REF MediaFragment* PoolFragmentAllocator::allocate_fragment(MediaFragment* fragment,
        OsclAny* ptr,
        uint32 len)
{
    return OSCL_NEW(MediaFragment, (*fragment, ptr, len));
}

OSCL_EXPORT_REF int32 PoolFragmentAllocator::deallocate_fragment(MediaFragment* fragment)
{
    fragment->Set(NULL, 0, NULL);
    return 1;
}

OsclAny PoolFragmentAllocator::free_buffer_to_pool(BufferStateEntry *entry)
{
    entry->next_entry = free_list;
    free_list = entry;
}


