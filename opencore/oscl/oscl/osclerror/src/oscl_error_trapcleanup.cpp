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

#include "oscl_error_trapcleanup.h"
#include "oscl_assert.h"
#include "oscl_error_codes.h"


OsclErrorTrapImp::OsclErrorTrapImp(Oscl_DefAlloc *alloc, int32 &aError)
{
    aError = 0;
    iAlloc = (alloc) ? alloc : &iDefAlloc;
#if defined(PVERROR_IMP_JUMPS)
    {
        OsclAny* ptr = iAlloc->ALLOCATE(sizeof(OsclJump));
        if (!ptr)
        {
            iJumpData = NULL;
            aError = OsclErrNoMemory;
        }
        else
        {
            iJumpData = new(ptr) OsclJump();
        }
    }
#endif
    OsclAny* ptr = iAlloc->ALLOCATE(sizeof(OsclTrapStack));
    if (!ptr)
    {
        iTrapStack = NULL;
        aError = OsclErrNoMemory;
    }
    else
    {
        iTrapStack = new(ptr) OsclTrapStack(iAlloc);
    }
    iLeave = OsclErrNone;
}


OsclErrorTrapImp::~OsclErrorTrapImp()
{
    if (iTrapStack)
    {
        iTrapStack->~OsclTrapStack();
        iAlloc->deallocate(iTrapStack);
    }
#if defined(PVERROR_IMP_JUMPS)
    if (iJumpData)
    {
        iJumpData->~OsclJump();
        iAlloc->deallocate(iJumpData);
    }
#endif
}


OSCL_EXPORT_REF OsclErrorTrapImp * OsclErrorTrapImp::Trap()
//static function to enter a trap level.
{
    int32 error;
    OsclErrorTrapImp *trap = GetErrorTrap(error);
    if (!trap)
        return NULL;//trap is non-functional.
    trap->iLeave = OsclErrNone;
    trap->iTrapStack->Trap();
#if defined(PVERROR_IMP_JUMPS)
    trap->iJumpData->PushMark();
#endif
    return trap;
}

OSCL_EXPORT_REF OsclErrorTrapImp* OsclErrorTrapImp::TrapNoTls(OsclErrorTrapImp* aTrap)
//static function to enter a trap level.  the trapimp can be passed in to avoid
//the overhead of a TLS lookup.
{
    //thread context check
    int32 error;
    OSCL_ASSERT(aTrap == NULL || aTrap == GetErrorTrap(error));

    OsclErrorTrapImp* trap = (aTrap) ? aTrap : GetErrorTrap(error);
    if (!trap)
        return NULL;//trap is non-functional.
    trap->iLeave = OsclErrNone;
    trap->iTrapStack->Trap();
#if defined(PVERROR_IMP_JUMPS)
    trap->iJumpData->PushMark();
#endif
    return trap;
}

OSCL_EXPORT_REF void OsclErrorTrapImp::UnTrap()
{
    //clear the global leave code
    iLeave = 0;

    bool notempty = iTrapStack->UnTrap();
    OSCL_UNUSED_ARG(notempty);

#if defined(PVERROR_IMP_JUMPS)
    iJumpData->PopMark();
#endif

    OSCL_ASSERT(!notempty);//ETrapLevelNotEmpty
}

///////////////
//OsclTrapStack
///////////////

inline void OsclTrapStack::Trap()
//enter a trap by marking the current top of the cleanup stack.
{
    if (iTop)
        PushTrapL(iTop->iTAny);
    else
        PushTrapL((OsclAny*)NULL);
}

inline bool OsclTrapStack::UnTrap()
//leave the current trap by popping the trap stack.
{
    //check for untrap without corresponding trap.
    OSCL_ASSERT(TrapTop());//ETrapLevelUnderflow

    //make sure all cleanup items in this level have
    //been popped.
    bool notempty = (iTop
                     && iTop->iTAny != TrapTop()->iTAny);

    PopTrap();

    return notempty;
}


OsclTrapStack::OsclTrapStack(Oscl_DefAlloc *alloc)
{
    iTop = NULL;
    iTrapTopIndex = (-1);
    iAlloc = alloc;
}

OsclTrapStack::~OsclTrapStack()
{
    //there should not be leftover items at this point
    OSCL_ASSERT(!iTop);
    OSCL_ASSERT(!TrapTop());

    //pop all items off the stack so memory gets freed.
    while (iTop)
        Pop();
    while (TrapTop())
        PopTrap();
}

void OsclTrapStack::PushL(_OsclHeapBase *aCBase)
//Push a CBase item onto the cleanup stack
{
    //Note: on Symbian you get a panic for doing
    //a push outside any TRAP statement, so generate an
    //error here also.
    OSCL_ASSERT(TrapTop());//ETrapPushAtLevelZero

    OsclAny* ptr = iAlloc->ALLOCATE(sizeof(OsclTrapStackItem));
    OsclError::LeaveIfNull(ptr);
    OsclTrapStackItem * item = new(ptr) OsclTrapStackItem(aCBase);
    Push(item);
}

void OsclTrapStack::PushL(OsclAny *aTAny)
//Push a nonOsclTrapStackCBase item onto the cleanup stack
{
    //Note: on Symbian you get a panic for doing
    //a push outside any TRAP statement, so generate an
    //error here also.
    OSCL_ASSERT(TrapTop());//ETrapPushAtLevelZero

    OsclAny* ptr = iAlloc->ALLOCATE(sizeof(OsclTrapStackItem));
    OsclError::LeaveIfNull(ptr);
    OsclTrapStackItem *item = new(ptr) OsclTrapStackItem(aTAny);
    Push(item);
}

inline void OsclTrapStack::PushTrapL(OsclAny *aTAny)
//Push a nonOsclTrapStackCBase item onto the trap mark stack
{
    pushTrapIndex();
    //construct this item using the stack top element memory
    new((OsclAny*)TrapTop()) OsclTrapStackItem(aTAny);
}

void OsclTrapStack::PushL(OsclTrapItem anItem)
{
    //Note: on Symbian you get a panic for doing
    //a push outside any TRAP statement, so generate an
    //error here also.
    OSCL_ASSERT(TrapTop());//ETrapPushAtLevelZero

    OsclAny* ptr = iAlloc->ALLOCATE(sizeof(OsclTrapStackItem));
    OsclError::LeaveIfNull(ptr);
    OsclTrapStackItem *item = new(ptr) OsclTrapStackItem(anItem);
    Push(item);
}

void OsclTrapStack::Push(OsclTrapStackItem *aItem)
//push the given item onto the stack.
{
    if (aItem)
    {
        aItem->iNext = iTop;
        iTop = aItem;
    }
}

void OsclTrapStack::Pop()
//pop the stack.
{
    if (!iTop)
        OsclError::Leave(OsclErrUnderflow);//ETrapPopUnderflow

    //check for a pop beyond the current trap level
    if (TrapTop() && iTop->iTAny == TrapTop()->iTAny)
        OsclError::Leave(OsclErrUnderflow);//ETrapPopAcrossLevels

    OsclTrapStackItem *next = iTop->iNext;
    iTop->~OsclTrapStackItem();
    iAlloc->deallocate(iTop);
    iTop = next;
}

inline void OsclTrapStack::PopTrap()
//pop the trap mark stack.
{
    OSCL_ASSERT(TrapTop());//ETrapPopUnderflow

    //call destructor on the item in-place
    TrapTop()->~OsclTrapStackItem();

    popTrapIndex();
}

void OsclTrapStack::Pop(int32 aCount)
{
    OSCL_ASSERT(aCount >= 0);//ETrapPopCountNegative
    for (int i = 0;i < aCount;i++)
        Pop();
}

void OsclTrapStack::PopDealloc()
{
    if (!iTop)
        OsclError::Leave(OsclErrUnderflow);//ETrapPopUnderflow;

    if (TrapTop() && iTop->iTAny == TrapTop()->iTAny)
        OsclError::Leave(OsclErrUnderflow);//ETrapPopAcrossLevels;

    if (iTop->iCBase)
    {
        //CBase destructor will be called.
        //Note: we assume the delete operator is
        //defined in the CBase-derived class.  If not,
        //we won't get correct memory mgmt here.
        delete(iTop->iCBase);
    }
    else if (iTop->iTrapOperation)
    {
        //user-supplied cleanup
        (iTop->iTrapOperation)(iTop->iTAny);
    }
    else if (iTop->iTAny)
    {
        //no destructor, just free memory.
        iAlloc->deallocate(iTop->iTAny);
    }
    Pop();
}

void OsclTrapStack::PopDealloc(int32 aCount)
{
    for (int i = 0;i < aCount;i++)
        PopDealloc();
}

void OsclTrapStack::Leaving()
//a leave is happening, so cleanup all items on the stack
//for the current trap level.
{
    //check for a leave outside any trap.
    OSCL_ASSERT(TrapTop());//ETrapLevelUnderflow

    while (iTop && iTop->iTAny != TrapTop()->iTAny)
        PopDealloc();
}



