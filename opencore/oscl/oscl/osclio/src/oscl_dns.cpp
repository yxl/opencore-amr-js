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
#include "oscl_dns.h"
#include "oscl_error.h"
#include "oscl_exception.h"
#include "oscl_dns_gethostbyname.h"
#include "oscl_dns_imp.h"

OSCL_EXPORT_REF OsclDNS* OsclDNS::NewL(
    Oscl_DefAlloc &alloc,
    OsclSocketServ& serv,
    OsclDNSObserver& obs,
    uint32 id)
{
    OsclAny *p = alloc.ALLOCATE(sizeof(OsclDNS));
    OsclError::LeaveIfNull(p);
    OsclDNS *self = OSCL_PLACEMENT_NEW(p, OsclDNS(alloc, obs, id));
    OsclError::PushL(self);
    self->ConstructL(serv);
    OsclError::Pop();
    return self;
}

OsclDNS::OsclDNS(
    Oscl_DefAlloc &alloc,
    OsclDNSObserver & obs,
    uint32 id)
        : iGetHostByNameMethod(NULL)
        , iAlloc(alloc)
        , iObserver(obs)
        , iId(id)
        , iDNS(NULL)
{
}

void OsclDNS::ConstructL(OsclSocketServ &aServ)
{
    iDNS = OsclDNSI::NewL(iAlloc);
    if (iDNS->Open(*aServ.iServ) != OsclErrNone)
        OsclError::Leave(OsclErrGeneral);
    iGetHostByNameMethod = OsclGetHostByNameMethod::NewL(iAlloc, iDNS, &iObserver, iId);
}


OSCL_EXPORT_REF OsclDNS::~OsclDNS()
{
    if (iGetHostByNameMethod)
    {
        iGetHostByNameMethod->AbortAll();
        iGetHostByNameMethod->~OsclGetHostByNameMethod();
        iAlloc.deallocate(iGetHostByNameMethod);
    }
    if (iDNS)
    {
        iDNS->~OsclDNSI();
        iAlloc.deallocate(iDNS);
    }
}

OSCL_EXPORT_REF TPVDNSEvent OsclDNS::GetHostByName(char *name, OsclNetworkAddress &addr,
        int32 aTimeoutMsec)
{
    return iGetHostByNameMethod->GetHostByName(name, &addr, aTimeoutMsec);
}

OSCL_EXPORT_REF void OsclDNS::CancelGetHostByName()
{
    iGetHostByNameMethod->CancelMethod();
}




