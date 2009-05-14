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
#ifndef PVMF_CPMPLUGIN_LICENSE_INTERFACE_TYPES_H_INCLUDED
#define PVMF_CPMPLUGIN_LICENSE_INTERFACE_TYPES_H_INCLUDED

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


//A class to hold detailed license status information
class PVMFCPMLicenseStatus
{
    public:
        PVMFCPMLicenseStatus():
                iNumLicenseChallengesSent(0)
                , iNumLicenseResponsesReceived(0)
                , iLastLicenseResponseResult(0)
                , iNumLicenseAckChallengesSent(0)
                , iNumLicenseAckResponsesReceived(0)
                , iLastLicenseAckResponseResult(0)
                , iNumFallbackLicenseChallengesSent(0)
                , iNumFallbackLicenseResponsesReceived(0)
                , iLastFallbackLicenseResponseResult(0)
                , iNumJoinChallengesSent(0)
                , iNumJoinResponsesReceived(0)
                , iLastJoinResponseResult(0)
                , iNumLeaveChallengesSent(0)
                , iNumLeaveResponsesReceived(0)
                , iLastLeaveResponseResult(0)
                , iLastErrorResult(0)
        {}


        OSCL_HeapString<OsclMemAllocator> iLastLicenseURL;
        uint32 iNumLicenseChallengesSent;
        uint32 iNumLicenseResponsesReceived;
        uint32 iLastLicenseResponseResult;

        uint32 iNumLicenseAckChallengesSent;
        uint32 iNumLicenseAckResponsesReceived;
        uint32 iLastLicenseAckResponseResult;

        OSCL_HeapString<OsclMemAllocator> iLastFallbackLicenseURL;
        uint32 iNumFallbackLicenseChallengesSent;
        uint32 iNumFallbackLicenseResponsesReceived;
        uint32 iLastFallbackLicenseResponseResult;

        OSCL_HeapString<OsclMemAllocator> iLastJoinURL;
        uint32 iNumJoinChallengesSent;
        uint32 iNumJoinResponsesReceived;
        uint32 iLastJoinResponseResult;

        OSCL_HeapString<OsclMemAllocator> iLastLeaveURL;
        uint32 iNumLeaveChallengesSent;
        uint32 iNumLeaveResponsesReceived;
        uint32 iLastLeaveResponseResult;

        uint32 iLastErrorResult;

        void Clear()
        {
            iLastLicenseURL = "";
            iNumLicenseChallengesSent = iNumLicenseResponsesReceived = iLastLicenseResponseResult = 0;
            iNumLicenseAckChallengesSent = iNumLicenseAckResponsesReceived = iLastLicenseAckResponseResult = 0;

            iLastFallbackLicenseURL = "";
            iNumFallbackLicenseChallengesSent = iNumFallbackLicenseResponsesReceived = iLastFallbackLicenseResponseResult = 0;

            iLastJoinURL = "";
            iNumJoinChallengesSent = iNumJoinResponsesReceived = iLastJoinResponseResult = 0;

            iLastLeaveURL = "";
            iNumLeaveChallengesSent = iNumLeaveResponsesReceived = iLastLeaveResponseResult = 0;

            iLastErrorResult = 0;
        }
        void Set(const PVMFCPMLicenseStatus& aStatus)
        {
            iLastLicenseURL = aStatus.iLastLicenseURL;
            iNumLicenseChallengesSent = aStatus.iNumLicenseChallengesSent;
            iNumLicenseResponsesReceived = aStatus.iNumLicenseResponsesReceived;
            iLastLicenseResponseResult = aStatus.iLastLicenseResponseResult;
            iNumLicenseAckChallengesSent = aStatus.iNumLicenseAckChallengesSent;
            iNumLicenseAckResponsesReceived = aStatus.iNumLicenseAckResponsesReceived;
            iLastLicenseAckResponseResult = aStatus.iLastLicenseAckResponseResult;
            iLastFallbackLicenseURL = aStatus.iLastFallbackLicenseURL;
            iNumFallbackLicenseChallengesSent = aStatus.iNumFallbackLicenseChallengesSent;
            iNumFallbackLicenseResponsesReceived = aStatus.iNumFallbackLicenseResponsesReceived;
            iLastFallbackLicenseResponseResult = aStatus.iLastFallbackLicenseResponseResult;
            iLastJoinURL = aStatus.iLastJoinURL;
            iNumJoinChallengesSent = aStatus.iNumJoinChallengesSent;
            iNumJoinResponsesReceived = aStatus.iNumJoinResponsesReceived;
            iLastJoinResponseResult = aStatus.iLastJoinResponseResult;
            iLastLeaveURL = aStatus.iLastLeaveURL;
            iNumLeaveChallengesSent = aStatus.iNumLeaveChallengesSent;
            iNumLeaveResponsesReceived = aStatus.iNumLeaveResponsesReceived;
            iLastLeaveResponseResult = aStatus.iLastLeaveResponseResult;
            iLastErrorResult = aStatus.iLastErrorResult;
        }
};

#endif //PVMF_CPMPLUGIN_LICENSE_INTERFACE_TYPES_H_INCLUDED

