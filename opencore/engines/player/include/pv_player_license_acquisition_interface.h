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
#ifndef PV_PLAYER_LICENSE_ACQUISITION_INTERFACE_H_INCLUDED
#define PV_PLAYER_LICENSE_ACQUISITION_INTERFACE_H_INCLUDED

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
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif

#define PV_PLAYER_LICENSE_ACQUISITION_INTERFACE_MIMETYPE "pvxxx/pvplayer/license_acquisition_interface"
#define PVPlayerLicenseAcquisitionInterfaceUuid PVUuid(0x501302d0,0xd041,0x4fb4,0xad,0x75,0xf0,0x3f,0x90,0x37,0xba,0x42)

class PVMFCPMLicenseStatus;

/**
 * License Acquisition interface for pvPlayer
 */
class PVPlayerLicenseAcquisitionInterface : public PVInterface
{
    public:

        /**
         * Method to get license. Content name is wide character string
         *
         * @param aLicenseData [in] This is any additional data to be used
         *                          by the DRM agent for license acquisition.
         *                          It's format is DRM-agent specific.
         * @param aDataSize [in] Amount of data (in bytes) in the license_data.
         * @param aTimeoutMsec [in] Timeout for the request, or (-1) to
         * 							indicate no timeout (infinite wait).
         * @param aContentName [in] Null terminated Unicode (UCS-2) string containing
         *                          the content name
         * @param aContextData [in] Optional opaque data that will be passed back to
         *                          the user with the command response
         *
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVCommandId AcquireLicense(OsclAny* aLicenseData, uint32 aDataSize,
                                           oscl_wchar *aContentName,
                                           int32 aTimeoutMsec,
                                           const OsclAny* aContextData = NULL) = 0;

        /**
         * Method to get license.
         *
         * @param aLicenseData [in] This is any additional data to be used
         *                          by the DRM agent for license acquisition.
         *                          It's format is DRM-agent specific.
         * @param aDataSize [in] Amount of data (in bytes) in the license_data.
         * @param aContentName [in] Null terminated UTF-8 string containing
         *                          the content name
         * @param aTimeoutMsec [in] Timeout for the request, or (-1) to
         * 							indicate no timeout (infinite wait).
         * @param aContextData [in] Optional opaque data that will be passed back to
         *                          the user with the command response
         *
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVCommandId AcquireLicense(OsclAny* aLicenseData, uint32 aDataSize,
                                           char * aContentName,
                                           int32 aTimeoutMsec,
                                           const OsclAny* aContextData = NULL) = 0;

        /**
         * Method to cancel AcquireLicense requests.
         *
         * @param aContextData [in] Optional opaque data that will be passed back to
         *                          the user with the command response
         *
         * @returns A unique command ID for asynchronous completion.
         */
        virtual PVCommandId CancelAcquireLicense(PVMFCommandId aCmdId, const OsclAny* aContextData = NULL) = 0;


        /**
         * Method to get detailed status on an ongoing or recently completed
         * license acquisition.
         */
        virtual PVMFStatus GetLicenseStatus(PVMFCPMLicenseStatus& aStatus) = 0;

};


#endif // PV_PLAYER_LICENSE_ACQUISITION_INTERFACE_H_INCLUDED

