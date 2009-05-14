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


#ifndef PV_PLAYER_NODE_REGISTRY_H_INCLUDED
#define PV_PLAYER_NODE_REGISTRY_H_INCLUDED

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif

#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
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

#ifndef PVMF_RECOGNIZER_TYPES_H_INCLUDED
#include "pvmf_recognizer_types.h"
#endif

#ifndef PV_PLAYER_NODE_REGISTRY_INTERFACE_H_INCLUDED
#include "pv_player_node_registry_interface.h"
#endif

#ifndef PVMF_RECOGNIZER_REGISTRY_H_INCLUDED
#include "pvmf_recognizer_registry.h"
#endif

#ifndef PV_PLAYER_CONFIG_H_INCLUDED
#include "pv_player_config.h"
#endif

#ifndef PV_LOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef USE_CML2_CONFIG
#ifndef PV_PLAYER_ENGINE_TUNABLES_H_INCLUDED
#include "pv_player_engine_tunables.h"
#endif
#endif

class OsclSharedLibrary;
class NodeRegistryPopulatorInterface;
class RecognizerPopulatorInterface;


// CLASS DECLARATION
/**
 * PVPlayerNodeRegistry maintains a list of nodes available which is queryable.
 * The utility also allows the node specified by PVUuid to be created and returned
 **/
class PVPlayerNodeRegistry : public PVPlayerNodeRegistryInterface
{
    public:
        /**
         * Object Constructor function
         **/
        PVPlayerNodeRegistry();

        void AddLoadableModules(const OSCL_String&);
        void RemoveLoadableModules();

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
        virtual PVMFStatus QueryRegistry(PVMFFormatType& aInputType, PVMFFormatType& aOutputType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);

        /**
         * The CreateNode for PVPlayerNodeRegistry. Used mainly for creating a node.
         *
         * @param aUuid UUID returned by the QueryRegistry
         *
         * @returns a pointer to node
         **/
        virtual PVMFNodeInterface* CreateNode(PVUuid& aUuid);

        /**
         * The ReleaseNode for PVPlayerNodeRegistry. Used for releasing a node.
         *
         * @param aUuid UUID recorded at the time of creation of the node.
         *
         * @param Pointer to the node to be released
         *
         * @returns True or False
         **/
        virtual bool ReleaseNode(PVUuid& aUuid, PVMFNodeInterface *aNode);

        /**
         * The RegisterNode for PVPlayerNodeRegistry. Used for registering nodes through the NodeInfo object.
         *
         * @param aNodeInfo NodeInfo object passed to the regisry class. This contains all nodes that need to be registered.
         *
         **/
        virtual void RegisterNode(const PVPlayerNodeInfo& aNodeInfo)
        {
            iType.push_back(aNodeInfo);
        };

        /**
         * UnregisterNode for PVPlayerNodeRegistry. Used to remove nodes from dynamic registry.
         *
         * @param aNodeInfo NodeInfo object passed to the regisry class. This contains all nodes that need to be unregistered.
          *
         **/
        virtual void UnregisterNode(const PVPlayerNodeInfo& aNodeInfo)
        {
            OSCL_UNUSED_ARG(aNodeInfo);
            // do nothing
        };

        /**
         * Object destructor function
         **/
        virtual ~PVPlayerNodeRegistry();


    private:
        /*
        ** The node list.
        */
        Oscl_Vector<PVPlayerNodeInfo, OsclMemAllocator> iType;

        OsclAny* iStaticPopulatorContext;
        friend class PVPlayerRegistryPopulator;

        /*
        ** Bookkeeping data for dynamically loaded nodes.
        */
        struct PVPlayerEngineNodeSharedLibInfo
        {
            OsclSharedLibrary* iLib;
            NodeRegistryPopulatorInterface* iNodeLibIfacePtr;
            OsclAny* iContext;
        };
        Oscl_Vector<struct PVPlayerEngineNodeSharedLibInfo*, OsclMemAllocator> iNodeLibInfoList;
        PVLogger* iLogger;
};


class PVMFRecognizerPluginFactory;
class PVMFCPMPluginAccessInterfaceFactory;

class PVPlayerRecognizerRegistryObserver
{
    public:
        virtual void RecognizeCompleted(PVMFFormatType aSourceFormatType, OsclAny* aContext) = 0;
        virtual ~PVPlayerRecognizerRegistryObserver() {}
};

/**
 * PVPlayerRecognizerRegistry maintains sets up the recognizer registry so the player engine
 * can determine the file type
 **/
class PVPlayerRecognizerRegistry : public OsclTimerObject,
            public PVMFRecognizerCommmandHandler
            , public PVPlayerRecognizerRegistryInterface
{
    public:
        PVPlayerRecognizerRegistry();
        virtual ~PVPlayerRecognizerRegistry();

        void AddLoadableModules(const OSCL_String&);
        void RemoveLoadableModules();

        /**
         * Determines the format type of the specified source file using the PVMF recognizer
         *
         * @param aSourceURL The source file to determine the format
         * @param aObserver The callback handler to call when recognize operation completes
         * @param aContext Optional opaque data that would be returned in callback
         *
         * @returns Status of query
         **/
        PVMFStatus QueryFormatType(OSCL_wString& aSourceURL, PVPlayerRecognizerRegistryObserver& aObserver, OsclAny* aContext = NULL);

        /**
         * Determines the format type of the specified source file using the PVMF recognizer
         *
         * @param aDataStreamFactory The factory to create source data stream to determine the format
         * @param aObserver The callback handler to call when recognize operation completes
         * @param aContext Optional opaque data that would be returned in callback
         *
         * @returns Status of query
         **/
        PVMFStatus QueryFormatType(PVMFCPMPluginAccessInterfaceFactory* aDataStreamFactory, PVPlayerRecognizerRegistryObserver& aObserver, OsclAny* aContext = NULL);

        /**
         * Cancels any pending format type query
         *
         * @param aContext Optional opaque data that would be returned in callback
         *
         * @returns None
         **/
        void CancelQuery(OsclAny* aContext = NULL);

        /**
         * The RegisterRecognizer for PVPlayerRecognizerRegistryInterface. Used for registering file format recognizer factory plugins.
         *
         * @param aRecognizerPluginFactory PVMFRecognizerPluginFactory object pointer passed to the regisry class.
         *
         **/
        void RegisterRecognizer(PVMFRecognizerPluginFactory* aRecognizerPluginFactory)
        {
            if (PVMFRecognizerRegistry::RegisterPlugin(*aRecognizerPluginFactory) == PVMFSuccess)
            {
                iRecognizerList.push_back(aRecognizerPluginFactory);
            }
        }

        /**
         * The UnregisterRecognizer for PVPlayerRecognizerRegistryInterface. Used for removing file format recognizer factory plugins.
         *
         * @param aRecognizerPluginFactory PVMFRecognizerPluginFactory object pointer passed to the regisry class.
         *
         **/
        void UnregisterRecognizer(PVMFRecognizerPluginFactory* aRecognizerPluginFactory)
        {
            for (uint i = 0; i < iRecognizerList.size(); i++)
            {
                if (iRecognizerList[i] == aRecognizerPluginFactory)
                {
                    iRecognizerList.erase(iRecognizerList.begin() + i);
                    PVMFRecognizerRegistry::RemovePlugin(*aRecognizerPluginFactory);
                }
            }
        }

    private:
        // From OsclTimerObject
        void Run();

        // From PVMFRecognizerCommmandHandler
        void RecognizerCommandCompleted(const PVMFCmdResp& aResponse);

        /*
        ** The Recognizer list
        */
        Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator> iRecognizerList;

        PVMFSessionId iRecSessionId;
        Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator> iRecognizerResult;
        PVMFCPMPluginAccessInterfaceFactory* iFileDataStreamFactory;
        PVMFCPMPluginAccessInterfaceFactory* iDataStreamFactory;

        PVMFFormatType iSourceFormatType;
        PVPlayerRecognizerRegistryObserver* iObserver;
        OsclAny* iCmdContext;
        PVMFCommandId iRecognizeCmdId;
        bool iCancelQuery;
        OsclAny* iCancelCmdContext;

        OsclAny* iStaticPopulatorContext;
        friend class PVPlayerRegistryPopulator;

        /*
        ** Bookkeeping data for dynamically loaded recognizers.
        */
        struct PVPlayerEngineRecognizerSharedLibInfo
        {
            OsclSharedLibrary* iLib;
            RecognizerPopulatorInterface* iRecognizerLibIfacePtr;
            OsclAny* iContext;
        };
        Oscl_Vector<struct PVPlayerEngineRecognizerSharedLibInfo*, OsclMemAllocator> iRecognizerLibInfoList;
        PVLogger* iLogger;
};

/*
** PVPlayerRegistryPopulator is used by Player engine to populate & de-populate
** the registries.
*/
class PVPlayerRegistryPopulator: public NodeRegistryPopulatorInterface
            , public RecognizerPopulatorInterface
{
    public:
        /*
        ** Populate both registries from static and loadable modules
        */
        static void Populate(PVPlayerNodeRegistry&, PVPlayerRecognizerRegistry&);

        /*
        ** Depopulate both registries.
        */
        static void Depopulate(PVPlayerNodeRegistry&, PVPlayerRecognizerRegistry&);

    private:
        /*
        ** These routines are implemented in the project-specific player config.
        ** They would normally populate using statically linked modules.
        */

        //from NodeRegistryPopulatorInterface
        void RegisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny*& aContext) ;
        void UnregisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny* aContext) ;

        //from RecognizerPopulatorInterface
        void RegisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny*& aContext) ;
        void UnregisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny* aContext) ;
};


#endif // PV_PLAYER_NODE_REGISTRY_H_INCLUDED


