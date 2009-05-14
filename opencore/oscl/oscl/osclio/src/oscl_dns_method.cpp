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

#include "osclconfig_io.h"
#include "oscl_scheduler_ao.h"
#include "oscl_dns_method.h"
#include "oscl_dns.h"
#include "oscl_socket_imp.h"
#include "oscl_dns_imp.h"

#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_DEBUG,m);

static const char* const TPVDNSFxnStr[] =
{
    "EPVDNSGetHostByName"
} ;

static const char* const TPVDNSEventStr[] =
{
    "EPVDNSSuccess"
    , "EPVDNSPending"
    , "EPVDNSTimeout"
    , "EPVDNSFailure"
    , "EPVDNSCancel"
} ;

//////////// Request AO //////////////////////
void OsclDNSRequestAO::NewRequest()
{
    PendForExec();
    LOGINFOMED((0, "OsclSocket: New Request %s", TPVDNSFxnStr[iDNSMethod->iDNSFxn]));
}

void OsclDNSRequestAO::RequestDone()
{
    iDNSMethod->Abort();
    RemoveFromScheduler();
}

int OsclDNSRequestAO::GetSocketError()
{
#if   PV_DNS_SERVER

    //Errors from PV socket server are in
    //iSocketError.
    return iSocketError;
#else

#error No Implementation!

#endif
}

void OsclDNSRequestAO::DoCancel()
{
    iDNSI->CancelFxn(iDNSMethod->iDNSFxn);
}

void OsclDNSRequestAO::Run()
{
    //the request was completed.
    RequestDone();
    switch (Status())
    {
        case OSCL_REQUEST_ERR_NONE:
            Success();
            LOGINFOMED((0, "OsclSocket: %s %s", TPVDNSFxnStr[iDNSMethod->iDNSFxn], TPVDNSEventStr[EPVDNSSuccess]));
            iDNSMethod->iDNSObserver->HandleDNSEvent(iDNSMethod->iId, iDNSMethod->iDNSFxn, EPVDNSSuccess, 0);
            break;
        case OSCL_REQUEST_ERR_CANCEL:
            LOGINFOMED((0, "OsclSocket: %s %s", TPVDNSFxnStr[iDNSMethod->iDNSFxn], TPVDNSEventStr[EPVDNSCancel]));
            iDNSMethod->iDNSObserver->HandleDNSEvent(iDNSMethod->iId, iDNSMethod->iDNSFxn, EPVDNSCancel, 0);
            break;
        default:
            LOGINFOMED((0, "OsclSocket: %s %s %d", TPVDNSFxnStr[iDNSMethod->iDNSFxn], TPVDNSEventStr[EPVDNSCancel], GetSocketError()));
            iDNSMethod->iDNSObserver->HandleDNSEvent(iDNSMethod->iId, iDNSMethod->iDNSFxn, EPVDNSFailure, GetSocketError());
            break;
    }
}

//////////// Method //////////////////////

void OsclDNSMethod::ConstructL(
    OsclDNSObserver*aObserver,
    OsclDNSRequestAO *aAO,
    uint32 aId)
{
    if (!aAO)
        OsclError::Leave(OsclErrGeneral);
    iDNSRequestAO = aAO;
    iDNSObserver = aObserver;
    iId = aId;
}

void OsclDNSMethod::Abort()
{
    Cancel();
    RemoveFromScheduler();
}

void OsclDNSMethod::AbortAll()
{
    Abort();
    if (iDNSRequestAO)
        iDNSRequestAO->Abort();
}

bool OsclDNSMethod::StartMethod(int32 aTimeoutMsec)
{
    //make sure there's not already a request
    //in progress
    if (IsAdded() || iDNSRequestAO->IsAdded())
        return false;

    AddToScheduler();
    iDNSRequestAO->AddToScheduler();

    //set the timeout
    if (aTimeoutMsec > 0)
        After(aTimeoutMsec*1000);

    iDNSRequestAO->iSocketError = 0;

    return true;
}

void OsclDNSMethod::MethodDone()
{
    iDNSRequestAO->Abort();
    RemoveFromScheduler();
}

void OsclDNSMethod::CancelMethod()
{
    //cancel the timeout if any
    Abort();
    //cancel the request.
    iDNSRequestAO->iDNSI->CancelFxn(iDNSFxn);
}

void OsclDNSMethod::Run()
{
    //the request timed out!
    MethodDone();
    LOGINFOMED((0, "OsclSocket: %s %s", TPVDNSFxnStr[iDNSFxn], TPVDNSEventStr[EPVDNSTimeout]));
    iDNSObserver->HandleDNSEvent(iId, iDNSFxn, EPVDNSTimeout, 0);
}


