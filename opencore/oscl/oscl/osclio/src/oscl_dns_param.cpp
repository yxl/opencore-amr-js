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

#include "oscl_dns_tuneables.h"

#include "oscl_dns_param.h"
#include "oscl_scheduler_ao.h"
#include "oscl_dns_request.h"
#include "oscl_assert.h"
#include "oscl_error.h"
#include "oscl_dns_method.h"
#include "oscl_dns_imp.h"
#include "pvlogger.h"


//Remove ref from either thread.
void DNSRequestParam::RemoveRef()
{
    uint32 count;
#if(PV_DNS_IS_THREAD)
    iLock.Lock();
#endif
    count = --iRefCount;
#if(PV_DNS_IS_THREAD)
    iLock.Unlock();
#endif
    if (count == 0)
        Destroy();
}

DNSRequestParam::DNSRequestParam(TPVDNSFxn aFxn)
        : iFxn(aFxn)
        , iDNSRequest(NULL)
#if (PV_DNS_IS_THREAD)
        , iThreadKillFlag(false)
#endif
{
#if (PV_DNS_IS_THREAD)
    iLock.Create();
    iStartup.Create();
#endif
    iRefCount = 1;
}

#if(PV_DNS_SERVER)
void DNSRequestParam::InThread()
//Called under get host by name request thread or under caller's API.
{
    //Add a ref to this object
    iRefCount++;

#if (PV_DNS_IS_THREAD)
    //Signal that thread startup is complete.
    //Note: A "kill" could occur at any time after this point so
    //it's important not to access any memory outside of
    //this class from now on.
    iStartup.Signal();
#endif

    int32 complete;
    int32 dnserr;

    OSCL_ASSERT(iFxn == EPVDNSGetHostByName);

    //this call blocks
    OsclDNSI::ProcessDnsRequest((GetHostByNameParam*)this, complete, dnserr);

#if(PV_DNS_IS_THREAD)

    //Check for kill flag.  If so, then the request was completed by
    //app thread.  In that case this thread just exits gracefully
    //without reporting completion to the original caller which may no longer
    //exist.

    iLock.Lock();

    bool kill = iThreadKillFlag;

    if (!kill)
        iDNSRequest->Complete(true, complete, dnserr);

    iLock.Unlock();

#else
    iDNSRequest->Complete(true, complete, dnserr);
#endif

    //Remove the ref to this object.
    RemoveRef();

}
#endif

GetHostByNameParam* GetHostByNameParam::Create(const char *name,
        OsclNetworkAddress* &addr)
{
    Oscl_TAlloc<GetHostByNameParam, TDNSRequestParamAllocator> alloc;
    GetHostByNameParam* self = alloc.allocate(1);
    if (self)
        OSCL_PLACEMENT_NEW(self, GetHostByNameParam(name, addr));
    return self;
}

void GetHostByNameParam::Destroy()
{
    Oscl_TAlloc<GetHostByNameParam, TDNSRequestParamAllocator> alloc;
    alloc.destruct_and_dealloc(this);
}

GetHostByNameParam::~GetHostByNameParam()
{
    TDNSRequestParamAllocator alloc;
    alloc.deallocate(iName);
    iName = NULL;
}

GetHostByNameParam::GetHostByNameParam(const char *name,
                                       OsclNetworkAddress* &addr)
        : DNSRequestParam(EPVDNSGetHostByName)
        , iName(NULL)
        , iAddr(addr)
{
    uint32 size = oscl_strlen(name);
    TDNSRequestParamAllocator alloc;
    iName = (char*)alloc.allocate(size + 8);
    OsclError::LeaveIfNull(iName);
    oscl_strncpy(iName, name, (size + 1));

    iAddr->ipAddr.Set("");
}








