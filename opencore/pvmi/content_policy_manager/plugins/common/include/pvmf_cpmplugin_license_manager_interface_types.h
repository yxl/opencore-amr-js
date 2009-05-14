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
#ifndef PVMF_CPMPLUGIN_LICENSE_MANAGER_INTERFACE_TYPES_H_INCLUDED
#define PVMF_CPMPLUGIN_LICENSE_MANAGER_INTERFACE_TYPES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif


//A class to hold a content ID.
class PVMFCPMContentId
{
    public:
        PVMFCPMContentId(): iData(NULL), iDataLen(0)
        {}

        PVMFCPMContentId(const PVMFCPMContentId& aVal)
        {
            iData = NULL;
            iDataLen = 0;
            Set(aVal);
        }

        ~PVMFCPMContentId()
        {
            if (iData)
                OSCL_FREE(iData);
            iData = NULL;
        }

        void Set(uint8* aData, uint32 aDataLen)
        {
            if (iData)
                OSCL_FREE(iData);
            iData = NULL;
            if (aDataLen)
            {
                iData = (uint8*)OSCL_MALLOC(aDataLen);
                if (iData)
                    oscl_memcpy(iData, aData, aDataLen);
                iDataLen = aDataLen;
            }
        }
        void Set(const PVMFCPMContentId& aId)
        {
            Set(aId.iData, aId.iDataLen);
        }

        uint8* iData;
        uint32 iDataLen;
};

//A class to hold information about a license request
class PVMFCPMLicenseUpdateInfo
{
    public:
        PVMFCPMLicenseUpdateInfo(): iValid(false)
        {}

        //Tells whether data in this class is valid
        bool iValid;

        //The 0-based index of this entry in the sync store.
        int32 iLicenseSyncIndex;

        //The ID of this entry
        PVMFCPMContentId iLicenseSyncId;

        //The URL of the license server for this content.
        OSCL_HeapString<OsclMemAllocator> iURL;

        void Clear()
        {
            iValid = false;
            iLicenseSyncId.Set(NULL, 0);
        }
        void Set(const PVMFCPMLicenseUpdateInfo& aInfo)
        {
            iValid = aInfo.iValid;
            iLicenseSyncIndex = aInfo.iLicenseSyncIndex;
            iLicenseSyncId.Set(aInfo.iLicenseSyncId);
            iURL = aInfo.iURL;
        }
};

//A class to hold license update status information
class PVMFCPMLicenseUpdateStatus
{
    public:
        PVMFCPMLicenseUpdateStatus():
                iNumLicenseChallengesSent(0)
                , iNumLicenseResponsesReceived(0)
                , iLastLicenseResponseResult(0)
                , iNumLicenseAckChallengesSent(0)
                , iNumLicenseAckResponsesReceived(0)
                , iLastLicenseAckResponseResult(0)
        {}

        //Information about the last entry processed.
        PVMFCPMLicenseUpdateInfo iLicenseUpdateInfo;

        //Number of license challenges sent
        uint32 iNumLicenseChallengesSent;

        //Number of license responses received
        uint32 iNumLicenseResponsesReceived;

        //Result of processing the last license response
        uint32 iLastLicenseResponseResult;

        //Number of license ack challenges sent
        uint32 iNumLicenseAckChallengesSent;

        //Number of license ack responses received
        uint32 iNumLicenseAckResponsesReceived;

        //Result of processing the last license ack response
        uint32 iLastLicenseAckResponseResult;

        void Clear()
        {
            iLicenseUpdateInfo.Clear();
            iNumLicenseChallengesSent = 0;
            iNumLicenseResponsesReceived = 0;
            iLastLicenseResponseResult = 0;
            iNumLicenseAckChallengesSent = 0;
            iNumLicenseAckResponsesReceived = 0;
            iLastLicenseAckResponseResult = 0;
        }
        void Set(const PVMFCPMLicenseUpdateStatus& aStatus)
        {
            iLicenseUpdateInfo.Set(aStatus.iLicenseUpdateInfo);
            iNumLicenseChallengesSent = aStatus.iNumLicenseChallengesSent;
            iNumLicenseResponsesReceived = aStatus.iNumLicenseResponsesReceived;
            iLastLicenseResponseResult = aStatus.iLastLicenseResponseResult;
            iNumLicenseAckChallengesSent = aStatus.iNumLicenseAckChallengesSent;
            iNumLicenseAckResponsesReceived = aStatus.iNumLicenseAckResponsesReceived;
            iLastLicenseAckResponseResult = aStatus.iLastLicenseAckResponseResult;
        }
};

//A class to hold license cleanup status
class PVMFCPMCleanLicenseStoreStatus
{
    public:
        PVMFCPMCleanLicenseStoreStatus():
                iLicensesProcessed(0)
                , iTotalLicenses(0)
        {}

        //Number of licenses processed
        uint32 iLicensesProcessed;

        //Total number of licenses in the store.
        uint32 iTotalLicenses;

        void Clear()
        {
            iLicensesProcessed = 0;
            iTotalLicenses = 0;
        }
        void Set(const PVMFCPMCleanLicenseStoreStatus& aStatus)
        {
            iLicensesProcessed = aStatus.iLicensesProcessed;
            iTotalLicenses = aStatus.iTotalLicenses;
        }
};

#endif //PVMF_CPMPLUGIN_LICENSE_MANAGER_INTERFACE_TYPES H_INCLUDED

