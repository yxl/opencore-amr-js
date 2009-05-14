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

#include "oscl_scheduler_ao.h"
#include "oscl_socket_recv_from.h"

//////////// Method /////////////////////

OsclRecvFromMethod *OsclRecvFromMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclRecvFromMethod));
    OsclError::LeaveIfNull(p);
    OsclRecvFromMethod* self = OSCL_PLACEMENT_NEW(p, OsclRecvFromMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclRecvFromMethod::ConstructL()
{
    OsclAny*p = Alloc().ALLOCATE(sizeof(OsclRecvFromRequest));
    OsclError::LeaveIfNull(p);
    OsclRecvFromRequest* self = OSCL_PLACEMENT_NEW(p, OsclRecvFromRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->OsclSocketRequestAO::ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclRecvFromMethod::~OsclRecvFromMethod()
{
    if (RecvFromRequest())
    {
        RecvFromRequest()->~OsclRecvFromRequest();
        Alloc().deallocate(RecvFromRequest());
    }
}

TPVSocketEvent OsclRecvFromMethod::RecvFrom(uint8* &aPtr, uint32 aMaxLen,
        OsclNetworkAddress &aAddress,
        int32 aTimeout,
        uint32 aMultiMax,
        Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen,
        Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource)
{
    if (!StartMethod(aTimeout))
        return EPVSocketFailure;

    RecvFromRequest()->RecvFrom(aPtr, aMaxLen, aAddress, aMultiMax, aPacketLen, aPacketSource);

    return EPVSocketPending;
}

uint8 *OsclRecvFromMethod::GetRecvData(int32 *aLength)
{
    return RecvFromRequest()->GetRecvData(aLength);
}

//////////// AO /////////////////////

void OsclRecvFromRequest::RecvFrom(uint8* &aPtr, uint32 aMaxLen,
                                   OsclNetworkAddress& aAddress, uint32 aMultiMax,
                                   Oscl_Vector<uint32, OsclMemAllocator>* aPacketLen,
                                   Oscl_Vector<OsclNetworkAddress, OsclMemAllocator>* aPacketSource)
{
    OsclAny *p = NewRequest(sizeof(RecvFromParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        if (aPacketLen)
            aPacketLen->clear();
        if (aPacketSource)
            aPacketSource->clear();
        iParam = OSCL_PLACEMENT_NEW(p, RecvFromParam(aPtr, aMaxLen, aAddress, 0, aMultiMax, aPacketLen, aPacketSource));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
            SocketI()->RecvFrom(*Param(), *this);
    }
}
void OsclRecvFromRequest::Success()
{
    SocketI()->RecvFromSuccess(*Param());
}

uint8 *OsclRecvFromRequest::GetRecvData(int32 *aLength)
{
    if (iParam)
    {
        if (aLength)
            *aLength = Param()->iBufRecv.iLen;
        return Param()->iBufRecv.iPtr;
    }
    else
    {
        if (aLength)
            *aLength = 0;
        return NULL;
    }
}





