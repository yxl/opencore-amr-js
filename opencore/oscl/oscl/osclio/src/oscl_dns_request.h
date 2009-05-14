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

#ifndef OSCL_DNS_REQUEST_H_INCLUDED
#define OSCL_DNS_REQUEST_H_INCLUDED

#include "oscl_dns_tuneables.h"
#if PV_DNS_SERVER

#include "oscl_namestring.h"
#include "oscl_dns.h"
#include "oscl_socket_types.h"
#include "oscl_dns_tuneables.h"

#if PV_DNS_IS_THREAD
#include "oscl_thread.h"
#include "oscl_mutex.h"
#endif

class DNSRequestParam;
class OsclDNSRequestAO;

/** This class defines the interface to the dns
implementation threads.
*/
class OsclDNSRequest
{
    public:
        OsclDNSRequest()
                : iDNSRequestAO(NULL)
                , iDNSRequestParam(NULL)
                , iActive(false)
        {
        }
        ~OsclDNSRequest()
        {
        }

        OsclDNSRequestAO *iDNSRequestAO;//the AO that is waiting on completion.
        DNSRequestParam *iDNSRequestParam;
        bool iActive;

#if PV_DNS_IS_THREAD
        static TOsclThreadFuncRet OSCL_THREAD_DECL ThreadFunc(TOsclThreadFuncArg arg);
#endif

        void CancelRequest();
        void Complete(bool, int32 aStatus, int32 aSockErr);
        void Activate(DNSRequestParam *iParam, OsclDNSRequestAO &a);
};

#endif


#endif

