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

#ifndef OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_BASIC_ERRORINFOMESSAGE_H_INCLUDED
#include "pvmf_basic_errorinfomessage.h"
#endif
#ifndef PVMF_ERRORINFOMESSAGE_EXTENSION_H_INCLUDED
#include "pvmf_errorinfomessage_extension.h"
#endif
#ifndef PVMF_CPMPLUGIN_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_authorization_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_H_INCLUDED
#include "pvmf_cpmplugin_authentication_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_access_interface_factory.h"
#endif
#ifndef CPM_H_INCLUDED
#include "cpm.h"
#endif
#ifndef CPM_INTERNAL_H_INCLUDED
#include "cpm_internal.h"
#endif
#ifndef CPM_PLUGIN_REGISTRY_H
#include "cpm_plugin_registry.h"
#endif
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif
// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

#include "oscl_registry_access_client.h"

static void _AddPluginInstance(CPMPluginRegistry* aRegistry, PVMFCPMPluginFactory* aFactory, const OSCL_String& aMimestring)
{
    if (aRegistry
            && aFactory)
    {
        //Create an instance of the plugin.
        PVMFCPMPluginInterface* plugin = aFactory->CreateCPMPlugin();
        if (plugin)
        {
            //Package the plugin with its user authentication data.
            CPMPluginContainer container(*plugin, NULL);
            //Add the plugin to the registry with its mime string.
            aRegistry->addPluginToRegistry((OSCL_String&)aMimestring, container);
        }
    }
}

static void _RemovePluginInstance(CPMPluginRegistry* aRegistry, PVMFCPMPluginFactory* aFactory, const OSCL_String& aMimestring)
{
    if (aRegistry)
    {
        if (aFactory)
        {
            CPMPluginContainer* container = aRegistry->lookupPlugin((OSCL_String&)aMimestring);
            if (container)
                aFactory->DestroyCPMPlugin(&container->PlugIn());
        }
        aRegistry->removePluginFromRegistry((OSCL_String&)aMimestring);
    }
}

#ifdef USE_LOADABLE_MODULES
#include "oscl_shared_library.h"
#include "osclconfig_lib.h"

static void _AddLoadablePlugins(CPMPluginRegistry* pRegistry)
{
    if (!pRegistry)
        return;

    //Create a list of all libs that implement CPM registry populator
    pRegistry->AccessSharedLibraryList() = OSCL_NEW(OsclSharedLibraryList, ());
    OSCL_HeapString<OsclMemAllocator> configPath = PV_DYNAMIC_LOADING_CONFIG_FILE_PATH;
    pRegistry->AccessSharedLibraryList()->Populate(configPath, PVMF_CPM_PLUGIN_REGISTRY_POPULATOR_UUID);

    //For each lib, add its factory to the registry.
    for (uint32 i = 0;i < pRegistry->AccessSharedLibraryList()->Size();i++)
    {
        OsclAny* temp = NULL;
        pRegistry->AccessSharedLibraryList()->QueryInterfaceAt(i, temp);
        PVMFCPMPluginRegistryPopulator* pop = (PVMFCPMPluginRegistryPopulator*) temp;
        if (pop)
        {
            OSCL_HeapString<OsclMemAllocator> mimestring;
            PVMFCPMPluginFactory* fac = pop->GetFactoryAndMimeString(mimestring);
            if (fac)
                _AddPluginInstance(pRegistry, fac, mimestring);
        }
    }
}
static void _RemoveLoadablePlugins(CPMPluginRegistry* aRegistry)
{
    if (!aRegistry)
        return;

    if (aRegistry->AccessSharedLibraryList())
    {
        //Loop through loaded modules & remove plugin from list.
        for (uint32 i = 0;i < aRegistry->AccessSharedLibraryList()->Size();i++)
        {
            OsclAny* temp = NULL;
            aRegistry->AccessSharedLibraryList()->QueryInterfaceAt(i, temp);
            PVMFCPMPluginRegistryPopulator* pop = (PVMFCPMPluginRegistryPopulator*) temp;
            if (pop)
            {
                OSCL_HeapString<OsclMemAllocator> mimestring;
                PVMFCPMPluginFactory* fac = pop->GetFactoryAndMimeString(mimestring);
                if (fac)
                {
                    _RemovePluginInstance(aRegistry, fac, mimestring);
                    pop->ReleaseFactory();
                }
            }
        }

        //Delete loaded module list
        OSCL_DELETE(aRegistry->AccessSharedLibraryList());
        aRegistry->AccessSharedLibraryList() = NULL;
    }
}
#endif //USE_LOADABLE_MODULES

static CPMPluginRegistry* PopulateCPMPluginRegistry()
{
    //Create registry
    CPMPluginRegistry* pRegistry = CPMPluginRegistryFactory::CreateCPMPluginRegistry();
    if (pRegistry)
    {
        //Add plugins from global component registry.
        OsclRegistryAccessClient cli;
        if (cli.Connect() == OsclErrNone)
        {
            //Get all the current CPM plugin factory functions.
            Oscl_Vector<OsclRegistryAccessElement, OsclMemAllocator> factories;
            OSCL_HeapString<OsclMemAllocator> id("X-CPM-PLUGIN");//PVMF_MIME_CPM_PLUGIN
            cli.GetFactories(id, factories);

            //Add each plugin
            for (uint32 i = 0;i < factories.size();i++)
            {
                if (factories[i].iFactory)
                {
                    _AddPluginInstance(pRegistry, (PVMFCPMPluginFactory*)factories[i].iFactory, factories[i].iMimeString);
                }
            }
            cli.Close();
        }
#ifdef USE_LOADABLE_MODULES
        //Add plugins from loadable modules.
        _AddLoadablePlugins(pRegistry);
#endif
    }
    return pRegistry;
}

static void DePopulateCPMPluginRegistry(CPMPluginRegistry* aRegistry)
{
    if (aRegistry)
    {
#ifdef USE_LOADABLE_MODULES
        //Remove dynamically loaded plugins
        _RemoveLoadablePlugins(aRegistry);
#endif
        //Remove plugins created by the global component registry.
        OsclRegistryAccessClient cli;
        if (cli.Connect() == OsclErrNone)
        {
            //Get all the current CPM plugin factory functions.
            Oscl_Vector<OsclRegistryAccessElement, OsclMemAllocator> factories;
            OSCL_HeapString<OsclMemAllocator> id("X-CPM-PLUGIN");//PVMF_MIME_CPM_PLUGIN
            cli.GetFactories(id, factories);

            for (uint32 i = 0;i < factories.size();i++)
            {
                if (factories[i].iFactory)
                {
                    _RemovePluginInstance(aRegistry, (PVMFCPMPluginFactory*)factories[i].iFactory, factories[i].iMimeString);
                }
            }
            cli.Close();
        }
        //Destroy the plugin registry
        CPMPluginRegistryFactory::DestroyCPMPluginRegistry(aRegistry);
    }
}

OSCL_EXPORT_REF
PVMFCPM* PVMFCPMFactory::CreateContentPolicyManager(PVMFCPMStatusObserver& aObserver)
{
    PVMFCPM* cpm = NULL;
    int32 err;
    OSCL_TRY(err,
             /*
              * Create Content Policy Manager
              */
             cpm = OSCL_NEW(PVMFCPMImpl, (aObserver));
            );
    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
    return (cpm);
}

OSCL_EXPORT_REF void PVMFCPMFactory::DestroyContentPolicyManager(PVMFCPM* aCPM)
{
    OSCL_DELETE(aCPM);
}

OSCL_EXPORT_REF PVMFCPMImpl::PVMFCPMImpl(PVMFCPMStatusObserver& aObserver,
        int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFCPMImpl")
        , iObserver(aObserver)
{
    iPluginRegistry = NULL;
    iLogger = NULL;
    iNumQueryAuthenticationInterfacePending = 0;
    iNumQueryAuthenticationInterfaceComplete = 0;
    iNumRegisteredPlugInInitPending = 0;
    iNumRegisteredPlugInInitComplete = 0;
    iNumRegisteredPlugInResetPending = 0;
    iNumRegisteredPlugInResetComplete = 0;
    iNumQueryMetaDataExtensionInterfacePending = 0;
    iNumQueryMetaDataExtensionInterfaceComplete = 0;
    iLicenseInterface = NULL;
    iNumQueryCapConfigExtensionInterfacePending = 0;
    iNumQueryCapConfigExtensionInterfaceComplete = 0;

    iGetMetaDataKeysFromPlugInsDone = false;
    iGetMetaDataKeysInProgress = false;

    iExtensionRefCount = 0;
    iGetLicenseCmdId = 0;

    int32 err = OsclErrNone;
    OSCL_TRY(err,
             /*
              * Create the input command queue.  Use a reserve to avoid lots of
              * dynamic memory allocation
              */
             iInputCommands.Construct(PVMF_CPM_COMMAND_ID_START,
                                      PVMF_CPM_INTERNAL_CMDQ_SIZE);

             /*
              * Create the "current command" queue.  It will only contain one
              * command at a time, so use a reserve of 1
              */
             iCurrentCommand.Construct(0, 1);
             /*
              * Create the "cancel command" queue.  It will only contain one
              * command at a time, so use a reserve of 1.
              */
             iCancelCommand.Construct(0, 1);

            );
    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
}

PVMFCPMImpl::~PVMFCPMImpl()
{
    /*
     * Cleanup commands
     * The command queues are self-deleting, but we want to
     * notify the observer of unprocessed commands.
     */
    while (!iCancelCommand.empty())
    {
        CommandComplete(iCancelCommand, iCancelCommand.front(), PVMFFailure);
    }
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
    }
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }

    //Disconnect from all plugins in case it wasn't done in Reset.
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
    {
        if (it->iConnected)
            it->iPlugInInterface->Disconnect(it->iPlugInSessionID);
        it->iConnected = false;
    }

    /* Clear all vectors */
    iPlugInParamsVec.clear();
    iActivePlugInParamsVec.clear();
    iContentUsageContextVec.clear();
    iListofActiveSessions.clear();
}

OSCL_EXPORT_REF void PVMFCPMImpl::ThreadLogon()
{
    iLogger = PVLogger::GetLoggerObject("PVMFCPMImpl");

    AddToScheduler();

    //Create the plugin registry and leave in case there
    // are no plugins.  This is done here in order to allow the
    // source node to avoid going through a series of unnecessary
    // async commands.

    /*
     * Create plugin params for all registered plugins. This container class
     * holds all the required info about the plugin, thereby obviating the need
     * to query the registry all the time for info
     */
    iNumRegisteredPlugInInitPending = 0;
    iNumRegisteredPlugInInitComplete = 0;
    iNumQueryMetaDataExtensionInterfacePending = 0;
    iNumQueryMetaDataExtensionInterfaceComplete = 0;

    //create the plugin registry, instantiating all the current plugins
    //from the factory registry.
    //first remove any old registry.
    if (iPluginRegistry)
    {
        DePopulateCPMPluginRegistry(iPluginRegistry);
        /** Cleanup all registered plugins*/
        iPluginRegistry = NULL;
        /* Clear all vectors */
        iPlugInParamsVec.clear();
        iActivePlugInParamsVec.clear();
        iContentUsageContextVec.clear();
        iListofActiveSessions.clear();
        /**/
    }

    iPluginRegistry = PopulateCPMPluginRegistry();

    //check for empty registry & leave.
    if (iPluginRegistry
            && iPluginRegistry->GetNumPlugIns() == 0)
    {
        CPMPluginRegistryFactory::DestroyCPMPluginRegistry(iPluginRegistry);
        iPluginRegistry = NULL;
        OSCL_LEAVE(OsclErrGeneral);
    }

}

OSCL_EXPORT_REF void PVMFCPMImpl::ThreadLogoff()
{
    //Note: registry cleanup logically belongs under Reset command, but
    //some nodes currently hang onto their access interfaces after
    //CPM is reset, which means the plugins need to survive until after
    //the access interfaces are cleaned up.
    //@TODO this should be moved to the CompleteCPMReset routine.
    if (iPluginRegistry)
    {
        DePopulateCPMPluginRegistry(iPluginRegistry);
        iPluginRegistry = NULL;
        //clear the plugin params vec since all plugins are destroyed.
        iPlugInParamsVec.clear();
    }

    Cancel();

    if (IsAdded())
    {
        RemoveFromScheduler();
    }
}

OSCL_EXPORT_REF void PVMFCPMImpl::addRef()
{
    iExtensionRefCount++;
}

OSCL_EXPORT_REF void PVMFCPMImpl::removeRef()
{
    --iExtensionRefCount;
}

OSCL_EXPORT_REF bool PVMFCPMImpl::queryInterface(const PVUuid& uuid,
        PVInterface*& iface)
{
    iface = NULL;
    if (uuid == KPVMFMetadataExtensionUuid)
    {
        PVMFMetadataExtensionInterface* myInterface =
            OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    else if (uuid == PVMFCPMPluginLicenseInterfaceUuid)
    {
        PVMFCPMPluginLicenseInterface* myInterface =
            OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    else if (uuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
    {
        PvmiCapabilityAndConfig* myInterface =
            OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PVMFCommandId PVMFCPMImpl::Init(const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:Init"));
    PVMFCPMCommand cmd;
    cmd.Construct(PVMF_CPM_INIT, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::OpenSession(PVMFSessionId& aSessionId,
                         const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:OpenSession"));
    PVMFCPMCommand cmd;
    cmd.Construct(PVMF_CPM_OPEN_SESSION,
                  &aSessionId,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::RegisterContent(PVMFSessionId aSessionId,
                             OSCL_wString& aSourceURL,
                             PVMFFormatType& aSourceFormat,
                             OsclAny* aSourceData,
                             const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:RegisterContent"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_REGISTER_CONTENT,
                  &aSourceURL,
                  &aSourceFormat,
                  aSourceData,
                  NULL,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFStatus
PVMFCPMImpl::GetContentAccessFactory(PVMFSessionId aSessionId,
                                     PVMFCPMPluginAccessInterfaceFactory*& aContentAccessFactory)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::GetContentAccessFactory"));

    aContentAccessFactory = NULL;
    CPMSessionInfo* sInfo = LookUpSessionInfo(aSessionId);
    if (sInfo != NULL)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInID == sInfo->iAccessPlugInID)
            {
                it->iPlugInAccessInterfaceFactory->addRef();
                aContentAccessFactory = it->iPlugInAccessInterfaceFactory;
                return PVMFSuccess;
            }
        }
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::GetContentAccessFactory - No Access Plugin"));
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::GetContentAccessFactory - Invalid Session ID"));
    }
    return PVMFFailure;
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::ApproveUsage(PVMFSessionId aSessionId,
                          PvmiKvp& aRequestedUsage,
                          PvmiKvp& aApprovedUsage,
                          PvmiKvp& aAuthorizationData,
                          PVMFCPMUsageID& aUsageID,
                          const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::ApproveUsage"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_APPROVE_USAGE,
                  &aRequestedUsage,
                  &aApprovedUsage,
                  &aAuthorizationData,
                  &aUsageID,
                  aContext);
    return QueueCommandL(cmd);
}

PVMFCPMContentType PVMFCPMImpl::GetCPMContentType(PVMFSessionId aSessionId)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::GetCPMContentType"));
    CPMSessionInfo* sInfo = LookUpSessionInfo(aSessionId);
    if (sInfo != NULL)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInID == sInfo->iAccessPlugInID)
            {
                return (it->iPlugInInterface->GetCPMContentType());
            }
        }
    }
    return PVMF_CPM_CONTENT_FORMAT_UNKNOWN;
}

OSCL_EXPORT_REF PVMFStatus PVMFCPMImpl::GetCPMContentFilename(PVMFSessionId aSessionId, OSCL_wString& aFileName)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::GetCPMContentFilename"));
    CPMSessionInfo* sInfo = LookUpSessionInfo(aSessionId);
    if (sInfo != NULL)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInID == sInfo->iAccessPlugInID)
            {
                return (it->iPlugInInterface->GetCPMContentFilename(aFileName));
            }
        }
    }
    return PVMF_CPM_CONTENT_FORMAT_UNKNOWN;
}


OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::UsageComplete(PVMFSessionId aSessionId,
                           PVMFCPMUsageID& aUsageID,
                           OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::UsageComplete"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_USAGE_COMPLETE,
                  NULL,
                  NULL,
                  &aUsageID,
                  NULL,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::CloseSession(PVMFSessionId& aSessionId,
                          const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::CloseSession"));
    PVMFCPMCommand cmd;
    cmd.Construct(PVMF_CPM_CLOSE_SESSION,
                  &aSessionId,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFCPMImpl::Reset(const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::Reset"));
    PVMFCPMCommand cmd;
    cmd.Construct(PVMF_CPM_RESET, aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::GetNodeMetadataKeys(PVMFSessionId aSessionId,
                                 PVMFMetadataList& aKeyList,
                                 uint32 aStartingKeyIndex,
                                 int32 aMaxKeyEntries,
                                 char* aQueryKeyString,
                                 const OsclAny* aContextData)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::GetNodeMetadataKeys"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_GET_METADATA_KEYS,
                  aKeyList,
                  aStartingKeyIndex,
                  aMaxKeyEntries,
                  aQueryKeyString,
                  aContextData);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::GetNodeMetadataValues(PVMFSessionId aSessionId,
                                   PVMFMetadataList& aKeyList,
                                   Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                                   uint32 aStartingValueIndex,
                                   int32 aMaxValueEntries,
                                   const OsclAny* aContextData)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::GetNodeMetadataValues"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_GET_METADATA_VALUES,
                  aKeyList,
                  aValueList,
                  aStartingValueIndex,
                  aMaxValueEntries,
                  aContextData);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFStatus
PVMFCPMImpl::ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
                                     uint32 aStartingKeyIndex,
                                     uint32 aEndKeyIndex)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::ReleaseNodeMetadataKeys called"));
    if (((int32)aStartingKeyIndex < 0) ||
            (aStartingKeyIndex > aEndKeyIndex) ||
            (aKeyList.size() == 0))
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::ReleaseNodeMetadataKeys() Invalid start/end index"));
        return PVMFErrArgument;
    }
    if (aEndKeyIndex >= aKeyList.size())
    {
        aEndKeyIndex = aKeyList.size() - 1;
    }

    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        uint32 plugInStartIndex = it->iMetaDataKeyStartIndex;
        uint32 plugInEndIndex = it->iMetaDataKeyEndIndex;

        uint32 releaseStartIndex = 0;
        uint32 releaseEndIndex = 0;

        if ((aStartingKeyIndex >= plugInStartIndex) &&
                (aStartingKeyIndex <= plugInEndIndex))
        {
            releaseStartIndex = aStartingKeyIndex;
            if (aEndKeyIndex > plugInEndIndex)
            {
                releaseEndIndex = plugInEndIndex;
            }
            else
            {
                releaseEndIndex = aEndKeyIndex;
            }

            if (NULL != it->iPlugInMetaDataExtensionInterface)
            {
                it->iPlugInMetaDataExtensionInterface->ReleaseNodeMetadataKeys(aKeyList,
                        releaseStartIndex,
                        releaseEndIndex);
            }
        }
        aStartingKeyIndex = releaseEndIndex + 1;
    }
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFStatus
PVMFCPMImpl::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
                                       uint32 aStartingValueIndex,
                                       uint32 aEndValueIndex)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::ReleaseNodeMetadataValues called"));

    if (((int32)aStartingValueIndex < 0) ||
            (aStartingValueIndex > aEndValueIndex) ||
            (aValueList.size() == 0))
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::ReleaseNodeMetadataValues() Invalid start/end index"));
        return PVMFErrArgument;
    }

    if (aEndValueIndex >= aValueList.size())
    {
        aEndValueIndex = aValueList.size() - 1;
    }

    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        uint32 plugInStartIndex = it->iMetaDataValueStartIndex;
        uint32 plugInEndIndex = it->iMetaDataValueEndIndex;

        uint32 releaseStartIndex = 0;
        uint32 releaseEndIndex = 0;

        if ((aStartingValueIndex >= plugInStartIndex) &&
                (aStartingValueIndex <= plugInEndIndex))
        {
            releaseStartIndex = aStartingValueIndex;
            if (aEndValueIndex > plugInEndIndex)
            {
                releaseEndIndex = plugInEndIndex;
            }
            else
            {
                releaseEndIndex = aEndValueIndex;
            }
            if (NULL != it->iPlugInMetaDataExtensionInterface)
            {
                it->iPlugInMetaDataExtensionInterface->ReleaseNodeMetadataValues(aValueList,
                        releaseStartIndex,
                        releaseEndIndex);
            }
        }
        aStartingValueIndex = releaseEndIndex + 1;
    }
    return PVMFSuccess;
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMImpl::QueryInterface(PVMFSessionId aSessionId,
                            const PVUuid& aUuid,
                            PVInterface*& aInterfacePtr,
                            const OsclAny* aContext)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::GetNodeMetadataValues"));
    PVMFCPMCommand cmd;
    cmd.PVMFCPMCommandBase::Construct(aSessionId,
                                      PVMF_CPM_QUERY_INTERFACE,
                                      aUuid,
                                      aInterfacePtr,
                                      aContext);
    return QueueCommandL(cmd);
}


/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to
 * memory allocation failure.
 */
PVMFCommandId PVMFCPMImpl::QueueCommandL(PVMFCPMCommand& aCmd)
{
    PVMFCommandId id;
    id = iInputCommands.AddL(aCmd);
    /* wakeup the AO */
    RunIfNotReady();
    return id;
}

/**
 * The various command handlers call this when a command is complete.
 */
void PVMFCPMImpl::CommandComplete(PVMFCPMCommandCmdQ& aCmdQ,
                                  PVMFCPMCommand& aCmd,
                                  PVMFStatus aStatus,
                                  OsclAny* aEventData,
                                  PVUuid* aEventUUID,
                                  int32* aEventCode)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
                      , aCmd.iId, aCmd.iCmd, aStatus, aCmd.iContext, aEventData));

    PVInterface* extif = NULL;
    PVMFBasicErrorInfoMessage* errormsg = NULL;
    if (aEventUUID && aEventCode)
    {
        errormsg = OSCL_NEW(PVMFBasicErrorInfoMessage, (*aEventCode, *aEventUUID, NULL));
        extif = OSCL_STATIC_CAST(PVInterface*, errormsg);
    }

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, extif, aEventData);

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    iObserver.CPMCommandCompleted(resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }

    /* Reschedule AO if input command queue is not empty */
    if (!iInputCommands.empty()
            && IsAdded())
    {
        RunIfNotReady();
    }
}

void PVMFCPMImpl::CommandComplete(PVMFCPMCommandCmdQ& aCmdQ,
                                  PVMFCPMCommand& aCmd,
                                  PVMFStatus aStatus,
                                  PVInterface* aErrorExtIntf)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::CommandComplete Id %d Cmd %d Status %d"
                      , aCmd.iId, aCmd.iCmd, aStatus));

    /* create response */
    PVMFCmdResp resp(aCmd.iId, aCmd.iContext, aStatus, aErrorExtIntf, NULL);

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    iObserver.CPMCommandCompleted(resp);

    /* Reschedule AO if input command queue is not empty */
    if (!iInputCommands.empty())
    {
        RunIfNotReady();
    }
}

void
PVMFCPMImpl::MoveCmdToCurrentQueue(PVMFCPMCommand& aCmd)
{
    int32 err = OsclErrNone;
    OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
    if (err != OsclErrNone)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::MoveCmdToCurrentQueue - No Memory"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }
    iInputCommands.Erase(&aCmd);
    return;
}

void
PVMFCPMImpl::MoveCmdToCancelQueue(PVMFCPMCommand& aCmd)
{
    /*
     * note: the StoreL cannot fail since the queue is never more than 1 deep
     * and we reserved space.
     */
    iCancelCommand.StoreL(aCmd);
    iInputCommands.Erase(&aCmd);
}

PVMFCPMCommandContext* PVMFCPMImpl::RequestNewInternalCmd()
{
    int32 i = 0;
    /* Search for the next free node command in the pool */
    while (i < PVMF_CPM_INTERNAL_CMDQ_SIZE)
    {
        if (iInternalCmdPool[i].oFree)
        {
            iInternalCmdPool[i].oFree = false;
            return &(iInternalCmdPool[i]);
        }
        ++i;
    }
    /* Free one not found so return NULL */
    return NULL;
}

/**
 * Called by the command handler AO to process a command from
 * the input queue.
 * Return true if a command was processed, false if the command
 * processor is busy and can't process another command now.
 */
bool PVMFCPMImpl::ProcessCommand(PVMFCPMCommand& aCmd)
{
    /*
     * normally this node will not start processing one command
     * until the prior one is finished.  However, a hi priority
     * command such as Cancel must be able to interrupt a command
     * in progress.
     */
    if (!iCurrentCommand.empty() && !aCmd.hipri() && aCmd.iCmd != PVMF_CPM_CANCEL_GET_LICENSE)
        return false;

    switch (aCmd.iCmd)
    {
        case PVMF_CPM_INIT:
            DoInit(aCmd);
            break;

        case PVMF_CPM_OPEN_SESSION:
            DoOpenSession(aCmd);
            break;

        case PVMF_CPM_REGISTER_CONTENT:
            DoRegisterContent(aCmd);
            break;

        case PVMF_CPM_APPROVE_USAGE:
            DoApproveUsage(aCmd);
            break;

        case PVMF_CPM_USAGE_COMPLETE:
            DoUsageComplete(aCmd);
            break;

        case PVMF_CPM_CLOSE_SESSION:
            DoCloseSession(aCmd);
            break;

        case PVMF_CPM_RESET:
            DoReset(aCmd);
            break;

        case PVMF_CPM_GET_METADATA_KEYS:
        {
            PVMFStatus status = DoGetMetadataKeys(aCmd);
            if (status != PVMFPending)
            {
                CommandComplete(iInputCommands, aCmd, status);
            }
            else
            {
                MoveCmdToCurrentQueue(aCmd);
            }
        }
        break;

        case PVMF_CPM_GET_METADATA_VALUES:
            DoGetMetadataValues(aCmd);
            break;

        case PVMF_CPM_QUERY_INTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_CPM_GET_LICENSE_W:
        {
            PVMFStatus status = DoGetLicense(aCmd, true);
            if (status == PVMFPending)
            {
                MoveCmdToCurrentQueue(aCmd);
            }
            else
            {
                CommandComplete(iInputCommands, aCmd, status);
            }
        }
        break;

        case PVMF_CPM_GET_LICENSE:
        {
            PVMFStatus status = DoGetLicense(aCmd);
            if (status == PVMFPending)
            {
                MoveCmdToCurrentQueue(aCmd);
            }
            else
            {
                CommandComplete(iInputCommands, aCmd, status);
            }
        }
        break;

        case PVMF_CPM_CANCEL_GET_LICENSE:
            DoCancelGetLicense(aCmd);
            break;

        default:
        {
            /* unknown command type */
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::ProcessCommand - Unknown Command"));
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
        }
        break;
    }

    return true;
}

void PVMFCPMImpl::DoInit(PVMFCPMCommand& aCmd)
{
    if (!iPluginRegistry)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoInit - No Plugin Registry"));
        CommandComplete(iInputCommands, aCmd, PVMFFailure);
    }

    for (uint32 i = 0; i < iPluginRegistry->GetNumPlugIns(); i++)
    {
        CPMPlugInParams plugInParams;

        iPluginRegistry->GetPluginMimeType(i, plugInParams.iPlugInMimeType);
        plugInParams.iPlugInID = i;

        CPMPluginContainer* container =
            iPluginRegistry->lookupPlugin(plugInParams.iPlugInMimeType);

        if (container)
        {
            PVMFCPMPluginInterface& iface = container->PlugIn();
            OsclAny* _pPlugInData = container->PlugInUserAuthenticationData();
            plugInParams.iPlugInInterface = &iface;
            plugInParams.iPlugInData = _pPlugInData;
            iPlugInParamsVec.push_back(plugInParams);
        }
    }
    /* Connect with all plugins */
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
    {
        it->iPlugInSessionID = it->iPlugInInterface->Connect(*this);
        it->iConnected = true;
    }
    PVMFStatus status = InitRegisteredPlugIns();
    if (status == PVMFSuccess)
    {
        MoveCmdToCurrentQueue(aCmd);
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoInit - InitRegisteredPlugIns Failed"));
        CommandComplete(iInputCommands, aCmd, status);
    }
}

PVMFStatus PVMFCPMImpl::InitRegisteredPlugIns()
{
    if (iPlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
        {
            /* Get Authentication interface */
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd = PVMF_CPM_INTERNAL_INIT_CMD;
                internalCmd->parentCmd = PVMF_CPM_INIT;
                internalCmd->plugInID = it->iPlugInID;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                it->iPlugInInterface->Init(it->iPlugInSessionID,
                                           cmdContextData);
                iNumRegisteredPlugInInitPending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No registered plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::InitRegisteredPlugIns - Registered plugin list empty"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteInitPlugIns()
{
    if (iNumRegisteredPlugInInitPending ==
            iNumRegisteredPlugInInitComplete)
    {
        PVMFStatus status = QueryForPlugInMetaDataExtensionInterface();

        if (status != PVMFSuccess)
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteInitPlugIns - QueryForPlugInAuthenticationInterface Failed"));
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            status);
        }
    }
}

PVMFStatus PVMFCPMImpl::QueryForPlugInMetaDataExtensionInterface()
{
    if (iPlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
        {
            /* Get MetaDataExtension interface */
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    PVMF_CPM_INTERNAL_QUERY_METADATA_EXTENSION_INTERFACE_CMD;
                internalCmd->parentCmd = PVMF_CPM_INIT;
                internalCmd->plugInID = it->iPlugInID;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                it->iPlugInMetaDataExtensionInterfacePVI = NULL;
                it->iPlugInInterface->QueryInterface(it->iPlugInSessionID,
                                                     KPVMFMetadataExtensionUuid,
                                                     it->iPlugInMetaDataExtensionInterfacePVI,
                                                     cmdContextData);
                iNumQueryMetaDataExtensionInterfacePending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No registered plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForPlugInMetaDataExtensionInterface - Registered plugin list empty"));
    return PVMFFailure;
}


void PVMFCPMImpl::CompleteMetaDataExtInterfaceQueryFromPlugIns()
{
    if (iNumQueryMetaDataExtensionInterfacePending ==
            iNumQueryMetaDataExtensionInterfaceComplete)
    {
        PVMFStatus status = QueryForPlugInCapConfigInterface();

        if (status != PVMFSuccess)
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteMetaDataExtInterfaceQueryFromPlugIns - QueryForPlugInCapConfigInterface Failed"));
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            status);
        }
    }
}

PVMFStatus PVMFCPMImpl::QueryForPlugInCapConfigInterface()
{
    if (iPlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
        {
            /* Get MetaDataExtension interface */
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    PVMF_CPM_INTERNAL_QUERY_CAP_CONFIG_INTERFACE_CMD;
                internalCmd->parentCmd = PVMF_CPM_INIT;
                internalCmd->plugInID = it->iPlugInID;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                it->iPlugInCapConfigExtensionInterfacePVI = NULL;
                it->iPlugInInterface->QueryInterface(it->iPlugInSessionID,
                                                     PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                                                     it->iPlugInCapConfigExtensionInterfacePVI,
                                                     cmdContextData);
                iNumQueryCapConfigExtensionInterfacePending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No registered plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForPlugInCapConfigInterface - Registered plugin list empty"));
    return PVMFFailure;
}


void PVMFCPMImpl::CompleteCapConfigExtInterfaceQueryFromPlugIns()
{
    if (iNumQueryCapConfigExtensionInterfacePending ==
            iNumQueryCapConfigExtensionInterfaceComplete)
    {
        PVMFStatus status = QueryForPlugInAuthenticationInterface();

        if (status != PVMFSuccess)
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteCapConfigExtInterfaceQueryFromPlugIns - QueryForPlugInAuthenticationInterface Failed"));
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            status);
        }
    }
}

PVMFStatus PVMFCPMImpl::QueryForPlugInAuthenticationInterface()
{
    if (iPlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
        {
            /* Get Authentication interface */
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    PVMF_CPM_INTERNAL_QUERY_AUTHENTICATION_INTERFACE_CMD;
                internalCmd->parentCmd = PVMF_CPM_INIT;
                internalCmd->plugInID = it->iPlugInID;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                it->iPlugInAuthenticationInterfacePVI = NULL;
                it->iPlugInInterface->QueryInterface(it->iPlugInSessionID,
                                                     PVMFCPMPluginAuthenticationInterfaceUuid,
                                                     it->iPlugInAuthenticationInterfacePVI,
                                                     cmdContextData);
                iNumQueryAuthenticationInterfacePending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No registered plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForPlugInAuthenticationInterface - Registered plugin list empty"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteCPMInit()
{
    if (iNumQueryAuthenticationInterfacePending ==
            iNumQueryAuthenticationInterfaceComplete)
    {
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFSuccess);
    }
}

void PVMFCPMImpl::DoOpenSession(PVMFCPMCommand& aCmd)
{
    OsclAny* temp = NULL;
    aCmd.Parse(temp);
    PVMFSessionId* sessionIdPtr = OSCL_STATIC_CAST(PVMFSessionId*, temp);

    /* Create a session info */
    CPMSessionInfo sessionInfo;
    sessionInfo.iSessionId = iListofActiveSessions.size();
    *sessionIdPtr = sessionInfo.iSessionId;
    iListofActiveSessions.push_back(sessionInfo);
    PVMFStatus status =
        AuthenticateWithAllRegisteredPlugIns(sessionInfo.iSessionId);
    if (status == PVMFSuccess)
    {
        MoveCmdToCurrentQueue(aCmd);
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoOpenSession - AuthenticateWithAllRegisteredPlugIns Failed"));
        CommandComplete(iInputCommands, aCmd, status);
    }
}

PVMFStatus PVMFCPMImpl::AuthenticateWithAllRegisteredPlugIns(PVMFSessionId aSessionId)
{
    if (iPlugInParamsVec.size() > 0)
    {
        CPMSessionInfo* sessionInfo = LookUpSessionInfo(aSessionId);

        if (sessionInfo != NULL)
        {
            Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
            for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
            {
                /* Get Authentication interface */
                PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
                if (internalCmd != NULL)
                {
                    internalCmd->cmd = PVMF_CPM_INTERNAL_AUTHENTICATE_CMD;
                    internalCmd->parentCmd = PVMF_CPM_OPEN_SESSION;
                    internalCmd->plugInID = it->iPlugInID;
                    OsclAny *cmdContextData =
                        OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                    it->iPlugInAuthenticationInterface->AuthenticateUser(it->iPlugInSessionID,
                            it->iPlugInData,
                            cmdContextData);
                    sessionInfo->iNumPlugInAunthenticateRequestsPending++;
                }
                else
                {
                    return PVMFErrNoMemory;
                }
            }
            return PVMFSuccess;
        }
        else
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::AuthenticateWithAllRegisteredPlugIns - Invalid Session ID"));
            return PVMFErrArgument;
        }
    }
    /* No registered plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::AuthenticateWithAllRegisteredPlugIns - Registered plugin list empty"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteOpenSession(CPMSessionInfo* aSessionInfo)
{
    if (aSessionInfo == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteOpenSession - Invalid Session ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        aSessionInfo->iNumPlugInAunthenticateRequestsComplete++;

        if (aSessionInfo->iNumPlugInAunthenticateRequestsComplete ==
                aSessionInfo->iNumPlugInAunthenticateRequestsPending)
        {
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFSuccess);
        }
    }
    return;
}

void PVMFCPMImpl::DoRegisterContent(PVMFCPMCommand& aCmd)
{
    OsclAny* temp1 = NULL;
    OsclAny* temp2 = NULL;
    OsclAny* aSourceData;
    OsclAny* placeHolder;

    aCmd.Parse(temp1,
               temp2,
               aSourceData,
               placeHolder);

    OSCL_wString* sourceURL = OSCL_STATIC_CAST(OSCL_wString*, temp1);
    PVMFFormatType* sourceFormatType = OSCL_STATIC_CAST(PVMFFormatType*, temp2);

    CPMSessionInfo* sInfo = LookUpSessionInfo(aCmd.iSession);

    if (sInfo != NULL)
    {
        sInfo->iSourceURL = *sourceURL;
        sInfo->iSourceFormatType = *sourceFormatType;
        sInfo->iSourceData = aSourceData;
        PVMFStatus status = PopulateListOfActivePlugIns(sInfo);
        if (status == PVMFSuccess)
        {
            status = QueryForAuthorizationInterface(sInfo);
            if (status == PVMFSuccess)
            {
                MoveCmdToCurrentQueue(aCmd);
            }
            else
            {
                PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoRegisterContent - QueryForAuthorizationInterface Failed"));
                CommandComplete(iInputCommands, aCmd, status);
            }
        }
        else
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoRegisterContent - PopulateListOfActivePlugIns Failed"));
            CommandComplete(iInputCommands, aCmd, status);
        }
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoRegisterContent - Invalid Session ID"));
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
    }
}

PVMFStatus PVMFCPMImpl::PopulateListOfActivePlugIns(CPMSessionInfo* aInfo)
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
    {
        PVMFStatus status =
            it->iPlugInInterface->SetSourceInitializationData(aInfo->iSourceURL,
                    aInfo->iSourceFormatType,
                    aInfo->iSourceData);
        if (status == PVMFSuccess)
        {
            /* Add this plug to the active list */
            iActivePlugInParamsVec.push_back(*it);
        }
    }
    if (iActivePlugInParamsVec.size() > 0)
    {
        return PVMFSuccess;
    }
    //no plugins care about this clip
    return PVMFErrNotSupported;
}

PVMFStatus PVMFCPMImpl::QueryForAuthorizationInterface(CPMSessionInfo* aInfo)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    PVMF_CPM_INTERNAL_QUERY_AUTHORIZATION_INTERFACE_CMD;
                internalCmd->parentCmd = PVMF_CPM_REGISTER_CONTENT;
                internalCmd->plugInID = it->iPlugInID;
                internalCmd->sessionid = aInfo->iSessionId;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                it->iPlugInAuthorizationInterfacePVI = NULL;
                it->iPlugInInterface->QueryInterface(it->iPlugInSessionID,
                                                     PVMFCPMPluginAuthorizationInterfaceUuid,
                                                     it->iPlugInAuthorizationInterfacePVI,
                                                     cmdContextData);
                aInfo->iNumPlugInAuthorizeInterfaceQueryRequestsPending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForAuthorizationInterface - No Active Plugins"));
    return PVMFFailure;
}

PVMFStatus PVMFCPMImpl::QueryForAccessInterfaceFactory(CPMSessionInfo* aInfo)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    PVMF_CPM_INTERNAL_QUERY_ACCESS_INTERFACE_FACTORY_CMD;
                internalCmd->parentCmd = PVMF_CPM_REGISTER_CONTENT;
                internalCmd->plugInID = it->iPlugInID;
                internalCmd->sessionid = aInfo->iSessionId;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                it->iPlugInAccessInterfaceFactoryPVI = NULL;
                it->iPlugInInterface->QueryInterface(it->iPlugInSessionID,
                                                     PVMFCPMPluginAccessInterfaceFactoryUuid,
                                                     it->iPlugInAccessInterfaceFactoryPVI,
                                                     cmdContextData);
                aInfo->iNumPlugInAccessInterfaceFactoryQueryRequestsPending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForAccessInterfaceFactory - No Active Plugins"));
    return PVMFFailure;
}

PVMFStatus PVMFCPMImpl::DetermineAccessPlugIn(CPMSessionInfo* aInfo)
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iPlugInAccessInterfaceFactory != NULL)
        {
            aInfo->iAccessPlugInID = it->iPlugInID;
            return PVMFSuccess;
        }
    }
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DetermineAccessPlugIn Failed"));
    return PVMFFailure;
}

PVMFStatus PVMFCPMImpl::QueryForLicenseInterface(CPMSessionInfo* aInfo)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd =
                    PVMF_CPM_INTERNAL_QUERY_LICENSE_INTERFACE_CMD;
                internalCmd->parentCmd = PVMF_CPM_REGISTER_CONTENT;
                internalCmd->plugInID = it->iPlugInID;
                internalCmd->sessionid = aInfo->iSessionId;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
                it->iPlugInLicenseInterfacePVI = NULL;
                it->iPlugInInterface->QueryInterface(it->iPlugInSessionID,
                                                     PVMFCPMPluginLicenseInterfaceUuid,
                                                     it->iPlugInLicenseInterfacePVI,
                                                     cmdContextData);
                aInfo->iNumPlugInLicenseAcquisitionInterfaceRequestsPending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForLicenseInterface - No Active Plugins"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteRegisterContentPhase1(CPMSessionInfo* aInfo)
{
    if (aInfo == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteRegisterContentPhase1 - Invalid Session ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        aInfo->iNumPlugInAuthorizeInterfaceQueryRequestsComplete++;

        if (aInfo->iNumPlugInAuthorizeInterfaceQueryRequestsComplete ==
                aInfo->iNumPlugInAuthorizeInterfaceQueryRequestsPending)
        {
            PVMFStatus status = QueryForAccessInterfaceFactory(aInfo);
            if (status != PVMFSuccess)
            {
                PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteRegisterContentPhase1 - QueryForAccessInterfaceFactory Failed"));
                CommandComplete(iCurrentCommand,
                                iCurrentCommand.front(),
                                status);
            }
        }
    }
    return;
}

void PVMFCPMImpl::CompleteRegisterContentPhase2(CPMSessionInfo* aInfo)
{
    if (aInfo == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteRegisterContentPhase2 - Invalid Session ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        aInfo->iNumPlugInAccessInterfaceFactoryQueryRequestsComplete++;

        if (aInfo->iNumPlugInAccessInterfaceFactoryQueryRequestsComplete ==
                aInfo->iNumPlugInAccessInterfaceFactoryQueryRequestsPending)
        {
            PVMFStatus status = DetermineAccessPlugIn(aInfo);
            if (status != PVMFSuccess)
            {
                CommandComplete(iCurrentCommand,
                                iCurrentCommand.front(),
                                status);
            }
            else
            {
                status = QueryForLicenseInterface(aInfo);
                if (status != PVMFSuccess)
                {
                    CommandComplete(iCurrentCommand,
                                    iCurrentCommand.front(),
                                    status);
                }
            }
        }
    }
    return;
}

void PVMFCPMImpl::CompleteRegisterContentPhase3(CPMSessionInfo* aInfo)
{
    if (aInfo == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteRegisterContentPhase3 - Invalid Session ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        aInfo->iNumPlugInLicenseAcquisitionInterfaceRequestsComplete++;

        if (aInfo->iNumPlugInLicenseAcquisitionInterfaceRequestsComplete ==
                aInfo->iNumPlugInLicenseAcquisitionInterfaceRequestsPending)
        {
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFSuccess);
        }
    }
    return;
}


void PVMFCPMImpl::DoApproveUsage(PVMFCPMCommand& aCmd)
{
    OsclAny* temp1 = NULL;
    OsclAny* temp2 = NULL;
    OsclAny* temp3 = NULL;
    OsclAny* temp4 = NULL;

    aCmd.Parse(temp1, temp2, temp3, temp4);

    PVMFCPMUsageID* usageID = OSCL_STATIC_CAST(PVMFCPMUsageID*, temp4);

    /* Create Usage context */
    *usageID = iContentUsageContextVec.size();
    CPMContentUsageContext usageContext;
    usageContext.iUsageID = *usageID;
    iContentUsageContextVec.push_back(usageContext);

    PVMFStatus status = RequestApprovalFromActivePlugIns(aCmd);
    if (status == PVMFSuccess)
    {
        MoveCmdToCurrentQueue(aCmd);
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoApproveUsage - RequestApprovalFromActivePlugIns Failed"));
        CommandComplete(iInputCommands, aCmd, status);
    }
}

PVMFStatus PVMFCPMImpl::RequestApprovalFromActivePlugIns(PVMFCPMCommand& aCmd)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        OsclAny* temp1 = NULL;
        OsclAny* temp2 = NULL;
        OsclAny* temp3 = NULL;
        OsclAny* temp4 = NULL;

        aCmd.Parse(temp1, temp2, temp3, temp4);

        PvmiKvp* requestedUsage = OSCL_STATIC_CAST(PvmiKvp*, temp1);
        PvmiKvp* approvedUsage = OSCL_STATIC_CAST(PvmiKvp*, temp2);
        PvmiKvp* authorizationData = OSCL_STATIC_CAST(PvmiKvp*, temp3);
        PVMFCPMUsageID* usageID = OSCL_STATIC_CAST(PVMFCPMUsageID*, temp4);

        CPMSessionInfo* sInfo = NULL; // initialize to ensure that if LookUpSeesionInfo() fail, sInfo will be NULL
        sInfo = LookUpSessionInfo(aCmd.iSession);
        OSCL_ASSERT(sInfo);
        if (!sInfo)
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::RequestApprovalFromActivePlugIns - No Session Info"));
            return PVMFFailure;
        }

        CPMContentUsageContext* usageContext = LookUpContentUsageContext(*usageID);
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInID == sInfo->iAccessPlugInID)
            {
                PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
                if (internalCmd != NULL)
                {
                    internalCmd->cmd = PVMF_CPM_INTERNAL_AUTHORIZE_CMD;
                    internalCmd->parentCmd = PVMF_CPM_APPROVE_USAGE;
                    internalCmd->plugInID = it->iPlugInID;
                    internalCmd->usageid = *usageID;
                    OsclAny *cmdContextData =
                        OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                    it->iPlugInAuthorizationInterface->AuthorizeUsage(it->iPlugInSessionID,
                            *requestedUsage,
                            *approvedUsage,
                            *authorizationData,
                            it->iAuthorizationRequestTimeOut,
                            cmdContextData);
                    OSCL_ASSERT(usageContext);
                    if (!usageContext)
                    {
                        return PVMFFailure;
                    }
                    usageContext->iNumAuthorizeRequestsPending++;
                }
                else
                {
                    return PVMFErrNoMemory;
                }
            }
        }
        return PVMFSuccess;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::RequestApprovalFromActivePlugIns - No Active Plugins"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteApproveUsage(CPMContentUsageContext* aContext)
{
    if (aContext == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteApproveUsage - Invalid Usage ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        aContext->iNumAuthorizeRequestsComplete++;

        if (aContext->iNumAuthorizeRequestsComplete ==
                aContext->iNumAuthorizeRequestsPending)
        {
            if (CheckForMetaDataInterfaceAvailability())
            {
                PVMFStatus status = QueryForMetaDataKeys(iCurrentCommand.front());
                if (status != PVMFSuccess)
                {
                    CommandComplete(iCurrentCommand,
                                    iCurrentCommand.front(),
                                    status);
                }
            }
            else
            {
                // No meta data
                CommandComplete(iCurrentCommand,
                                iCurrentCommand.front(),
                                PVMFSuccess);
            }
        }
    }
    return;
}

bool PVMFCPMImpl::CheckForMetaDataInterfaceAvailability()
{
    uint32 num = 0;
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInMetaDataExtensionInterface != NULL)
            {
                num++;
            }
        }
        if (num > 0)
        {
            return true;
        }
    }
    return false;
}

PVMFStatus PVMFCPMImpl::QueryForMetaDataKeys(PVMFCPMCommand& aParentCmd)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInMetaDataExtensionInterface != NULL)
            {
                PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
                if (internalCmd != NULL)
                {
                    internalCmd->cmd =
                        PVMF_CPM_INTERNAL_GET_PLUGIN_META_DATA_KEYS_CMD;
                    internalCmd->parentCmd = aParentCmd.iCmd;
                    internalCmd->plugInID = it->iPlugInID;
                    OsclAny *cmdContextData =
                        OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                    it->iNumMetaDataKeysAvailable  = 0;
                    it->iAvailableMetadataKeys.clear();

                    it->iNumMetaDataKeysAvailable =
                        it->iPlugInMetaDataExtensionInterface->GetNumMetadataKeys();
                    it->iPlugInMetaDataExtensionInterface->GetNodeMetadataKeys(it->iPlugInSessionID,
                            it->iAvailableMetadataKeys,
                            0,
                            it->iNumMetaDataKeysAvailable,
                            NULL,
                            cmdContextData);
                }
                else
                {
                    return PVMFErrNoMemory;
                }
            }
            else
            {
                it->iGetMetaDataKeysComplete = true;
            }
        }
        return PVMFSuccess;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::QueryForMetaDataKeys - No Active Plugins"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteGetMetaDataKeys(uint32 aPlugInID)
{
    CPMPlugInParams* pluginInParams = LookUpPlugInParamsFromActiveList(aPlugInID);
    if (pluginInParams == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteGetMetaDataKeys - Invalid PlugIn ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        pluginInParams->iGetMetaDataKeysComplete = true;
        if (CheckForGetMetaDataKeysCompletion())
        {
            PVMFStatus status = PVMFSuccess;
            if (iGetMetaDataKeysInProgress == true)
            {
                status = CompleteDoGetMetadataKeys(iCurrentCommand.front());
            }
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            status);
            iGetMetaDataKeysFromPlugInsDone = true;
        }
    }
    return;
}

bool PVMFCPMImpl::CheckForGetMetaDataKeysCompletion()
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iPlugInMetaDataExtensionInterface != NULL)
        {
            if (it->iGetMetaDataKeysComplete == false)
            {
                return false;
            }
        }
    }
    return true;
}


void PVMFCPMImpl::DoUsageComplete(PVMFCPMCommand& aCmd)
{
    OsclAny* placeHolder1;
    OsclAny* placeHolder2;
    OsclAny* placeHolder3;
    OsclAny* temp = NULL;

    aCmd.Parse(OSCL_STATIC_CAST(OsclAny*&, placeHolder1),
               OSCL_STATIC_CAST(OsclAny*&, placeHolder2),
               temp,
               OSCL_STATIC_CAST(OsclAny*&, placeHolder3));

    PVMFCPMUsageID* usageID = OSCL_STATIC_CAST(PVMFCPMUsageID*, temp);

    PVMFStatus status = SendUsageCompleteToRegisteredPlugIns(*usageID);
    if (status == PVMFSuccess)
    {
        MoveCmdToCurrentQueue(aCmd);
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoUsageComplete - SendUsageCompleteToRegisteredPlugIns Failed"));
        CommandComplete(iInputCommands, aCmd, status);
    }
}

PVMFStatus PVMFCPMImpl::SendUsageCompleteToRegisteredPlugIns(PVMFCPMUsageID aID)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        CPMContentUsageContext* usageContext = LookUpContentUsageContext(aID);
        if (usageContext != NULL)
        {
            Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
            for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
            {
                PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
                if (internalCmd != NULL)
                {
                    internalCmd->cmd = PVMF_CPM_INTERNAL_USAGE_COMPLETE_CMD;
                    internalCmd->parentCmd = PVMF_CPM_USAGE_COMPLETE;
                    internalCmd->plugInID = it->iPlugInID;
                    internalCmd->usageid = aID;
                    OsclAny *cmdContextData =
                        OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                    it->iPlugInAuthorizationInterface->UsageComplete(it->iPlugInSessionID,
                            cmdContextData);
                    usageContext->iNumUsageCompleteRequestsPending++;
                }
                else
                {
                    return PVMFErrNoMemory;
                }
            }
            return PVMFSuccess;
        }
        else
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::SendUsageCompleteToRegisteredPlugIns - Invalid UsageContext"));
            return PVMFFailure;
        }
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::SendUsageCompleteToRegisteredPlugIns - No Active Plugins"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteUsageComplete(CPMContentUsageContext* aContext)
{
    if (aContext == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteUsageComplete - Invalid Usage ID"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFFailure);
    }
    else
    {
        aContext->iNumUsageCompleteRequestsComplete++;

        if (aContext->iNumUsageCompleteRequestsComplete ==
                aContext->iNumUsageCompleteRequestsPending)
        {
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFSuccess);
        }
    }
    return;
}

void PVMFCPMImpl::DoCloseSession(PVMFCPMCommand& aCmd)
{
    OsclAny* temp = NULL;
    aCmd.Parse(temp);
    PVMFSessionId* sessionId = OSCL_STATIC_CAST(PVMFSessionId*, temp);

    CPMSessionInfo* sessionInfo = NULL;

    Oscl_Vector<CPMSessionInfo, OsclMemAllocator>::iterator it;
    for (it = iListofActiveSessions.begin(); it != iListofActiveSessions.end(); it++)
    {
        if (it->iSessionId == *sessionId)
        {
            sessionInfo = it;
            break;
        }
    }
    if (sessionInfo == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoCloseSession - Invalid Session ID"));
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
        return;
    }
    iListofActiveSessions.erase(it);
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    return;
}

void PVMFCPMImpl::DoReset(PVMFCPMCommand& aCmd)
{
    PVMFStatus status = ResetRegisteredPlugIns();
    if (status == PVMFSuccess)
    {
        MoveCmdToCurrentQueue(aCmd);
    }
    else
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoReset - ResetRegisteredPlugIns Failed"));
        CommandComplete(iInputCommands, aCmd, status);
    }
    iNumRegisteredPlugInInitPending = 0;
    iNumRegisteredPlugInInitComplete = 0;
    iNumQueryMetaDataExtensionInterfacePending = 0;
    iNumQueryMetaDataExtensionInterfaceComplete = 0;

    iGetMetaDataKeysFromPlugInsDone = false;
    iGetMetaDataKeysInProgress = false;

}

PVMFStatus PVMFCPMImpl::ResetRegisteredPlugIns()
{
    if (iPlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
        {
            /* Get Authentication interface */
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd = PVMF_CPM_INTERNAL_RESET_CMD;
                internalCmd->parentCmd = PVMF_CPM_RESET;
                internalCmd->plugInID = it->iPlugInID;
                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                it->iPlugInInterface->Reset(it->iPlugInSessionID,
                                            cmdContextData);
                iNumRegisteredPlugInResetPending++;
            }
            else
            {
                return PVMFErrNoMemory;
            }
        }
        return PVMFSuccess;
    }
    /* No registered plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::ResetRegisteredPlugIns - Registered plugin list empty"));
    return PVMFFailure;
}

void PVMFCPMImpl::CompleteCPMReset()
{
    if (iNumRegisteredPlugInResetPending ==
            iNumRegisteredPlugInResetComplete)
    {
        /* Disonnect from all plugins */
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
        {
            if (it->iConnected)
                it->iPlugInInterface->Disconnect(it->iPlugInSessionID);
            it->iConnected = false;
        }
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFSuccess);
    }
}

void PVMFCPMImpl::DoQueryInterface(PVMFCPMCommand& aCmd)
{
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFCPMCommandBase::Parse(uuid, ptr);

    PVMFStatus status = PVMFErrNotSupported;
    PVInterface* iFace = NULL;
    if (queryInterface(*uuid, iFace))
    {
        status = PVMFSuccess;
        *ptr = OSCL_STATIC_CAST(PVInterface*, iFace);
    }
    CommandComplete(iInputCommands, aCmd, status);
    return;
}

void PVMFCPMImpl::Run()
{
    /*
     * Process commands.
     */
    if (!iInputCommands.empty())
    {
        ProcessCommand(iInputCommands.front());
    }
}

void PVMFCPMImpl::CPMPluginCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVMFCPMCommandContext *cmdContextData =
        OSCL_REINTERPRET_CAST(PVMFCPMCommandContext*, aResponse.GetContext());

    cmdContextData->oFree = true;

    if (cmdContextData->cmd == PVMF_CPM_INTERNAL_CANCEL_GET_LICENSE)
    {
        PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::CompleteCancelGetLicense - status=%d", aResponse.GetCmdStatus()));
        CommandComplete(iCancelCommand,
                        iCancelCommand.front(),
                        aResponse.GetCmdStatus());
        return;
    }
    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        if (((cmdContextData->cmd == PVMF_CPM_INTERNAL_QUERY_METADATA_EXTENSION_INTERFACE_CMD) ||
                (cmdContextData->cmd == PVMF_CPM_INTERNAL_QUERY_CAP_CONFIG_INTERFACE_CMD) ||
                (cmdContextData->cmd == PVMF_CPM_INTERNAL_QUERY_LICENSE_INTERFACE_CMD)) &&
                (aResponse.GetCmdStatus() == PVMFErrNotSupported))
        {
            /*
             * Do nothing - Not all plugins have to support:
             * metadata, license interface
             */
        }
        else
        {
            /*
             * This call is in response to a command issued by CPM to one
             * of its plugins. This also means that there is current
             * command in the iCurrentCommand queue blocking the command loop
             * waiting for the all plugin commands to complete. Report failure
             * on it in case the parent id of this command is same as that of
             * the current blocking command. In case more than one child node
             * report error on same parent command, then the first one to
             * report would report failure on the parent command, and subsequent
             * ones would just log errors and return.
             */
            if (iCurrentCommand.size() > 0)
            {
                if (cmdContextData->parentCmd == iCurrentCommand.front().iCmd)
                {
                    /* pass up any extra error info, if any */
                    CommandComplete(iCurrentCommand,
                                    iCurrentCommand.front(),
                                    aResponse.GetCmdStatus(),
                                    aResponse.GetEventExtensionInterface());
                    return;
                }
            }
        }
    }

    switch (cmdContextData->cmd)
    {
        case PVMF_CPM_INTERNAL_INIT_CMD:
            iNumRegisteredPlugInInitComplete++;
            CompleteInitPlugIns();
            break;

        case PVMF_CPM_INTERNAL_QUERY_METADATA_EXTENSION_INTERFACE_CMD:
        {
            CPMPlugInParams* plugInParams =
                LookUpPlugInParams(cmdContextData->plugInID);
            if (plugInParams)
            {
                plugInParams->iPlugInMetaDataExtensionInterface =
                    OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*,
                                     plugInParams->iPlugInMetaDataExtensionInterfacePVI);
                plugInParams->iPlugInMetaDataExtensionInterfacePVI = NULL;
            }
            iNumQueryMetaDataExtensionInterfaceComplete++;
            CompleteMetaDataExtInterfaceQueryFromPlugIns();
        }
        break;

        case PVMF_CPM_INTERNAL_QUERY_AUTHENTICATION_INTERFACE_CMD:
        {
            CPMPlugInParams* plugInParams =
                LookUpPlugInParams(cmdContextData->plugInID);
            if (plugInParams)
            {
                plugInParams->iPlugInAuthenticationInterface =
                    OSCL_STATIC_CAST(PVMFCPMPluginAuthenticationInterface*,
                                     plugInParams->iPlugInAuthenticationInterfacePVI);
                plugInParams->iPlugInAuthenticationInterfacePVI = NULL;
            }
            iNumQueryAuthenticationInterfaceComplete++;
            CompleteCPMInit();
        }
        break;

        case PVMF_CPM_INTERNAL_AUTHENTICATE_CMD:
        {
            CPMSessionInfo* sessionInfo =
                LookUpSessionInfo(cmdContextData->sessionid);
            CompleteOpenSession(sessionInfo);
        }
        break;

        case PVMF_CPM_INTERNAL_QUERY_AUTHORIZATION_INTERFACE_CMD:
        {
            CPMPlugInParams* plugInParams =
                LookUpPlugInParamsFromActiveList(cmdContextData->plugInID);
            if (plugInParams)
            {
                plugInParams->iPlugInAuthorizationInterface =
                    OSCL_STATIC_CAST(PVMFCPMPluginAuthorizationInterface*,
                                     plugInParams->iPlugInAuthorizationInterfacePVI);
                plugInParams->iPlugInAuthorizationInterfacePVI = NULL;
            }
            CPMSessionInfo* sessionInfo =
                LookUpSessionInfo(cmdContextData->sessionid);
            CompleteRegisterContentPhase1(sessionInfo);
        }
        break;

        case PVMF_CPM_INTERNAL_QUERY_ACCESS_INTERFACE_FACTORY_CMD:
        {
            CPMPlugInParams* plugInParams =
                LookUpPlugInParamsFromActiveList(cmdContextData->plugInID);
            if (plugInParams)
            {
                plugInParams->iPlugInAccessInterfaceFactory =
                    OSCL_STATIC_CAST(PVMFCPMPluginAccessInterfaceFactory*,
                                     plugInParams->iPlugInAccessInterfaceFactoryPVI);
                plugInParams->iPlugInAccessInterfaceFactoryPVI = NULL;
            }
            CPMSessionInfo* sessionInfo =
                LookUpSessionInfo(cmdContextData->sessionid);
            CompleteRegisterContentPhase2(sessionInfo);
        }
        break;

        case PVMF_CPM_INTERNAL_QUERY_LICENSE_INTERFACE_CMD:
        {
            CPMPlugInParams* plugInParams =
                LookUpPlugInParamsFromActiveList(cmdContextData->plugInID);
            if (plugInParams)
            {
                plugInParams->iPlugInLicenseInterface =
                    OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*,
                                     plugInParams->iPlugInLicenseInterfacePVI);
                plugInParams->iPlugInLicenseInterfacePVI = NULL;
            }
            CPMSessionInfo* sessionInfo =
                LookUpSessionInfo(cmdContextData->sessionid);
            CompleteRegisterContentPhase3(sessionInfo);
        }
        break;

        case PVMF_CPM_INTERNAL_AUTHORIZE_CMD:
        {
            CPMContentUsageContext* usageContext =
                LookUpContentUsageContext(cmdContextData->usageid);
            CompleteApproveUsage(usageContext);
            CPMPlugInParams* plugInParams =
                LookUpPlugInParams(cmdContextData->plugInID);
            if (plugInParams)
                plugInParams->iAuthorized = true;
        }
        break;

        case PVMF_CPM_INTERNAL_USAGE_COMPLETE_CMD:
        {
            CPMContentUsageContext* usageContext =
                LookUpContentUsageContext(cmdContextData->usageid);
            CompleteUsageComplete(usageContext);
            CPMPlugInParams* plugInParams =
                LookUpPlugInParams(cmdContextData->plugInID);
            if (plugInParams)
                plugInParams->iAuthorized = false;
        }
        break;

        case PVMF_CPM_INTERNAL_RESET_CMD:
            iNumRegisteredPlugInResetComplete++;
            CompleteCPMReset();
            break;

        case PVMF_CPM_INTERNAL_GET_PLUGIN_META_DATA_KEYS_CMD:
            CompleteGetMetaDataKeys(cmdContextData->plugInID);
            break;

        case PVMF_CPM_INTERNAL_GET_PLUGIN_META_DATA_VALUES_CMD:
        {
            CompleteGetMetaDataValues(cmdContextData);
        }
        break;

        case PVMF_CPM_INTERNAL_GET_LICENSE_CMD:
        {
            CompleteGetLicense();
        }
        break;

        case PVMF_CPM_INTERNAL_QUERY_CAP_CONFIG_INTERFACE_CMD:
        {
            CPMPlugInParams* plugInParams =
                LookUpPlugInParams(cmdContextData->plugInID);
            if (plugInParams)
            {
                plugInParams->iPlugInCapConfigExtensionInterface =
                    OSCL_STATIC_CAST(PvmiCapabilityAndConfig*,
                                     plugInParams->iPlugInCapConfigExtensionInterfacePVI);
                plugInParams->iPlugInCapConfigExtensionInterfacePVI = NULL;
            }
            iNumQueryCapConfigExtensionInterfaceComplete++;
            CompleteCapConfigExtInterfaceQueryFromPlugIns();
        }
        break;

        default:
            break;
    }
    return;
}

CPMSessionInfo*
PVMFCPMImpl::LookUpSessionInfo(PVMFSessionId aID)
{
    Oscl_Vector<CPMSessionInfo, OsclMemAllocator>::iterator it;
    for (it = iListofActiveSessions.begin(); it != iListofActiveSessions.end(); it++)
    {
        if (it->iSessionId == aID)
        {
            return (it);
        }
    }
    return NULL;
}

CPMContentUsageContext* PVMFCPMImpl::LookUpContentUsageContext(PVMFCPMUsageID aID)
{
    Oscl_Vector<CPMContentUsageContext, OsclMemAllocator>::iterator it;
    for (it = iContentUsageContextVec.begin(); it != iContentUsageContextVec.end(); it++)
    {
        if (it->iUsageID == aID)
        {
            return (it);
        }
    }
    return NULL;
}

CPMPlugInParams* PVMFCPMImpl::LookUpPlugInParams(uint32 aID)
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iPlugInParamsVec.begin(); it != iPlugInParamsVec.end(); it++)
    {
        if (it->iPlugInID == aID)
        {
            return (it);
        }
    }
    return NULL;
}

CPMPlugInParams* PVMFCPMImpl::LookUpPlugInParamsFromActiveList(uint32 aID)
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iPlugInID == aID)
        {
            return (it);
        }
    }
    return NULL;
}

OSCL_EXPORT_REF uint32
PVMFCPMImpl::GetNumMetadataKeys(char* aQueryKeyString)
{
    uint32 numMetaDataKeys = 0;
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (NULL != it->iPlugInMetaDataExtensionInterface)
            {
                numMetaDataKeys +=
                    it->iPlugInMetaDataExtensionInterface->GetNumMetadataKeys(aQueryKeyString);
            }
        }
        return numMetaDataKeys;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::GetNumMetadataKeys - No Active Plugins"));
    return numMetaDataKeys;
}

OSCL_EXPORT_REF uint32
PVMFCPMImpl::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    uint32 numMetaDataValues = 0;
    if (iActivePlugInParamsVec.size() > 0)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (NULL != it->iPlugInMetaDataExtensionInterface)
            {
                numMetaDataValues +=
                    it->iPlugInMetaDataExtensionInterface->GetNumMetadataValues(aKeyList);
            }
        }
        return numMetaDataValues;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::GetNumMetadataValues - No Active Plugins"));
    return numMetaDataValues;
}

PVMFStatus
PVMFCPMImpl::DoGetMetadataKeys(PVMFCPMCommand& aCmd)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::DoGetMetadataKeys Called"));
    if (iActivePlugInParamsVec.size() == 0)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetMetadataKeys - No Active Plugins"));
        return PVMFErrInvalidState;
    }

    iGetMetaDataKeysInProgress = true;

    if (iGetMetaDataKeysFromPlugInsDone == false)
    {
        if (CheckForMetaDataInterfaceAvailability())
        {
            PVMFStatus status = QueryForMetaDataKeys(aCmd);
            if (status != PVMFSuccess)
            {
                PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetMetadataKeys - QueryForMetaDataKeys Failed"));
                return status;
            }
            return PVMFPending;
        }
    }
    return (CompleteDoGetMetadataKeys(aCmd));
}
PVMFStatus
PVMFCPMImpl::CompleteDoGetMetadataKeys(PVMFCPMCommand& aCmd)
{
    iGetMetaDataKeysInProgress = false;

    int32 leavecode = OsclErrNone;
    PVMFMetadataList* keylistptr = NULL;
    int32 starting_index;
    int32 max_entries;
    char* query_key = NULL;
    aCmd.PVMFCPMCommand::Parse(keylistptr,
                               starting_index,
                               max_entries,
                               query_key);

    /* Check parameters */
    if ((keylistptr == NULL) ||
            (starting_index < 0)  ||
            (max_entries == 0))
    {
        /* Invalid starting index and/or max entries */
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteDoGetMetadataKeys - Invalid Args"));
        return PVMFErrArgument;
    }

    /* Copy the requested keys from all active plugins */
    uint32 num_entries = 0;
    int32 num_added = 0;
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        it->iMetaDataKeyStartIndex = keylistptr->size();
        for (uint32 lcv = 0; lcv < it->iAvailableMetadataKeys.size(); lcv++)
        {
            if (query_key == NULL)
            {
                /* No query key so this key is counted */
                ++num_entries;
                if (num_entries > (uint32)starting_index)
                {
                    /* Past the starting index so copy the key */
                    leavecode = OsclErrNone;
                    leavecode = PushKVPKey(it->iAvailableMetadataKeys[lcv], *keylistptr);
                    if (OsclErrNone != leavecode)
                    {
                        return PVMFErrNoMemory;
                    }
                    num_added++;
                }
            }
            else
            {
                /* Check if the key matches the query key */
                if (pv_mime_strcmp((char*)it->iAvailableMetadataKeys[lcv].get_cstr(), query_key) >= 0)
                {
                    /* This key is counted */
                    ++num_entries;
                    if (num_entries > (uint32)starting_index)
                    {
                        /* Past the starting index so copy the key */
                        leavecode = OsclErrNone;
                        leavecode = PushKVPKey(it->iAvailableMetadataKeys[lcv], *keylistptr);
                        if (OsclErrNone != leavecode)
                        {
                            return PVMFErrNoMemory;
                        }
                        num_added++;
                    }
                }
            }
            /* Check if max number of entries have been copied */
            if ((max_entries > 0) && (num_added >= max_entries))
            {
                break;
            }
        }
        it->iMetaDataValueEndIndex = keylistptr->size();
    }
    return PVMFSuccess;
}

void
PVMFCPMImpl::DoGetMetadataValues(PVMFCPMCommand& aCmd)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::DoGetMetadataValues Called"));
    MoveCmdToCurrentQueue(aCmd);

    if (iActivePlugInParamsVec.size() == 0)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetMetadataValues - No Active Plugins"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFErrInvalidState);
        return;
    }

    iKeyListPtr = NULL;
    iValueListPtr = NULL;
    iGetMetaDataValuesStartingIndex = 0;
    iGetMetaDataValuesMaxEntries = 0;


    aCmd.PVMFCPMCommand::Parse(iKeyListPtr,
                               iValueListPtr,
                               iGetMetaDataValuesStartingIndex,
                               iGetMetaDataValuesMaxEntries);

    /* Check the parameters */
    if (iKeyListPtr == NULL || iValueListPtr == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetMetadataValues - Null Args"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFErrArgument);
        return;
    }

    uint32 numkeys = iKeyListPtr->size();

    if ((iGetMetaDataValuesStartingIndex < 0) ||
            (iGetMetaDataValuesStartingIndex > (int32)(numkeys - 1)) ||
            ((int32)numkeys <= 0) ||
            (iGetMetaDataValuesMaxEntries == 0))
    {
        /* Don't do anything */
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetMetadataValues - Invalid Args"));
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFErrArgument);
        return;
    }

    if (iActivePlugInParamsVec.size() > 0)
    {
        if (IsGetMetaDataValuesFromPlugInsComplete() == false)
        {
            CPMPlugInParams* plugInParams =
                LookUpNextPlugInForGetMetaDataValues();

            SendGetMetaDataValuesToPlugIn(plugInParams);
            return;
        }
        else
        {
            PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::DoGetMetadataValues - No Active Plugins With MetaData Support"));
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFSuccess);
            return;
        }
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetMetadataValues - No Active Plugins"));
    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFFailure);
    return;
}

CPMPlugInParams* PVMFCPMImpl::LookUpNextPlugInForGetMetaDataValues()
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iGetMetaDataValuesComplete == false)
        {
            return (it);
        }
    }
    return NULL;
}

bool PVMFCPMImpl::IsGetMetaDataValuesFromPlugInsComplete()
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iPlugInMetaDataExtensionInterface == NULL)
        {
            /* No metadata ext intf - so treat it as complete */
            it->iGetMetaDataValuesComplete = true;
        }
        else
        {
            if (it->iGetMetaDataValuesComplete == false)
            {
                return false;
            }
        }
    }
    return true;
}

void
PVMFCPMImpl::SendGetMetaDataValuesToPlugIn(CPMPlugInParams* aParams)
{
    if (aParams != NULL)
    {
        PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
        if (internalCmd != NULL)
        {
            internalCmd->cmd =
                PVMF_CPM_INTERNAL_GET_PLUGIN_META_DATA_VALUES_CMD;
            internalCmd->parentCmd = PVMF_CPM_GET_METADATA_VALUES;
            internalCmd->plugInID = aParams->iPlugInID;
            OsclAny *cmdContextData =
                OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

            aParams->iNumMetaDataValuesAvailable =
                aParams->iPlugInMetaDataExtensionInterface->GetNumMetadataValues((PVMFMetadataList&)(*iKeyListPtr));
            aParams->iMetaDataValueStartIndex = iValueListPtr->size();
            aParams->iPlugInMetaDataExtensionInterface->GetNodeMetadataValues(aParams->iPlugInSessionID,
                    (PVMFMetadataList&)(*iKeyListPtr),
                    (Oscl_Vector<PvmiKvp, OsclMemAllocator>&)(*iValueListPtr),
                    0,
                    aParams->iNumMetaDataValuesAvailable,
                    cmdContextData);
        }
        else
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::RequestMetaDataValues - SendGetMetaDataValuesToPlugIn Failed"));
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFErrNoMemory);
            return;
        }
    }
}

void
PVMFCPMImpl::CompleteGetMetaDataValues(PVMFCPMCommandContext* aContext)
{
    if (iActivePlugInParamsVec.size() > 0)
    {
        CPMPlugInParams* currPlugInParams =
            LookUpPlugInParamsFromActiveList(aContext->plugInID);
        OSCL_ASSERT(currPlugInParams);
        if (!currPlugInParams)
            return;	// unlikely: lookup failed.
        currPlugInParams->iMetaDataValueEndIndex = iValueListPtr->size();
        currPlugInParams->iGetMetaDataValuesComplete = true;

        if (IsGetMetaDataValuesFromPlugInsComplete() == false)
        {
            CPMPlugInParams* nextPlugInParams =
                LookUpNextPlugInForGetMetaDataValues();

            SendGetMetaDataValuesToPlugIn(nextPlugInParams);
        }
        else
        {
            /* Reset for future retrievals */
            Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
            for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
            {
                it->iGetMetaDataValuesComplete = false;
            }
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFSuccess);
        }
        return;
    }
    /* No active plugins */
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::CompleteGetMetaDataValues - No Active Plugins"));
    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFFailure);
    return;
}

PVMFStatus PVMFCPMImpl::getParametersSync(PvmiMIOSession aSession,
        PvmiKeyType aIdentifier,
        PvmiKvp*& aParameters,
        int& num_parameter_elements,
        PvmiCapabilityContext aContext)
{
    OSCL_UNUSED_ARG(aSession);
    OSCL_UNUSED_ARG(aIdentifier);
    OSCL_UNUSED_ARG(aParameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
    OSCL_UNUSED_ARG(aContext);

    return PVMFErrNotSupported;
}


PVMFStatus PVMFCPMImpl::releaseParameters(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    PVMFStatus status = PVMFFailure;
    if (aParameters)
    {
        Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInCapConfigExtensionInterface != NULL)
            {
                status =
                    it->iPlugInCapConfigExtensionInterface->releaseParameters(aSession,
                            aParameters,
                            num_elements);
            }
        }
    }
    return status;
}


void PVMFCPMImpl::setParametersSync(PvmiMIOSession aSession,
                                    PvmiKvp* aParameters,
                                    int num_elements,
                                    PvmiKvp*& aRet_kvp)
{
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iPlugInCapConfigExtensionInterface != NULL)
        {
            int32 err = OsclErrNone;
            OSCL_TRY(err,
                     it->iPlugInCapConfigExtensionInterface->setParametersSync(aSession,
                             aParameters,
                             num_elements,
                             aRet_kvp););
            /* Ignore error - Not all plugins need support all config params */
        }
    }
}


PVMFStatus PVMFCPMImpl::verifyParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements)
{
    PVMFStatus status = PVMFFailure;
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
    {
        if (it->iPlugInCapConfigExtensionInterface != NULL)
        {
            status =
                it->iPlugInCapConfigExtensionInterface->verifyParametersSync(aSession,
                        aParameters,
                        num_elements);
        }
    }
    return status;
}
PVMFCommandId
PVMFCPMImpl::GetLicense(PVMFSessionId aSessionId,
                        OSCL_wString& aContentName,
                        OsclAny* aData,
                        uint32 aDataSize,
                        int32 aTimeoutMsec,
                        OsclAny* aContextData)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:GetLicense - Wide"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_GET_LICENSE_W,
                  aContentName,
                  aData,
                  aDataSize,
                  aTimeoutMsec,
                  aContextData);
    return QueueCommandL(cmd);
}

PVMFCommandId
PVMFCPMImpl::GetLicense(PVMFSessionId aSessionId,
                        OSCL_String&  aContentName,
                        OsclAny* aData,
                        uint32 aDataSize,
                        int32 aTimeoutMsec,
                        OsclAny* aContextData)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:GetLicense"));
    PVMFCPMCommand cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_GET_LICENSE,
                  aContentName,
                  aData,
                  aDataSize,
                  aTimeoutMsec,
                  aContextData);
    return QueueCommandL(cmd);
}

PVMFCommandId
PVMFCPMImpl::CancelGetLicense(PVMFSessionId aSessionId, PVMFCommandId aCmdId, OsclAny* aContextData)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:CancelGetLicense"));
    PVMFCPMCommand cmd;
    cmd.PVMFCPMCommandBase::Construct(aSessionId,
                                      PVMF_CPM_CANCEL_GET_LICENSE,
                                      aCmdId,
                                      aContextData);
    return QueueCommandL(cmd);
}

PVMFStatus PVMFCPMImpl::GetLicenseStatus(
    PVMFCPMLicenseStatus& aStatus)
{
    if (iLicenseInterface)
        return iLicenseInterface->GetLicenseStatus(aStatus);
    return PVMFFailure;
}

PVMFStatus PVMFCPMImpl::DoGetLicense(PVMFCPMCommand& aCmd,
                                     bool aWideCharVersion)
{
    iLicenseInterface = NULL;
    CPMSessionInfo* sInfo = LookUpSessionInfo(aCmd.iSession);
    CPMPlugInParams* pluginParamsPtr = NULL;
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    if (sInfo != NULL)
    {
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInID == sInfo->iAccessPlugInID)
            {
                iLicenseInterface = it->iPlugInLicenseInterface;
                pluginParamsPtr = it;
            }
        }
    }

    if (iLicenseInterface == NULL)
    {
        PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetLicense - No License Interface"));
        return PVMFErrNotSupported;
    }

    if (aWideCharVersion == true)
    {
        OSCL_wString* contentName = NULL;
        OsclAny* data = NULL;
        uint32 dataSize = 0;
        int32 timeoutMsec = 0;
        aCmd.Parse(contentName,
                   data,
                   dataSize,
                   timeoutMsec);

        PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
        if (internalCmd != NULL)
        {
            internalCmd->cmd = PVMF_CPM_INTERNAL_GET_LICENSE_CMD;
            internalCmd->parentCmd = PVMF_CPM_GET_LICENSE_W;
            internalCmd->plugInID = pluginParamsPtr->iPlugInID;
            OsclAny *cmdContextData =
                OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
            iGetLicenseCmdId =
                iLicenseInterface->GetLicense(pluginParamsPtr->iPlugInSessionID,
                                              *contentName,
                                              data,
                                              dataSize,
                                              timeoutMsec,
                                              cmdContextData);
        }
        else
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetLicense - RequestNewInternalCmd Failed"));
            return PVMFErrNoMemory;
        }
    }
    else
    {
        OSCL_String* contentName = NULL;
        OsclAny* data = NULL;
        uint32 dataSize = 0;
        int32 timeoutMsec = 0;
        aCmd.Parse(contentName,
                   data,
                   dataSize,
                   timeoutMsec);
        PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
        if (internalCmd != NULL)
        {
            internalCmd->cmd = PVMF_CPM_INTERNAL_GET_LICENSE_CMD;
            internalCmd->parentCmd = PVMF_CPM_GET_LICENSE;
            internalCmd->plugInID = pluginParamsPtr->iPlugInID;
            OsclAny *cmdContextData =
                OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);
            iGetLicenseCmdId =
                iLicenseInterface->GetLicense(pluginParamsPtr->iPlugInSessionID,
                                              *contentName,
                                              data,
                                              dataSize,
                                              timeoutMsec,
                                              cmdContextData);
        }
        else
        {
            PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoGetLicense - RequestNewInternalCmd Failed"));
            return PVMFErrNoMemory;
        }
    }
    return PVMFPending;
}

void PVMFCPMImpl::CompleteGetLicense()
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl::CompleteGetLicense - Success"));
    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFSuccess);
}

void PVMFCPMImpl::DoCancelGetLicense(PVMFCPMCommand& aCmd)
{
    PVMF_CPM_LOGINFO((0, "PVMFCPMImpl:DoCancelGetLicense is called"));
    /* extract the command ID from the parameters.*/
    PVMFCommandId id;
    aCmd.PVMFCPMCommandBase::Parse(id);
    PVMFStatus status = PVMFErrArgument;

    iLicenseInterface = NULL;
    CPMSessionInfo* sInfo = LookUpSessionInfo(aCmd.iSession);
    CPMPlugInParams* pluginParamsPtr = NULL;
    Oscl_Vector<CPMPlugInParams, OsclMemAllocator>::iterator it;
    if (sInfo != NULL)
    {
        for (it = iActivePlugInParamsVec.begin(); it != iActivePlugInParamsVec.end(); it++)
        {
            if (it->iPlugInID == sInfo->iAccessPlugInID)
            {
                iLicenseInterface = it->iPlugInLicenseInterface;
                pluginParamsPtr = it;
            }
        }
    }

    /* first check "current" command if any */
    PVMFCPMCommand* cmd = iCurrentCommand.FindById(id);
    if (cmd)
    {
        if (cmd->iCmd == PVMF_CPM_GET_LICENSE_W || cmd->iCmd == PVMF_CPM_GET_LICENSE)
        {
            PVMFCPMCommandContext* internalCmd = RequestNewInternalCmd();
            if (internalCmd != NULL)
            {
                internalCmd->cmd = PVMF_CPM_INTERNAL_CANCEL_GET_LICENSE;
                internalCmd->parentCmd = PVMF_CPM_CANCEL_GET_LICENSE;

                OSCL_ASSERT(pluginParamsPtr);

                if (!pluginParamsPtr)

                {

                    status = PVMFErrCorrupt;

                    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoCancelGetLicense - data corrupted"));

                    CommandComplete(iInputCommands, aCmd, status);

                    return;

                }

                internalCmd->plugInID = pluginParamsPtr->iPlugInID;

                OsclAny *cmdContextData =
                    OSCL_REINTERPRET_CAST(OsclAny*, internalCmd);

                iLicenseInterface->CancelGetLicense(pluginParamsPtr->iPlugInSessionID, iGetLicenseCmdId, cmdContextData);

                /*
                 * the queued commands are all asynchronous commands to the
                 * CPM module. CancelGetLicense can cancel only for GetLicense cmd.
                 * We need to wait CPMPluginCommandCompleted.
                 */
                MoveCmdToCancelQueue(aCmd);
                return;
            }
            else
            {
                PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoCancelGetLicense - RequestNewInternalCmd Failed"));
                status = PVMFErrNoMemory;
            }
        }
    }
    PVMF_CPM_LOGERROR((0, "PVMFCPMImpl::DoCancelGetLicense - current cmd is not GetLicense"));
    CommandComplete(iInputCommands, aCmd, status);
    return;
}

OSCL_EXPORT_REF bool CPMPluginRegistryImpl::addPluginToRegistry(OSCL_String& aMimeType,
        CPMPluginContainer& aPlugInContainer)
{
    CPMPluginContainer*container = lookupPlugin(aMimeType);
    if (container)
        return false;//duplicate mime type!
    container = OSCL_NEW(CPMPluginContainer, (aPlugInContainer));
    iCPMPluginRegistry.insert(value_type(aMimeType, container));
    iListofPlugInMimeTypes.push_back(aMimeType);
    return true;
}

OSCL_EXPORT_REF void CPMPluginRegistryImpl::removePluginFromRegistry(OSCL_String& aMimeType)
{
    CPMPluginContainer*container = lookupPlugin(aMimeType);
    if (container)
    {
        OSCL_DELETE(container);
    }
    for (uint32 i = 0;i < iListofPlugInMimeTypes.size();i++)
    {
        if (iListofPlugInMimeTypes[i] == aMimeType)
        {
            iListofPlugInMimeTypes.erase(&iListofPlugInMimeTypes[i]);
            iCPMPluginRegistry.erase(iCPMPluginRegistry.find(aMimeType));
        }
    }
}

OSCL_EXPORT_REF CPMPluginContainer* CPMPluginRegistryImpl::lookupPlugin(OSCL_String& aMimeType)
{
    Oscl_Map<string_key_type, CPMPluginContainer*, OsclMemAllocator, string_key_compare_class>::iterator it;
    it = iCPMPluginRegistry.find(aMimeType);

    /* Workaround for the ADS1.2 compiler*/
    if (!(it == iCPMPluginRegistry.end()))
    {
        return (((*it).second));
    }
    return NULL;
}

OSCL_EXPORT_REF uint32 CPMPluginRegistryImpl::GetNumPlugIns()
{
    return (iCPMPluginRegistry.size());
}

OSCL_EXPORT_REF bool CPMPluginRegistryImpl::GetPluginMimeType(uint32 aIndex, OSCL_String& aMimeType)
{
    if (aIndex < GetNumPlugIns())
    {
        aMimeType = iListofPlugInMimeTypes[aIndex];
        return true;
    }
    return false;//invalid index.
}

OSCL_EXPORT_REF CPMPluginRegistryImpl::CPMPluginRegistryImpl()
{
    iRefCount = 0;
    iSharedLibList = NULL;
}

#ifdef USE_LOADABLE_MODULES
#include "oscl_shared_library.h"
#endif

OSCL_EXPORT_REF CPMPluginRegistryImpl::~CPMPluginRegistryImpl()
{
    //just in case plugins weren't removed, go through and cleanup
    for (uint32 i = 0;i < iListofPlugInMimeTypes.size();i++)
    {
        CPMPluginContainer* container = lookupPlugin(iListofPlugInMimeTypes[i]);
        if (container)
            OSCL_DELETE(container);
    }
#ifdef USE_LOADABLE_MODULES
    if (iSharedLibList)
        OSCL_DELETE(iSharedLibList);
#endif
}

CPMPluginRegistry* CPMPluginRegistryFactory::CreateCPMPluginRegistry()
{
    return (CPMPluginRegistry*)OSCL_NEW(CPMPluginRegistryImpl, ());
}

void CPMPluginRegistryFactory::DestroyCPMPluginRegistry(CPMPluginRegistry* aReg)
{
    CPMPluginRegistryImpl* impl = (CPMPluginRegistryImpl*)aReg;
    OSCL_DELETE(impl);
}

int32 PVMFCPMImpl::PushKVPKey(OSCL_String& aString, PVMFMetadataList& aKeyList)
{
    int32 leavecode = OsclErrNone;
    OSCL_TRY(leavecode, aKeyList.push_back(aString));
    return leavecode;
}
