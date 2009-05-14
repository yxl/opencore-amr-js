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
#ifndef PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#ifndef PVMI_KVP_INCLUDED
#include "pvmi_kvp.h"
#endif

#define PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/authorization_interface"
#define PVMFCPMPluginAuthorizationInterfaceUuid PVUuid(0xfd47907a,0xffde,0x463e,0xa3,0x35,0xd4,0x98,0xff,0xdf,0xd0,0xea)

/**
 * Authorization interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginAuthorizationInterface : public PVInterface
{
    public:
        /**
         * Asynchronous method to authorize usage with a plug-in.
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aRequestedUsage The kvp that contains the requested usage.
         * @param aApprovedUsage The kvp that contains the approved intent.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId AuthorizeUsage(PVMFSessionId aSessionId,
                                             PvmiKvp& aRequestedUsage,
                                             PvmiKvp& aApprovedUsage,
                                             PvmiKvp& aAuthorizationData,
                                             uint32&  aRequestTimeOutInMS,
                                             OsclAny* aContext = NULL) = 0;

        virtual PVMFCommandId UsageComplete(PVMFSessionId aSessionId,
                                            const OsclAny* aContext = NULL) = 0;

};


#endif //PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_H_INCLUDED

