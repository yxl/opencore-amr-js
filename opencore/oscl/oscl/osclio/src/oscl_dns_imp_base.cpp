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
** DNS server base class implementation
*/

#include "osclconfig_io.h"
#include "oscl_scheduler_ao.h"
#include "oscl_dns_imp_base.h"
#include "oscl_socket_serv_imp.h"
#include "oscl_error.h"
#include "oscl_assert.h"
#include "oscl_dns_method.h"

//Common section...

OsclDNSIBase::OsclDNSIBase(Oscl_DefAlloc &a) : iAlloc(a)
{
    iSocketServ = NULL;
}

OsclDNSIBase::~OsclDNSIBase()
{
}

void OsclDNSIBase::CancelFxn(TPVDNSFxn aFxn)
{
    switch (aFxn)
    {
        case EPVDNSGetHostByName:
            CancelGetHostByName();
            break;
        default:
            OSCL_ASSERT(false);
            break;
    }
}









