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
#include "oscl_socket_bind.h"

//////////// Method /////////////////////

OsclBindMethod *OsclBindMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclBindMethod));
    OsclError::LeaveIfNull(p);
    OsclBindMethod* self = OSCL_PLACEMENT_NEW(p, OsclBindMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclBindMethod::ConstructL()
{
    OsclAny*p = Alloc().ALLOCATE(sizeof(OsclBindRequest));
    OsclError::LeaveIfNull(p);
    OsclBindRequest* self = OSCL_PLACEMENT_NEW(p, OsclBindRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclBindMethod::~OsclBindMethod()
{
    if (BindRequest())
    {
        BindRequest()->~OsclBindRequest();
        Alloc().deallocate(BindRequest());
    }
}

TPVSocketEvent OsclBindMethod::Bind(OsclNetworkAddress& aAddress,
                                    int32 aTimeout)
{
    if (!StartMethod(aTimeout))
        return EPVSocketFailure;

    BindRequest()->Bind(aAddress);

    return EPVSocketPending;
}

//////////// AO /////////////////////


void OsclBindRequest::Bind(OsclNetworkAddress &aAddress)
{
    OsclAny *p = NewRequest(sizeof(BindParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        iParam = OSCL_PLACEMENT_NEW(p, BindParam(aAddress));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
        {
            SocketI()->BindAsync(*Param(), *this);
        }
    }
}







