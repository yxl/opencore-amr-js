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
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                     O S C L _ V E C T O R

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclbase OSCL Base
 * Additional osclbase comment
 * @{
 */

/*! \file oscl_vector.cpp
    \brief Implementation for oscl_vector.h
*/

#include "oscl_vector.h"


OSCL_EXPORT_REF void Oscl_Vector_Base::reserve(uint32 n)
{
    if (n > bufsize)
    {
        OsclAny* oldmem = elems;
        elems = pOpaqueType->allocate(n * sizeof_T);
        OsclAny* newelems = elems;
        OsclAny* oldelems = oldmem;
        for (uint32 i = 0; i < numelems; i++)
        {
            pOpaqueType->construct(newelems, oldelems);
            newelems = increment_T(newelems, 1);
            oldelems = increment_T(oldelems, 1);
        }
        if (oldmem)
        {
            destroy(oldmem, increment_T(oldmem, numelems));
            pOpaqueType->deallocate(oldmem);
        }
        bufsize = n;
    }
}

OSCL_EXPORT_REF void Oscl_Vector_Base::construct(Oscl_Opaque_Type_Alloc* aType)
{
    numelems = 0;
    bufsize = 0;
    elems = NULL;
    pOpaqueType = aType;
}

OSCL_EXPORT_REF void Oscl_Vector_Base::construct(Oscl_Opaque_Type_Alloc* aType, uint32 n)
{
    numelems = 0;
    bufsize = n;
    pOpaqueType = aType;
    elems = pOpaqueType->allocate(n * sizeof_T);
}

OSCL_EXPORT_REF void Oscl_Vector_Base::construct(Oscl_Opaque_Type_Alloc* aType, const Oscl_Vector_Base& x)
{
    numelems = x.numelems;
    bufsize = x.numelems;//only enough for current elements
    pOpaqueType = aType;
    elems = pOpaqueType->allocate(bufsize * sizeof_T);
    uninitialized_copy(x.begin(), x.end(), begin());
}

OSCL_EXPORT_REF void Oscl_Vector_Base::push_back(const OsclAny* x)
{
    if (numelems == bufsize)
    {
        uint32 new_bufsize = (bufsize) ? 2 * bufsize : 2;
        reserve(new_bufsize);
    }
    pOpaqueType->construct(end(), x);
    numelems++;
}

OSCL_EXPORT_REF void Oscl_Vector_Base::pop_back()
{
    numelems--;
    pOpaqueType->destroy(end());
}

OSCL_EXPORT_REF void Oscl_Vector_Base::push_front(const OsclAny* x)
{
    OSCL_ASSERT(x);
    if (numelems == bufsize)
    {
        uint32 new_bufsize = (bufsize) ? 2 * bufsize : 2;
        reserve(new_bufsize);
    }
    for (uint32 i = numelems; i > 0; i--)
    {
        pOpaqueType->construct(increment_T(elems, i), increment_T(elems, i - 1));
        pOpaqueType->destroy(increment_T(elems, i - 1));
    }
    numelems++;
    pOpaqueType->construct(begin(), x);
}

OSCL_EXPORT_REF OsclAny* Oscl_Vector_Base::insert(OsclAny* pos, const OsclAny* x)
{
    OSCL_ASSERT(x);
    if (numelems == bufsize)
    {
        uint32 new_bufsize = (bufsize) ? 2 * bufsize : 2;
        OsclAny* oldPtr = elems;
        reserve(new_bufsize);
        int32 diff = ((char*)elems) - ((char*)oldPtr);
        pos = (OsclAny*)(((char*)pos) + diff);
    }
    OsclAny* ptr = increment_T(elems, numelems);
    while (ptr > pos)
    {
        OsclAny* previous = increment_T(ptr, -1);
        pOpaqueType->construct(ptr, previous);
        pOpaqueType->destroy(previous);
        ptr = previous;
    }
    numelems++;
    pOpaqueType->construct(pos, x);
    return pos;
}

OSCL_EXPORT_REF OsclAny* Oscl_Vector_Base::erase(OsclAny* pos)
{
    //note: use move instead of copy here since we assume
    //it's not necessary to construct/destruct just to move
    //an element.
    pOpaqueType->destroy(pos);
    if (increment_T(pos, 1) != end())
        move(increment_T(pos, 1), end(), pos);
    numelems--;
    return pos;
}

OSCL_EXPORT_REF OsclAny* Oscl_Vector_Base::erase(OsclAny* first, OsclAny* last)
{
    //note: use move instead of copy here since we assume
    //it's not necessary to construct/destruct just to move
    //an element.
    destroy(first, last);
    move(last, end(), first);
    numelems -= (((uint32)last - (uint32)first) / sizeof_T);
    return first;
}

OSCL_EXPORT_REF void Oscl_Vector_Base::assign_vector(const Oscl_Vector_Base& x)
{
    if (x.size() > capacity())
    {
        // allocate space and copy
        OsclAny* tmp = pOpaqueType->allocate((uint32)x.end() - (uint32)x.begin());
        uninitialized_copy(x.begin(), x.end(), tmp);
        destroy(begin(), end());
        if (elems)
            pOpaqueType->deallocate(elems);
        elems = tmp;
        bufsize = x.size();
    }
    else if (size() >= x.size())
    {
        OsclAny* i = copy(x.begin(), x.end(), begin());
        destroy(i, end());
    }
    else
    {
        copy(x.begin(), increment_T(x.begin(), size()), begin());
        uninitialized_copy(increment_T(x.begin(), size()), x.end(), end());
    }
    numelems = x.size();
}

OSCL_EXPORT_REF void Oscl_Vector_Base::destroy()
{
    if (elems)
    {
        destroy(begin(), end());
        pOpaqueType->deallocate(elems);
        elems = NULL;
        numelems = 0;
        bufsize = 0;
    }
}

OsclAny* Oscl_Vector_Base::move(OsclAny* first, OsclAny* last, OsclAny* result)
{
    oscl_memmove(result, first, (uint32)last - (uint32)first);
    return last;
}

OsclAny* Oscl_Vector_Base::copy(OsclAny* first, OsclAny* last, OsclAny* result)
{
    while (first != last)
    {
        pOpaqueType->destroy(result);
        pOpaqueType->construct(result, first);
        result = increment_T(result, 1);
        first = increment_T(first, 1);
    }
    return result;
}

OsclAny* Oscl_Vector_Base::uninitialized_copy(OsclAny* first, OsclAny* last, OsclAny* result)
{
    while (first != last)
    {
        pOpaqueType->construct(result, first);
        result = increment_T(result, 1);
        first = increment_T(first, 1);
    }
    return result;
}

void Oscl_Vector_Base::destroy(OsclAny* first, OsclAny* last)
{
    while (first != last)
    {
        pOpaqueType->destroy(first);
        first = increment_T(first, 1);
    }
}







