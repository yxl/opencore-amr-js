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


#ifndef PVMF_SM_FSP_REGISTRY_INTERFACE_H_INCLUDED
#define PVMF_SM_FSP_REGISTRY_INTERFACE_H_INCLUDED

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_SM_FSP_BASE_IMPL_H
#include "pvmf_sm_fsp_base_impl.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif


#define PVMF_MIME_DATA_SOURCE_RTSP_TUNNELLING		"X-PVMF-DATA-SRC-RTSP-TUNNELLING"

// FORWARD DECLARATION
class OsclSharedLibrary;


// CLASS DECLARATION
/**
 * PVMFFSPInfo is a class which will maintain feature specific plugin info
 **/
class PVMFSMFSPInfo
{
    public:
        /**
         * Object Constructor function
         **/
        PVMFSMFSPInfo()
        {
#ifdef USE_LOADABLE_MODULES
            iSharedLibrary    = NULL;
            iSMFSPCreateFunc  = NULL;
            iSMFSPReleaseFunc = NULL;
#endif
        }

        /**
         * Copy Constructor function
         **/
        PVMFSMFSPInfo(const PVMFSMFSPInfo& aInfo)
        {
            iSMFSPUUID = aInfo.iSMFSPUUID;
            iSMFSPCreateFunc = aInfo.iSMFSPCreateFunc;
            iSMFSPReleaseFunc = aInfo.iSMFSPReleaseFunc;
            iSourceFormatTypes = aInfo.iSourceFormatTypes;
            iSharedLibrary = aInfo.iSharedLibrary;
        }

        /**
         * Object destructor function
         **/
        ~PVMFSMFSPInfo()
        {
        }

        PVUuid iSMFSPUUID;
        PVMFSMFSPBaseNode*(*iSMFSPCreateFunc)(int32);
        bool (*iSMFSPReleaseFunc)(PVMFSMFSPBaseNode *);
        Oscl_Vector<PVMFFormatType, OsclMemAllocator> iSourceFormatTypes;
        OsclSharedLibrary* iSharedLibrary;
};


class PVMFFSPRegistryInterface
{
    public:
        /**
         * The QueryRegistry for PVMFFSPRegistry. Used mainly for Seaching of the UUID
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
        virtual PVMFStatus QueryRegistry(PVMFFormatType& aInputType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids) = 0;

        /**
         * The CreateNode for PVMFFSPRegistry. Used mainly for creating a FSP.
         *
         * @param aUuid UUID returned by the QueryRegistry
         *
         * @returns a pointer to node
         **/
        virtual PVMFSMFSPBaseNode* CreateSMFSP(PVUuid& aUuid) = 0;

        /**
         * The ReleaseNode for PVMFFSPRegistry. Used for releasing a node.
         *
         * @param aUuid UUID recorded at the time of creation of the node.
         *
         * @param Pointer to the node to be released
         *
         * @returns True or False
         **/
        virtual bool ReleaseSMFSP(PVUuid& aUuid, PVMFSMFSPBaseNode *aSMFSP) = 0;

        /**
         * The RegisterNode for PVMFFSPRegistry. Used for registering nodes through the SMFSPInfo object.
         *
         * @param aSMFSPInfo SMFSPInfo object passed to the regisry class. This contains all fsps that need to be registered.
         *
         **/
        virtual void RegisterSMFSP(const PVMFSMFSPInfo& aSMFSPInfo) = 0;

#ifdef USE_LOADABLE_MODULES
        /**
         * The UnregisterFSP for PVMFFSPRegistry. Used for unregistering nodes through the FSPInfo object.
         *
         * @param aSMFSPInfo SMFSPInfo object passed to the regisry class. This contains all fsps that need to be unregistered.
         *
         **/
        virtual void UnregisterSMFSP(const PVMFSMFSPInfo& aSMFSPInfo) = 0;
#endif
};

#endif // PVMF_SM_FSP_REGISTRY_INTERFACE_H_INCLUDED


