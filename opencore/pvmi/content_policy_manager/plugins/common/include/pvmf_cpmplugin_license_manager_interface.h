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
#ifndef PVMF_CPMPLUGIN_LICENSE_MANAGER_INTERFACE_H_INCLUDED
#define PVMF_CPMPLUGIN_LICENSE_MANAGER_INTERFACE_H_INCLUDED

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
#include "pvmf_cpmplugin_license_manager_interface_types.h"

#define PVMF_CPMPLUGIN_LICENSE_MANAGER_INTERFACE_MIMETYPE "pvxxx/pvmf/cpm/plugin/license_manager_interface"
#define PVMFCPMPluginLicenseManagerInterfaceUuid PVUuid(0x05b8186a,0xc2b1,0x11db,0x83,0x14,0x08,0x00,0x20,0x0c,0x9a,0x66)

/**
 * License Manager interface for all Content Policy Manager Plugins
 */
class PVMFCPMPluginLicenseManagerInterface : public PVInterface
{
    public:

        /**
         * Method to clean license store
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] Optional opaque data associated with the request.
         * @param [in] Size of the optional opaque data.
         * @param [in] aContextData: optional caller data to accompany the
         *   request.  The value will be included in the completion callback.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId CleanLicenseStore(PVMFSessionId aSessionId
                                                , OsclAny* aData = NULL
                                                                   , uint32 aDataSize = 0
                                                                                        , OsclAny* aContext = NULL) = 0;

        /**
         * Method to get the status of an ongoing or recently completed
         * clean license store command.
         *
         * @param [out] aStatus: clean store status output
         *
         * @returns: PVMFSuccess if status is available, an error
         *   otherwise.
         */
        virtual PVMFStatus GetCleanLicenseStoreStatus(
            PVMFCPMCleanLicenseStoreStatus& aStatus) = 0;

        /**
         * Method to delete a license from the store
         *
         * @param [in] The assigned plugin session ID to use for this request
         * @param [in] PVMFCPMContentId: content ID data blob.
         * @param [in] aContextData: optional caller data to accompany the
         *   request.  The value will be included in the completion callback.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId DeleteLicense(PVMFSessionId aSessionId
                                            , const PVMFCPMContentId& aContentId
                                            , OsclAny* aContext = NULL) = 0;

        /**
         * Method to update current licenses by requesting new licenses
         * from the server.  When the number of licenses is large, the update can be done
         * in multiple calls by using the aStartingIndex and aMaxNumberofLicenses parameters
         * to control the range of licenses updated.  The "hours remaining" and "count remaining"
         * parameters can be used to control how time and count-based licenses are updated.
         *
         * To get status during or after this operation, use GetLicenseUpdateStatus.
         * To interrupt and cancel the sequence, use the plugin CancelCommand API.
         *
         * @param [in] aSessionId: The assigned plugin session ID to use for this request
         * @param [out] aLastLicenseProcessed: The 0-based index of the last license processed
         *   by the request.
         * @param [in] aStartingIndex: The 0-based index of the first license to process.
         * @param [in] aMaxNumberOfLicenses: The maximum number of licenses to update in this
         *   request.  To update all licenses, use (-1).
         * @param [in] aHoursRemaining: For time-based licenses, update only those that have
         *    less than the specified value of hours remaining.  To update all time-based
         *    licenses regardless of the time remaining, use (-1).
         * @param [in] aCountRemaining: For counted licenses, update only those that have
         *    less than the specified number of play counts remaining.  To update all counted
         *    licenses regardless of the counts remaining, use (-1).
         * @param [in] aCustomData, aCustomDataSize: Optional opaque data for additional inputs.
         * @param [in] aTimeoutMsec: Optional timeout in milliseconds
         *    for each server communication.  Use -1 to indicate infinite wait.
         * @param [in] aContextData: Optional caller data to accompany the
         *   request.  The value will be included in the completion callback.
         *
         * @returns A unique command id for asynchronous completion.
         */
        virtual PVMFCommandId UpdateLicenses(
            PVMFSessionId aSessionId
            , int32& aLastLicenseProcessed
            , uint32 aStartingIndex = 0
                                      , int32 aMaxNumberOfLicenses = (-1)
                                                                     , int32 aHoursRemaining = (-1)
                                                                                               , int32 aCountRemaining = (-1)
                                                                                                                         , OsclAny* aCustomData = NULL
                                                                                                                                                  , uint32 aCustomDataSize = 0
                                                                                                                                                                             , int32 aTimeoutMsec = (-1)
                                                                                                                                                                                                    , OsclAny* aContext = NULL) = 0;

        /**
         * Method to get the status of an ongoing or recently completed
         * license update sequence.
         *
         * @param [out] aStatus: meter status output
         *
         * @returns: PVMFSuccess if meter status is available, an error
         *   otherwise.
         */
        virtual PVMFStatus GetLicenseUpdateStatus(
            PVMFCPMLicenseUpdateStatus& aStatus) = 0;

};


#endif //PVMF_CPMPLUGIN_LICENSE_INTERFACE_H_INCLUDED

