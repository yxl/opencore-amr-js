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
#ifndef PVMF_SM_FSP_BASE_IMPL_H
#include "pvmf_sm_fsp_base_impl.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_STREAMING_MANAGER_NODE_H_INCLUDED
#include "pvmf_streaming_manager_node.h"
#endif
#ifndef PVMF_MEDIA_PRESENTATION_INFO_H_INCLUDED
#include "pvmf_media_presentation_info.h"
#endif
#ifndef PVMF_STREAMING_DATA_SOURCE_H_INCLUDED
#include "pvmf_streaming_data_source.h"
#endif

#define PVMF_STREAMING_MANAGER_NODE_MAX_CPM_METADATA_KEYS 256

///////////////////////////////////////////////////////////////////////////////
//Implementation of PVMFCPMPluginLicenseInterface
///////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFSMFSPBaseNode::GetLicenseURL(PVMFSessionId aSessionId,
        OSCL_wString& aContentName,
        OSCL_wString& aLicenseURL)
{
    OSCL_UNUSED_ARG(aSessionId);
    OSCL_UNUSED_ARG(aContentName);
    OSCL_UNUSED_ARG(aLicenseURL);
    //must use Async method.
    PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::GetLicenseURL - Error Not Supported"));
    return PVMFErrNotSupported;
}

PVMFStatus PVMFSMFSPBaseNode::GetLicenseURL(PVMFSessionId aSessionId,
        OSCL_String&  aContentName,
        OSCL_String&  aLicenseURL)
{
    OSCL_UNUSED_ARG(aSessionId);
    OSCL_UNUSED_ARG(aContentName);
    OSCL_UNUSED_ARG(aLicenseURL);
    //must use Async method.
    PVMF_SM_FSP_BASE_LOGERR((0, "PVMFSMFSPBaseNode::GetLicenseURL - Error Not Supported"));
    return PVMFErrNotSupported;
}

/**
 * Queue an asynchronous node command - GetLicense(Unicode)
 */
PVMFCommandId PVMFSMFSPBaseNode::GetLicense(PVMFSessionId aSessionId
        , OSCL_wString& aContentName
        , OsclAny* aLicenseData
        , uint32 aDataSize
        , int32 aTimeoutMsec
        , OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetLicense - Unicode Variant Called"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_GET_LICENSE_W,
                                            aContentName,
                                            aLicenseData,
                                            aDataSize,
                                            aTimeoutMsec,
                                            aContext);
    return QueueCommandL(cmd);
}

/**
 * Queue an asynchronous node command - GetLicense(ASCII)
 */
PVMFCommandId PVMFSMFSPBaseNode::GetLicense(PVMFSessionId aSessionId
        , OSCL_String&  aContentName
        , OsclAny* aLicenseData
        , uint32 aDataSize
        , int32 aTimeoutMsec
        , OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetLicense - called"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommand::Construct(aSessionId,
                                            PVMF_SMFSP_NODE_GET_LICENSE,
                                            aContentName,
                                            aLicenseData,
                                            aDataSize,
                                            aTimeoutMsec,
                                            aContext);
    return QueueCommandL(cmd);

}

/**
 * Queue an asynchronous node command - CancelGetLicense
 */
PVMFCommandId PVMFSMFSPBaseNode::CancelGetLicense(PVMFSessionId aSessionId
        , PVMFCommandId aCmdId
        , OsclAny* aContext)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CancelGetLicense - called"));
    PVMFSMFSPBaseNodeCommand cmd;
    cmd.PVMFSMFSPBaseNodeCommandBase::Construct(aSessionId,
            PVMF_SMFSP_NODE_CANCEL_GET_LICENSE,
            aCmdId,
            aContext);
    return QueueCommandL(cmd);
}

PVMFStatus PVMFSMFSPBaseNode::GetLicenseStatus(PVMFCPMLicenseStatus& aStatus)
{
    if (iCPMLicenseInterface)
        return iCPMLicenseInterface->GetLicenseStatus(aStatus);
    return PVMFFailure;
}

void PVMFSMFSPBaseNode::InitCPM()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::InitCPM() In"));

    iCPMInitCmdId = iCPM->Init();
}

void PVMFSMFSPBaseNode::OpenCPMSession()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::OpenCPMSession() In"));

    iCPMOpenSessionCmdId = iCPM->OpenSession(iCPMSessionID);
}

void PVMFSMFSPBaseNode::CPMRegisterContent()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CPMRegisterContent() In"));

    if (iSourceContextDataValid)
    {
        iCPMRegisterContentCmdId = iCPM->RegisterContent(iCPMSessionID,
                                   iSessionSourceInfo->_sessionURL,
                                   iSessionSourceInfo->_sessionType,
                                   (OsclAny*) & iSourceContextData);
    }
    else
    {
        iCPMRegisterContentCmdId = iCPM->RegisterContent(iCPMSessionID,
                                   iSessionSourceInfo->_sessionURL,
                                   iSessionSourceInfo->_sessionType,
                                   (OsclAny*) & iCPMSourceData);
    }
}

void PVMFSMFSPBaseNode::GetCPMLicenseInterface()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetCPMLicenseInterface() In"));

    iCPMLicenseInterfacePVI = NULL;
    iCPMGetLicenseInterfaceCmdId =
        iCPM->QueryInterface(iCPMSessionID,
                             PVMFCPMPluginLicenseInterfaceUuid,
                             iCPMLicenseInterfacePVI);
}

void PVMFSMFSPBaseNode::GetCPMCapConfigInterface()
{
    iCPMCapConfigInterfacePVI = NULL;
    iCPMGetCapConfigCmdId =
        iCPM->QueryInterface(iCPMSessionID,
                             PVMI_CAPABILITY_AND_CONFIG_PVUUID,
                             iCPMCapConfigInterfacePVI);
}


bool PVMFSMFSPBaseNode::GetCPMContentAccessFactory()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetCPMContentAccessFactory() In"));

    PVMFStatus status = iCPM->GetContentAccessFactory(iCPMSessionID,
                        iCPMContentAccessFactory);
    if (status != PVMFSuccess)
    {
        return false;
    }
    return true;
}

bool PVMFSMFSPBaseNode::GetCPMMetaDataExtensionInterface()
{
    PVInterface* temp = NULL;
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::GetCPMMetaDataExtensionInterface() In"));

    bool retVal =
        iCPM->queryInterface(KPVMFMetadataExtensionUuid, temp);
    iCPMMetaDataExtensionInterface = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, temp);

    return retVal;
}

void PVMFSMFSPBaseNode::RequestUsage()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::RequestUsage() In"));
    iCPMRequestUsageId = iCPM->ApproveUsage(iCPMSessionID,
                                            iRequestedUsage,
                                            iApprovedUsage,
                                            iAuthorizationDataKvp,
                                            iUsageID,
                                            iCPMContentAccessFactory);
}

void PVMFSMFSPBaseNode::SendUsageComplete()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::SendUsageComplete() In"));

    iCPMUsageCompleteCmdId = iCPM->UsageComplete(iCPMSessionID, iUsageID);
}

void PVMFSMFSPBaseNode::CloseCPMSession()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CloseCPMSession() In"));

    iCPMCloseSessionCmdId = iCPM->CloseSession(iCPMSessionID);
}

void PVMFSMFSPBaseNode::ResetCPM()
{
    iCPMResetCmdId = iCPM->Reset();
}

void PVMFSMFSPBaseNode::GetCPMMetaDataKeys()
{
    if (iCPMMetaDataExtensionInterface != NULL)
    {
        iCPMMetadataKeys.clear();
        iCPMGetMetaDataKeysCmdId =
            iCPMMetaDataExtensionInterface->GetNodeMetadataKeys(iCPMSessionID,
                    iCPMMetadataKeys,
                    0,
                    PVMF_STREAMING_MANAGER_NODE_MAX_CPM_METADATA_KEYS);
    }
}

PVMFStatus
PVMFSMFSPBaseNode::CheckCPMCommandCompleteStatus(PVMFCommandId aID,
        PVMFStatus aStatus)
{
    PVMFStatus status = aStatus;
    if (aID == iCPMGetLicenseInterfaceCmdId)
    {
        if (aStatus == PVMFErrNotSupported)
        {
            /* License Interface is Optional */
            status = PVMFSuccess;
        }
    }

    if ((status != PVMFSuccess))
    {
        OSCL_ASSERT(aID != iCPMResetCmdId);
        if (iCurrErrHandlingCommand.size() > 0)
        {
            if (PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR == iCurrErrHandlingCommand.front().iCmd)
            {

                //skip to next step, cpm cleanup steps are not coupled so moving to next step even if
                //some of inbetween step fails may not be harmful
                status = PVMFSuccess;
            }
        }
    }

    return status;
}

void PVMFSMFSPBaseNode::CPMCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CPMCommandCompleted() In"));

    PVMFCommandId id = aResponse.GetCmdId();
    PVMFStatus status =
        CheckCPMCommandCompleteStatus(id, aResponse.GetCmdStatus());

    if (id == iCPMCancelGetLicenseCmdId)
    {
        /*
         * if this command is CancelGetLicense, we will return success or fail here.
         */
        OSCL_ASSERT(!iCancelCommand.empty());
        CommandComplete(iCancelCommand,
                        iCancelCommand.front(),
                        status);
        return;
    }
    else
    {
        /*
         * if there was any pending cancel, we just ignore CPM process.
         */
        if (iCurrentCommand.empty() && iCurrErrHandlingCommand.empty())
        {
            return;
        }
        if (!iCancelCommand.empty())
        {
            if (iCancelCommand.front().iCmd != PVMF_SMFSP_NODE_CANCEL_GET_LICENSE)
                return;
        }
    }

    if (status != PVMFSuccess)
    {
        if (id == iCPMRequestUsageId)
        {
            /*
             * Only when PVMFErrLicenseRequired is replied for license authentication,
             * Set iCPMInitPending into true.
             */
            if (aResponse.GetCmdStatus() == PVMFErrLicenseRequired)
                iCPMInitPending = true;
        }
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        aResponse.GetCmdStatus(),
                        NULL,
                        NULL,
                        NULL,
                        aResponse.GetEventExtensionInterface());

    }
    else
    {
        /*
         * process the response, and issue the next command in
         * the sequence.
         */

        if (id == iCPMInitCmdId)
        {
            OpenCPMSession();
        }
        else if (id == iCPMOpenSessionCmdId)
        {
            CPMRegisterContent();
        }
        else if (id == iCPMRegisterContentCmdId)
        {
            GetCPMCapConfigInterface();
        }
        else if (id == iCPMGetCapConfigCmdId)
        {
            iCPMCapConfigInterface = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, iCPMCapConfigInterfacePVI);
            iCPMCapConfigInterfacePVI = NULL;
            // got capConfig interface, set kvps for CPM plugins
            if (!SetCPMKvps()) return;
            GetCPMLicenseInterface();
        }
        else if (id == iCPMGetLicenseInterfaceCmdId)
        {
            iCPMLicenseInterface = OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*, iCPMLicenseInterfacePVI);
            iCPMLicenseInterfacePVI = NULL;
            iCPMContentType = iCPM->GetCPMContentType(iCPMSessionID);
            OSCL_ASSERT(iCPMContentType == PVMF_CPM_FORMAT_ACCESS_BEFORE_AUTHORIZE);
            GetCPMContentAccessFactory();
            GetCPMMetaDataExtensionInterface();
            RequestUsage();
        }
        else if (id == iCPMRequestUsageId)
        {
            OSCL_ASSERT(iCPMContentType == PVMF_CPM_FORMAT_ACCESS_BEFORE_AUTHORIZE);
            /* End of Node Init sequence. */
            OSCL_ASSERT(!iCurrentCommand.empty());
            OSCL_ASSERT(iCurrentCommand.front().iCmd == PVMF_SMFSP_NODE_INIT);
            CompleteDRMInit();
        }
        else if (id == iCPMGetMetaDataKeysCmdId)
        {
            /* End of GetNodeMetaDataKeys */
            PVMFStatus status =
                CompleteGetMetadataKeys(iCurrentCommand.front());
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            status);
        }
        else if (id == iCPMUsageCompleteCmdId)
        {
            if (iDecryptionInterface != NULL)
            {
                iDecryptionInterface->Reset();
                /* Remove the decrpytion interface */
                PVUuid uuid = PVMFCPMPluginDecryptionInterfaceUuid;
                iCPMContentAccessFactory->DestroyPVMFCPMPluginAccessInterface(uuid, iDecryptionInterface);
                iDecryptionInterface = NULL;
            }
            CloseCPMSession();
        }
        else if (id == iCPMCloseSessionCmdId)
        {
            ResetCPM();
        }
        else if (id == iCPMResetCmdId)
        {
            if (EPVMFNodeError != iInterfaceState)
            {
                /* End of Node Reset sequence */
                OSCL_ASSERT(!iCurrentCommand.empty());
                OSCL_ASSERT(iCurrentCommand.front().iCmd == PVMF_SMFSP_NODE_RESET);
                iDRMResetPending = false;
                CompleteReset();
            }
            else
            {
                /* End of Node Reset sequence */
                OSCL_ASSERT(!iCurrErrHandlingCommand.empty());
                OSCL_ASSERT(PVMF_SMFSP_NODE_RESET_DUE_TO_ERROR == iCurrErrHandlingCommand.front().iCmd);
                iDRMResetPending = false;
                HandleError(aResponse);
            }
        }
        else if (id == iCPMGetMetaDataValuesCmdId)
        {
            /* End of GetNodeMetaDataValues */
            OSCL_ASSERT(!iCurrentCommand.empty());
            OSCL_ASSERT(iCurrentCommand.front().iCmd == PVMF_SMFSP_NODE_GETNODEMETADATAVALUES);
            CompleteGetMetaDataValues();
        }
        else if (id == iCPMGetLicenseCmdId)
        {
            CompleteGetLicense();
        }
        else
        {
            /* Unknown cmd ?? - error */
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFFailure);
        }
    }
}

void PVMFSMFSPBaseNode::CompleteGetMetaDataValues()
{
    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFSuccess);
}

void PVMFSMFSPBaseNode::CompleteDRMInit()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteDRMInit - In"));
    if (iApprovedUsage.value.uint32_value !=
            iRequestedUsage.value.uint32_value)
    {
        CommandComplete(iCurrentCommand,
                        iCurrentCommand.front(),
                        PVMFErrAccessDenied,
                        NULL, NULL, NULL);
        return;
    }
    //If protected content, get the decryption interface, if the intent is play
    if (iCPMSourceData.iIntent & BITMASK_PVMF_SOURCE_INTENT_PLAY)
    {
        PVUuid uuid = PVMFCPMPluginDecryptionInterfaceUuid;
        PVInterface* intf =
            iCPMContentAccessFactory->CreatePVMFCPMPluginAccessInterface(uuid);
        PVMFCPMPluginAccessInterface* interimPtr =
            OSCL_STATIC_CAST(PVMFCPMPluginAccessInterface*, intf);
        iDecryptionInterface = OSCL_STATIC_CAST(PVMFCPMPluginAccessUnitDecryptionInterface*, interimPtr);
        if (iDecryptionInterface == NULL)
        {
            CommandComplete(iCurrentCommand,
                            iCurrentCommand.front(),
                            PVMFErrAccessDenied);
            return;
        }
        iDecryptionInterface->Init();

        RequestUsageComplete();
    }

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteDRMInit Success"));
    //License authentication was successfull. Init is completed at protected clip
    SetState(EPVMFNodeInitialized);
    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFSuccess);

    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteDRMInit - Out"));
    return;
}

bool PVMFSMFSPBaseNode::SetCPMKvps()
{
    if (iCPMCapConfigInterface && !iCPMKvpStore.isEmpty())
    {
        PVMFKvpVector *aVector = iCPMKvpStore.getKVPStore();
        for (uint32 i = 0; i < aVector->size(); i++)
        {
            if (PVMFSuccess != SetCPMKvp((*aVector)[i]))
            {
                CommandComplete(iCurrentCommand,
                                iCurrentCommand.front(),
                                PVMFFailure);
                iCPMKvpStore.destroy();
                return false;
            }
        }
        iCPMKvpStore.destroy();
    }
    return true;
}

PVMFStatus PVMFSMFSPBaseNode::SetCPMKvp(PvmiKvp& aKVP)
{
    PVMFStatus status = PVMFSuccess;
    PvmiKvp *aErrorKVP = NULL;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iCPMCapConfigInterface->setParametersSync(NULL, &aKVP, 1, aErrorKVP));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFSMFSPBaseNode::PushKeyToMetadataList - Memory allocation failure when copying metadata key"));
                         status = leavecode);
    return status;
}

PVMFStatus PVMFSMFSPBaseNode::DoGetLicense(PVMFSMFSPBaseNodeCommand& aCmd,
        bool aWideCharVersion)
{
    if (iCPMLicenseInterface == NULL)
    {
        return PVMFErrNotSupported;
    }

    if (aWideCharVersion == true)
    {

        OSCL_wString* contentName = NULL;
        OsclAny* data = NULL;
        uint32 dataSize = 0;
        int32 timeoutMsec = 0;
        aCmd.PVMFSMFSPBaseNodeCommand::Parse(contentName,
                                             data,
                                             dataSize,
                                             timeoutMsec);
        iCPMGetLicenseCmdId =
            iCPMLicenseInterface->GetLicense(iCPMSessionID,
                                             *contentName,
                                             data,
                                             dataSize,
                                             timeoutMsec);
    }
    else
    {
        OSCL_String* contentName = NULL;
        OsclAny* data = NULL;
        uint32 dataSize = 0;
        int32 timeoutMsec = 0;
        aCmd.PVMFSMFSPBaseNodeCommand::Parse(contentName,
                                             data,
                                             dataSize,
                                             timeoutMsec);
        iCPMGetLicenseCmdId =
            iCPMLicenseInterface->GetLicense(iCPMSessionID,
                                             *contentName,
                                             data,
                                             dataSize,
                                             timeoutMsec);
    }
    return PVMFPending;
}

void PVMFSMFSPBaseNode::CompleteGetLicense()
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::CompleteGetLicense - called"));
    CommandComplete(iCurrentCommand,
                    iCurrentCommand.front(),
                    PVMFSuccess);
}

void PVMFSMFSPBaseNode::DoCancelGetLicense(PVMFSMFSPBaseNodeCommand& aCmd)
{
    PVMF_SM_FSP_BASE_LOGSTACKTRACE((0, "PVMFSMFSPBaseNode::DoCancelGetLicense() Called"));
    PVMFStatus status = PVMFErrArgument;

    if (iCPMLicenseInterface == NULL)
    {
        status = PVMFErrNotSupported;
    }
    else
    {
        /* extract the command ID from the parameters.*/
        PVMFCommandId id;
        aCmd.PVMFSMFSPBaseNodeCommandBase::Parse(id);

        /* first check "current" command if any */
        PVMFSMFSPBaseNodeCommand* cmd = iCurrentCommand.FindById(id);
        if (cmd)
        {
            if (cmd->iCmd == PVMF_SMFSP_NODE_GET_LICENSE_W || cmd->iCmd == PVMF_SMFSP_NODE_GET_LICENSE)
            {
                iCPMCancelGetLicenseCmdId =
                    iCPMLicenseInterface->CancelGetLicense(iCPMSessionID, iCPMGetLicenseCmdId);

                /*
                 * the queued commands are all asynchronous commands to the
                 * CPM module. CancelGetLicense can cancel only for GetLicense cmd.
                 * We need to wait CPMCommandCompleted.
                 */
                MoveCmdToCancelQueue(aCmd);
                return;
            }
        }

        /*
         * next check input queue.
         * start at element 1 since this cancel command is element 0.
         */
        cmd = iInputCommands.FindById(id, 1);
        if (cmd)
        {
            if (cmd->iCmd == PVMF_SMFSP_NODE_GET_LICENSE_W || cmd->iCmd == PVMF_SMFSP_NODE_GET_LICENSE)
            {
                /* cancel the queued command */
                CommandComplete(iInputCommands, *cmd, PVMFErrCancelled, NULL, NULL);
                // report cancel success
                CommandComplete(iInputCommands, aCmd, PVMFSuccess);
                return;
            }
        }
    }
    /* if we get here the command isn't queued so the cancel fails */
    CommandComplete(iInputCommands, aCmd, status);
    return;
}
