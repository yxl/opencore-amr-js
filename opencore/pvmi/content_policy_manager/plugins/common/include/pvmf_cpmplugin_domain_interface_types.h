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
#ifndef PVMF_CPMPLUGIN_DOMAIN_INTERFACE_TYPES_H_INCLUDED
#define PVMF_CPMPLUGIN_DOMAIN_INTERFACE_TYPES_H_INCLUDED

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

#define PVMF_EMPTY_GUID PVUuid(0,0,0,0,0,0,0,0,0,0,0)

class PVMFCPMDomainId
{
    public:
        PVMFCPMDomainId()
                : iServiceId(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                , iAccountId(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
                , iRevision(0)
        {}
        PVMFCPMDomainId(const PVMFCPMDomainId& aVal)
        {
            iServiceId = aVal.iServiceId;
            iAccountId = aVal.iAccountId;
            iRevision = aVal.iRevision;
        }
        void SetServiceId(uint8 b0, uint8 b1, uint8 b2, uint8 b3
                          , uint8 b4, uint8 b5, uint8 b6, uint8 b7, uint8 b8
                          , uint8 b9, uint8 b10, uint8 b11, uint8 b12
                          , uint8 b13, uint8 b14, uint8 b15)
        {
            uint8* pos = (uint8*) & iServiceId;
            *pos++ = b0;
            *pos++ = b1;
            *pos++ = b2;
            *pos++ = b3;
            *pos++ = b4;
            *pos++ = b5;
            *pos++ = b6;
            *pos++ = b7;
            *pos++ = b8;
            *pos++ = b9;
            *pos++ = b10;
            *pos++ = b11;
            *pos++ = b12;
            *pos++ = b13;
            *pos++ = b14;
            *pos++ = b15;
        }
        void SetAccountId(uint8 b0, uint8 b1, uint8 b2, uint8 b3
                          , uint8 b4, uint8 b5, uint8 b6, uint8 b7, uint8 b8
                          , uint8 b9, uint8 b10, uint8 b11, uint8 b12
                          , uint8 b13, uint8 b14, uint8 b15)
        {
            uint8* pos = (uint8*) & iAccountId;
            *pos++ = b0;
            *pos++ = b1;
            *pos++ = b2;
            *pos++ = b3;
            *pos++ = b4;
            *pos++ = b5;
            *pos++ = b6;
            *pos++ = b7;
            *pos++ = b8;
            *pos++ = b9;
            *pos++ = b10;
            *pos++ = b11;
            *pos++ = b12;
            *pos++ = b13;
            *pos++ = b14;
            *pos++ = b15;
        }
        PVUuid iServiceId;
        PVUuid iAccountId;
        uint32 iRevision;
};

//Data associated with a Domain Join request
class PVMFCPMDomainJoinData
{
    public:
        PVMFCPMDomainJoinData()
                : iDomainUrl(NULL)
                , iFlags(0)
                , iFriendlyName(NULL)
                , iCustomData(NULL)
                , iCustomDataLen(0)
        {}

        OSCL_String* iDomainUrl;	//the domain server URL

        uint32 iFlags;				//Flag that indicates the type of custom data.

        PVMFCPMDomainId iDomainId;	//Domain ID to be registered with the server.

        OSCL_String* iFriendlyName;	//Pointer to the friendly name. Can be NULL.

        uint8* iCustomData;			//Optional custom data.  Can be NULL
        uint32 iCustomDataLen;		//length of the custom data in bytes.
};

//Data associated with a Domain Leave request
class PVMFCPMDomainLeaveData
{
    public:
        PVMFCPMDomainLeaveData()
                : iDomainUrl(NULL)
                , iFlags(0)
                , iCustomData(NULL)
                , iCustomDataLen(0)
        {}

        OSCL_String* iDomainUrl;	//Domain URL.

        uint32 iFlags;				//Flag that indicates the type of custom data.

        PVMFCPMDomainId iDomainId;	//Domain ID to be unregistered with the server.

        uint8* iCustomData;			//Pointer to a buffer that contains the custom data
        //sent to the server. The format of the custom data
        //is based on the value of iFlags. It may be NULL.

        uint32 iCustomDataLen;		//Size (in bytes) of the custom data buffer.
        //It must be 0 if iCustomData is NULL.
};

//Data output by GetDomain request.
class PVMFCPMDomainCertData
{
    public:
        PVMFCPMDomainId iDomainId;//Domain ID

        OSCL_HeapString<OsclMemAllocator> iUrl;//a URL in the domain cert.  Not used currently.
};


#endif //PVMF_CPMPLUGIN_DOMAIN_INTERFACE_TYPES_H_INCLUDED

