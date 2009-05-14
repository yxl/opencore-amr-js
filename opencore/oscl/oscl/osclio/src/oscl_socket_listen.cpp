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
#include "oscl_socket_listen.h"

//////////// Method /////////////////////

OsclListenMethod *OsclListenMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclListenMethod));
    OsclError::LeaveIfNull(p);
    OsclListenMethod* self = OSCL_PLACEMENT_NEW(p, OsclListenMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclListenMethod::ConstructL()
{
    OsclAny*p = Alloc().ALLOCATE(sizeof(OsclListenRequest));
    OsclError::LeaveIfNull(p);
    OsclListenRequest* self = OSCL_PLACEMENT_NEW(p, OsclListenRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclListenMethod::~OsclListenMethod()
{
    if (ListenRequest())
    {
        ListenRequest()->~OsclListenRequest();
        Alloc().deallocate(ListenRequest());
    }
}

TPVSocketEvent OsclListenMethod::Listen(uint32 qsize,
                                        int32 aTimeout)
{
    if (!StartMethod(aTimeout))
        return EPVSocketFailure;

    ListenRequest()->Listen(qsize);

    return EPVSocketPending;
}

//////////// AO /////////////////////


void OsclListenRequest::Listen(uint32 qsize)
{
    OsclAny *p = NewRequest(sizeof(ListenParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        iParam = OSCL_PLACEMENT_NEW(p, ListenParam(qsize));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
        {
            SocketI()->ListenAsync(*Param(), *this);
        }
    }
}







