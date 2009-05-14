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

#ifndef OSCL_DNS_TUNEABLES_H_INCLUDED
#define OSCL_DNS_TUNEABLES_H_INCLUDED


/*!
** This file contains default definitions of all the
** the tuning parameters for the Oscl DNS.
*/

/*!
** If parameters are defined in osclconfig_io.h, those definitions
** will take precedence over the ones in this file.
*/
#include "osclconfig_io.h"
#include "osclconfig_proc.h"

/*!
** Enable/disable the PV DNS server here.
*/
#ifndef PV_DNS_SERVER
#define PV_DNS_SERVER 1
#endif

#if(PV_DNS_SERVER)

/*!
** PV_DNS_IS_THREAD chooses either the threaded or AO-based
** implementation of the PV DNS request.  Note: AO-based
** option is not good here, since some DNS requests will
** block the caller until completion.
*/
#if !defined PV_DNS_IS_THREAD
#define PV_DNS_IS_THREAD OSCL_HAS_THREAD_SUPPORT
#endif

//for test-- to force AO implementation
//#undef PV_DNS_IS_THREAD
//#define PV_DNS_IS_THREAD 0


#endif//PV_DNS_SERVER
#endif

