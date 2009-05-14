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


#ifndef OSCL_DNS_IMP_H_INCLUDED
#define OSCL_DNS_IMP_H_INCLUDED
/*!
** A common include file for OsclDNSI declaration
*/

#include "oscl_dns_tuneables.h"
#if   PV_DNS_SERVER
#include "oscl_dns_imp_pv.h"
#else
#error No Implementation!
#endif


#endif



