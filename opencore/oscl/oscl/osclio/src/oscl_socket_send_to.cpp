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
#include "oscl_socket_method.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_send_to.h"

//////////// Method /////////////////////

OsclSendToMethod *OsclSendToMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclSendToMethod));
    OsclError::LeaveIfNull(p);
    OsclSendToMethod* self = OSCL_PLACEMENT_NEW(p, OsclSendToMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclSendToMethod::ConstructL()
{
    OsclAny*p = Alloc().ALLOCATE(sizeof(OsclSendToRequest));
    OsclError::LeaveIfNull(p);
    OsclSendToRequest* self = OSCL_PLACEMENT_NEW(p, OsclSendToRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclSendToMethod::~OsclSendToMethod()
{
    if (SendToRequest())
    {
        SendToRequest()->~OsclSendToRequest();
        Alloc().deallocate(SendToRequest());
    }
}


TPVSocketEvent OsclSendToMethod::SendTo(const uint8* &aPtr, uint32 aLen,
                                        OsclNetworkAddress& aAddress,
                                        int32 aTimeout)
{
    if (!StartMethod(aTimeout))
        return EPVSocketFailure;

    SendToRequest()->SendTo(aPtr, aLen, aAddress);

    return EPVSocketPending;
}

uint8 *OsclSendToMethod::GetSendData(int32 *aLength)
{
    return SendToRequest()->GetSendData(aLength);
}

//////////// AO /////////////////////

void OsclSendToRequest::SendTo(const uint8* &aPtr, uint32 aLen, OsclNetworkAddress &aAddress)
{
    OsclAny *p = NewRequest(sizeof(SendToParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        iParam = OSCL_PLACEMENT_NEW(p, SendToParam(aPtr, aLen, aAddress, 0));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
            SocketI()->SendTo(*Param(), *this);
    }
}
void OsclSendToRequest::Success()
{
    SocketI()->SendToSuccess(*Param());
}

uint8 *OsclSendToRequest::GetSendData(int32 *aLength)
{
    if (iParam)
    {
        if (aLength)
            *aLength = Param()->iXferLen;
        return (uint8*)Param()->iBufSend.iPtr;
    }
    else
    {
        if (aLength)
            *aLength = 0;
        return NULL;
    }
}








