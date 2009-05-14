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

#ifndef OSCL_DNS_METHOD_H_INCLUDED
#define OSCL_DNS_METHOD_H_INCLUDED

#include "osclconfig_io.h"
#include "oscl_socket_types.h"
#include "oscl_scheduler_ao.h"
#include "oscl_dns.h"
#include "pvlogger.h"

class OsclDNSObserver;
class OsclDNSRequestAO;
class OsclSocketServ;
class OsclDNSI;
class OsclDNS;

/** This is the base class for all socket methods.
* It provides the timeout on socket requests.
*/
class OsclDNSMethod : public OsclTimerObject
{
    public:
        OsclDNSMethod(Oscl_DefAlloc &a, const char *name, TPVDNSFxn fxn)
                : OsclTimerObject(OsclActiveObject::EPriorityNominal, name)
                , iDNSObserver(NULL)
                , iId(0)
                , iAlloc(a)
                , iDNSFxn(fxn)
                , iDNSRequestAO(NULL)
        {
            iLogger = PVLogger::GetLoggerObject("oscldns");
        }

        void Abort();
        void AbortAll();
        void CancelMethod();
        void Run();

        OsclDNSObserver *iDNSObserver;
        uint32 iId;
        Oscl_DefAlloc &iAlloc;
        TPVDNSFxn iDNSFxn;
        PVLogger* iLogger;

    protected:
        void ConstructL(
            OsclDNSObserver*aObserver,
            OsclDNSRequestAO *aAO,
            uint32 aId);

        bool StartMethod(int32 aTimeoutMsec);
        void MethodDone();

        OsclDNSRequestAO *iDNSRequestAO;
};


/** This is the base class for all requests to the
* socket server.
*/
class OsclDNSRequestAO : public OsclActiveObject
{
    protected:

        OsclDNSRequestAO(const char *name)
                : OsclActiveObject(OsclActiveObject::EPriorityNominal, name)
                , iDNSI(NULL)
                , iDNSMethod(NULL)
                , iSocketError(0)
        {
        }

        void ConstructL(
            OsclDNSI *aDNS,
            OsclDNSMethod *aMethod)
        {
            if (!aMethod)
                OsclError::Leave(OsclErrGeneral);
            iDNSMethod = aMethod;
            iLogger = iDNSMethod->iLogger;
            if (!aDNS)
                OsclError::Leave(OsclErrGeneral);
            iDNSI = aDNS;
        }

        void Abort()
        {
            Cancel();
            RemoveFromScheduler();
        }
        void NewRequest();
        void RequestDone();
        int GetSocketError();
        OsclSocketServI *Serv();
        void DoCancel();
        void Run();

        virtual void Success() {}

        OsclDNSI *iDNSI;
        OsclDNSMethod *iDNSMethod;
        int32 iSocketError;
        PVLogger* iLogger;

        friend class OsclDNSI;
        friend class OsclDNSMethod;
        friend class OsclDNSRequest;
        friend class DNSRequestParam;
};

#endif

