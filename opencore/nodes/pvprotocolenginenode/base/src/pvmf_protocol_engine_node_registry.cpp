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
#include "pvmf_protocol_engine_node_registry.h"
#include "pvmf_protocol_engine_node_common.h"



#if (BUILD_PROGRESSIVE_DOWNLOAD_PLUGIN)
#include "pvmf_protocol_engine_node_progressive_download_container_factory.h"
#endif

#if(BUILD_PROGRESSIVE_STREAMING_PLUGIN)
#include "pvmf_protocol_engine_node_progressive_streaming_container_factory.h"
#endif

#if(BUILD_SHOUTCAST_PLUGIN)
#include "pvmf_protocol_engine_node_shoutcast_container_factory.h"
#endif

#if(BUILD_FASTTRACK_DOWNLOAD_PLUGIN)
#include "pvmf_protocol_engine_node_fasttrack_download_container_factory.h"
#endif

#if(BUILD_WMHTTPSTREAMING_PLUGIN)
#include "pvmf_protocol_engine_node_wm_http_streaming_container_factory.h"
#endif

#ifdef USE_LOADABLE_MODULES
#include "oscl_shared_library.h"
#include "oscl_library_list.h"
#include "oscl_configfile_list.h"
#include "osclconfig_lib.h"
#include "oscl_shared_lib_interface.h"
#endif //USE_LOADABLE_MODULES


#define DEFAULT_NODE_REGISTRY_RESERVED_NUMBER 16
#define PROGRESSIVE_STREAMING_TYPE_BIT 1


PVMFProtocolEngineNodeRegistry::PVMFProtocolEngineNodeRegistry() : iTypeVecIndex(0xFFFFFFFF)
{
    PVMFProtocolEngineContainerInfo aContainerInfo;
    iTypeVec.reserve(DEFAULT_NODE_REGISTRY_RESERVED_NUMBER);

#if(BUILD_PROGRESSIVE_DOWNLOAD_PLUGIN)
    aContainerInfo.iSourceType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
    aContainerInfo.iProtocolEngineContainerUUID = KPVMFProtocolEngineNodeProgressiveDownloadUuid;
    aContainerInfo.iProtocolEngineContainerCreateFunc = PVMFProtocolEngineNodeProgressiveDownloadContainerFactory::Create;
    aContainerInfo.iProtocolEngineContainerReleaseFunc = PVMFProtocolEngineNodeProgressiveDownloadContainerFactory::Delete;
    iTypeVec.push_back(aContainerInfo);
    aContainerInfo.clear();
#endif

#if(BUILD_PROGRESSIVE_STREAMING_PLUGIN)
    aContainerInfo.iSourceType = PVMF_MIME_DATA_SOURCE_HTTP_URL;
    aContainerInfo.iSourceExtraInfo = PROGRESSIVE_STREAMING_TYPE_BIT;
    aContainerInfo.iProtocolEngineContainerUUID = KPVMFProtocolEngineNodeProgressiveStreamingUuid;
    aContainerInfo.iProtocolEngineContainerCreateFunc = PVMFProtocolEngineNodeProgressiveStreamingContainerFactory::Create;
    aContainerInfo.iProtocolEngineContainerReleaseFunc = PVMFProtocolEngineNodeProgressiveStreamingContainerFactory::Delete;
    iTypeVec.push_back(aContainerInfo);
    aContainerInfo.clear();
#endif

#if(BUILD_SHOUTCAST_PLUGIN)
    aContainerInfo.iSourceType = PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL;
    aContainerInfo.iProtocolEngineContainerUUID = KPVMFProtocolEngineNodeShoutcastUuid;
    aContainerInfo.iProtocolEngineContainerCreateFunc = PVMFProtocolEngineNodeShoutcastContainerFactory::Create;
    aContainerInfo.iProtocolEngineContainerReleaseFunc = PVMFProtocolEngineNodeShoutcastContainerFactory::Delete;
    iTypeVec.push_back(aContainerInfo);
    aContainerInfo.clear();
#endif

#if(BUILD_FASTTRACK_DOWNLOAD_PLUGIN)
    aContainerInfo.iSourceType = PVMF_MIME_DATA_SOURCE_PVX_FILE;
    aContainerInfo.iProtocolEngineContainerUUID = KPVMFProtocolEngineNodeFasttrackDownloadContainerUuid;
    aContainerInfo.iProtocolEngineContainerCreateFunc = PVMFProtocolEngineNodeFasttrackDownloadContainerFactory::Create;
    aContainerInfo.iProtocolEngineContainerReleaseFunc = PVMFProtocolEngineNodeFasttrackDownloadContainerFactory::Delete;
    iTypeVec.push_back(aContainerInfo);
    aContainerInfo.clear();
#endif

#if(BUILD_WMHTTPSTREAMING_PLUGIN)
    aContainerInfo.iSourceType = PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL;
    aContainerInfo.iProtocolEngineContainerUUID = KPVMFProtocolEngineNodeWMHttpStreamingUuid;
    aContainerInfo.iProtocolEngineContainerCreateFunc = PVMFProtocolEngineNodeWMHttpStreamingContainerFactory::Create;
    aContainerInfo.iProtocolEngineContainerReleaseFunc = PVMFProtocolEngineNodeWMHttpStreamingContainerFactory::Delete;
    iTypeVec.push_back(aContainerInfo);
#endif

    iLogger = PVLogger::GetLoggerObject("PVMFProtocolEngineNode");;
}


PVMFProtocolEngineNodeRegistry::~PVMFProtocolEngineNodeRegistry()
{
    iTypeVec.clear();
    RemoveLoadableModules();
}

bool PVMFProtocolEngineNodeRegistry::CheckPluginAvailability(PVMFFormatType& aSourceFormat, OsclAny* aSourceData)
{
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::CheckPluginAvailability() IN"));

    PVMFFormatType aSourceType = aSourceFormat;
    if (aSourceFormat == PVMF_MIME_DATA_SOURCE_HTTP_URL && aSourceData == NULL) aSourceType = PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL;

    for (uint32 i = 0; i < iTypeVec.size(); i++)
    {
        if (iTypeVec[i].iSourceType == aSourceType)
        {
            iTypeVecIndex = i;
            return true;
        }
    }

    // load plug-ins for further checkup
    uint32 j = iTypeVec.size();
    AddLoadableModules();

    for (; j < iTypeVec.size(); j++)
    {
        if (iTypeVec[j].iSourceType == aSourceType)
        {
            iTypeVecIndex = j;
            return true;
        }
    }

    return false;
}

ProtocolContainer* PVMFProtocolEngineNodeRegistry::CreateProtocolEngineContainer(OsclAny* &aPluginInfo, PVMFProtocolEngineNode* aNode)
{
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::CreateProtocolEngineContainer() IN"));

    if (iTypeVecIndex == 0xFFFFFFFF) return NULL;
    RecheckPlugin(aPluginInfo);
    ProtocolContainer* aProtocolContainer = (*(iTypeVec[iTypeVecIndex].iProtocolEngineContainerCreateFunc))(aNode);
    aPluginInfo = (OsclAny*)(iTypeVec[iTypeVecIndex].iSourceExtraInfo);
    return aProtocolContainer;
}

bool PVMFProtocolEngineNodeRegistry::ReleaseProtocolEngineContainer(ProtocolContainer *aContainer)
{
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::ReleaseProtocolEngineContainer() IN"));

    if (NULL == aContainer) return false;
    if (iTypeVecIndex == 0xFFFFFFFF) return false;
    (*(iTypeVec[iTypeVecIndex].iProtocolEngineContainerReleaseFunc))(aContainer);
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::ReleaseProtocolEngineContainer() OUT"));
    return true;
}


bool PVMFProtocolEngineNodeRegistry::RecheckPlugin(OsclAny* aNewPluginInfo)
{
    uint32 isProgessiveStreaming = (aNewPluginInfo == NULL ? 0 : (uint32)aNewPluginInfo);
    if (isProgessiveStreaming != iTypeVec[iTypeVecIndex].iSourceExtraInfo)
    {
        for (uint32 i = 0; i < iTypeVec.size(); i++)
        {
            if (iTypeVec[i].iSourceExtraInfo == isProgessiveStreaming &&
                    iTypeVec[i].iSourceType == iTypeVec[iTypeVecIndex].iSourceType)
            {
                iTypeVecIndex = i;
                return true;
            }
        }
    }
    return false;
}

void PVMFProtocolEngineNodeRegistry::AddLoadableModules()
{
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::AddLoadableModules() IN"));

#ifdef USE_LOADABLE_MODULES

    OsclConfigFileList aCfgList;
    // collects all config files from the project specified directory
    if (NULL != PV_DYNAMIC_LOADING_CONFIG_FILE_PATH)
    {
        OSCL_HeapString<OsclMemAllocator> configFilePath = PV_DYNAMIC_LOADING_CONFIG_FILE_PATH;
        aCfgList.Populate(configFilePath);
    }
    // populate libraries from all config files
    for (uint k = 0; k < aCfgList.Size(); k++)
    {
        OsclLibraryList libList;
        libList.Populate(PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_POPULATOR_INTERFACE, aCfgList.GetConfigfileAt(k));

        for (uint32 i = 0; i < libList.Size(); i++)
        {
            OsclSharedLibrary* lib = OSCL_NEW(OsclSharedLibrary, ());
            if (lib->LoadLib(libList.GetLibraryPathAt(i)) == OsclLibSuccess)
            {
                OsclAny* interfacePtr = NULL;
                OsclLibStatus result = lib->QueryInterface(PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_POPULATOR_INTERFACE, (OsclAny*&)interfacePtr);
                if (result == OsclLibSuccess && interfacePtr != NULL)
                {
                    struct PVProtocolEngineNodeSharedLibInfo *libInfo =
                                    (struct PVProtocolEngineNodeSharedLibInfo *)oscl_malloc(sizeof(struct PVProtocolEngineNodeSharedLibInfo));
                    if (NULL != libInfo)
            {
                        libInfo->iLib = lib;

                        PVMFProtocolEngineNodeRegistryPopulatorInterface* nodeLibIntPtr = OSCL_DYNAMIC_CAST(PVMFProtocolEngineNodeRegistryPopulatorInterface*, interfacePtr);
                        libInfo->iNodeLibIfacePtr = nodeLibIntPtr;
                        nodeLibIntPtr->Register(this);

                        // save for depopulation later
                        iNodeLibInfoList.push_front(libInfo);
                        continue;
                    }
                }
            }
            lib->Close();
            OSCL_DELETE(lib);
        }
    }
#endif
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::AddLoadableModules() OUT"));
}

void PVMFProtocolEngineNodeRegistry::RemoveLoadableModules()
{
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::RemoveLoadableModules() IN"));

#ifdef USE_LOADABLE_MODULES
    // remove all dynamic nodes now
    // unregister node one by one
    while (!iNodeLibInfoList.empty())
    {
        struct PVProtocolEngineNodeSharedLibInfo *libInfo = iNodeLibInfoList.front();
        iNodeLibInfoList.erase(iNodeLibInfoList.begin());

        OsclSharedLibrary* lib = libInfo->iLib;
        PVMFProtocolEngineNodeRegistryPopulatorInterface* nodeIntPtr = libInfo->iNodeLibIfacePtr;
        oscl_free(libInfo);

        nodeIntPtr->Unregister(this);

        lib->Close();
        OSCL_DELETE(lib);
    }
#endif
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::RemoveLoadableModules() OUT"));
}

void PVMFProtocolEngineNodeRegistry::RegisterProtocolEngineContainer(PVMFProtocolEngineContainerInfo *aInfo)
{
    LOGINFO((0, "PVMFProtocolEngineNodeRegistry::RegisterProtocolEngineContainer() IN"));
    if (aInfo) iTypeVec.push_back(*aInfo);
}








