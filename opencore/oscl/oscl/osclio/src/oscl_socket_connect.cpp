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
#include "oscl_socket_connect.h"

//////////// Method /////////////////////

OsclConnectMethod *OsclConnectMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclConnectMethod));
    OsclError::LeaveIfNull(p);
    OsclConnectMethod* self = OSCL_PLACEMENT_NEW(p, OsclConnectMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclConnectMethod::ConstructL()
{
    OsclAny*p = Alloc().ALLOCATE(sizeof(OsclConnectRequest));
    OsclError::LeaveIfNull(p);
    OsclConnectRequest* self = OSCL_PLACEMENT_NEW(p, OsclConnectRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclConnectMethod::~OsclConnectMethod()
{
    if (ConnectRequest())
    {
        ConnectRequest()->~OsclConnectRequest();
        Alloc().deallocate(ConnectRequest());
    }
}

TPVSocketEvent OsclConnectMethod::Connect(OsclNetworkAddress& aAddress,
        int32 aTimeout)
{
    if (!StartMethod(aTimeout))
        return EPVSocketFailure;

    ConnectRequest()->Connect(aAddress);

    return EPVSocketPending;
}

//////////// AO /////////////////////


void OsclConnectRequest::Connect(OsclNetworkAddress &aAddress)
{
    OsclAny *p = NewRequest(sizeof(ConnectParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        iParam = OSCL_PLACEMENT_NEW(p, ConnectParam(aAddress));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
        {
            SocketI()->Connect(*Param(), *this);
        }
    }
}







