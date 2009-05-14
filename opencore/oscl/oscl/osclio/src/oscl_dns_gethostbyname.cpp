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
#include "oscl_dns_gethostbyname.h"
#include "oscl_dns_imp.h"
#include "osclconfig_io.h"
#include "oscl_dns_request.h"
#include "oscl_dns_param.h"

//////////// Method /////////////////////

OsclGetHostByNameMethod *OsclGetHostByNameMethod::NewL(Oscl_DefAlloc &a,
        OsclDNSI *aDNS,
        OsclDNSObserver *aObserver,
        uint32 aId)
{
    OsclAny*p = a.ALLOCATE(sizeof(OsclGetHostByNameMethod));
    OsclError::LeaveIfNull(p);
    OsclGetHostByNameMethod* self = OSCL_PLACEMENT_NEW(p, OsclGetHostByNameMethod(a));
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL(aDNS, aObserver, aId);
    OsclError::Pop();
    return self;
}

void OsclGetHostByNameMethod::ConstructL(
    OsclDNSI *aDNS,
    OsclDNSObserver *aObserver,
    uint32 aId)
{
    iGetHostByNameRequest = OsclGetHostByNameRequest::NewL(aDNS, this);
    OsclDNSMethod::ConstructL(aObserver, iGetHostByNameRequest, aId);
}

OsclGetHostByNameMethod::~OsclGetHostByNameMethod()
{
    if (iGetHostByNameRequest)
    {
        iGetHostByNameRequest->~OsclGetHostByNameRequest();
        iAlloc.deallocate(iGetHostByNameRequest);
    }
}

TPVDNSEvent OsclGetHostByNameMethod::GetHostByName(char *name, OsclNetworkAddress *addr,
        int32 aTimeout)
{
    if (!StartMethod(aTimeout))
        return EPVDNSFailure;

    iGetHostByNameRequest->GetHostByName(name, addr);

    return EPVDNSPending;
}

//////////// AO /////////////////////

OsclGetHostByNameRequest *OsclGetHostByNameRequest::NewL(
    OsclDNSI *aDNS,
    OsclGetHostByNameMethod *aMethod)
{
    OsclAny*p = aMethod->iAlloc.ALLOCATE(sizeof(OsclGetHostByNameRequest));
    OsclError::LeaveIfNull(p);
    OsclGetHostByNameRequest* self = OSCL_PLACEMENT_NEW(p, OsclGetHostByNameRequest());
    OsclError::LeaveIfNull(self);
    OsclError::PushL(self);
    self->ConstructL(aDNS,  aMethod);
    OsclError::Pop();
    return self;
}

OsclGetHostByNameRequest::~OsclGetHostByNameRequest()
{
    if (iParam)
        iParam->RemoveRef();
    iParam = NULL;
}

void OsclGetHostByNameRequest::GetHostByName(char *name, OsclNetworkAddress *addr)
{
    NewRequest();

    if (iParam)
        iParam->RemoveRef();
    iParam = NULL;

    if (!name || !addr)
    {
        PendComplete(OsclErrGeneral);
        return;
    }

    iParam = GetHostByNameParam::Create(name, addr);
    if (!iParam)
        PendComplete(OsclErrNoMemory);
    else
        iDNSI->GetHostByName(*iParam, *this);
}

void OsclGetHostByNameRequest::Success()
{
    iDNSI->GetHostByNameSuccess(*iParam);
}


