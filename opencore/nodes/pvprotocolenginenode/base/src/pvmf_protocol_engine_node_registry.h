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
 *  @file pvmf_protocol_engine_node_registry.h
 *  @brief PVMFProtocolEngineNodeRegistry maintains a list of protocol constainers available which is queryable. The utility
 *   also allows the node specified by PVUuid to be created and returned *
 */


#ifndef PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_H_INCLUDED

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_INTERFACE_H_INCLUDED
#include "pvmf_protocol_engine_node_registry_interface.h"
#endif

#ifndef PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_POPULATOR_H_INCLUDED
#include "pvmf_protocol_engine_node_registry_populator_interface.h"
#endif


// CLASS DECLARATION
/**
 * PVMFProtocolEngineNodeRegistry maintains a list of protocol constainers available which is queryable.
 * The utility also allows the node specified by PVUuid to be created and returned
 **/

class ProtocolContainer;
class OsclSharedLibrary;
class PVLogger;
class PVMFProtocolEngineNodeRegistry : public PVMFProtocolEngineNodeRegistryInterface
{
    public:
        /**
         * Object Constructor function
         **/
        PVMFProtocolEngineNodeRegistry();

        /**
         * The CheckPluginAvailability for PVMFProtocolEngineNodeRegistry. Used for releasing a protocol-specific protocol container.
         *
         * @param aSourceFormat source format type passed in from upper layer up to the app.
         *
         * @param aSourceData source opaque data
         *
         * @returns True or False
         **/
        bool CheckPluginAvailability(PVMFFormatType& aSourceFormat, OsclAny* aSourceData = NULL);

        /**
         * The CreateProtocolEngineContainer for PVMFProtocolEngineNodeRegistry. Used mainly for creating a protocol-specific protocol container.
         *
         * @param aRecreateInfo opaque info for creating ProtocolEngineContainer
         * @param aNode an PVMFProtocolEngineNode instance
         *
         * @returns a pointer to protocol container
         **/
        ProtocolContainer* CreateProtocolEngineContainer(OsclAny* &aPluginInfo, PVMFProtocolEngineNode* aNode);

        /**
         * The ReleaseProtocolEngineContainer for PVMFProtocolEngineNodeRegistry. Used for releasing a protocol-specific protocol container.
         *
         * @param aUuid UUID recorded at the time of creation of the node.
         *
         * @param Pointer to the protocol-based protocol container to be released
         *
         * @returns True or False
         **/
        bool ReleaseProtocolEngineContainer(ProtocolContainer *aContainer);

        void RegisterProtocolEngineContainer(PVMFProtocolEngineContainerInfo *aInfo);

        /**
         * Object destructor function
         **/
        ~PVMFProtocolEngineNodeRegistry();

    private:
        bool RecheckPlugin(OsclAny* aNewPluginInfo);

        void AddLoadableModules();
        void RemoveLoadableModules();

    private:
        Oscl_Vector<PVMFProtocolEngineContainerInfo, OsclMemAllocator> iTypeVec;
        uint32 iTypeVecIndex;

        struct PVProtocolEngineNodeSharedLibInfo
        {
            OsclSharedLibrary* iLib;
            PVMFProtocolEngineNodeRegistryPopulatorInterface* iNodeLibIfacePtr;
            OsclAny* iContext;
        };
        Oscl_Vector<struct PVProtocolEngineNodeSharedLibInfo*, OsclMemAllocator> iNodeLibInfoList;
        PVLogger* iLogger;
};


#endif // PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_H_INCLUDED


