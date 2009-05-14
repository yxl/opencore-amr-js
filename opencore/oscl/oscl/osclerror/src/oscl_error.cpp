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


#include "oscl_error.h"
#include "oscl_assert.h"
#include "oscl_error_trapcleanup.h"
#include "oscl_error_imp.h"
#include "oscl_heapbase.h"
#include "pvlogger.h"

//
// OsclErrorTrap
//
OSCL_EXPORT_REF int32 OsclErrorTrap::Init(Oscl_DefAlloc *aAlloc)
//Init the error trap for this thread.
{

    //make sure OsclError is only initialized once per thread
    int32 error;
    if (OsclErrorTrapImp::GetErrorTrap(error))
        return OsclErrAlreadyInstalled;
    if (error)
        return error;//from GetErrorTrap

    _OsclBasicAllocator defalloc;
    OsclAny *ptr = (aAlloc) ? aAlloc->ALLOCATE(sizeof(OsclErrorTrapImp))
                   : defalloc.ALLOCATE(sizeof(OsclErrorTrapImp));
    if (!ptr)
        return OsclErrNoMemory;

    OsclErrorTrapImp* self = new(ptr) OsclErrorTrapImp(aAlloc, error);
    if (error)
    {//not successfully constructed.
        self->~OsclErrorTrapImp();
        (aAlloc) ? aAlloc->deallocate(ptr) : defalloc.deallocate(ptr);
        return error;
    }

    OsclErrorTrapImp::SetErrorTrap(self, error);

    return error;
}

OSCL_EXPORT_REF int32 OsclErrorTrap::Cleanup()
//Cleanup the error trap for this thread.
{
    int32 error;
    OsclErrorTrapImp *errortrap = OsclErrorTrapImp::GetErrorTrap(error);
    if (errortrap)
    {
        Oscl_DefAlloc *alloc = errortrap->iAlloc;
        errortrap->~OsclErrorTrapImp();
        alloc->deallocate(errortrap);
        OsclErrorTrapImp::SetErrorTrap(NULL, error);
    }
    else if (!error)
        error = OsclErrNotInstalled;//no errortrap.

    return error;
}

OSCL_EXPORT_REF OsclErrorTrapImp* OsclErrorTrap::GetErrorTrapImp()
{
    return OsclErrorTrapImp::GetErrorTrap();
}

//
// OsclError
//
OSCL_EXPORT_REF void OsclError::PushL(_OsclHeapBase * aPtr)
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    trap->iTrapStack->PushL(aPtr);
}

OSCL_EXPORT_REF void OsclError::PushL(OsclAny* aPtr)
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    //convert from TAny to TCleanupItem so the correct
    //deallocator will get used.
    OsclTrapItem item(OsclErrorTrapImp::TrapOperation, aPtr);
    trap->iTrapStack->PushL(item);
}

OSCL_EXPORT_REF void OsclError::PushL(OsclTrapItem anItem)
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    trap->iTrapStack->PushL(anItem);
}

OSCL_EXPORT_REF void OsclError::Pop()
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    trap->iTrapStack->Pop();
}

OSCL_EXPORT_REF void OsclError::Pop(int32 aCount)
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    trap->iTrapStack->Pop(aCount);
}

OSCL_EXPORT_REF void OsclError::PopDealloc()
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    trap->iTrapStack->PopDealloc();
}

OSCL_EXPORT_REF void OsclError::PopDealloc(int32 aCount)
{
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap();
    if (!trap)
    {
        OsclError::Leave(OsclErrNotInstalled);
        return;
    }
    trap->iTrapStack->PopDealloc(aCount);
}

OSCL_EXPORT_REF void OsclError::Leave(int32 aReason)
{
    //log the leave
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, PVLogger::GetLoggerObject("OsclError"), PVLOGMSG_ERR
                    , (0, "OsclError::Leave! reason %d", aReason));


    //set the global leave code if errortrap is installed.
    OsclErrorTrapImp *errortrap = OsclErrorTrapImp::GetErrorTrap();
    if (errortrap)
    {
        OSCL_ASSERT(!errortrap->iLeave);//to avoid infinite recursion
        errortrap->iLeave = aReason;
    }

    //Process the cleanup stack.
    if (errortrap)
        errortrap->iTrapStack->Leaving();

    PVError_DoLeave();
}

OSCL_EXPORT_REF void OsclError::LeaveIfNull(OsclAny* p)
{
    if (!p)
        Leave(OsclErrNoMemory);
}

OSCL_EXPORT_REF void OsclError::LeaveIfError(int32 aReason)
{
    if (aReason != OsclErrNone)
        Leave(aReason);
}






