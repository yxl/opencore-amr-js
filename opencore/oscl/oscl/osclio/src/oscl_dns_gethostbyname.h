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

#ifndef OSCL_DNS_GETHOSTBYNAME_H_INCLUDED
#define OSCL_DNS_GETHOSTBYNAME_H_INCLUDED

//#include "oscl_socket_serv_imp.h"
#include "oscl_dns_method.h"
#include "oscl_dns.h"
#include "osclconfig_io.h"


class OsclGetHostByNameRequest;

class OsclGetHostByNameMethod : public OsclDNSMethod
{
    public:
        static OsclGetHostByNameMethod *NewL(Oscl_DefAlloc &a,
                                             OsclDNSI *aDNS,
                                             OsclDNSObserver *aObserver,
                                             uint32 aId);

        ~OsclGetHostByNameMethod();

        TPVDNSEvent GetHostByName(char *name, OsclNetworkAddress *addr,
                                  int32 aTimeout);


    private:
        OsclGetHostByNameMethod(Oscl_DefAlloc &a)
                : OsclDNSMethod(a, "osclghbnmethod", EPVDNSGetHostByName)
                , iGetHostByNameRequest(NULL)
        {}

        void ConstructL(OsclDNSI* aDNS,
                        OsclDNSObserver *aObserver,
                        uint32 aId);

        OsclGetHostByNameRequest *iGetHostByNameRequest;
};

class GetHostByNameParam;

class OsclGetHostByNameRequest : public OsclDNSRequestAO
{
    private:
        static OsclGetHostByNameRequest *NewL(
            OsclDNSI *aDNS,
            OsclGetHostByNameMethod *aMethod);

        ~OsclGetHostByNameRequest();


        OsclGetHostByNameRequest()
                : OsclDNSRequestAO("osclghbnrequest")
                , iParam(NULL)
        {}

        void ConstructL(OsclDNSI *aDNS,
                        OsclGetHostByNameMethod *aMethod)
        {
            OsclDNSRequestAO::ConstructL(aDNS, aMethod);
        }

        void Success();

        void GetHostByName(char *name, OsclNetworkAddress *addr);

        GetHostByNameParam *iParam;
        friend class OsclGetHostByNameMethod;

};



#endif

