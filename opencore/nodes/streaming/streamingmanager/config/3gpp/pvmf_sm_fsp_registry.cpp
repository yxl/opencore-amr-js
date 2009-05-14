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
#ifndef PVMF_SM_FSP_REGISTRY_H
#include "pvmf_sm_fsp_registry.h"
#endif

#ifndef PVMF_SM_RTSP_UNICAST_NODE_FACTORY_H_INCLUDED
#include "pvmf_sm_rtsp_unicast_node_factory.h"
#endif

PVMFSMFSPRegistry::PVMFSMFSPRegistry()
{
    PVMFSMFSPInfo fspInfo;
    iType.reserve(10);

    fspInfo.iSourceFormatTypes.clear();
    fspInfo.iSourceFormatTypes.push_back(PVMF_MIME_DATA_SOURCE_RTSP_URL);
    fspInfo.iSourceFormatTypes.push_back(PVMF_MIME_DATA_SOURCE_SDP_FILE);
    fspInfo.iSMFSPUUID = KPVMFSMRTSPUnicastNodeUuid;
    fspInfo.iSMFSPCreateFunc = PVMFSMRTSPUnicastNodeFactory::CreateSMRTSPUnicastNodeFactory;
    fspInfo.iSMFSPReleaseFunc = PVMFSMRTSPUnicastNodeFactory::DeleteSMRTSPUnicastNodeFactory;
    iType.push_back(fspInfo);
}

PVMFSMFSPRegistry::~PVMFSMFSPRegistry()
{
    iType.clear();
}

PVMFStatus PVMFSMFSPRegistry::QueryRegistry(PVMFFormatType& aInputType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    uint32 SearchCount = 0;
    bool matchfound = false;

    // Find all nodes that support the specified src format
    while (SearchCount < iType.size())
    {
        uint32 inputsearchcount = 0;
        while (inputsearchcount < iType[SearchCount].iSourceFormatTypes.size())
        {
            // Check if the input format matches
            if (iType[SearchCount].iSourceFormatTypes[inputsearchcount] == aInputType)
            {
                // Set the the input flag to true since we found the match in the search
                matchfound = true;
                break;
            }
            inputsearchcount++;
        }
        if (matchfound)
            break;
        SearchCount++;
    }

    if (matchfound)
    {
        aUuids.push_back(iType[SearchCount].iSMFSPUUID);
        matchfound = true;
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

PVMFSMFSPBaseNode* PVMFSMFSPRegistry::CreateSMFSP(PVUuid& aUuid)
{
    bool iFoundFlag = false;
    uint32 FSPSearchCount = 0;

    while (FSPSearchCount < iType.size())
    {
        //Search if the UUID's will match
        if (iType[FSPSearchCount].iSMFSPUUID == aUuid)
        {
            //Since the UUID's match set the flag to true
            iFoundFlag = true;
            break;
        }

        FSPSearchCount++;

    }

    if (iFoundFlag)
    {
        //Call the appropriate Node creation function & return Node pointer
        return (*(iType[FSPSearchCount].iSMFSPCreateFunc))(OsclActiveObject::EPriorityNominal);
    }
    else
    {
        return NULL;
    }
}

bool PVMFSMFSPRegistry::ReleaseSMFSP(PVUuid& aUuid, PVMFSMFSPBaseNode *aSMFSP)
{
    bool iFoundFlag = false;
    uint32 FSPSearchCount = 0;

    while (FSPSearchCount < iType.size())
    {
        //Search if the UUID's will match
        if (iType[FSPSearchCount].iSMFSPUUID == aUuid)
        {
            //Since the UUID's match set the flag to true
            iFoundFlag = true;
            break;
        }

        FSPSearchCount++;

    }

    if (iFoundFlag)
    {
        //Call the appropriate Node creation function
        bool del_stat = (*(iType[FSPSearchCount].iSMFSPReleaseFunc))(aSMFSP);
        return del_stat;
    }
    return false;
}



