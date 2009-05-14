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
/**
 *  @file pv_player_node_registry.h
 *  @brief PVPlayerNodeRegistry maintains a list of nodes available which is queryable. The utility
 *   also allows the node specified by PVUuid to be created and returned
 *
 */


#ifndef PV_PLAYER_NODE_REGISTRY_INTERFACE_H_INCLUDED
#define PV_PLAYER_NODE_REGISTRY_INTERFACE_H_INCLUDED

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

#ifndef PVMF_RECOGNIZER_PLUGIN_H_INCLUDED
#include "pvmf_recognizer_plugin.h"
#endif

class OsclSharedLibrary;

// CLASS DECLARATION
/**
 * PVPlayerNodeInfo is a class which will maintain node info
 **/
class PVPlayerNodeInfo
{
    public:
        /**
         * Object Constructor function
         **/
        PVPlayerNodeInfo()
        {
            iNodeCreateFunc = NULL;
            iNodeReleaseFunc = NULL;
        }

        /**
         * Copy Constructor function
         **/
        PVPlayerNodeInfo(const PVPlayerNodeInfo& aInfo)
        {
            iNodeUUID = aInfo.iNodeUUID;
            iNodeCreateFunc = aInfo.iNodeCreateFunc;
            iNodeReleaseFunc = aInfo.iNodeReleaseFunc;
            iInputTypes = aInfo.iInputTypes;
            iOutputType = aInfo.iOutputType;
        }

        /**
         * Object destructor function
         **/
        ~PVPlayerNodeInfo()
        {
        }

        PVUuid iNodeUUID;
        PVMFNodeInterface*(*iNodeCreateFunc)(int32);
        bool (*iNodeReleaseFunc)(PVMFNodeInterface *);
        Oscl_Vector<PVMFFormatType, OsclMemAllocator> iInputTypes;
        Oscl_Vector<PVMFFormatType, OsclMemAllocator> iOutputType;
};


class PVPlayerNodeRegistryInterface
{
    public:
        /**
         * The QueryRegistry for PVPlayerNodeRegistry. Used mainly for Seaching of the UUID
         * whether it is available or not & returns Success if it is found else failure.
         *
         * @param aInputType Input Format Type
         *
         * @param aOutputType Output Format Type
         *
         * @param aUuids Reference to the UUID registered
         *
         * @returns Success or Failure
         **/
        virtual PVMFStatus QueryRegistry(PVMFFormatType& aInputType, PVMFFormatType& aOutputType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids) = 0;

        /**
         * The CreateNode for PVPlayerNodeRegistry. Used mainly for creating a node.
         *
         * @param aUuid UUID returned by the QueryRegistry
         *
         * @returns a pointer to node
         **/
        virtual PVMFNodeInterface* CreateNode(PVUuid& aUuid) = 0;

        /**
         * The ReleaseNode for PVPlayerNodeRegistry. Used for releasing a node.
         *
         * @param aUuid UUID recorded at the time of creation of the node.
         *
         * @param Pointer to the node to be released
         *
         * @returns True or False
         **/
        virtual bool ReleaseNode(PVUuid& aUuid, PVMFNodeInterface *aNode) = 0;

        /**
         * The RegisterNode for PVPlayerNodeRegistry. Used for registering nodes through the NodeInfo object.
         *
         * @param aNodeInfo NodeInfo object passed to the regisry class. This contains all nodes that need to be registered.
         *
         **/
        virtual void RegisterNode(const PVPlayerNodeInfo& aNodeInfo) = 0;

        /**
         * The UnregisterNode for PVPlayerNodeRegistry. Used for unregistering nodes through the NodeInfo object.
         *
         * @param aNodeInfo NodeInfo object passed to the regisry class. This contains all nodes that need to be unregistered.
         *
         **/
        virtual void UnregisterNode(const PVPlayerNodeInfo& aNodeInfo) = 0;
};

class PVPlayerRecognizerRegistryInterface
{
    public:
        /**
         * The RegisterRecognizer for PVPlayerRecognizerRegistry. Used for registering plugins through the PVMFRecognizerPluginFactory* object.
         *
         * @param PVMFRecognizerPluginFactory* object passed to the regisry class. This contains all nodes that need to be registered.
         *
         **/
        virtual void RegisterRecognizer(PVMFRecognizerPluginFactory* aRecognizerPluginFactory) = 0;

        /**
         * The UnregisterRecognizer for PVPlayerRecognizerRegistry. Used for unregistering plugins through the PVMFRecognizerPluginFactory* object.
         *
         * @param PVMFRecognizerPluginFactory* object passed to the regisry class. This contains all nodes that need to be unregistered.
         *
         **/
        virtual void UnregisterRecognizer(PVMFRecognizerPluginFactory* aRecognizerPluginFactory) = 0;

};

/*
** NodeRegistryPopulatorInterface is an abstract interface that is used to register and
** unregister nodes in a registry.  A registry uses this interface to allow registry populators
** to add and remove objects in the registry.
*/
#define PV_NODE_REGISTRY_POPULATOR_INTERFACE OsclUuid(0x1d4769f0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x66)

class NodeRegistryPopulatorInterface
{
    public:
        /*
        ** RegisterAllNodes will register one or more nodes in the registry
        ** @param aRegistry: the registry
        ** @param aContext (out): a returned context value.  The registry must
        **    include this value in the UnregisterAllNodes call.
        */
        virtual void RegisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny*& aContext) = 0;

        /*
        ** UnregisterAllNodes will unregister one or more nodes in the registry
        ** @param aRegistry: the registry
        ** @param aContext (in): the context value that was returned in the RegisterAllNodes
        **    call.
        */
        virtual void UnregisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny* aContext) = 0;
};

/*
** RecognizerPopulatorInterface is an abstract interface that is used to register and
** unregister recognizers in a registry.  A registry uses this interface to allow registry populators
** to add and remove objects in the registry.
*/
#define PV_RECOGNIZER_POPULATOR_INTERFACE OsclUuid(0x6d3413a0,0xca0c,0x11dc,0x95,0xff,0x08,0x00,0x20,0x0c,0x9a,0x66)

class RecognizerPopulatorInterface
{
    public:
        /*
        ** RegisterAllRecognizers will register one or more recognizers in the registry
        ** @param aRegistry: the registry
        ** @param aContext (out): a returned context value.  The registry must
        **    include this value in the UnregisterAllRecognizers call.
        */
        virtual void RegisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny*& aContext) = 0;

        /*
        ** UnregisterAllNodes will unregister one or more recognizers in the registry
        ** @param aRegistry: the registry
        ** @param aContext (in): the context value that was returned in the RegisterAllRecognizers
        **    call.
        */
        virtual void UnregisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny* aContext) = 0;
};

#endif // PV_PLAYER_NODE_REGISTRY_INTERFACE_H_INCLUDED


