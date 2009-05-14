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
#include "oscl_socket_shutdown.h"
#include "oscl_socket_imp.h"
#include "oscl_socket_shutdown.h"

//////////// Method /////////////////////

OsclShutdownMethod *OsclShutdownMethod::NewL(OsclIPSocketI& c)
{
    OsclAny*p = c.Alloc().ALLOCATE(sizeof(OsclShutdownMethod));
    OsclError::LeaveIfNull(p);
    OsclShutdownMethod* self = OSCL_PLACEMENT_NEW(p, OsclShutdownMethod(c));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    return self;
}

void OsclShutdownMethod::ConstructL()
{
    OsclAny*p = Alloc().ALLOCATE(sizeof(OsclShutdownRequest));
    OsclError::LeaveIfNull(p);
    OsclShutdownRequest* self = OSCL_PLACEMENT_NEW(p, OsclShutdownRequest(*this));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL();
    OsclError::Pop();
    iSocketRequestAO = self;
    OsclSocketMethod::ConstructL(iSocketRequestAO);
}

OsclShutdownMethod::~OsclShutdownMethod()
{
    if (ShutdownRequest())
    {
        ShutdownRequest()->~OsclShutdownRequest();
        Alloc().deallocate(ShutdownRequest());
    }
}

TPVSocketEvent OsclShutdownMethod::Shutdown(TPVSocketShutdown aHow,
        int32 aTimeout)
{
    if (!StartMethod(aTimeout))
        return EPVSocketFailure;

    ShutdownRequest()->Shutdown(aHow);

    return EPVSocketPending;
}

//////////// AO /////////////////////

void OsclShutdownRequest::Shutdown(TPVSocketShutdown aHow)
{
    OsclAny *p = NewRequest(sizeof(ShutdownParam));
    if (!p)
        PendComplete(OsclErrNoMemory);
    else
    {
        iParam = OSCL_PLACEMENT_NEW(p, ShutdownParam(aHow));
        if (!iParam)
            PendComplete(OsclErrNoMemory);
        else
            SocketI()->Shutdown(*Param(), *this);
    }
}




