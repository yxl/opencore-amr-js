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
/************************************************************************/
/*  file name       : media_fragment.h                                           */
/*  file contents   : media fragment class & utilities       */
/*  draw            : '96.10.29                                         */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*      Copyright (C) 2002 PacketVideo                                   */
/************************************************************************/

#ifndef MEDIA_FRAGMENT_H
#define MEDIA_FRAGMENT_H

#include "oscl_mem.h"
#include "oscl_media_data.h"

class DefaultBufferMgr : public BufferMgr
{
    public:
        DefaultBufferMgr()
        {

        }
        virtual ~DefaultBufferMgr()
        {

        }
        OsclAny BufferReleased(OsclAny* ptr, BufferState* state = NULL)
        {
            OSCL_DEFAULT_FREE(ptr);
            if (state)
                OSCL_DELETE(state);
        }
};


/* the memory pool manager should allocate this.  The free function pointer iin the
buffer_state should point to a routine that returns it to the pool */

class MediaFragment
{
    public:
        MediaFragment(): _state(NULL)
        {
            _fragment.ptr = NULL;
            _fragment.len = 0;
        }
        OSCL_IMPORT_REF MediaFragment(const MediaFragment& that, OsclAny* ptr = NULL, int32 len = 0);

        virtual ~MediaFragment()
        {
            if (_state)
                _state->decrement_refcnt();
        }
        inline uint8* GetPtr()
        {
            return (uint8*)_fragment.ptr;
        }
        inline int32 GetLen()
        {
            return _fragment.len;
        }

        OsclAny Trim(uint8* ptr, int32 len)
        {
//		if( (ptr >= _fragment.ptr && ptr < ((uint8*)_fragment.ptr+_fragment.len)) &&
//			(((uint8*)ptr +len) <= ((uint8*)_fragment.ptr+_fragment.len)) ){
            _fragment.ptr = ptr;
            _fragment.len = len;
//		}
        }

        virtual BufferFragment* GetFragment()
        {
            return &_fragment;
        }
        virtual BufferState* GetState()
        {
            return _state;
        }

    protected:
        BufferFragment _fragment;
        BufferState* _state;
    private:
        MediaFragment(BufferFragment& frag, BufferState* state):
                _fragment(frag), _state(state)
        {
            if (state)
                _state->increment_refcnt();
        }

        OsclAny Set(BufferFragment& frag, BufferState* state)
        {
            BufferState* old_state = _state;
            _state = state;

            if (_state)
            {
                _state->increment_refcnt();
            }
            _fragment = frag;
            if (old_state)
            {
                old_state->decrement_refcnt();
            }
        }

        OsclAny Set(OsclAny* ptr, int32 len, BufferState* state)
        {
            BufferState* old_state = _state;
            _state = state;

            if (_state)
            {
                _state->increment_refcnt();
            }
            _fragment.len = len;
            _fragment.ptr = ptr;

            if (old_state)
            {
                old_state->decrement_refcnt();
            }
        }

        friend class DefFragmentAllocator;
        friend class PoolFragmentAllocator;
        friend class MediaPacket;

        friend class AudioEncodeFragmentAllocator;
        friend class VideoEncodeFragmentAllocator;
};

typedef int32 FragmentType;
const FragmentType FRAG_TYPE_DEFAULT = 0;

/* Default media fragment and allocator. */
class FragmentAllocator
{
    public:
        virtual ~FragmentAllocator() {}
        OSCL_IMPORT_REF virtual MediaFragment* allocate_fragment(uint32 sz) = 0;
        OSCL_IMPORT_REF virtual MediaFragment* allocate_fragment(MediaFragment* fragment,
                OsclAny* ptr = NULL, uint32 len = 0) = 0;
        OSCL_IMPORT_REF virtual int32 deallocate_fragment(MediaFragment* fragment) = 0;
};

class DefFragmentAllocator :  public FragmentAllocator
{
    public:
        DefFragmentAllocator()
        {

        }
        ~DefFragmentAllocator()
        {

        }
        OSCL_IMPORT_REF virtual MediaFragment* allocate_fragment(uint32 sz);
        OSCL_IMPORT_REF virtual MediaFragment* allocate_fragment(MediaFragment* fragment,
                OsclAny* ptr = NULL, uint32 len = 0);
        OSCL_IMPORT_REF virtual int32 deallocate_fragment(MediaFragment* fragment);
    private:
        DefaultBufferMgr buffer_state_listener;
};


/* Media fragment pool and allocator. */

class BufferStateEntry
{
    public:
        BufferStateEntry() : next_entry(NULL)
        {
            frag.ptr = NULL;
            frag.len = 0;
        };
        ~BufferStateEntry() {};
        BufferState buf_state;
        BufferFragment frag;
        MediaFragment media_frag;
        BufferStateEntry *next_entry;
};

class PoolFragmentAllocator;

class BufferPoolMgr : public BufferMgr
{
    public:
        BufferPoolMgr(PoolFragmentAllocator *pPool)
        {
            pBufferPool = pPool;
        }
        virtual ~BufferPoolMgr() {};
        OSCL_IMPORT_REF OsclAny BufferReleased(OsclAny* ptr, BufferState* state = NULL);
    private:
        PoolFragmentAllocator *pBufferPool;
};


class PoolFragmentAllocator :  public FragmentAllocator
{
    public:
        OSCL_IMPORT_REF PoolFragmentAllocator(int32 max_frags, int32 max_frag_size);
        OSCL_IMPORT_REF ~PoolFragmentAllocator();

        OSCL_IMPORT_REF virtual MediaFragment* allocate_fragment(uint32 sz);
        OSCL_IMPORT_REF virtual MediaFragment* allocate_fragment(MediaFragment* fragment,
                OsclAny* ptr = NULL, uint32 len = 0);
        OSCL_IMPORT_REF virtual int32 deallocate_fragment(MediaFragment* fragment);

        OsclAny free_buffer_to_pool(BufferStateEntry *entry);
    private:

        int32 num_frags;
        int32 frag_size;
        BufferPoolMgr *buffer_state_listener;
        BufferStateEntry *free_list;
        BufferStateEntry *entire_list;
        uint8 *buffer;
};



#endif /* MEDIA_FRAGMENT_H */
