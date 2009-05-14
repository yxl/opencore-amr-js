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
#include "pv_player_node_registry.h"

#include "pvmi_datastreamsyncinterface_ref_factory.h"

#ifdef USE_LOADABLE_MODULES
#include "oscl_shared_library.h"
#include "oscl_library_list.h"
#include "oscl_configfile_list.h"
#include "osclconfig_lib.h"
#include "oscl_shared_lib_interface.h"

#include "pvmf_node_shared_lib_interface.h"

#endif //USE_LOADABLE_MODULES

#include "pvmf_omx_videodec_factory.h"

#if BUILD_VIDEO_DEC_NODE
#include "pvmf_videodec_factory.h"
#endif


void PVPlayerRegistryPopulator::Populate(PVPlayerNodeRegistry& aNode, PVPlayerRecognizerRegistry& aRec)
{

#ifdef USE_LOADABLE_MODULES
    //add loadable modules

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
        aNode.AddLoadableModules(aCfgList.GetConfigfileAt(k));
        aRec.AddLoadableModules(aCfgList.GetConfigfileAt(k));
    }
#endif

    //add static modules
    PVPlayerRegistryPopulator pop;
    pop.RegisterAllNodes(&aNode, aNode.iStaticPopulatorContext);
    pop.RegisterAllRecognizers(&aRec, aRec.iStaticPopulatorContext);

}

void PVPlayerRegistryPopulator::Depopulate(PVPlayerNodeRegistry& aNode, PVPlayerRecognizerRegistry& aRec)
{
    aNode.RemoveLoadableModules();
    aRec.RemoveLoadableModules();

    PVPlayerRegistryPopulator pop;
    pop.UnregisterAllNodes(&aNode, aNode.iStaticPopulatorContext);
    pop.UnregisterAllRecognizers(&aRec, aRec.iStaticPopulatorContext);
}


PVPlayerNodeRegistry::PVPlayerNodeRegistry()
{
    iType.reserve(20);
    iLogger = PVLogger::GetLoggerObject("pvplayerengine.playernoderegistry");
}


PVPlayerNodeRegistry::~PVPlayerNodeRegistry()
{
    iType.clear();
    iLogger = NULL;
}

void PVPlayerNodeRegistry::AddLoadableModules(const OSCL_String& aConfigFilePath)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::AddLoadableModules() IN"));
#ifdef USE_LOADABLE_MODULES

    OsclLibraryList libList;
    libList.Populate(PV_NODE_REGISTRY_POPULATOR_INTERFACE, aConfigFilePath);

    for (unsigned int i = 0; i < libList.Size(); i++)
    {
        OsclSharedLibrary* lib = OSCL_NEW(OsclSharedLibrary, ());
        if (lib->LoadLib(libList.GetLibraryPathAt(i)) == OsclLibSuccess)
        {
            OsclAny* interfacePtr = NULL;
            OsclLibStatus result = lib->QueryInterface(PV_NODE_REGISTRY_POPULATOR_INTERFACE, (OsclAny*&)interfacePtr);
            if (result == OsclLibSuccess && interfacePtr != NULL)
            {
                struct PVPlayerEngineNodeSharedLibInfo *libInfo = (struct PVPlayerEngineNodeSharedLibInfo *)oscl_malloc(sizeof(struct PVPlayerEngineNodeSharedLibInfo));
                if (NULL != libInfo)
                {
                    libInfo->iLib = lib;

                    NodeRegistryPopulatorInterface* nodeIntPtr = OSCL_DYNAMIC_CAST(NodeRegistryPopulatorInterface*, interfacePtr);
                    libInfo->iNodeLibIfacePtr = nodeIntPtr;
                    nodeIntPtr->RegisterAllNodes(this,  libInfo->iContext);

                    // save for depopulation later
                    iNodeLibInfoList.push_front(libInfo);
                    continue;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerNodeRegistry::AddLoadableModules() QueryInterface() of PV_NODE_POPULATOR_INTERFACE for library %s failed.", libList.GetLibraryPathAt(i).get_cstr()));

            }
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerNodeRegistry::AddLoadableModules() LoadLib() of library %s failed.", libList.GetLibraryPathAt(i).get_cstr()));
        }
        lib->Close();
        OSCL_DELETE(lib);
    }
#else
    OSCL_UNUSED_ARG(aConfigFilePath);
#endif
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::AddLoadableModules() OUT"));
}

void PVPlayerNodeRegistry::RemoveLoadableModules()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::RemoveLoadableModules() IN"));
#ifdef USE_LOADABLE_MODULES
    // remove all dynamic nodes now
    // unregister node one by one
    while (!iNodeLibInfoList.empty())
    {
        struct PVPlayerEngineNodeSharedLibInfo *libInfo = iNodeLibInfoList.front();
        iNodeLibInfoList.erase(iNodeLibInfoList.begin());

        OsclSharedLibrary* lib = libInfo->iLib;
        NodeRegistryPopulatorInterface* nodeIntPtr = libInfo->iNodeLibIfacePtr;
        OsclAny* context = libInfo->iContext;
        oscl_free(libInfo);

        nodeIntPtr->UnregisterAllNodes(this, context);

        lib->Close();
        OSCL_DELETE(lib);
    }
#endif
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::RemoveLoadableModules() OUT"));
}


PVMFStatus PVPlayerNodeRegistry::QueryRegistry(PVMFFormatType& aInputType, PVMFFormatType& aOutputType, Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::QueryRegistry() IN"));
    uint32 SearchCount = 0;
    bool matchfound = false;

    // Find all nodes that support the specified input and ouput format pair
    while (SearchCount < iType.size())
    {
        uint32 inputsearchcount = 0, outputsearchcount = 0;
        bool iInputFoundFlag = false, iOutputFoundFlag = false;

        while (inputsearchcount < iType[SearchCount].iInputTypes.size())
        {
            // Check if the input format matches
            if (iType[SearchCount].iInputTypes[inputsearchcount] == aInputType)
            {
                // Set the the input flag to true since we found the match in the search
                iInputFoundFlag = true;
                break;
            }
            inputsearchcount++;
        }

        //Check the flag of input format if it is true check for the output format, if not return failure
        if (iInputFoundFlag)
        {
            while (outputsearchcount < iType[SearchCount].iOutputType.size())
            {
                if (iType[SearchCount].iOutputType[outputsearchcount] == aOutputType)
                {
                    //set the the output flag to true since we found the match in the search
                    iOutputFoundFlag = true;
                    break;
                }

                outputsearchcount++;
            }

            if (iOutputFoundFlag)
            {
                // There's a match so add this node UUID to the list.
                aUuids.push_back(iType[SearchCount].iNodeUUID);
                matchfound = true;
            }
        }

        SearchCount++;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::QueryRegistry() OUT"));
    if (matchfound)
    {
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}


PVMFNodeInterface* PVPlayerNodeRegistry::CreateNode(PVUuid& aUuid)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::CreateNode() IN"));
    bool iFoundFlag = false;
    uint32 NodeSearchCount = 0;

    while (NodeSearchCount < iType.size())
    {
        //Search if the UUID's will match
        if (iType[NodeSearchCount].iNodeUUID == aUuid)
        {
            //Since the UUID's match set the flag to true
            iFoundFlag = true;
            break;
        }

        NodeSearchCount++;

    }

    if (iFoundFlag)
    {
        OsclActiveObject::OsclActivePriority priority = OsclActiveObject::EPriorityNominal;
        PVPlayerNodeInfo* nodeInfo = &iType[NodeSearchCount];
        PVMFNodeInterface* nodeInterface = NULL;

#if VIDEO_DEC_NODE_LOW_PRIORITY
        //Call the appropriate Node creation function & return Node pointer
        if (KPVMFOMXVideoDecNodeUuid == aUuid)
        {
            priority = OsclActiveObject::EPriorityLow;
        }
#endif
        if (NULL != nodeInfo->iNodeCreateFunc)
        {
            nodeInterface = (*(iType[NodeSearchCount].iNodeCreateFunc))(priority);
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::CreateNode() OUT"));
        return nodeInterface;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::CreateNode() OUT"));
        return NULL;
    }


}

bool PVPlayerNodeRegistry::ReleaseNode(PVUuid& aUuid, PVMFNodeInterface* aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::ReleaseNode() IN"));
    bool iFoundFlag = false;
    uint32 NodeSearchCount = 0;

    while (NodeSearchCount < iType.size())
    {
        //Search if the UUID's will match
        if (iType[NodeSearchCount].iNodeUUID == aUuid)
        {
            //Since the UUID's match set the flag to true
            iFoundFlag = true;
            break;
        }

        NodeSearchCount++;

    }

    if (iFoundFlag)
    {
        //Call the appropriate Node release function
        bool del_stat = (*(iType[NodeSearchCount].iNodeReleaseFunc))(aNode);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::ReleaseNode() OUT"));
        return del_stat;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerNodeRegistry::ReleaseNode() OUT"));
    return false;
}


PVPlayerRecognizerRegistry::PVPlayerRecognizerRegistry()
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVPlayerRecognizerRegistry")
{
    AddToScheduler();

    iRecSessionId = 0;
    iRecognizerResult.reserve(4);
    iFileDataStreamFactory = NULL;
    iDataStreamFactory = NULL;
    iSourceFormatType = PVMF_MIME_FORMAT_UNKNOWN;
    iObserver = NULL;
    iCmdContext = NULL;
    iCancelQuery = false;
    iCancelCmdContext = NULL;

    if (PVMFRecognizerRegistry::Init() != PVMFSuccess)
    {
        OSCL_ASSERT(false);
        return;
    }

    iLogger = PVLogger::GetLoggerObject("pvplayerengine.playerrecognizerregistry");
}


PVPlayerRecognizerRegistry::~PVPlayerRecognizerRegistry()
{
    if (iFileDataStreamFactory)
    {
        OSCL_DELETE(OSCL_STATIC_CAST(PVMIDataStreamSyncInterfaceRefFactory*, iFileDataStreamFactory));
        iFileDataStreamFactory = NULL;
    }

    PVMFRecognizerRegistry::Cleanup();
    iLogger = NULL;
}

void PVPlayerRecognizerRegistry::AddLoadableModules(const OSCL_String& aConfigFilePath)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::AddLoadableModules() IN"));
#ifdef USE_LOADABLE_MODULES
    OsclLibraryList libList;
    libList.Populate(PV_RECOGNIZER_POPULATOR_INTERFACE, aConfigFilePath);

    for (unsigned int i = 0; i < libList.Size(); i++)
    {
        OsclSharedLibrary* lib = OSCL_NEW(OsclSharedLibrary, ());
        if (lib->LoadLib(libList.GetLibraryPathAt(i)) == OsclLibSuccess)
        {
            OsclAny* interfacePtr = NULL;
            OsclLibStatus result = lib->QueryInterface(PV_RECOGNIZER_POPULATOR_INTERFACE, (OsclAny*&)interfacePtr);
            if (result == OsclLibSuccess && interfacePtr != NULL)
            {
                struct PVPlayerEngineRecognizerSharedLibInfo *libInfo = (struct PVPlayerEngineRecognizerSharedLibInfo *)oscl_malloc(sizeof(struct PVPlayerEngineRecognizerSharedLibInfo));
                if (NULL != libInfo)
                {
                    libInfo->iLib = lib;

                    RecognizerPopulatorInterface* recognizerIntPtr = OSCL_DYNAMIC_CAST(RecognizerPopulatorInterface*, interfacePtr);

                    libInfo->iRecognizerLibIfacePtr = recognizerIntPtr;

                    recognizerIntPtr->RegisterAllRecognizers(this, libInfo->iContext);

                    // save for depopulation later
                    iRecognizerLibInfoList.push_front(libInfo);
                    continue;
                }
            }
        }
        lib->Close();
        OSCL_DELETE(lib);
    }
#else
    OSCL_UNUSED_ARG(aConfigFilePath);
#endif
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::AddLoadableModules() OUT"));
}

void PVPlayerRecognizerRegistry::RemoveLoadableModules()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::RemoveLoadableModules() IN"));
#ifdef USE_LOADABLE_MODULES
    // remove all the dynamic plugins now
    // unregister the plugins one by one
    while (!iRecognizerLibInfoList.empty())
    {
        struct PVPlayerEngineRecognizerSharedLibInfo *libInfo = iRecognizerLibInfoList.front();
        iRecognizerLibInfoList.erase(iRecognizerLibInfoList.begin());

        OsclSharedLibrary* lib = libInfo->iLib;
        RecognizerPopulatorInterface* recognizerIntPtr = libInfo->iRecognizerLibIfacePtr;
        OsclAny* context = libInfo->iContext;
        oscl_free(libInfo);

        recognizerIntPtr->UnregisterAllRecognizers(this, context);

        lib->Close();
        OSCL_DELETE(lib);
    }
#endif
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::RemoveLoadableModules() OUT"));
}

PVMFStatus PVPlayerRecognizerRegistry::QueryFormatType(OSCL_wString& aSourceURL, PVPlayerRecognizerRegistryObserver& aObserver, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::QueryFormatType() IN"));
    if (iObserver != NULL)
    {
        // Previous query still ongoing
        return PVMFErrBusy;
    }
    iObserver = &aObserver;
    iCmdContext = aCmdContext;

    // Create a datastream wrapper factory for standard file
    if (iFileDataStreamFactory)
    {
        OSCL_DELETE(OSCL_STATIC_CAST(PVMIDataStreamSyncInterfaceRefFactory*, iFileDataStreamFactory));
        iFileDataStreamFactory = NULL;
    }
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iFileDataStreamFactory = OSCL_STATIC_CAST(PVMFDataStreamFactory*, OSCL_NEW(PVMIDataStreamSyncInterfaceRefFactory, (aSourceURL))));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         return PVMFErrNoMemory;
                        );

    // Open the session with recognizer
    PVMFRecognizerRegistry::OpenSession(iRecSessionId, *this);

    // Request file recognition
    iRecognizerResult.clear();
    iRecognizeCmdId = PVMFRecognizerRegistry::Recognize(iRecSessionId, *iFileDataStreamFactory, NULL, iRecognizerResult, NULL);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::QueryFormatType() OUT"));
    return PVMFSuccess;
}

PVMFStatus PVPlayerRecognizerRegistry::QueryFormatType(PVMFCPMPluginAccessInterfaceFactory* aDataStreamFactory, PVPlayerRecognizerRegistryObserver& aObserver, OsclAny* aCmdContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::QueryFormatType() IN"));
    if (iObserver != NULL)
    {
        // Previous query still ongoing
        return PVMFErrBusy;
    }

    if (aDataStreamFactory == NULL)
    {
        return PVMFErrArgument;
    }

    iObserver = &aObserver;
    iCmdContext = aCmdContext;

    // delete the previous DataStreamFactory created by PVPlayerRecognizerRegistry
    if (iFileDataStreamFactory)
    {
        OSCL_DELETE(OSCL_STATIC_CAST(PVMIDataStreamSyncInterfaceRefFactory*, iFileDataStreamFactory));
        iFileDataStreamFactory = NULL;
    }

    iDataStreamFactory = aDataStreamFactory;

    // Open the session with recognizer
    PVMFRecognizerRegistry::OpenSession(iRecSessionId, *this);

    // Request file recognition
    iRecognizerResult.clear();
    iRecognizeCmdId = PVMFRecognizerRegistry::Recognize(iRecSessionId, *iDataStreamFactory, NULL, iRecognizerResult, NULL);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::QueryFormatType() OUT"));
    return PVMFSuccess;
}


void PVPlayerRecognizerRegistry::CancelQuery(OsclAny* aContext)
{
    if (iObserver == NULL)
    {
        // No pending recognize request
        OSCL_LEAVE(OsclErrInvalidState);
        return;
    }

    iCancelQuery = true;
    iCancelCmdContext = aContext;

    if (!IsBusy())
    {
        // The recognition pending so cancel it
        PVMFRecognizerRegistry::CancelCommand(iRecSessionId, iRecognizeCmdId);
    }
    // Else the recognition already completed so just cancel it in the Run()
}


void PVPlayerRecognizerRegistry::Run()
{
    // Close the session with recognizer
    PVMFRecognizerRegistry::CloseSession(iRecSessionId);

    // Destroy the data stream wrapper factory for standard file
    if (iFileDataStreamFactory)
    {
        OSCL_DELETE(OSCL_STATIC_CAST(PVMIDataStreamSyncInterfaceRefFactory*, iFileDataStreamFactory));
        iFileDataStreamFactory = NULL;
    }

    // Tell the engine the result
    if (iObserver)
    {
        iObserver->RecognizeCompleted(iSourceFormatType, iCmdContext);

        if (iCancelQuery)
        {
            iObserver->RecognizeCompleted(iSourceFormatType, iCancelCmdContext);
            iCancelQuery = false;
            iCancelCmdContext = NULL;
        }
    }
    iObserver = NULL;
    iCmdContext = NULL;
}

void PVPlayerRecognizerRegistry::RecognizerCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::RecognizerCommandCompleted() IN"));
    iSourceFormatType = PVMF_MIME_FORMAT_UNKNOWN;

    if (aResponse.GetCmdId() == iRecognizeCmdId)
    {
        // Recognize() command completed
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::RecognizeCommandCompleted() - Recognize returned Success"));
            Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>::iterator it;
            for (it = iRecognizerResult.begin(); it != iRecognizerResult.end(); it++)
            {
                if (it->iRecognitionConfidence == PVMFRecognizerConfidenceCertain)
                {   //@TODO: validate whether there are more than one claims Certain.
                    iSourceFormatType = it->iRecognizedFormat.get_str();
                    break;
                }
                if (it->iRecognitionConfidence == PVMFRecognizerConfidencePossible)
                {
                    iSourceFormatType = it->iRecognizedFormat.get_str();
                }
            }
        }
        else if (aResponse.GetCmdStatus() == PVMFErrCancelled)
        {
            // If cancelled, need to wait for the cancel command to complete before
            // calling Run
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::RecognizerCommandCompleted() - Recognize returned Cancelled - OUT"));
            return;
        }
    }

    RunIfNotReady();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerRecognizerRegistry::RecognizerCommandCompleted() OUT"));
}





