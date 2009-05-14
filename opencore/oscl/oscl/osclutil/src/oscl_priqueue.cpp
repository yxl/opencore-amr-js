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

/*!
 * \file oscl_priqueue.cpp
 * \brief Implements a priority queue data structure similar to STL.
 *
 *
 */

#include "oscl_priqueue.h"


OSCL_EXPORT_REF void OsclPriorityQueueBase::push_heap(OsclAny* first, OsclAny* last)
{
    /*
    * The task of the push_heap function is to take the last
    * entry and continue to swap it with its parent until
    * the heap property is statisfied (i.e., all children are not
    * greater than the parent).  It is assumed that the new value
    * has already been pushed on the end of the container. The
    * first and last arguments should be the begin() and end() iterators
    * of the container.
    */

    int index = delta_T(first, last) - 1;
    int parent_index = (index - 1) / 2;
    while (index > 0 && pOpaqueType->compare_LT(pVec->increment_T(first, parent_index), pVec->increment_T(first, index)))
    {
        // swap the current index and the parent
        pOpaqueType->swap(pVec->increment_T(first, index), pVec->increment_T(first, parent_index));
        index = parent_index;
        parent_index = (index - 1) / 2;
    }
}

OSCL_EXPORT_REF void OsclPriorityQueueBase::pop_heap(OsclAny* first, OsclAny* last)
{

    /* This function works by swapping the first and last values in
    * the container and then pushes down the top element until the
    * parent-child relationship is satisfied (i.e., parent not less than
    * either of its children
    */

    // swap the first and last values
    pOpaqueType->swap(first, pVec->increment_T(last, -1));

    int index = 0;
    int child = 2 * index + 1;
    int new_last_index = delta_T(first, last) - 1;

    while (child < new_last_index)
    {
        if (((child + 1) < new_last_index)
                && (pOpaqueType->compare_LT(pVec->increment_T(first, child), pVec->increment_T(first, child + 1))))
        {
            // make certain the child index is ordered according the comparison
            child += 1;
        }

        if (pOpaqueType->compare_LT(pVec->increment_T(first, index), pVec->increment_T(first, child)))
        {
            // swap them
            pOpaqueType->swap(pVec->increment_T(first, index), pVec->increment_T(first, child));
            index = child;
            child = 2 * index + 1;
        }
        else
        {
            break;
        }
    }
}

OSCL_EXPORT_REF OsclAny* OsclPriorityQueueBase::find_heap(const OsclAny* input, OsclAny* first, OsclAny* last)
{
    /*
    * Find iterator pointing to the input element.
    * It is assumed that the input element is present.
    * If not present, it will return NULL.
    */
    for (OsclAny* pos = first;pos < last;pos = pVec->increment_T(pos, 1))
    {
        if (pOpaqueType->compare_EQ(pos, input))
            return pos;
    }
    return NULL;
}

//Remove an arbitrary element, by value.
//If there are multiple matches, this removes the first one it finds.
//Returns number of items removed(either 0 or 1).
OSCL_EXPORT_REF int OsclPriorityQueueBase::remove(const OsclAny* input)
{
    //First find the element to remove
    OsclAny* pos = find_heap(input, pVec->begin(), pVec->end());
    if (pos)
    {
        if (pVec->increment_T(pos, 1) == pVec->end())
        {
            // It's the last element-- just remove it without any re-ordering.
            pVec->pop_back();
        }
        else
        {
            // Move the element to the end & remove.
            pop_heap(pos, pVec->end());
            pVec->pop_back();
            // Re-order the front part of the queue.
            push_heap(pVec->begin(), pVec->increment_T(pos, 1));
        }
        return 1;
    }
    return 0;
}


