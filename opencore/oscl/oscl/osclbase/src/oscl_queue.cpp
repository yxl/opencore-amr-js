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

//                     O S C L _ Q U E U E

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup osclbase OSCL Base
 * Additional osclbase comment
 * @{
 */

/*! \file oscl_queue.cpp
    \brief Implementation for oscl_queue.h
*/

#include "oscl_queue.h"


OSCL_EXPORT_REF void Oscl_Queue_Base::reserve(uint32 n)
{
    if (n > bufsize)
    {
        OsclAny* newElems = pOpaqueType->allocate(n * sizeof_T);
        // transfer old contents using normal operations on (this) queue
        // constructing new copy in "new__" variables.  Note that data
        //  won't wrap here, so ok to just use [i++].
        uint32 i = 0;
        while (! empty())
        {
            pOpaqueType->construct(increment_T(newElems, i++), front());
            pop();
        }
        if (elems)
        {
            pOpaqueType->deallocate(elems);
        }
        // now set new values
        elems = newElems;
        bufsize = n;
        numelems = i;
        ifront = 0;
        irear = numelems > 0 ? ifront + numelems - 1 : bufsize - 1;
    }
}

OSCL_EXPORT_REF void Oscl_Queue_Base::construct(Oscl_Opaque_Type_Alloc* aType)
{
    numelems = 0;
    bufsize = 0;
    elems = NULL;
    pOpaqueType = aType;
}

OSCL_EXPORT_REF void Oscl_Queue_Base::construct(Oscl_Opaque_Type_Alloc* aType, uint32 n)
{
    numelems = 0;
    bufsize = n;
    pOpaqueType = aType;
    elems = pOpaqueType->allocate(n * sizeof_T);
}

OSCL_EXPORT_REF void Oscl_Queue_Base::destroy()
{
    if (elems)
    {
        clear();
        pOpaqueType->deallocate(elems);
        elems = NULL;
        numelems = 0;
        bufsize = 0;
    }
}

OSCL_EXPORT_REF void Oscl_Queue_Base::push(const OsclAny* x)
{
    if (numelems == bufsize)
    {
        // increase to 1.25x previous size, but at least +4
        uint32 new_bufsize = bufsize +
                             ((bufsize > 16) ? bufsize >> 2 : 4);
        reserve(new_bufsize);
    }
    ++irear;
    if (irear >= bufsize)
    {
        irear = 0;      // wrap around
    }

    pOpaqueType->construct(increment_T(elems, irear), x);
    numelems++;
}

OSCL_EXPORT_REF void Oscl_Queue_Base::pop()
{
    OSCL_ASSERT(! empty());
    pOpaqueType->destroy(increment_T(elems, ifront));
    ifront++;
    if (ifront >= bufsize)      // (== check is sufficient)
    {
        ifront = 0;
    }
    numelems--;
}

OSCL_EXPORT_REF void Oscl_Queue_Base::clear()
{
    while (! empty())
        pop();
}









