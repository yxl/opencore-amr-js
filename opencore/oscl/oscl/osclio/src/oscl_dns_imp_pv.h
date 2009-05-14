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


#ifndef OSCL_DNS_IMP_PV_H_INCLUDED
#define OSCL_DNS_IMP_PV_H_INCLUDED

#include "oscl_socket_imp_base.h"
#include "oscl_dns_request.h"
#include "oscl_dns_imp_base.h"

class OsclSocketServI;

/*!
** OsclDNSI, non-Symbian implementation
*/
class OsclDNSI: public OsclDNSIBase
{
    public:
        static OsclDNSI* NewL(Oscl_DefAlloc &a);
        ~OsclDNSI();

        //synchronous methods
        int32 Open(OsclSocketServI& aServer);
        int32 Close();

        //asynchronous methods.

        void GetHostByName(GetHostByNameParam& , OsclDNSRequestAO&);
        void GetHostByNameSuccess(GetHostByNameParam&);


    private:
        OsclDNSI(Oscl_DefAlloc &a);

        bool IsReady(OsclDNSRequestAO& aObject);

        void CancelGetHostByName();

        friend class OsclDNSRequest;

        //active requests
        OsclDNSRequest iGetHostByNameRequest;
        static void ProcessDnsRequest(GetHostByNameParam*, int32&, int32&);

        friend class DNSRequestParam;
};

#endif



