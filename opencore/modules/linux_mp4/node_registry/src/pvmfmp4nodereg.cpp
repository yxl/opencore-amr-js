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
   registry with the nodes required for mp4 playback.
*/
#ifndef PVMF_NODE_SHARED_LIB_INTERFACE_H_INCLUDED
#include "pvmf_node_shared_lib_interface.h"
#endif

#ifndef PVMF_MP4FFPARSER_FACTORY_H_INCLUDED
#include "pvmf_mp4ffparser_factory.h"
#endif

#ifndef PVMP4FFREC_FACTORY_H_INCLUDED
#include "pvmp4ffrec_factory.h"
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

#ifndef PVMFMP4NODEREG_H_INCLUDED
#include "pvmfmp4nodereg.h"
#endif

#define MP4_LIB_NAME "libopencore_mp4local.so"

#define NODE_REGISTRY_LIB_NAME_MAX_LENGTH 64

typedef PVMFNodeInterface*(* LPFN_NODE_CREATE_FUNC)(int32);

typedef bool (* LPFN_NODE_RELEASE_FUNC)(PVMFNodeInterface *);

// Factory functions
PVMFNodeInterface* Mp4NodesCoreLibraryLoader::CreateMp4ParserNode(int32 aPriority)
{
    OsclSharedLibrary* mp4SharedLibrary = NULL;
    OSCL_StackString<NODE_REGISTRY_LIB_NAME_MAX_LENGTH> libname(MP4_LIB_NAME);

    // Need to load the library for the node
    mp4SharedLibrary = OSCL_NEW(OsclSharedLibrary, (libname));
    OsclLibStatus result = mp4SharedLibrary->LoadLib();
    if (OsclLibSuccess != result)
    {
        return NULL;
    }

    mp4SharedLibrary->AddRef();

    // Query for create function
    OsclAny* interfacePtr = NULL;

    mp4SharedLibrary->QueryInterface(PV_NODE_INTERFACE, (OsclAny*&)interfacePtr);

    NodeSharedLibraryInterface* nodeIntPtr = OSCL_DYNAMIC_CAST(NodeSharedLibraryInterface*, interfacePtr);

    OsclAny* createFuncTemp = nodeIntPtr->QueryNodeInterface(KPVMFMP4FFParserNodeUuid, PV_CREATE_NODE_INTERFACE);

    LPFN_NODE_CREATE_FUNC nodeCreateFunc = OSCL_DYNAMIC_CAST(PVMFNodeInterface * (*)(int32), createFuncTemp);

    if (NULL != nodeCreateFunc)
    {
        PVMFNodeInterface* node = NULL;
        // call the real node factory function
        node = (*(nodeCreateFunc))(aPriority);
        if (NULL == node)
        {
            mp4SharedLibrary->RemoveRef();

            if (OsclLibSuccess == mp4SharedLibrary->Close())
            {
                // Close will unload the library if refcount is 0
                OSCL_DELETE(mp4SharedLibrary);
            }

            return NULL;
        }
        node->SetSharedLibraryPtr(mp4SharedLibrary);
        return node;
    }
    return NULL;
}


bool Mp4NodesCoreLibraryLoader::DeleteMp4ParserNode(PVMFNodeInterface* aNode)
{
    bool bStatus = false;
    OsclSharedLibrary* mp4SharedLibrary = NULL;

    if (NULL == aNode)
    {
        return false;
    }

    // Retrieve shared library pointer
    mp4SharedLibrary = aNode->GetSharedLibraryPtr();

    if (NULL != mp4SharedLibrary)
    {
        // Query for release function
        OsclAny* interfacePtr = NULL;

        mp4SharedLibrary->QueryInterface(PV_NODE_INTERFACE, (OsclAny*&)interfacePtr);

        NodeSharedLibraryInterface* nodeIntPtr = OSCL_DYNAMIC_CAST(NodeSharedLibraryInterface*, interfacePtr);

        OsclAny* releaseFuncTemp = nodeIntPtr->QueryNodeInterface(KPVMFMP4FFParserNodeUuid, PV_RELEASE_NODE_INTERFACE);

        LPFN_NODE_RELEASE_FUNC nodeReleaseFunc = OSCL_DYNAMIC_CAST(bool (*)(PVMFNodeInterface*), releaseFuncTemp);

        if (NULL != nodeReleaseFunc)
        {
            bStatus = (*(nodeReleaseFunc))(aNode);
        }

        mp4SharedLibrary->RemoveRef();

        if (OsclLibSuccess == mp4SharedLibrary->Close())
        {
            // Close will unload the library if refcount is 0
            OSCL_DELETE(mp4SharedLibrary);
        }
    }

    return bStatus;
}

class Mp4NodesRegistryInterface: public OsclSharedLibraryInterface,
            public NodeRegistryPopulatorInterface,
            public RecognizerPopulatorInterface
{
    public:
        Mp4NodesRegistryInterface() {};

        // From NodeRegistryPopulatorInterface
        void RegisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny*& aContext)
        {
            PVPlayerNodeInfo nodeinfo;

            OSCL_StackString<NODE_REGISTRY_LIB_NAME_MAX_LENGTH> libname = MP4_LIB_NAME;

            Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator>* nodeList = new Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator>;

            //For PVMFMP4FFParserNode
            nodeinfo.iInputTypes.clear();
            nodeinfo.iInputTypes.push_back(PVMF_MIME_MPEG4FF);
            nodeinfo.iNodeUUID = KPVMFMP4FFParserNodeUuid;
            nodeinfo.iOutputType.clear();
            nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
            nodeinfo.iNodeCreateFunc = (Mp4NodesCoreLibraryLoader::CreateMp4ParserNode);
            nodeinfo.iNodeReleaseFunc = (Mp4NodesCoreLibraryLoader::DeleteMp4ParserNode);

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
            PVMFRecognizerPluginFactory* tmpfac = NULL;

            Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator>* pluginList =
                new Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator>;

            tmpfac =
                OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVMP4FFRecognizerFactory, ()));
            aRegistry->RegisterRecognizer(tmpfac);

            pluginList->push_back(tmpfac);

            aContext = (OsclAny *)pluginList;
        };


        void UnregisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny* aContext)
        {
            if (NULL != aContext)
            {
                Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator>* pluginList = (Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator>*)aContext;

                while (!pluginList->empty())
                {
                    PVMFRecognizerPluginFactory* tmpfac = pluginList->front();

                    aRegistry->UnregisterRecognizer(tmpfac);

                    pluginList->erase(pluginList->begin());

                    OSCL_DELETE(tmpfac);
                }

                delete pluginList;
            }
        };

        // From OsclSharedLibraryInterface
        OsclAny* SharedLibraryLookup(const OsclUuid& aInterfaceId)
        {
            if (aInterfaceId == PV_NODE_REGISTRY_POPULATOR_INTERFACE)
            {
                return OSCL_STATIC_CAST(NodeRegistryPopulatorInterface*, this);
            }
            else if (aInterfaceId == PV_RECOGNIZER_POPULATOR_INTERFACE)
            {
                return OSCL_STATIC_CAST(RecognizerPopulatorInterface*, this);
            }
            return NULL;
        };
};


extern "C"
{
    OsclSharedLibraryInterface *PVGetInterface(void)
    {
        return OSCL_NEW(Mp4NodesRegistryInterface, ());
    }
    void PVReleaseInterface(OsclSharedLibraryInterface* aInstance)
    {
        OSCL_DELETE(aInstance);
    }
}

