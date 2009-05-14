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
#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED

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
#ifndef OSCL_BASE_MACROS_H_INCLUDED
#include "oscl_base_macros.h"
#endif

#include "pvmf_return_codes.h"
#include "pvmf_event_handling.h"

#define PVMF_CPMPLUGIN_LICENSE_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/license_interface"
#define PVMFCPMPluginLicenseInterfaceUuid PVUuid(0xfc8fdd13,0x1c46,0x4c7a,0x9e,0xef,0xc8,0x7d,0x34,0x96,0xb2,0x18)

class PVMFCPMLicenseStatus;

/**
 * License interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginLicenseInterface : public PVInterface
{
    public:
        /**
         * Method to get the license URL
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated unicode string containing the content name
         * @param [out] Null terminated unicode string containing the license URL
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                         OSCL_wString& aContentName,
                                         OSCL_wString& aLicenseURL) = 0;

        /**
         * Method to get the license URL
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated string containing the content name
         * @param [out] Null terminated string containing the license URL
         *
         * @returns PVMFSuccess
         *          PVMFFailure - In case of errors
         */
        virtual PVMFStatus GetLicenseURL(PVMFSessionId aSessionId,
                                         OSCL_String&  aContentName,
                                         OSCL_String&  aLicenseURL) = 0;

        /**
         * Method to get license
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated unicode string containing the content name
         * @param [in] Optional opaque data associated with the request.
         * @param [in] Size of the optional opaque data.
         * @param [in] Timeout for the request in milliseconds, or (-1) for
         *   infinite wait.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId GetLicense(PVMFSessionId aSessionId
                                         , OSCL_wString& aContentName
                                         , OsclAny* aLicenseData = NULL
                                                                   , uint32 aDataSize = 0
                                                                                        , int32 aTimeoutMsec = (-1)
                                                                                                               , OsclAny* aContext = NULL) = 0;

        /**
         * Method to get license
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Null terminated string containing the content name
         * @param [in] Optional opaque data associated with the request.
         * @param [in] Size of the optional opaque data.
         * @param [in] Timeout for the request in milliseconds, or (-1) for
         *   infinite wait.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId GetLicense(PVMFSessionId aSessionId
                                         , OSCL_String&  aContentName
                                         , OsclAny* aLicenseData = NULL
                                                                   , uint32 aDataSize = 0
                                                                                        , int32 aTimeoutMsec = (-1)
                                                                                                               , OsclAny* aContext = NULL) = 0;

        /**
         * Method to cancel GetLicense requests.
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param aContext [in] Optional opaque data that will be passed back to
         *                          the user with the command response
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId CancelGetLicense(PVMFSessionId aSessionId
                                               , PVMFCommandId aCmdId
                                               , OsclAny* aContext = NULL) = 0;

        /**
         * Method to get the status of an ongoing or recently completed
         * license acquire sequence.
         *
         * @param [out] aStatus: license status output
         *
         * @returns: PVMFSuccess if license status is available, an error
         *   otherwise. Nodes should override if needed.
         */
        virtual PVMFStatus GetLicenseStatus(
            PVMFCPMLicenseStatus& aStatus)
        {
            OSCL_UNUSED_ARG(&aStatus);
            return PVMFErrNotSupported;
        };
};


#endif //PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED

