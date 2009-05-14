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

/*!
** PV DNS server implementation
*/

#include "oscl_dns_tuneables.h"
#if (PV_DNS_SERVER)
#include "oscl_scheduler_ao.h"
#include "oscl_dns_imp_pv.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_error.h"
#include "oscl_assert.h"
#include "oscl_dns_method.h"
#include "oscl_dns_param.h"

OsclDNSI::OsclDNSI(Oscl_DefAlloc &a) : OsclDNSIBase(a)
{
}

OsclDNSI::~OsclDNSI()
{
    Close();
}

OsclDNSI* OsclDNSI::NewL(Oscl_DefAlloc &a)
{
    OsclAny*p = a.ALLOCATE(sizeof(OsclDNSI));
    OsclError::LeaveIfNull(p);
    OsclDNSI *self = OSCL_PLACEMENT_NEW(p, OsclDNSI(a));
    OsclError::LeaveIfNull(self);
    return self;
}



int32 OsclDNSI::Close()
{

    //nothing needed
    return 0;

}

void OsclDNSI::GetHostByName(GetHostByNameParam &aParam, OsclDNSRequestAO &ao)
{
    if (!IsReady(ao))
        return;

    iGetHostByNameRequest.Activate(&aParam, ao);

}

void OsclDNSI::GetHostByNameSuccess(GetHostByNameParam &aParam)
{
    //Nothing needed.
    OSCL_UNUSED_ARG(aParam);
}

void OsclDNSI::CancelGetHostByName()
{
    iGetHostByNameRequest.CancelRequest();
}

int32 OsclDNSI::Open(OsclSocketServI& aServer)
{
    iSocketServ = &aServer;


    //nothing needed
    return 0;

}

bool OsclDNSI::IsReady(OsclDNSRequestAO &ao)
//see if server is ready to handle dns requests.
{
    //Make sure server is ok.
    //Note: we only really need the server active
    //for Symbian, but to make the DNS calling protocol
    //consistent, require an active server on all
    //platforms.
    if (!iSocketServ)
    {
        //use the pv server err code
        ao.iSocketError = PVSOCK_ERR_SOCK_NO_SERV;
        ao.PendComplete(OSCL_REQUEST_ERR_GENERAL);
        return false;
    }
    if (!iSocketServ->IsServConnected())
    {
        //report the error from the server, if any.
        ao.iSocketError = iSocketServ->iServError;
        //use the pv server err code
        if (ao.iSocketError == 0)
            ao.iSocketError = PVSOCK_ERR_SERV_NOT_CONNECTED;
        ao.PendComplete(OSCL_REQUEST_ERR_GENERAL);
        return false;
    }
    return true;
}

void OsclDNSI::ProcessDnsRequest(GetHostByNameParam* aParam, int32& aComplete, int32& aSockerr)
//called under server thread or AO.
//This routine must not access any object memory outside of aParam.
{
    aComplete = OSCL_REQUEST_ERR_GENERAL;
    aSockerr = 0;

#if defined(OsclGethostbyname) && defined(OsclGetDottedAddr)

    //make the platform-specific gethostbyname call...
    TOsclHostent *hostent;
    bool ok;
    int sockerr = 0;

    //this call blocks
    OsclGethostbyname(aParam->iName, hostent, ok, sockerr);

    if (!ok)
    {
        aSockerr = sockerr;
    }
    else
    {
        //convert to dotted address notation
        char *dottedaddr;
        OsclGetDottedAddr(hostent, dottedaddr, ok);
        if (ok)
        {
            aParam->iAddr->ipAddr.Set(dottedaddr);
            aComplete = OSCL_REQUEST_ERR_NONE;
            aSockerr = sockerr;
        }
        else
        {
            aSockerr = sockerr;
        }
    }
#endif
}

#endif //OS switch







