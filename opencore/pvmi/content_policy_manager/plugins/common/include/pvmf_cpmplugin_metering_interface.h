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
#ifndef PVMF_CPMPLUGIN_METERING_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_METERING_INTERFACE_H_INCLUDED

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
#include "pvmf_cpmplugin_metering_interface_types.h"

#define PVMF_CPMPLUGIN_METERING_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/metering_interface"
#define PVMFCPMPluginMeteringInterfaceUuid PVUuid(0x5efe8be0,0xb62f,0x11db,0xab,0xbd,0x08,0x00,0x20,0x0c,0x9a,0x66)


/**
 * Metering interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginMeteringInterface : public PVInterface
{
    public:
        /**
         * Method to save a meter certificate in the store.
         * This method is intended to be used in scenarios where
         * it is not possible to obtain the metering certificate
         * directly from the server.
         *
         * @param [in] aSessionId: The observer session Id.
         * @param [in] aMeterCert: The meter certificate data as a wide-string.
         * @param [out] optional output metering ID
         * @param [in] aTimeoutMsec: Optional timeout in milliseconds
         *    for each server communication.  Use -1 to indicate infinite wait.
         * @param [in] aContextData: Optional user data that will be returned
         *    in the command completion callback.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId UpdateMeterCertificate(
            PVMFSessionId aSessionId
            , OSCL_wString& aMeterCert
            , PVMFCPMMeterId* aMeterId = NULL
                                         , int32 aTimeoutMsec = (-1)
                                                                , OsclAny* aContextData = NULL) = 0;

        /**
         * Method to report metering data for a specific Meter ID.
         * The meter cert store will be searched for a certificate with
         * the given meter ID.  If the meter ID is found but there is no
         * certificate, then an attempt will be made to acquire the
         * certificate from the server.  If the certificate is acquired
         * successfully then metering will be reported for that certificate.
         *
         * To get status during or after this operation, use GetMeteringStatus.
         * To interrupt and cancel metering, use the plugin CancelCommand.
         *
         * @param [in] aSessionId: The observer session Id.
         * @param [in] aMaxDataSize: The maximum data size for
         *    each metering data buffer to be sent to the server.
         *    If there is more data than this size, then multiple
         *    server transactions will be done automatically.
         * @param [in] aMeterId: The metering Id.
         * @param [out] aStatus: Optional meter status output
         * @param [in] aTimeoutMsec: Optional timeout in milliseconds
         *    for each server communication.  Use -1 to indicate infinite wait.
         * @param [in] aContextData: Optional user data that will be returned
         *    in the command completion callback.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId ReportMeteringData(
            PVMFSessionId aSessionId
            , uint32 aMaxDataSize
            , const PVMFCPMMeterId& aMeterId
            , int32 aTimeoutMsec = (-1)
                                   , OsclAny* aContextData = NULL) = 0;

        /**
         * Method to report metering data by iterating through the meter
         * cert store.
         * This method may acquire meter certs from a server.
         *
         * To get status during or after this operation, use GetMeteringStatus.
         * To interrupt and cancel metering, use the plugin CancelCommand..
         *
         * @param [in] aSessionId: The observer session Id.
         * @param [in] aMaxDataSize: The maximum data size for
         *    each metering data buffer to be sent to the server.
         *    If there is more data than this size, then multiple
         *    server transactions will be done automatically.
         * @param [in] aTimeoutMsec: Optional timeout in milliseconds
         *    for each server communication.  Use -1 to indicate infinite wait.
         * @param [in] aContextData: Optional user data that will be returned
         *    in the command completion callback.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId ReportMeteringData(
            PVMFSessionId aSessionId
            , uint32 aMaxDataSize
            , int32 aTimeoutMsec = (-1)
                                   , OsclAny* aContextData = NULL) = 0;

        /**
         * Method to get a list of all MIDs in the current meter cert store.
         *
         * @param [out] aMeterCertList: List of all MIDs.
         * @param [out] aErrCode: Error code
         *
         * @returns: PVMFSuccess or an error.
         */
        virtual PVMFStatus GetMeterCertMIDList(
            Oscl_Vector<PVMFCPMMeterId, OsclMemAllocator>& aMeterCertList,
            uint32& aErrCode) = 0;

        /**
         * Method to get the status of an ongoing or recently completed
         * metering sequence.
         *
         * @param [out] aStatus: meter status output
         *
         * @returns: PVMFSuccess if meter status is available, an error
         *   otherwise.
         */
        virtual PVMFStatus GetMeteringStatus(
            PVMFCPMMeterStatus& aStatus) = 0;

};

#endif //PVMF_CPMPLUGIN_DOMAIN_INTERFACE_H_INCLUDED

