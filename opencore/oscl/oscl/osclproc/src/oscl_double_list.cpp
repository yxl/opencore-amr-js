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
#include "osclconfig_proc.h"

//Doubly-linked list implementation

#include "oscl_scheduler.h"
#include "oscl_error.h"
#include "oscl_assert.h"

#if (OSCL_DISABLE_INLINES)
//#include "oscl_aostatus.inl"
#endif

//////////////////////////////////
// OsclDoubleLink
//////////////////////////////////

void OsclDoubleLink::InsertAfter(OsclDoubleLink *aLink)
//add this item after aLink
{
    iPrev = aLink;
    if (aLink)
    {
        iNext = aLink->iNext;
        aLink->iNext = this;
        if (iNext)
            iNext->iPrev = this;
    }
}

void OsclDoubleLink::InsertBefore(OsclDoubleLink *aLink)
//add this item before aLink
{
    iNext = aLink;
    if (aLink)
    {
        iPrev = aLink->iPrev;
        aLink->iPrev = this;
        if (iPrev)
            iPrev->iNext = this;
    }
}

void OsclDoubleLink::Remove()
//remove this item.
{
    if (iNext)
    {
        iNext->iPrev = iPrev;
        if (iPrev)
            iPrev->iNext = iNext;
    }
    iNext = iPrev = NULL;
}


//////////////////////////////////
// OsclDoubleListBase
//////////////////////////////////
OsclDoubleListBase::OsclDoubleListBase()
{
    iOffset = (-1);//invalid.
    iHead.iNext = iHead.iPrev = &iHead;
}

bool OsclDoubleListBase::IsEmpty()const
{
    return ((void*)iHead.iNext == (void*)&iHead);
}

void OsclDoubleListBase::SetOffset(int32 aOffset)
//offset contains the offset into the TAny item of its TDblQueLinkBase item.
{
    //offset should not be negative.
    OSCL_ASSERT(aOffset >= 0);

    //just save it for later.
    iOffset = aOffset;
}

void OsclDoubleListBase::InsertHead(OsclAny* aPtr)
{
    //offset must be set before calling this
    OSCL_ASSERT(iOffset >= 0);

    //find the item link
    OsclDoubleLink * link = (OsclDoubleLink *)OsclPtrAdd(aPtr, iOffset);
    if (IsEmpty())
    {
        //make the head of the que point to this item link
        iHead.iNext = link;
        iHead.iPrev = link;

        //make the item link point back to the head
        link->iPrev = &iHead;
        link->iNext = &iHead;
    }
    else
    {
        link->InsertBefore(iHead.iNext);
    }
}

void OsclDoubleListBase::InsertTail(OsclAny* aPtr)
{
    //offset must be set before calling this
    OSCL_ASSERT(iOffset >= 0);

    //find the item link
    OsclDoubleLink * link = (OsclDoubleLink *)OsclPtrAdd(aPtr, iOffset);

    if (IsEmpty())
        InsertHead(aPtr);
    else
        link->InsertAfter(iHead.iPrev);
}

void OsclDoubleListBase::Insert(OsclAny* aPtr)
//sorted queue insert.  This inserts an item at the
//end of its priority group.
{
    //offset must be set before calling this
    OSCL_ASSERT(iOffset >= 0);
    if (IsEmpty())
        InsertHead(aPtr);
    else
    {
        //find the item link
        OsclPriorityLink* link = (OsclPriorityLink*)OsclPtrAdd(aPtr, iOffset);

        OsclPriorityLink* itemlink = (OsclPriorityLink*)iHead.iNext;
        while (itemlink)
        {
            //add before first item with lower priority.
            if (link->iPriority > itemlink->iPriority)
            {
                link->InsertBefore(itemlink);
                return;
            }

            if (itemlink->iNext == &iHead)
                break;
            itemlink = (OsclPriorityLink*)itemlink->iNext;
        }
        //no lower pri items-- add to end.
        link->InsertAfter(iHead.iPrev);
    }
}
