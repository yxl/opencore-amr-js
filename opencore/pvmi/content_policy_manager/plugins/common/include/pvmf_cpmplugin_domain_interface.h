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
#ifndef PVMF_CPMPLUGIN_DOMAIN_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_DOMAIN_INTERFACE_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif
#include "pvmf_return_codes.h"
#include "pvmf_event_handling.h"
#include "pvmf_cpmplugin_domain_interface_types.h"

#define PVMF_CPMPLUGIN_DOMAIN_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/domain_interface"
#define PVMFCPMPluginDomainInterfaceUuid PVUuid(0x379e6e10,0x8a53,0x11db,0xb6,0x06,0x08,0x00,0x20,0x0c,0x9a,0x66)


/**
 * License interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginDomainInterface : public PVInterface
{
    public:
        /**
         * Method to get the current number of domains
         *
         * @param [out] aCount: number of domains
         * @param [out] errcode: error code in case of failure.
         *
         * @returns PVMFSuccess or an error.
         */
        virtual PVMFStatus GetDomainCount(
            uint32& aCount
            , uint32& errcode) = 0;

        /**
         * Method to get the current domains.
         *
         * @param [in] aIndex: zero-based index of the desired domain info.
         *
         * @param [out] aDomainCertData the domain cert data.
         *
         * @param [out] errcode: error code in case of failure.
         *
         * @returns PVMFSuccess or an error.
         */
        virtual PVMFStatus GetDomain(
            uint32 aIndex
            , PVMFCPMDomainCertData& aDomainCertData
            , uint32& errcode) = 0;

        /**
         * Method to check existence of a domain certificate.
         *
         * @param [in] aDomainId: ID of the domain to check
         *
         * @param [out] errcode: error code in case of failure.
         *
         * @returns PVMFSuccess or an error.
         */
        virtual PVMFStatus CheckDomainCertificate(
            const PVMFCPMDomainId& aDomainId
            , uint32& errcode) = 0;

        /**
         * Method to join a domain
         *
         * @param [in] aSessionId: The assigned plugin session ID to use for this request
         *
         * @param [in] aJoinData: request parameters.
         *
         * @param [in] Timeout for the request in milliseconds, or (-1) for
         *   infinite wait.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId JoinDomain(
            PVMFSessionId aSessionId
            , const PVMFCPMDomainJoinData& aJoinData
            , int32 aTimeoutMsec = (-1)) = 0;

        /**
         * Method to leave a domain
         *
         * @param [in] aSessionId: The assigned plugin session ID to use for this request
         *
         * @param [in] aLeaveData: request parameters.
         *
         * @param [in] Timeout for the request in milliseconds, or (-1) for
         *   infinite wait.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId LeaveDomain(
            PVMFSessionId aSessionId
            , const PVMFCPMDomainLeaveData& aLeaveData
            , int32 aTimeoutMsec = (-1)) = 0;

};

#endif //PVMF_CPMPLUGIN_DOMAIN_INTERFACE_H_INCLUDED

