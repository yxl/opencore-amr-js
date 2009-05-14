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
/* This file defines a PV NodeSharedLibrary interface that populates the
   registry with the streaming manager node.
*/

#ifndef PVMFRTSPNODEREG_H_INCLUDED
#include "pvmfrtspnodereg.h"
#endif

#ifndef PVMF_NODE_SHARED_LIB_INTERFACE_H_INCLUDED
#include "pvmf_node_shared_lib_interface.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_RECOGNIZER_REGISTRY_H_INCLUDED
#include "pvmf_recognizer_registry.h"
#endif

#ifndef OSCLCONFIG_H_INCLUDED
#include "osclconfig.h"
#endif

#ifndef OSCL_SHARED_LIBRARY_H_INCLUDED
#include "oscl_shared_library.h"
#endif

#define NODE_REGISTRY_LIB_NAME_MAX_LENGTH 64

#define RTSP_LIB_NAME "libopencore_rtsp.so"

typedef PVMFNodeInterface*(* LPFN_NODE_CREATE_FUNC)(int32);

typedef bool (* LPFN_NODE_RELEASE_FUNC)(PVMFNodeInterface *);

// Factory functions
PVMFNodeInterface* StreamingNodesCoreLibraryLoader::CreateStreamingManagerNode(int32 aPriority)
{
    OsclSharedLibrary* streamingSharedLibrary = NULL;
    OSCL_StackString<NODE_REGISTRY_LIB_NAME_MAX_LENGTH> libname(RTSP_LIB_NAME);

    // Need to load the library for the node
    streamingSharedLibrary = OSCL_NEW(OsclSharedLibrary, (libname));
    OsclLibStatus result = streamingSharedLibrary->LoadLib();
    if (OsclLibSuccess != result)
    {
        return NULL;
    }

    streamingSharedLibrary->AddRef();

    // Query for create function
    OsclAny* interfacePtr = NULL;

    streamingSharedLibrary->QueryInterface(PV_NODE_INTERFACE, (OsclAny*&)interfacePtr);

    NodeSharedLibraryInterface* nodeIntPtr = OSCL_DYNAMIC_CAST(NodeSharedLibraryInterface*, interfacePtr);

    OsclAny* createFuncTemp = nodeIntPtr->QueryNodeInterface(KPVMFRTSPStreamingModuleUuid, PV_CREATE_NODE_INTERFACE);

    LPFN_NODE_CREATE_FUNC nodeCreateFunc = OSCL_DYNAMIC_CAST(PVMFNodeInterface * (*)(int32), createFuncTemp);

    if (NULL != nodeCreateFunc)
    {
        PVMFNodeInterface* node = NULL;
        // call the real node factory function
        node = (*(nodeCreateFunc))(aPriority);
        if (NULL == node)
        {
            streamingSharedLibrary->RemoveRef();

            if (OsclLibSuccess == streamingSharedLibrary->Close())
            {
                // Close will unload the library if refcount is 0
                OSCL_DELETE(streamingSharedLibrary);
            }

            return NULL;
        }
        node->SetSharedLibraryPtr(streamingSharedLibrary);
        return node;
    }
    return NULL;
}

bool StreamingNodesCoreLibraryLoader::DeleteStreamingManagerNode(PVMFNodeInterface* aNode)
{
    bool bStatus = false;
    OsclSharedLibrary* streamingSharedLibrary = NULL;

    if (NULL == aNode)
    {
        return false;
    }

    // Retrieve shared library pointer
    streamingSharedLibrary = aNode->GetSharedLibraryPtr();

    if (NULL != streamingSharedLibrary)
    {
        // Query fro release function
        OsclAny* interfacePtr = NULL;

        streamingSharedLibrary->QueryInterface(PV_NODE_INTERFACE, (OsclAny*&)interfacePtr);

        NodeSharedLibraryInterface* nodeIntPtr = OSCL_DYNAMIC_CAST(NodeSharedLibraryInterface*, interfacePtr);

        OsclAny* releaseFuncTemp = nodeIntPtr->QueryNodeInterface(KPVMFRTSPStreamingModuleUuid, PV_RELEASE_NODE_INTERFACE);

        LPFN_NODE_RELEASE_FUNC nodeReleaseFunc = OSCL_DYNAMIC_CAST(bool (*)(PVMFNodeInterface*), releaseFuncTemp);

        if (NULL != nodeReleaseFunc)
        {
            bStatus = (*(nodeReleaseFunc))(aNode);
        }

        streamingSharedLibrary->RemoveRef();

        if (OsclLibSuccess == streamingSharedLibrary->Close())
        {
            // Close will unload the library if refcount is 0
            OSCL_DELETE(streamingSharedLibrary);
        }
    }

    return bStatus;
}



class StreamingNodesRegistryInterface: public OsclSharedLibraryInterface,
            public NodeRegistryPopulatorInterface,
            public RecognizerPopulatorInterface
{
    public:
        StreamingNodesRegistryInterface() {};

        // From NodeRegistryPopulatorInterface
        void RegisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny*& aContext)
        {
            PVPlayerNodeInfo nodeinfo;

            OSCL_StackString<NODE_REGISTRY_LIB_NAME_MAX_LENGTH> libname(RTSP_LIB_NAME);

            Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator>* nodeList = new Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator>;

            //For PVMFStreamingManagerNode
            nodeinfo.iInputTypes.clear();
            nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_RTSP_URL);
            nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_SDP_FILE);
            nodeinfo.iNodeUUID = KPVMFRTSPStreamingModuleUuid;
            nodeinfo.iOutputType.clear();
            nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
            nodeinfo.iNodeCreateFunc = (StreamingNodesCoreLibraryLoader::CreateStreamingManagerNode);
            nodeinfo.iNodeReleaseFunc = (StreamingNodesCoreLibraryLoader::DeleteStreamingManagerNode);

            aRegistry->RegisterNode(nodeinfo);

            nodeList->push_back(nodeinfo);

            aContext = (OsclAny *)nodeList;
        };

        // From NodeRegistryPopulatorInterface
        void UnregisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny* aContext)
        {
            if (NULL != aContext)
            {
                Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator>* nodeList = (Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator> *)aContext;

                while (!nodeList->empty())
                {
                    PVPlayerNodeInfo tmpnode = nodeList->front();
                    aRegistry->UnregisterNode(tmpnode);
                    nodeList->erase(nodeList->begin());
                }

                delete nodeList;
            }
        };


        // From RecognizerPopulatorInterface
        void RegisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny*& aContext)
        {
            OSCL_UNUSED_ARG(aRegistry);
            OSCL_UNUSED_ARG(aContext);
        };


        void UnregisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny* aContext)
        {
            OSCL_UNUSED_ARG(aRegistry);
            OSCL_UNUSED_ARG(aContext);
        };

        // From OsclSharedLibraryInterface
        OsclAny* SharedLibraryLookup(const OsclUuid& aInterfaceId)
        {
            if (aInterfaceId == PV_NODE_REGISTRY_POPULATOR_INTERFACE)
            {
                return OSCL_STATIC_CAST(NodeRegistryPopulatorInterface*, this);
            }
            return NULL;
        };
};


extern "C"
{
    OsclSharedLibraryInterface* PVGetInterface(void)
    {
        return OSCL_NEW(StreamingNodesRegistryInterface, ());
    }
    void PVReleaseInterface(OsclSharedLibraryInterface* aInstance)
    {
        OSCL_DELETE(aInstance);
    }
}

