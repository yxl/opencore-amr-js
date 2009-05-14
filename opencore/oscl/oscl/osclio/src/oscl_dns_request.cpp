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
#if PV_DNS_SERVER
//PV DNS Server implementation

#include "oscl_scheduler_ao.h"
#include "oscl_dns_request.h"
#include "oscl_assert.h"
#include "oscl_error.h"
#include "oscl_dns_method.h"
#include "oscl_dns_imp.h"
#include "pvlogger.h"
#include "oscl_dns_param.h"


void OsclDNSRequest::CancelRequest()
//cancel an active request
{
    Complete(false, OSCL_REQUEST_ERR_CANCEL, 0);
}

void OsclDNSRequest::Complete(bool reqthread, int32 aStatus, int32 aSockErr)
//called by either request thread or app thread
//when dns request is complete.
{
    if (iActive)
    {
        //de-activate this request
        iActive = false;
        iDNSRequestAO->iSocketError = aSockErr;

#if(PV_DNS_IS_THREAD)
        if (!reqthread
                && iDNSRequestParam)
        {
            //This call is from app thread.
            //Set the "kill" flag for the thread before
            //completing the request.  Setting kill thread
            //will prevent any callback from the thread.
            iDNSRequestParam->iLock.Lock();
            iDNSRequestParam->iThreadKillFlag = true;
            iDNSRequestAO->PendComplete(aStatus);
            iDNSRequestParam->iLock.Unlock();
            return;
        }
#endif
        iDNSRequestAO->PendComplete(aStatus);
    }
}

void OsclDNSRequest::Activate(DNSRequestParam* p, OsclDNSRequestAO &a)
//activate a dns request
{
    //make sure we don't have multiple requests...
    //calling logic should prevent this, but just in case.
    if (iActive)
        a.PendComplete(OSCL_REQUEST_ERR_GENERAL);

    p->iDNSRequest = this;
    iDNSRequestAO = &a;
    iDNSRequestParam = p;
    iActive = true;

#if(PV_DNS_IS_THREAD)
    //launch the thread.
    OsclThread thread;
    OsclProcStatus::eOsclProcError err = thread.Create(
                                             (TOsclThreadFuncPtr)ThreadFunc,
                                             1024,
                                             (TOsclThreadFuncArg)p);
    if (err == OsclProcStatus::SUCCESS_ERROR)
    {
        //Wait on the thread to start
        p->iStartup.Wait();
    }
    else
    {
        iDNSRequestAO->PendComplete(OSCL_REQUEST_ERR_GENERAL);
    }
    //the request is completed under the DNS thread.

#else
    //non-threaded implementation.
    //call the thread routine directly-- this call blocks.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, a.iLogger, PVLOGMSG_NOTICE, (0, "PVOSCLIO:OsclDNSRequest:Issuing blocking call to DNS..."));
    ((DNSRequestParam*)p)->InThread();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, a.iLogger, PVLOGMSG_NOTICE, (0, "PVOSCLIO:OsclDNSRequest:...request complete"));
#endif
}


#if PV_DNS_IS_THREAD

TOsclThreadFuncRet OSCL_THREAD_DECL OsclDNSRequest::ThreadFunc(TOsclThreadFuncArg arg)
//generic dns request thread.
{
    //call the request-specific thread routine.
    if (arg)
        ((DNSRequestParam*)arg)->InThread();

    OSCL_UNUSED_RETURN(0);		// Exit doesn't return
}

#endif //PV_DNS_IS_THREAD

#endif //PV_DNS_SERVER







