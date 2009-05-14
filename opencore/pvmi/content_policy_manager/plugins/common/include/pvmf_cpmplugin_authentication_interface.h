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
#ifndef PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_H_INCLUDED

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

#define PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/authentication_interface"
#define PVMFCPMPluginAuthenticationInterfaceUuid PVUuid(0xca20319a,0x33f9,0x484f,0x8d,0x1c,0xa5,0x1e,0x23,0x4c,0xe6,0x02)

/**
 * Authentication interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginAuthenticationInterface : public PVInterface
{
    public:
        /**
         * Asynchronous method to authenticate with a plug-in.
         *
         * @param aSessionId The assigned plugin session ID to use for this request
         * @param aAuthenticationData The authentication data that may be needed by the plugin to
         *                            authenticate
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId AuthenticateUser(PVMFSessionId aSessionId,
                                               OsclAny* aAuthenticationData,
                                               OsclAny* aContext = NULL) = 0;
};


#endif //PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_H_INCLUDED

