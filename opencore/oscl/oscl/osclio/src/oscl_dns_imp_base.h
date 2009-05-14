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


#ifndef OSCL_DNS_IMP_BASE_H_INCLUDED
#define OSCL_DNS_IMP_BASE_H_INCLUDED

#include "oscl_socket_imp.h"
#include "oscl_dns_request.h"
#include "oscl_dns.h"

class OsclSocketServI;
class GetHostByNameParam;
class OsclDNSRequestAO;

/*!
** OsclDNSIBase is a common base class for all
** implementations.
*/
class OsclDNSIBase
{
    public:
        virtual ~OsclDNSIBase();

        //synchronous methods
        virtual int32 Open(OsclSocketServI& aServer) = 0;
        virtual int32 Close() = 0;

        //asynchronous methods.

        virtual void GetHostByName(GetHostByNameParam& , OsclDNSRequestAO&) = 0;
        virtual void GetHostByNameSuccess(GetHostByNameParam&) = 0;

        void CancelFxn(TPVDNSFxn);

    protected:
        OsclDNSIBase(Oscl_DefAlloc &a);

        Oscl_DefAlloc &iAlloc;

        virtual bool IsReady(OsclDNSRequestAO& aObject) = 0;

        virtual void CancelGetHostByName() = 0;

        //link to socket server.
        OsclSocketServI *iSocketServ;

        friend class OsclDNSRequest;
        friend class OsclGetHostByNameRequest;
};

#endif



