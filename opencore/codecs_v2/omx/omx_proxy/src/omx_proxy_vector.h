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
#ifndef OMX_PROXY_VECTOR_H_INCLUDED
#define OMX_PROXY_VECTOR_H_INCLUDED

#ifndef OSCL_DEFALLOC_H_INCLUDED
#include "oscl_defalloc.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_BASE_ALLOC_H_INCLUDED
#include "oscl_base_alloc.h"
#endif

#ifndef PV_OMXDEFS_H_INCLUDED
#include "pv_omxdefs.h"
#endif

#if PROXY_INTERFACE

//Memory allocator/deallocator class that don't use OSCL TLS
class Oscl_Vector_Allocator : public Oscl_DefAlloc
{
    public:
        virtual ~Oscl_Vector_Allocator() {};

        OsclAny* allocate(const uint32 size)
        {
            OsclAny* tmp = iDefAlloc.allocate(size);

            return tmp;
        }

        OsclAny deallocate(OsclAny* p)
        {
            iDefAlloc.deallocate(p);
        }
    private:
        _OsclBasicAllocator iDefAlloc;
};

/**	Added a new class with similar functionality but a different fail mechanism
*   than the oscl_try & oscl_leave methods
*   New class so that we need not make private members of oscl_vector as protected
*   These routines can be used without oscl initializations,
*   as they don't depend upon TLS
**/

template<class T, class Alloc>
class Oscl_Vector_New
{
    public:
        typedef T value_type;
        typedef T* pointer;
        typedef const T& const_reference;
        typedef T* iterator;

        Oscl_Vector_New() :
                elems(0), numelems(0), bufsize(0)
        {}

        Oscl_Vector_New(uint32 n) :
                numelems(0), bufsize(n)
        {
            elems = defAlloc.ALLOCATE(bufsize);
        }

        Oscl_Vector_New(const Oscl_Vector_New<T, Alloc>& x)
        {
            numelems = x.numelems;
            bufsize = x.numelems; // only allocate enough for current elements
            elems = defAlloc.ALLOCATE(bufsize);
            uninitialized_copy(x.begin(), x.end(), begin());
        }

        virtual ~Oscl_Vector_New()
        {
            if (elems)
            {
                destroy(begin(), end());
                defAlloc.deallocate(elems);
            }
        }

        Oscl_Vector_New<T, Alloc>& operator=(const Oscl_Vector_New<T, Alloc>& x)
        {
            if (&x != this)
            {
                if (x.size() > capacity())
                {
                    // allocate space and copy
                    T* tmp = defAlloc.ALLOCATE(x.end() - x.begin());
                    uninitialized_copy(x.begin(), x.end(), tmp);
                    destroy(begin(), end());
                    if (elems)
                        defAlloc.deallocate(elems);
                    elems = tmp;
                    bufsize = x.size();
                }
                else if (size() >= x.size())
                {
                    iterator i = copy(x.begin(), x.end(), begin());
                    destroy(i, end());
                }
                else
                {
                    copy(x.begin(), x.begin() + size(), begin());
                    uninitialized_copy(x.begin() + size(), x.end(), end());
                }
                numelems = x.size();
            }
            return *this;
        }

        uint32 size() const
        {
            return numelems;
        }
        bool empty() const
        {
            return numelems == 0;
        }
        uint32 capacity() const
        {
            return bufsize;
        }

        bool reserve(uint32 n)
        {
            if (n > bufsize)
            {
                T *oldelems = elems;
                elems = defAlloc.ALLOCATE(n);

                if (elems == NULL)
                    return false;

                for (uint32 i = 0; i < numelems; i++)
                    construct(&elems[i], oldelems[i]);

                if (oldelems)
                {
                    destroy(oldelems, oldelems + numelems);
                    defAlloc.deallocate(oldelems);
                }
                bufsize = n;
            }
            return true;
        }

        bool push_back(const T& x)
        {
            if (numelems == bufsize)
            {
                uint32 new_bufsize = (bufsize) ? 2 * bufsize : 2;
                if ((status = reserve(new_bufsize)) == false)
                    return false;
            }
            construct(end(), x);
            numelems++;
            return true;
        }

        void clear()
        {
            erase(begin(), end());
        }

        T& operator[](uint32 n)
        {
            return (*(begin() + n));
        }

        const T& operator[](uint32 n) const
        {
            return (*(begin() + n));
        }

        void destroy()
        {
            if (elems)
            {
                destroy(begin(), end());
                defAlloc.deallocate(elems);
                elems = NULL;
                numelems = 0;
                bufsize = 0;
            }
        }

        iterator begin() const
        {
            return elems;
        }

        iterator end() const
        {
            return elems + numelems;
        }

        iterator erase(iterator pos)
        {
            if (pos + 1 != end()) copy(pos + 1, end(), pos);
            numelems--;
            destroy(end());
            return pos;
        }

        iterator erase(iterator first, iterator last)
        {
            iterator it = copy(last, end(), first);
            destroy(it, end());
            numelems -= (last - first);
            return first;
        }

    private:
        bool status;
        T *elems;
        uint32 numelems;
        uint32 bufsize;
        Oscl_TAlloc<T, Alloc> defAlloc;

        void construct(pointer p, const_reference x)
        {
            OSCL_PLACEMENT_NEW(p, value_type(x));
        }

        iterator copy(iterator first, iterator last, iterator result)
        {
            while (first != last) *result++ = *first++;
            return result;
        }
        iterator uninitialized_copy(iterator first, iterator last, iterator result)
        {
            while (first != last) construct(result++, *first++);
            return result;
        }

        void destroy(iterator first)
        {
            OSCL_UNUSED_ARG(first);
            first->~T();
        }
        void destroy(iterator first, iterator last)
        {
            while (first != last)
            {
                first->~T();
                first++;
            }
        }

};
#endif //PROXY_INTERFACE
#endif

