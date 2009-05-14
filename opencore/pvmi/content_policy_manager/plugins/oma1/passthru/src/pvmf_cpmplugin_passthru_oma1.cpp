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
#ifndef PVMF_CPMPLUGIN_PASSTHRU_OMA1_H_INCLUDED
#include "pvmf_cpmplugin_passthru_oma1.h"
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
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
#include "pvmf_local_data_source.h"
#include "pvmf_cpmplugin_passthru_oma1_factory.h"
#include "pvmf_cpmplugin_passthru_oma1_types.h"

#ifndef PVMI_DRM_KVP_H_INCLUDED
#include "pvmi_drm_kvp.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

OSCL_EXPORT_REF PVMFOma1PassthruPluginFactory::PVMFOma1PassthruPluginFactory()
{
    iFailAuthorizeUsage = false; // To simulate "authorizeusage" failure
    iCancelAcquireLicense = false; //to simulate "cancelgetLicense"
    iCPMContentType = PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS; //default value
    iSourceInitDataNotSupported = false; //To simulate SourceInitializationData failure
}

//This constructor has only been introduced for test case scenarios simulation
OSCL_EXPORT_REF PVMFOma1PassthruPluginFactory::PVMFOma1PassthruPluginFactory(bool aAuthorizeUsage, bool aCancelAcquireLicense,
        bool aSourceInitDataNotSupported,
        PVMFCPMContentType aCPMContentType)
{
    iFailAuthorizeUsage = aAuthorizeUsage;// To simulate "authorizeusage" failure
    iCancelAcquireLicense = aCancelAcquireLicense; //To simulate "cancelAcquire"
    iCPMContentType = aCPMContentType; //what test case sets
    iSourceInitDataNotSupported = aSourceInitDataNotSupported;
}

//Oma1 Passthru plugin factory.
OSCL_EXPORT_REF PVMFCPMPluginInterface* PVMFOma1PassthruPluginFactory::CreateCPMPlugin()
{
    return PVMFCPMPassThruPlugInOMA1::CreatePlugIn(iFailAuthorizeUsage, iCancelAcquireLicense,
            iSourceInitDataNotSupported, iCPMContentType);
}

OSCL_EXPORT_REF void PVMFOma1PassthruPluginFactory::DestroyCPMPlugin(PVMFCPMPluginInterface* aPlugIn)
{
    PVMFCPMPassThruPlugInOMA1::DestroyPlugIn(aPlugIn);
}

PVMFCPMPluginInterface* PVMFCPMPassThruPlugInOMA1::CreatePlugIn(bool aFailAuthorizeUsage, bool aCancelAcquireLicense, bool aSourceInitDataNotSupported, PVMFCPMContentType aCPMContentType)
{
    PVMFCPMPassThruPlugInOMA1* plugIn = NULL;
    int32 err;
    OSCL_TRY(err,
             /*
              * Create pass thru OMA1.0 Plugin
              */
             plugIn = OSCL_NEW(PVMFCPMPassThruPlugInOMA1, (aFailAuthorizeUsage, aCancelAcquireLicense, aSourceInitDataNotSupported, aCPMContentType));
            );
    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }

    return (OSCL_STATIC_CAST(PVMFCPMPluginInterface*, plugIn));
}

void PVMFCPMPassThruPlugInOMA1::DestroyPlugIn(PVMFCPMPluginInterface* aPlugIn)
{
    OSCL_DELETE(aPlugIn);
}

/**
 * Plugin Constructor & Destructor
 */
OSCL_EXPORT_REF PVMFCPMPassThruPlugInOMA1::PVMFCPMPassThruPlugInOMA1(bool aFailAuthorizeUsage, bool aCancelAcquireLicense,
        bool aSourceInitDataNotSupported, PVMFCPMContentType aCPMContentType,
        int32 aPriority)
        : OsclActiveObject(aPriority, "PVMFCPMPassThruPlugInOMA1"),
        iExtensionRefCount(0)
{
    iLogger = NULL;
    oSourceSet = false;
    iFileHandle = NULL;
    iDataStreamReadCapacityObserver = NULL;
    // To simulate "authorizeusage" failure
    iFailAuthorizeUsage = aFailAuthorizeUsage;
    //to simualte cancelGetLic
    iCancelAcquireLicense = aCancelAcquireLicense;
    //to simulate "notsupported" content
    iSourceInitDataNotSupported = aSourceInitDataNotSupported;

    iCPMContentType = aCPMContentType;
    int32 err;
    OSCL_TRY(err,
             /*
              * Create the input command queue.  Use a reserve to avoid lots of
              * dynamic memory allocation
              */
             iInputCommands.Construct(PVMF_CPM_PASSTHRU_PLUGIN_OMA1_COMMAND_ID_START,
                                      PVMF_CPM_PASSTHRU_PLUGIN_OMA1_INTERNAL_CMDQ_SIZE);

             /*
              * Create the "current command" queue.  It will only contain one
              * command at a time, so use a reserve of 1
              */
             iCurrentCommand.Construct(0, 1);

            );
    if (err != OsclErrNone)
    {
        OSCL_LEAVE(err);
    }
    //set the fixed metadata query and value strings.
    iMetaData[EIsProtectedOMA1].iQuery = PVMF_DRM_INFO_IS_PROTECTED_QUERY;
    iMetaData[EIsProtectedOMA1].iValue = PVMF_DRM_INFO_IS_PROTECTED_VALUE;

    iMetaData[ELicAvailableOMA1].iQuery = PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_QUERY;
    iMetaData[ELicAvailableOMA1].iValue = PVMF_DRM_INFO_IS_LICENSE_AVAILABLE_VALUE;

    iMetaData[ELicTypeOMA1].iQuery = PVMF_DRM_INFO_LICENSE_TYPE_QUERY;
    iMetaData[ELicTypeOMA1].iValue = PVMF_DRM_INFO_LICENSE_TYPE_VALUE;

    iMetaData[EForwardLockOMA1].iQuery = PVMF_DRM_INFO_IS_FORWARD_LOCKED_QUERY;
    iMetaData[EForwardLockOMA1].iValue = PVMF_DRM_INFO_IS_FORWARD_LOCKED_VALUE;

    for (uint32 i = 0;i < ELicLastOMA1;i++)
    {
        iMetaData[i].iKvp.key = iMetaData[i].iValue.get_str();
        iMetaData[i].iKvp.length = 0;
        iMetaData[i].iKvp.capacity = 0;
    }

}

PVMFCPMPassThruPlugInOMA1::~PVMFCPMPassThruPlugInOMA1()
{
    oSourceSet = false;
    /*
     * Cleanup commands
     * The command queues are self-deleting, but we want to
     * notify the observer of unprocessed commands.
     */
    while (!iCurrentCommand.empty())
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFFailure);
    }
    while (!iInputCommands.empty())
    {
        CommandComplete(iInputCommands, iInputCommands.front(), PVMFFailure);
    }
}

void PVMFCPMPassThruPlugInOMA1::ThreadLogon()
{
    if (!iLoggedOn)
    {
        iLogger = PVLogger::GetLoggerObject("PVMFCPMPassThruPlugInOMA1");
        AddToScheduler();
        iLoggedOn = true;
    }
}

void PVMFCPMPassThruPlugInOMA1::ThreadLogoff()
{
    if (iLoggedOn)
    {
        iLogger = NULL;
        Cancel();
        RemoveFromScheduler();
        iLoggedOn = false;
    }
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::QueryUUID(PVMFSessionId s,
                                     const PvmfMimeString& aMimeType,
                                     Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
                                     bool aExactUuidsOnly,
                                     const OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:QueryUUID"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_QUERYUUID,
            aMimeType,
            aUuids,
            aExactUuidsOnly,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::QueryInterface(PVMFSessionId s,
        const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:QueryInterface"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_QUERYINTERFACE,
            aUuid,
            aInterfacePtr,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:Init"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_INIT,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:Reset"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_RESET,
            aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::AuthenticateUser(PVMFSessionId aSessionId,
        OsclAny* aAuthenticationData,
        OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:AuthenticateUser"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_PASSTHRU_PLUGIN_OMA1_AUTHENTICATE,
                  aAuthenticationData,
                  NULL,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::AuthorizeUsage(PVMFSessionId aSessionId,
        PvmiKvp& aRequestedUsage,
        PvmiKvp& aApprovedUsage,
        PvmiKvp& aAuthorizationData,
        uint32&  aRequestTimeOutInMS,
        OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:AuthorizeUsage"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_PASSTHRU_PLUGIN_OMA1_AUTHORIZE_USAGE,
                  &aRequestedUsage,
                  &aApprovedUsage,
                  &aAuthorizationData,
                  &aRequestTimeOutInMS,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::UsageComplete(PVMFSessionId s, const OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:UsageComplete"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(s,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_USAGE_COMPLETE,
            aContext);
    return QueueCommandL(cmd);
}

PVMFStatus
PVMFCPMPassThruPlugInOMA1::SetSourceInitializationData(OSCL_wString& aSourceURL,
        PVMFFormatType& aSourceFormat,
        OsclAny* aSourceData)
{
    OSCL_UNUSED_ARG(aSourceData);

    //In a real OMA1 plugin, we would be able to verify whether the source is really OMA1
    //content or not.  This passthru plugin is not smart enough to do that, so it
    //generally accepts all formats.  However, it rejects ASF content to avoid conflicts
    //with the PV Janus plugin.
    if (aSourceFormat == PVMF_MIME_ASFFF || PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL)
    {
        return PVMFErrNotSupported;
    }
    else
    {
        iFilename = aSourceURL;
        iSourceFormatType = aSourceFormat;
        /*
        if(aSourceData)
        {
        	iFileHandle=((PVMFLocalDataSource*)aSourceData)->iFileHandle;
        }
        else
        {
        	iFileHandle=NULL;
        }
        */
        if (iSourceInitDataNotSupported)
        {
            iSourceInitDataNotSupported = false; //reset for next time
            return PVMFErrNotSupported;
        }
        oSourceSet = true;
        return PVMFSuccess;
    }
}

PVMFCPMContentType
PVMFCPMPassThruPlugInOMA1::GetCPMContentType()
{
    return iCPMContentType;
}

PVMFStatus PVMFCPMPassThruPlugInOMA1::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}

PVInterface*
PVMFCPMPassThruPlugInOMA1::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl* iface = NULL;
        iface = OSCL_NEW(PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl, (*this));

        PVMIDataStreamSyncInterface* interimPtr1
        = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, iface);

        return (OSCL_STATIC_CAST(PVInterface*, interimPtr1));
    }

    return NULL;
}

void
PVMFCPMPassThruPlugInOMA1::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid,
        PVInterface* aPtr)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* interimPtr =
            (PVMIDataStreamSyncInterface*)aPtr;
        PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl* datastream =
            (PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl*)interimPtr;
        OSCL_DELETE(datastream);
    }
    else
    {
        OSCL_ASSERT(false);
    }
}

void
PVMFCPMPassThruPlugInOMA1::SetStreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObs)
{
    iDataStreamReadCapacityObserver = aObs;
}


/**
 * This routine is called by various command APIs to queue an
 * asynchronous command for processing by the command handler AO.
 * This function may leave if the command can't be queued due to
 * memory allocation failure.
 */
PVMFCommandId PVMFCPMPassThruPlugInOMA1::QueueCommandL(PVMFCPMPassThruPlugInOMA1Command& aCmd)
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
void PVMFCPMPassThruPlugInOMA1::CommandComplete(PVMFCPMPassThruPlugInOMA1CmdQ& aCmdQ,
        PVMFCPMPassThruPlugInOMA1Command& aCmd,
        PVMFStatus aStatus,
        OsclAny* aEventData,
        PVUuid* aEventUUID,
        int32* aEventCode)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:CommandComplete Id %d Cmd %d Status %d Context %d Data %d"
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
    PVMFSessionId session = aCmd.iSession;

    /* Erase the command from the queue */
    aCmdQ.Erase(&aCmd);

    /* Report completion to the session observer */
    ReportCmdCompleteEvent(session, resp);

    if (errormsg)
    {
        errormsg->removeRef();
    }
    //Reschedule if there are more commands.
    if (!iInputCommands.empty()
            && IsAdded())
    {
        RunIfNotReady();
    }
}

void
PVMFCPMPassThruPlugInOMA1::MoveCmdToCurrentQueue(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    int32 err;
    OSCL_TRY(err, iCurrentCommand.StoreL(aCmd););
    if (err != OsclErrNone)
    {
        PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGERROR((0, "PVMFCPMPassThruPlugInOMA1:MoveCmdToCurrentQueue - No Memory"));
        CommandComplete(iInputCommands, aCmd, PVMFErrNoMemory);
        return;
    }
    iInputCommands.Erase(&aCmd);
    return;
}

/**
 * Called by the command handler AO to process a command from
 * the input queue.
 * Return true if a command was processed, false if the command
 * processor is busy and can't process another command now.
 */
bool PVMFCPMPassThruPlugInOMA1::ProcessCommand(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    /*
     * normally this node will not start processing one command
     * until the prior one is finished.  However, a hi priority
     * command such as Cancel must be able to interrupt a command
     * in progress.
     */
    if (!iCurrentCommand.empty() && !aCmd.hipri() && aCmd.iCmd != PVMF_CPM_PASSTHRU_PLUGIN_OMA1_CANCEL_GET_LICENSE)
        return false;

    switch (aCmd.iCmd)
    {
        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_QUERYUUID:
            DoQueryUuid(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_QUERYINTERFACE:
            DoQueryInterface(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_INIT:
            DoInit(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_RESET:
            DoReset(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_AUTHENTICATE:
            DoAuthenticate(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_AUTHORIZE_USAGE:
            DoAuthorizeUsage(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_USAGE_COMPLETE:
            DoUsageComplete(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_GET_LICENSE:
        {
            PVMFStatus aCmdStatus = DoGetLicense(aCmd);
            switch (aCmdStatus)
            {
                case PVMFPending:
                    MoveCmdToCurrentQueue(aCmd);
                    break;
            }
        }
        break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_CANCEL_GET_LICENSE:
            DoCancelGetLicense(aCmd);
            break;


        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_METADATA_KEYS:
            DoMetadataKeys(aCmd);
            break;

        case PVMF_CPM_PASSTHRU_PLUGIN_OMA1_METADATA_VALUES:
            DoMetadataValues(aCmd);
            break;

        default:
        {
            /* unknown command type */
            PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGWARNING((0, "PVMFCPMPassThruPlugInOMA1:ProcessCommand - Unknown Command"));
            CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
        }
        break;
    }

    return true;
}

/**
 * Called by the command handler AO to do the Query UUID
 */
void PVMFCPMPassThruPlugInOMA1::DoQueryUuid(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    aCmd.PVMFCPMPassThruPlugInOMA1CommandBase::Parse(mimetype, uuidvec, exactmatch);

    if (exactmatch == false)
    {
        CommandComplete(iInputCommands, aCmd, PVMFErrArgument);
    }
    /*
     * Try to match the input mimetype against any of
     * the interfaces supported by this plugin
     */
    if (*mimetype == PVMF_CPMPLUGIN_AUTHENTICATION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginAuthenticationInterfaceUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_CPMPLUGIN_AUTHORIZATION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginAuthorizationInterfaceUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_MIMETYPE)
    {
        PVUuid uuid(PVMFCPMPluginAccessInterfaceFactoryUuid);
        uuidvec->push_back(uuid);
    }
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
 * Called by the command handler AO to do the Query Interface.
 */
void PVMFCPMPassThruPlugInOMA1::DoQueryInterface(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    PVUuid* uuid;
    PVInterface** ptr;
    aCmd.PVMFCPMPassThruPlugInOMA1CommandBase::Parse(uuid, ptr);

    if ((*uuid == PVMFCPMPluginAuthorizationInterfaceUuid) ||
            (*uuid == PVMFCPMPluginAccessInterfaceFactoryUuid))
    {
        if (oSourceSet == false)
        {
            CommandComplete(iInputCommands, aCmd, PVMFErrInvalidState);
            return;
        }
    }

    if (queryInterface(*uuid, *ptr))
    {
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    }
    else
    {
        /* not supported */
        *ptr = NULL;
        CommandComplete(iInputCommands, aCmd, PVMFErrNotSupported);
    }
}

bool PVMFCPMPassThruPlugInOMA1::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (uuid == PVMFCPMPluginAuthenticationInterfaceUuid)
    {
        PVMFCPMPluginAuthenticationInterface* myInterface =
            OSCL_STATIC_CAST(PVMFCPMPluginAuthenticationInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFCPMPluginAuthorizationInterfaceUuid)
    {
        PVMFCPMPluginAuthorizationInterface* myInterface =
            OSCL_STATIC_CAST(PVMFCPMPluginAuthorizationInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFCPMPluginAccessInterfaceFactoryUuid)
    {
        PVMFCPMPluginAccessInterfaceFactory* myInterface =
            OSCL_STATIC_CAST(PVMFCPMPluginAccessInterfaceFactory*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFCPMPluginLicenseInterfaceUuid)
    {
        PVMFCPMPluginLicenseInterface* myInterface =
            OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == KPVMFMetadataExtensionUuid)
    {
        PVMFMetadataExtensionInterface* myInterface
        = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }
    addRef();
    return true;
}

void PVMFCPMPassThruPlugInOMA1::DoInit(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    /* Nothing to do */
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMPassThruPlugInOMA1::DoReset(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    oSourceSet = false;
    /* Nothing to do */
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMPassThruPlugInOMA1::DoAuthenticate(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    OsclAny* authenticationData;
    OsclAny* dummy;
    aCmd.Parse(authenticationData, dummy);

    /* Return success always, till siemens api become available */
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMPassThruPlugInOMA1::DoAuthorizeUsage(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    if (iFailAuthorizeUsage)
    {
        iFailAuthorizeUsage = false; //reset for next try
        CommandComplete(iInputCommands, aCmd, PVMFErrLicenseRequired);
        return;
    }
    MakeMetadata();

    OsclAny* temp1 = NULL;
    OsclAny* temp2 = NULL;
    OsclAny* temp3 = NULL;
    OsclAny* temp4 = NULL;

    aCmd.Parse(temp1, temp2, temp3, temp4);

    PvmiKvp* requestedUsage = OSCL_STATIC_CAST(PvmiKvp*, temp1);
    PvmiKvp* approvedUsage = OSCL_STATIC_CAST(PvmiKvp*, temp2);
    PvmiKvp* authorizationData = OSCL_STATIC_CAST(PvmiKvp*, temp3);
    uint32*  requestTimeOutInMS = OSCL_STATIC_CAST(uint32*, temp4);

    /* No check needed - approve everything */
    approvedUsage->value.uint32_value = requestedUsage->value.uint32_value;
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
    OSCL_UNUSED_ARG(authorizationData);
    OSCL_UNUSED_ARG(requestTimeOutInMS);
}

void PVMFCPMPassThruPlugInOMA1::DoUsageComplete(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

PVMFStatus PVMFCPMPassThruPlugInOMA1::DoGetLicense(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    if (!iCancelAcquireLicense)
    {
        CommandComplete(iInputCommands, aCmd, PVMFSuccess);
        return PVMFSuccess;
    }
    else
    {
        iCancelAcquireLicense = false; //Reset for next time
        return PVMFPending;
    }
}

void PVMFCPMPassThruPlugInOMA1::DoCancelGetLicense(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    //Complete all pending commands, basically "GetLicense"
    while (iCurrentCommand.size() > 0)
    {
        CommandComplete(iCurrentCommand, iCurrentCommand.front(), PVMFErrCancelled);
    }
    //complete the CancelGetLicense
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMPassThruPlugInOMA1::DoMetadataKeys(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

void PVMFCPMPassThruPlugInOMA1::DoMetadataValues(PVMFCPMPassThruPlugInOMA1Command& aCmd)
{
    CommandComplete(iInputCommands, aCmd, PVMFSuccess);
}

/**
  * This AO handles API commands. The AO will process one command
  * per call. It will re-schedule itself and run continuously
  * until it runs out of things to do.
  */
void PVMFCPMPassThruPlugInOMA1::Run()
{
    /*
     * Process commands.
     */
    if (!iInputCommands.empty())
    {
        ProcessCommand(iInputCommands.front());
    }
}

OSCL_EXPORT_REF PVMFStatus
PVMFCPMPassThruPlugInOMA1::GetLicenseURL(PVMFSessionId aSessionId,
        OSCL_wString& aContentName,
        OSCL_wString& aLicenseURL)
{
    OSCL_UNUSED_ARG(aSessionId);
    OSCL_UNUSED_ARG(aContentName);
    OSCL_UNUSED_ARG(aLicenseURL);
    return PVMFErrNotSupported;
}

OSCL_EXPORT_REF PVMFStatus
PVMFCPMPassThruPlugInOMA1::GetLicenseURL(PVMFSessionId aSessionId,
        OSCL_String&  aContentName,
        OSCL_String&  aLicenseURL)
{
    OSCL_UNUSED_ARG(aSessionId);
    OSCL_UNUSED_ARG(aContentName);
    OSCL_UNUSED_ARG(aLicenseURL);
    return PVMFErrNotSupported;
}

OSCL_EXPORT_REF PVMFCommandId PVMFCPMPassThruPlugInOMA1::GetLicense(PVMFSessionId aSessionId
        , OSCL_wString& aContentName
        , OsclAny* aLicenseData
        , uint32 aDataSize
        , int32 aTimeoutMsec
        , OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:GetLicense"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_PASSTHRU_PLUGIN_OMA1_GET_LICENSE,
                  &aContentName,
                  aLicenseData,
                  &aDataSize,
                  &aTimeoutMsec,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId PVMFCPMPassThruPlugInOMA1::GetLicense(PVMFSessionId aSessionId
        , OSCL_String&  aContentName
        , OsclAny* aLicenseData
        , uint32 aDataSize
        , int32 aTimeoutMsec
        , OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1:GetLicense"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.Construct(aSessionId,
                  PVMF_CPM_PASSTHRU_PLUGIN_OMA1_GET_LICENSE,
                  &aContentName,
                  aLicenseData,
                  &aDataSize,
                  &aTimeoutMsec,
                  aContext);
    return QueueCommandL(cmd);
}

OSCL_EXPORT_REF PVMFCommandId
PVMFCPMPassThruPlugInOMA1::CancelGetLicense(PVMFSessionId aSessionId, PVMFCommandId aCmdId, OsclAny* aContext)
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGINFO((0, "PVMFCPMPassThruPlugInOMA1::CancelGetLicense - called"));
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(aSessionId, PVMF_CPM_PASSTHRU_PLUGIN_OMA1_CANCEL_GET_LICENSE, aCmdId, aContext);
    return QueueCommandL(cmd);
}

/**
 * Synchronous method to return the number of metadata keys for the specified query key string
 *
 * @param aQueryKeyString A NULL terminated character string specifying a subset of metadata keys to count.
 *                        If the string is NULL, total number of all keys will be returned
 *
 * @returns The number of metadata keys
 **/
OSCL_EXPORT_REF uint32 PVMFCPMPassThruPlugInOMA1::GetNumMetadataKeys(char* aQueryKeyString)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::GetNumMetadataKeys() called"));
    uint32 total = 0;
    for (uint32 i = 0;i < ELicLastOMA1;i++)
    {
        if (iMetaData[i].iValid)
        {
            if (!aQueryKeyString
                    || iMetaData[i].iQuery == aQueryKeyString)
            {
                total++;
            }
        }
    }
    return total;
}


/**
 * Synchronous method to return the number of metadata values for the specified list of key strings
 *
 * @param aKeyList A reference to a metadata key list to specifying the values of interest to count
 *
 * @returns The number of metadata values based on the provided key list
 **/
OSCL_EXPORT_REF uint32 PVMFCPMPassThruPlugInOMA1::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::GetNumMetadataValues() called"));
    uint32 total = 0;
    for (uint32 i = 0;i < aKeyList.size();i++)
    {
        for (uint32 j = 0;j < ELicLastOMA1;j++)
        {
            if (iMetaData[j].iValid
                    && iMetaData[j].iQuery == aKeyList[i])
            {
                total++;
                break;//goto next query.
            }
        }
    }
    return total;
}

/**
 * Asynchronous method to retrieve a list of metadata keys. The subset of all available keys in the node can
 * be specified by providing a combination of query key string, starting index, and maximum number of keys to retrieve
 *
 * @param aSessionId The assigned node session ID to use for this request
 * @param aKeyList A reference to a metadata key list to add the metadata keys
 * @param aStartingKeyIndex Index into the node's total key list that corresponds to the first key entry to retrieve
 * @param aMaxKeyEntries The maximum number of key entries to add to aKeyList. If there is no maximum, set to -1.
 * @param aQueryKeyString Optional NULL terminated character string to select a subset of keys
 * @param aContext Optional opaque data to be passed back to user with the command response
 *
 * @returns A unique command ID for asynchronous completion
 **/
OSCL_EXPORT_REF PVMFCommandId PVMFCPMPassThruPlugInOMA1::GetNodeMetadataKeys(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        uint32 aStartingKeyIndex,
        int32 aMaxKeyEntries ,
        char* aQueryKeyString ,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::GetNodeMetadataKeys() called"));
    int32 total = 0;
    for (uint32 i = aStartingKeyIndex;i < ELicLastOMA1;i++)
    {
        if (iMetaData[i].iValid)
        {
            if (aMaxKeyEntries >= 0
                    && total >= aMaxKeyEntries)
                break;
            if (!aQueryKeyString
                    || iMetaData[i].iQuery == aQueryKeyString)
            {
                aKeyList.push_back(iMetaData[i].iQuery);
                total++;
            }
        }
    }
    //MyCmdResp resp(aSessionId,aContextData);
    //return QueueCmdResp(resp,PVMFSuccess);
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(aSessionId,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_METADATA_KEYS,
            aContextData);
    return QueueCommandL(cmd);
}

/**
 * Asynchronous method to retrieve a list of metadata values. The subset of all available values in the node can
 * be specified by providing a combination of key list, starting index, and maximum number of values to retrieve
 *
 * @param aSessionId The assigned node session ID to use for this request
 * @param aKeyList A reference to a metadata key list specifying the metadata values to retrieve
 * @param aValueList A reference to a metadata value list to add the metadata values
 * @param aStartingValueIndex Index into the node's value list specified by the key list that corresponds to the first value entry to retrieve
 * @param aMaxValueEntries The maximum number of value entries to add to aValueList. If there is no maximum, set to -1.
 * @param aContext Optional opaque data to be passed back to user with the command response
 *
 * @returns A unique command ID for asynchronous completion
 **/
OSCL_EXPORT_REF PVMFCommandId PVMFCPMPassThruPlugInOMA1::GetNodeMetadataValues(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 aStartingValueIndex,
        int32 aMaxValueEntries ,
        const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::GetNodeMetadataValues() called"));
    int32 total = 0;
    for (uint32 j = 0;j < aKeyList.size();j++)
    {
        for (int32 i = aStartingValueIndex;i < ELicLastOMA1;i++)
        {
            if (iMetaData[i].iValid)
            {
                if (aMaxValueEntries >= 0
                        && total >= aMaxValueEntries)
                    break;
                if (iMetaData[i].iQuery == aKeyList[j])
                {
                    aValueList.push_back(iMetaData[i].iKvp);
                    total++;
                    break;//go to next key.
                }
            }
        }
    }
    PVMFCPMPassThruPlugInOMA1Command cmd;
    cmd.PVMFCPMPassThruPlugInOMA1CommandBase::Construct(aSessionId,
            PVMF_CPM_PASSTHRU_PLUGIN_OMA1_METADATA_VALUES,
            aContextData);
    return QueueCommandL(cmd);
}

/**
 * Synchronous method to free up the specified range of metadata keys in the list. It is assumed that caller of this function
 * knows that start and end indices should correspond to metadata keys returned by this particular instance of the
 * metadata extension interface using GetNodeMetadataKeys().
 *
 * @param aKeyList A reference to a metadata key list to free the key entries
 * @param aStartingKeyIndex Index into aKeyList that corresponds to the first key entry to release
 * @param aEndKeyIndex Index into aKeyList that corresponds to the last key entry to release
 *
 * @returns PVMFSuccess if the release of specified keys succeeded. PVMFErrArgument if indices are invalid or the list is empty.
 *          PVMFFailure otherwise.
 **/
OSCL_EXPORT_REF PVMFStatus PVMFCPMPassThruPlugInOMA1::ReleaseNodeMetadataKeys(PVMFMetadataList& aKeyList,
        uint32 aStartingKeyIndex,
        uint32 aEndKeyIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::ReleaseNodeMetadataKeys() called"));
    //nothing needed.  metadata memory persists through the life of the plugin.
    OSCL_UNUSED_ARG(aKeyList);
    OSCL_UNUSED_ARG(aStartingKeyIndex);
    OSCL_UNUSED_ARG(aEndKeyIndex);
    return PVMFSuccess;
}

/**
 * Synchronous method to free up the specified range of metadata values in the list. It is assumed that caller of this function
 * knows that start and end indices should correspond to metadata values returned by this particular instance of the
 * metadata extension interface using GetNodeMetadataValues().
 *
 * @param aValueList A reference to a metadata value list to free the value entries
 * @param aStartingValueIndex Index into aValueList that corresponds to the first value entry to release
 * @param aEndValueIndex Index into aValueList that corresponds to the last value entry to release
 *
 * @returns PVMFSuccess if the release of specified values succeeded. PVMFErrArgument if indices are invalid or the list is empty.
 *          PVMFFailure otherwise.
 **/
OSCL_EXPORT_REF PVMFStatus PVMFCPMPassThruPlugInOMA1::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 aStartingValueIndex,
        uint32 aEndValueIndex)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::ReleaseNodeMetadataValues() called"));
    OSCL_UNUSED_ARG(aValueList);
    OSCL_UNUSED_ARG(aStartingValueIndex);
    OSCL_UNUSED_ARG(aEndValueIndex);
    return PVMFSuccess;
}

//Make Metadata from license state data.
void PVMFCPMPassThruPlugInOMA1::MakeMetadata()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMPassThruPlugInOMA1::MakeMetadata() called"));

    //always set "is protected" here.  we rely on the source nodes
    //to only use this plugin for protected content.
    iMetaData[EIsProtectedOMA1].iValid = true;
    iMetaData[EIsProtectedOMA1].iKvp.value.bool_value = true;

    iMetaData[ELicAvailableOMA1].iValid = true;
    iMetaData[ELicAvailableOMA1].iKvp.value.bool_value = true;

    iLicType = "unlimited";
    iMetaData[ELicTypeOMA1].iValid = true;
    iMetaData[ELicTypeOMA1].iKvp.value.pChar_value = iLicType.get_str();

    iMetaData[EForwardLockOMA1].iValid = true;
    iMetaData[EForwardLockOMA1].iKvp.value.bool_value = true;
}

/**
** PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl
*/
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl(PVMFCPMPassThruPlugInOMA1&c)
        : iContainer(c)
{
    iFileObject = NULL;
    iLogger = iContainer.iLogger;
    iFs.Connect();
    iDataStreamReadCapacityObserver = iContainer.iDataStreamReadCapacityObserver;
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl"));
}

PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::~PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl()
{
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::~PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl"));
    if (iFileObject)
        OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    iFs.Close();
    iLogger = NULL;
    iDataStreamReadCapacityObserver = NULL;
}

OSCL_EXPORT_REF bool
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::queryInterface(const PVUuid& uuid,
        PVInterface*& iface)
{
    iface = NULL;
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::queryInterface"));
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface
        = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode,
        bool nonblocking)
{
    OSCL_UNUSED_ARG(aSessionID);
    OSCL_UNUSED_ARG(nonblocking);
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::OpenSession"));
    if (!iFileObject)
        iFileObject = OSCL_NEW(Oscl_File, (0, iContainer.iFileHandle));

    int32 result;
    if ((aMode == PVDS_READ_ONLY) || (aMode == PVDS_READ_WRITE))
    {
        result = iFileObject->Open(iContainer.iFilename.get_cstr(), Oscl_File::MODE_READ, iFs);
        PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::OpenSession - File Open returning %d", result));
        return (result == 0) ? PVDS_SUCCESS : PVDS_FAILURE;
    }
    else
    {
        return PVDS_UNSUPPORTED_MODE;
    }

    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::CloseSession(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);
    if (!iFileObject)
    {
        PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::CloseSession returning %d", -1));
        return PVDS_FAILURE;
    }
    int32 result = iFileObject->Close();
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::CloseSession returning %d", result));
    OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    if (result == 0)
        return PVDS_SUCCESS;

    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::QueryRandomAccessCapability()
{
    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::QueryReadCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);
    if (iDataStreamReadCapacityObserver != NULL)
    {
        return (iDataStreamReadCapacityObserver->GetStreamReadCapacity(capacity));
    }
    else
    {
        if (!iFileObject)
            return PVDS_FAILURE;
        int32 result = (TOsclFileOffsetInt32)iFileObject->Size();
        PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::QueryReadCapacity returning %d", result));
        if (result < 0)
        {
            return PVDS_FAILURE;
        }
        else
        {
            capacity = result;
            return PVDS_SUCCESS;
        }
    }
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::RequestReadCapacityNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(capacity);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::QueryWriteCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(capacity);
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::RequestWriteCapacityNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(capacity);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::CancelNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(aID);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::Read(PvmiDataStreamSession sessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    OSCL_UNUSED_ARG(sessionID);
    if (!iFileObject)
        return PVDS_FAILURE;

    uint32 result = iFileObject->Read(buffer, size, numelements);
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::ReadAndUnlockContent returning %d", result));

    numelements = result;
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::Write(PvmiDataStreamSession sessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);
    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::Seek(PvmiDataStreamSession sessionID,
        int32 offset,
        PvmiDataStreamSeekType origin)
{
    OSCL_UNUSED_ARG(sessionID);
    if (!iFileObject)
        return PVDS_FAILURE;
    Oscl_File::seek_type seekType;
    switch (origin)
    {
        case PVDS_SEEK_SET:
            seekType = Oscl_File::SEEKSET;
            break;
        case PVDS_SEEK_CUR:
            seekType = Oscl_File::SEEKCUR;
            break;
        case PVDS_SEEK_END:
            seekType = Oscl_File::SEEKEND;
            break;
        default:
            return PVDS_FAILURE;
    }
    int32 result = iFileObject->Seek(offset, seekType);
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::SeekContent returning %d", result));
    if (result != 0)
    {
        return PVDS_FAILURE;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF uint32
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::GetCurrentPointerPosition(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);
    if (!iFileObject)
        return PVDS_FAILURE;
    int32 result = (TOsclFileOffsetInt32)iFileObject->Tell();
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::GetCurrentContentPosition returning %d", result));
    return (uint32)(result);
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::Flush(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);
    if (!iFileObject)
    {
        PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::Flush returning %d", -1));
        return PVDS_FAILURE;
    }
    int32 result = iFileObject->Flush();
    PVMF_CPMPLUGIN_PASSTHRUOMA1_LOGDEBUG((0, "PVMFCPMPassThruPlugInOMA1DataStreamSyncInterfaceImpl::Flush returning %d", result));
    OSCL_UNUSED_ARG(result);
    return PVDS_SUCCESS;
}

