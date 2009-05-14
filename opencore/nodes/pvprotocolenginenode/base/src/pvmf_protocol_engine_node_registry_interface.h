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
#ifndef PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_INTERFACE_H_INCLUDED
#define PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_INTERFACE_H_INCLUDED

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif


class PVMFProtocolEngineNode;
class ProtocolContainer;

// CLASS DECLARATION
/**
 * PVMFProtocolEngineContainerInfo is a class which will maintain feature specific plugin info
 **/

class PVMFProtocolEngineContainerInfo
{
    public:
        /**
         * Object Constructor function
         **/
        PVMFProtocolEngineContainerInfo()
        {
            clear();
        }

        /**
         * Copy Constructor function
         **/
        PVMFProtocolEngineContainerInfo(const PVMFProtocolEngineContainerInfo& aInfo)
        {
            iSourceType = aInfo.iSourceType;
            iSourceExtraInfo = aInfo.iSourceExtraInfo;
            iProtocolEngineContainerUUID = aInfo.iProtocolEngineContainerUUID;
            iProtocolEngineContainerCreateFunc = aInfo.iProtocolEngineContainerCreateFunc;
            iProtocolEngineContainerReleaseFunc = aInfo.iProtocolEngineContainerReleaseFunc;
        }

        void clear()
        {
            iSourceExtraInfo = 0;
            iProtocolEngineContainerCreateFunc  = NULL;
            iProtocolEngineContainerReleaseFunc = NULL;

        }

        /**
         * Object destructor function
         **/
        ~PVMFProtocolEngineContainerInfo()
        {
        }

    public:
        PVMFFormatType iSourceType;
        uint32 iSourceExtraInfo; // bit0=1, progressive streaming type
        PVUuid iProtocolEngineContainerUUID;
        ProtocolContainer*(*iProtocolEngineContainerCreateFunc)(PVMFProtocolEngineNode *);
        bool (*iProtocolEngineContainerReleaseFunc)(ProtocolContainer *);
};


// CLASS DECLARATION
/**
 * PVMFProtocolEngineNodeRegistry maintains a list of protocol constainers available which is queryable.
 * The utility also allows the node specified by PVUuid to be created and returned
 **/

class PVMFProtocolEngineNodeRegistryInterface
{
    public:
        /**
         * The CheckPluginAvailability for PVMFProtocolEngineNodeRegistry. Used for releasing a protocol-specific protocol container.
         *
         * @param aSourceFormat source format type passed in from upper layer up to the app.
         *
         * @param aSourceData source opaque data
         *
         * @returns True or False
         **/
        virtual bool CheckPluginAvailability(PVMFFormatType& aSourceFormat, OsclAny* aSourceData = NULL) = 0;

        /**
         * The CreateProtocolEngineContainer for PVMFProtocolEngineNodeRegistry. Used mainly for creating a protocol-specific protocol container.
         *
         * @param aRecreateInfo opaque info for creating ProtocolEngineContainer
         * @param aNode an PVMFProtocolEngineNode instance
         *
         * @returns a pointer to protocol container
         **/
        virtual ProtocolContainer* CreateProtocolEngineContainer(OsclAny* &aPluginInfo, PVMFProtocolEngineNode* aNode) = 0;

        /**
         * The ReleaseProtocolEngineContainer for PVMFProtocolEngineNodeRegistry. Used for releasing a protocol-specific protocol container.
         *
         * @param aUuid UUID recorded at the time of creation of the node.
         *
         * @param Pointer to the protocol-based protocol container to be released
         *
         * @returns True or False
         **/
        virtual bool ReleaseProtocolEngineContainer(ProtocolContainer *aContainer) = 0;

        virtual void RegisterProtocolEngineContainer(PVMFProtocolEngineContainerInfo *aInfo) = 0;

        /**
         * Object destructor function
         **/
        virtual ~PVMFProtocolEngineNodeRegistryInterface()
        {
            ;
        }
};


#endif // PVMF_PROTOCOL_ENGINE_NODE_REGISTRY_INTERFACE_H_INCLUDED


