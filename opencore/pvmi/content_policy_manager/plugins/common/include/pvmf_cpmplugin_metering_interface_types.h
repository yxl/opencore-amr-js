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
#ifndef PVMF_CPMPLUGIN_METERING_INTERFACE_TYPES_H_INCLUDED
#define PVMF_CPMPLUGIN_METERING_INTERFACE_TYPES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif

//A class to hold a metering ID.
class PVMFCPMMeterId
{
    public:
        PVMFCPMMeterId(): iData(NULL), iDataLen(0)
        {}

        PVMFCPMMeterId(const PVMFCPMMeterId& aVal)
        {
            iData = NULL;
            iDataLen = 0;
            Set(aVal);
        }

        ~PVMFCPMMeterId()
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
        void Set(const PVMFCPMMeterId& aId)
        {
            Set(aId.iData, aId.iDataLen);
        }

        uint8* iData;
        uint32 iDataLen;
};

//A class to hold information about a metering certificate
class PVMFCPMMeterCertInfo
{
    public:
        PVMFCPMMeterCertInfo(): iValid(false)
        {}

        //Tells whether data in this class is valid
        bool iValid;

        //The ID of this metering certificate
        PVMFCPMMeterId iMeterId;

        //The URL of the metering service.
        OSCL_wHeapString<OsclMemAllocator> iURL;

        //Tells if the server is V2 (Janus).
        bool iIsV2;

        void Clear()
        {
            iValid = false;
            iMeterId.Set(NULL, 0);
            iIsV2 = false;
        }
        void Set(const PVMFCPMMeterCertInfo& aInfo)
        {
            iValid = aInfo.iValid;
            if (iValid)
            {
                iMeterId.Set(aInfo.iMeterId);
                iURL = aInfo.iURL;
                iIsV2 = aInfo.iIsV2;
            }
        }
};

//A class to hold meter status information
class PVMFCPMMeterStatus
{
    public:
        PVMFCPMMeterStatus():
                iNumMeterCertChallengesSent(0)
                , iNumMeterCertResponsesReceived(0)
                , iLastMeterCertResponseResult(0)
                , iNumMeterChallengesSent(0)
                , iNumMeterResponsesReceived(0)
                , iLastMeterResponseResult(0)
        {}

        //Information about the last entry retrieved from
        //the meter cert store.
        PVMFCPMMeterCertInfo iMeterCertInfo;


        //Number of meter cert challenges sent
        uint32 iNumMeterCertChallengesSent;

        //Number of meter cert responses received
        uint32 iNumMeterCertResponsesReceived;

        //Result of processing the last meter cert response
        uint32 iLastMeterCertResponseResult;

        //Number of meter challenges sent
        uint32 iNumMeterChallengesSent;

        //Number of meter responses received
        uint32 iNumMeterResponsesReceived;

        //result of processing the last meter response
        uint32 iLastMeterResponseResult;

        //The URL of the  metering server for this content.
        OSCL_HeapString<OsclMemAllocator> iLastMeterURL;

        void Clear()
        {
            iMeterCertInfo.Clear();
            iNumMeterCertChallengesSent = 0;
            iNumMeterCertResponsesReceived = 0;
            iLastMeterCertResponseResult = 0;
            iNumMeterChallengesSent = 0;
            iNumMeterResponsesReceived = 0;
            iLastMeterResponseResult = 0;
            iLastMeterURL = "";
        }
        void Set(const PVMFCPMMeterStatus& aStatus)
        {
            iMeterCertInfo.Set(aStatus.iMeterCertInfo);
            iNumMeterCertChallengesSent = aStatus.iNumMeterCertChallengesSent;
            iNumMeterCertResponsesReceived = aStatus.iNumMeterCertResponsesReceived;
            iLastMeterCertResponseResult = aStatus.iLastMeterCertResponseResult;
            iNumMeterChallengesSent = aStatus.iNumMeterChallengesSent;
            iNumMeterResponsesReceived = aStatus.iNumMeterResponsesReceived;
            iLastMeterResponseResult = aStatus.iLastMeterResponseResult;
            iLastMeterURL = aStatus.iLastMeterURL;
        }
};


#endif //PVMF_CPMPLUGIN_METERING_INTERFACE_TYPES_H_INCLUDED

