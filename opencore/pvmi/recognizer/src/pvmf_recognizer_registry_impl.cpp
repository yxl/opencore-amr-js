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
#ifndef OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#include "pvmf_recognizer_registry_impl.h"

#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);
#define LOGINFOHI(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOMED(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFOLOW(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_INFO,m);
#define LOGINFO(m) LOGINFOMED(m)

PVMFRecognizerRegistryImpl::PVMFRecognizerRegistryImpl() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVMFRecognizerRegistryImpl")
{
    AddToScheduler();

    iRefCount = 1;

    iNextSessionId = 0;
    iRecognizerSessionList.reserve(1);

    iNextCommandId = 0;
    iRecognizerPendingCmdList.reserve(2);
    iRecognizerCurrentCmd.reserve(1);
    iRecognizerCmdToCancel.reserve(1);

    iDataStreamFactory = NULL;
    iDataStream = NULL;
    oRecognizePending = false;
    iDataStreamCallBackStatus = PVMFSuccess;

    iLogger = PVLogger::GetLoggerObject("PVMFRecognizer");
}


PVMFRecognizerRegistryImpl::~PVMFRecognizerRegistryImpl()
{
    iDataStreamFactory = NULL;

    while (iRecognizerPluginFactoryList.empty() == false)
    {
        // The plug-in factories were not removed before shutting down the
        // registry. This could lead to memory leaks since the registry
        // cannot call the destructor for the derived factory class. So assert.
        OSCL_ASSERT(false);

        // Destroy factory using the base class
        PVMFRecognizerPluginFactory* pluginfactory = iRecognizerPluginFactoryList[0];
        iRecognizerPluginFactoryList.erase(iRecognizerPluginFactoryList.begin());
        OSCL_DELETE(pluginfactory);
    }

    iLogger = NULL;
}


PVMFStatus PVMFRecognizerRegistryImpl::RegisterPlugin(PVMFRecognizerPluginFactory& aPluginFactory)
{
    // Check that plug-in factory is not already registered
    if (FindPluginFactory(aPluginFactory) == -1)
    {
        // Add the plug-in factory to the list
        iRecognizerPluginFactoryList.push_back(&aPluginFactory);
    }

    return PVMFSuccess;
}


PVMFStatus PVMFRecognizerRegistryImpl::RemovePlugin(PVMFRecognizerPluginFactory& aPluginFactory)
{
    // Find the specified plug-in factory and remove from list
    int32 factoryindex = FindPluginFactory(aPluginFactory);
    if (factoryindex == -1)
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::RemovePlugin Failed!"));
        return PVMFErrArgument;
    }

    iRecognizerPluginFactoryList.erase(iRecognizerPluginFactoryList.begin() + factoryindex);
    return PVMFSuccess;
}


PVMFStatus PVMFRecognizerRegistryImpl::OpenSession(PVMFSessionId& aSessionId, PVMFRecognizerCommmandHandler& aCmdHandler)
{
    // TEMP: Currently only allow one session at a time
    if (iRecognizerSessionList.empty() == false)
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::OpenSession Failed!"));
        return PVMFErrBusy;
    }

    // Add this session to the list
    PVMFRecRegSessionInfo recsessioninfo;
    recsessioninfo.iRecRegSessionId = iNextSessionId;
    recsessioninfo.iRecRegCmdHandler = &aCmdHandler;
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iRecognizerSessionList.push_back(recsessioninfo));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         return PVMFErrNoMemory;
                        );

    aSessionId = recsessioninfo.iRecRegSessionId;

    // Increment the session ID counter
    ++iNextSessionId;

    return PVMFSuccess;
}


PVMFStatus PVMFRecognizerRegistryImpl::CloseSession(PVMFSessionId aSessionId)
{
    if (iRecognizerSessionList.empty() == true)
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::CloseSession Failed!"));
        return PVMFErrInvalidState;
    }

    // Search for the session in the list by the ID
    uint32 i;
    for (i = 0; i < iRecognizerSessionList.size(); ++i)
    {
        if (iRecognizerSessionList[i].iRecRegSessionId == aSessionId)
        {
            break;
        }
    }

    // Check if the session was not found
    if (i >= iRecognizerSessionList.size())
    {
        return PVMFErrArgument;
    }

    // Erase the session from the list to close the session
    iRecognizerSessionList.erase(iRecognizerSessionList.begin() + i);
    return PVMFSuccess;
}


PVMFCommandId PVMFRecognizerRegistryImpl::Recognize(PVMFSessionId aSessionId, PVMFDataStreamFactory& aSourceDataStreamFactory, PVMFRecognizerMIMEStringList* aFormatHint,
        Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>& aRecognizerResult, OsclAny* aCmdContext, uint32 aTimeout)
{
    if ((iRecognizerSessionList.empty() == true) || (oRecognizePending == true))
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::Recognize OsclErrInvalidState"));
        OSCL_LEAVE(OsclErrInvalidState);
    }
    if (aSessionId != iRecognizerSessionList[0].iRecRegSessionId)
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::Recognize OsclErrArgument"));
        OSCL_LEAVE(OsclErrArgument);
    }
    // TEMP: Only allow one recognize command at a time
    if (!(iRecognizerPendingCmdList.empty() == true && iRecognizerCurrentCmd.empty() == true))
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::Recognize OsclErrBusy"));
        OSCL_LEAVE(OsclErrBusy);
    }
    // TEMP: Only allow timeout of 0
    if (aTimeout > 0)
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::Recognize OsclErrArgument - aTimeout>0"));
        OSCL_LEAVE(OsclErrArgument);
    }

    // Save the passed-in parameters in a vector
    Oscl_Vector<PVMFRecRegImplCommandParamUnion, OsclMemAllocator> paramvector;
    paramvector.reserve(4);
    PVMFRecRegImplCommandParamUnion paramval;
    paramval.pOsclAny_value = (OsclAny*) & aSourceDataStreamFactory;
    paramvector.push_back(paramval);
    paramval.pOsclAny_value = (OsclAny*)aFormatHint;
    paramvector.push_back(paramval);
    paramval.pOsclAny_value = (OsclAny*) & aRecognizerResult;
    paramvector.push_back(paramval);
    paramval.uint32_value = aTimeout;
    paramvector.push_back(paramval);

    // Add the command to the pending list
    return AddRecRegCommand(aSessionId, PVMFRECREG_COMMAND_RECOGNIZE, aCmdContext, &paramvector, true);
}


PVMFCommandId PVMFRecognizerRegistryImpl::CancelCommand(PVMFSessionId aSessionId, PVMFCommandId aCommandToCancelId, OsclAny* aCmdContext)
{
    if (iRecognizerSessionList.empty() == true)
    {
        OSCL_LEAVE(OsclErrInvalidState);
        return 0;
    }
    if (aSessionId != iRecognizerSessionList[0].iRecRegSessionId)
    {
        OSCL_LEAVE(OsclErrArgument);
        return 0;
    }

    // Save the passed-in parameters in a vector
    Oscl_Vector<PVMFRecRegImplCommandParamUnion, OsclMemAllocator> paramvector;
    paramvector.reserve(1);
    PVMFRecRegImplCommandParamUnion paramval;
    paramval.int32_value = aCommandToCancelId;
    paramvector.push_back(paramval);

    // Add the command to the pending list
    return AddRecRegCommand(aSessionId, PVMFRECREG_COMMAND_CANCELCOMMAND, aCmdContext, &paramvector, true);
}


void PVMFRecognizerRegistryImpl::Run()
{
    int32 leavecode = 0;

    // Check if CancelCommand() request was made
    if (!iRecognizerPendingCmdList.empty())
    {
        if (iRecognizerPendingCmdList.top().GetCmdType() == PVMFRECREG_COMMAND_CANCELCOMMAND)
        {
            // Process it right away
            PVMFRecRegImplCommand cmd(iRecognizerPendingCmdList.top());
            iRecognizerPendingCmdList.pop();
            DoCancelCommand(cmd);
            return;
        }
    }

    // Handle other requests normally
    if (!iRecognizerPendingCmdList.empty() && iRecognizerCurrentCmd.empty())
    {
        // Retrieve the first pending command from queue
        PVMFRecRegImplCommand cmd(iRecognizerPendingCmdList.top());
        iRecognizerPendingCmdList.pop();

        // Put in on the current command queue
        leavecode = 0;
        OSCL_TRY(leavecode, iRecognizerCurrentCmd.push_front(cmd));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             OSCL_ASSERT(false);
                             // Can't complete the commmand since it cannot be added to the current command queue
                             return;);

        // Process the command according to the cmd type
        PVMFStatus cmdstatus = PVMFSuccess;
        switch (cmd.GetCmdType())
        {
            case PVMFRECREG_COMMAND_RECOGNIZE:
                DoRecognize();
                break;

            case PVMFRECREG_COMMAND_CANCELCOMMAND:
                // CancelCommand() should not be handled here
                OSCL_ASSERT(false);
                // Just handle as "not supported"
                cmdstatus = PVMFErrNotSupported;
                break;

            default:
                cmdstatus = PVMFErrNotSupported;
                break;
        }

        if (cmdstatus != PVMFSuccess)
        {
            CompleteCurrentRecRegCommand(cmdstatus);
        }
    }
    else if (oRecognizePending == true)
    {
        CompleteRecognize(iDataStreamCallBackStatus);
    }
}


int32 PVMFRecognizerRegistryImpl::FindPluginFactory(PVMFRecognizerPluginFactory& aFactory)
{
    // Check if the specified factory exists in the list
    for (uint32 i = 0; i < iRecognizerPluginFactoryList.size(); ++i)
    {
        if (iRecognizerPluginFactoryList[i] == &aFactory)
        {
            // Yes so return the index
            return ((int32)i);
        }
    }

    // No so return -1 meaning not found
    return -1;
}


PVMFRecognizerPluginInterface* PVMFRecognizerRegistryImpl::CreateRecognizerPlugin(PVMFRecognizerPluginFactory& aFactory)
{
    return aFactory.CreateRecognizerPlugin();
}


void PVMFRecognizerRegistryImpl::DestroyRecognizerPlugin(PVMFRecognizerPluginFactory& aFactory, PVMFRecognizerPluginInterface* aPlugin)
{
    aFactory.DestroyRecognizerPlugin(aPlugin);
}


PVMFCommandId PVMFRecognizerRegistryImpl::AddRecRegCommand(PVMFSessionId aSessionId, int32 aCmdType, OsclAny* aContextData, Oscl_Vector<PVMFRecRegImplCommandParamUnion, OsclMemAllocator>* aParamVector, bool aAPICommand)
{
    PVMFRecRegImplCommand cmd(aSessionId, aCmdType, iNextCommandId, aContextData, aParamVector, aAPICommand);
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iRecognizerPendingCmdList.push(cmd));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         OSCL_LEAVE(OsclErrNoMemory);
                         return 0;);
    RunIfNotReady();
    ++iNextCommandId;

    return cmd.GetCmdId();
}


void PVMFRecognizerRegistryImpl::CompleteCurrentRecRegCommand(PVMFStatus aStatus, const uint32 aCurrCmdIndex, PVInterface* aExtInterface)
{
    if (iRecognizerCurrentCmd.empty() == true)
    {
        // No command to complete. Assert
        OSCL_ASSERT(false);
        return;
    }

    // Save the command complete on stack and remove from queue
    PVMFRecRegImplCommand cmdtocomplete(iRecognizerCurrentCmd[aCurrCmdIndex]);
    iRecognizerCurrentCmd.clear();

    // Make callback if API command
    if (cmdtocomplete.IsAPICommand())
    {
        if (iRecognizerSessionList.empty() == false)
        {
            OSCL_ASSERT(iRecognizerSessionList[aCurrCmdIndex].iRecRegSessionId == cmdtocomplete.GetSessionId());
            PVMFCmdResp cmdresp(cmdtocomplete.GetCmdId(), cmdtocomplete.GetContext(), aStatus, aExtInterface);
            iRecognizerSessionList[aCurrCmdIndex].iRecRegCmdHandler->RecognizerCommandCompleted(cmdresp);
        }
    }

    // Need to make this AO active if there are pending commands
    if (iRecognizerPendingCmdList.empty() == false)
    {
        RunIfNotReady();
    }
}

PVMFStatus PVMFRecognizerRegistryImpl::GetMaxRequiredSizeForRecognition(uint32& aMaxSize)
{
    aMaxSize = 0;
    for (uint32 i = 0; i < iRecognizerPluginFactoryList.size(); ++i)
    {
        uint32 bytes = 0;
        // Create the recognizer plugin
        PVMFRecognizerPluginInterface* recplugin =
            CreateRecognizerPlugin(*(iRecognizerPluginFactoryList[i]));
        if (recplugin)
        {
            // Perform recognition with this recognizer plug-ing
            PVMFStatus status =
                recplugin->GetRequiredMinBytesForRecognition(bytes);
            // Done with this recognizer so release it
            DestroyRecognizerPlugin(*(iRecognizerPluginFactoryList[i]), recplugin);

            if (status == PVMFSuccess)
            {
                if (bytes > aMaxSize)
                {
                    aMaxSize = bytes;
                }
            }
            else
            {
                return status;
            }
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVMFRecognizerRegistryImpl::GetMinRequiredSizeForRecognition(uint32& aMinSize)
{
    aMinSize = 0x7FFFFFF;
    for (uint32 i = 0; i < iRecognizerPluginFactoryList.size(); ++i)
    {
        uint32 bytes = 0;
        // Create the recognizer plugin
        PVMFRecognizerPluginInterface* recplugin =
            CreateRecognizerPlugin(*(iRecognizerPluginFactoryList[i]));
        if (recplugin)
        {
            // Perform recognition with this recognizer plug-ing
            PVMFStatus status =
                recplugin->GetRequiredMinBytesForRecognition(bytes);
            // Done with this recognizer so release it
            DestroyRecognizerPlugin(*(iRecognizerPluginFactoryList[i]), recplugin);

            if (status == PVMFSuccess)
            {
                if (bytes < aMinSize)
                {
                    aMinSize = bytes;
                }
            }
            else
            {
                return status;
            }
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVMFRecognizerRegistryImpl::CheckForDataAvailability()
{
    if (iDataStreamFactory != NULL)
    {
        iDataStream = NULL;

        PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
        PVInterface* intf =
            iDataStreamFactory->CreatePVMFCPMPluginAccessInterface(uuid);

        iDataStream = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, intf);

        uint32 maxSize = 0;
        if (GetMaxRequiredSizeForRecognition(maxSize) == PVMFSuccess)
        {
            if (iDataStream->OpenSession(iDataStreamSessionID, PVDS_READ_ONLY) == PVDS_SUCCESS)
            {
                uint32 capacity = 0;
                PvmiDataStreamStatus status =
                    iDataStream->QueryReadCapacity(iDataStreamSessionID, capacity);

                if (capacity < maxSize)
                {
                    if (status == PVDS_END_OF_STREAM)
                    {
                        uuid = PVMIDataStreamSyncInterfaceUuid;
                        iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid,
                                OSCL_STATIC_CAST(PVInterface*, iDataStream));
                        iDataStream = NULL;
                        return PVMFFailure;
                    }
                    int32 errcode = 0;
                    OSCL_TRY(errcode,
                             iRequestReadCapacityNotificationID =
                                 iDataStream->RequestReadCapacityNotification(iDataStreamSessionID,
                                         *this,
                                         maxSize);
                            );
                    OSCL_FIRST_CATCH_ANY(errcode,
                                         uuid = PVMIDataStreamSyncInterfaceUuid;
                                         iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid,
                                                 OSCL_STATIC_CAST(PVInterface*, iDataStream));
                                         iDataStream = NULL;
                                         return PVMFFailure);

                    return PVMFPending;
                }
                uuid = PVMIDataStreamSyncInterfaceUuid;
                iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid,
                        OSCL_STATIC_CAST(PVInterface*, iDataStream));
                iDataStream = NULL;
                return PVMFSuccess;
            }
            else
            {
                uuid = PVMIDataStreamSyncInterfaceUuid;
                iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid,
                        OSCL_STATIC_CAST(PVInterface*, iDataStream));
                iDataStream = NULL;
                return PVMFFailure;
            }
        }
    }
    return PVMFFailure;
}

void PVMFRecognizerRegistryImpl::CompleteRecognize(PVMFStatus aStatus)
{
    oRecognizePending = false;
    PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
    iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid,
            OSCL_STATIC_CAST(PVInterface*, iDataStream));
    iDataStream = NULL;
    if (aStatus == PVMFSuccess)
    {
        iDataStreamFactory =
            (PVMFDataStreamFactory*) iRecognizerCurrentCmd[0].GetParam(0).pOsclAny_value;
        PVMFRecognizerMIMEStringList* hintlist = (PVMFRecognizerMIMEStringList*) iRecognizerCurrentCmd[0].GetParam(1).pOsclAny_value;
        Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>* recresult = (Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>*) iRecognizerCurrentCmd[0].GetParam(2).pOsclAny_value;

        // Validate the parameters
        if (iDataStreamFactory == NULL || recresult == NULL)
        {
            CompleteCurrentRecRegCommand(PVMFErrArgument);
            return;
        }

        // TEMP: Perform the recognition operation by checking with each registered recognizer once
        for (uint32 i = 0; i < iRecognizerPluginFactoryList.size(); ++i)
        {
            // Create the recognizer plugin
            PVMFRecognizerPluginInterface* recplugin =
                CreateRecognizerPlugin(*(iRecognizerPluginFactoryList[i]));
            if (recplugin)
            {
                LOGINFO((0, "PVMFRecognizerRegistryImpl::CompleteRecognize Calling recognizer i=%d", i));
                uint32 currticks = OsclTickCount::TickCount();
                uint32 starttime = OsclTickCount::TicksToMsec(currticks);
                OSCL_UNUSED_ARG(starttime);

                // Perform recognition with this recognizer plug-ing
                recplugin->Recognize(*iDataStreamFactory, hintlist, *recresult);
                // Done with this recognizer so release it

                currticks = OsclTickCount::TickCount();
                uint32 endtime = OsclTickCount::TicksToMsec(currticks);
                OSCL_UNUSED_ARG(endtime);
                DestroyRecognizerPlugin(*(iRecognizerPluginFactoryList[i]), recplugin);

                if (!recresult->empty())
                {
                    // Get the result of the recognizer operation from the vector
                    LOGINFO((0, "PVMFRecognizerRegistryImpl::CompleteRecognize Out of recognizer i=%d  result=%d, time=%d",
                             i, (recresult->back()).iRecognitionConfidence, (endtime - starttime)));
                    //LOGINFO((0,"PVMFRecognizerRegistryImpl::CompleteRecognize out of recognizer i=%d  result=%d, time=%d, mime=%s",
                    //i,(recresult->back()).iRecognitionConfidence, (endtime-starttime), (recresult->back()).iRecognizedFormat.get_cstr()));
                    if ((recresult->back()).iRecognitionConfidence == PVMFRecognizerConfidenceCertain)
                    {
                        CompleteCurrentRecRegCommand(PVMFSuccess);
                        return;
                    }
                }
            }

        }
        // Complete the recognizer command
        CompleteCurrentRecRegCommand(PVMFSuccess);
    }
    else
    {
        // Complete the recognizer command
        CompleteCurrentRecRegCommand(aStatus);
    }
}

void PVMFRecognizerRegistryImpl::DoRecognize()
{
    // Retrieve the command parameters
    iDataStreamFactory =
        (PVMFDataStreamFactory*) iRecognizerCurrentCmd[0].GetParam(0).pOsclAny_value;
    PVMFRecognizerMIMEStringList* hintlist = (PVMFRecognizerMIMEStringList*) iRecognizerCurrentCmd[0].GetParam(1).pOsclAny_value;
    Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>* recresult = (Oscl_Vector<PVMFRecognizerResult, OsclMemAllocator>*) iRecognizerCurrentCmd[0].GetParam(2).pOsclAny_value;

    // Validate the parameters
    if (iDataStreamFactory == NULL || recresult == NULL)
    {
        CompleteCurrentRecRegCommand(PVMFErrArgument);
        return;
    }

    PVMFStatus status = CheckForDataAvailability();
    if (status == PVMFFailure)
    {
        CompleteCurrentRecRegCommand(PVMFFailure);
        return;
    }
    else if (status == PVMFSuccess)
    {
        // TEMP: Perform the recognition operation by checking with each registered recognizer once
        for (uint32 i = 0; i < iRecognizerPluginFactoryList.size(); ++i)
        {
            // Create the recognizer plugin
            PVMFRecognizerPluginInterface* recplugin =
                CreateRecognizerPlugin(*(iRecognizerPluginFactoryList[i]));
            if (recplugin)
            {
                LOGINFO((0, "PVMFRecognizerRegistryImpl::DoRecognize Calling recognizer i=%d", i));
                uint32 currticks = OsclTickCount::TickCount();
                uint32 starttime = OsclTickCount::TicksToMsec(currticks);
                OSCL_UNUSED_ARG(starttime);

                // Perform recognition with this recognizer plug-ing
                recplugin->Recognize(*iDataStreamFactory, hintlist, *recresult);
                // Done with this recognizer so release it

                currticks = OsclTickCount::TickCount();
                uint32 endtime = OsclTickCount::TicksToMsec(currticks);
                OSCL_UNUSED_ARG(endtime);

                DestroyRecognizerPlugin(*(iRecognizerPluginFactoryList[i]), recplugin);

                if (!recresult->empty())
                {
                    LOGINFO((0, "PVMFRecognizerRegistryImpl::DoRecognize Out of recognizer i=%d  result=%d, time=%d",
                             i, (recresult->back()).iRecognitionConfidence, (endtime - starttime)));
                    //LOGINFO((0,"PVMFRecognizerRegistryImpl::DoRecognize out of recognizer i=%d  result=%d, time=%d, mime=%s",
                    //i,(recresult->back()).iRecognitionConfidence, (endtime-starttime), (recresult->back()).iRecognizedFormat.get_cstr()));
                    if ((recresult->back()).iRecognitionConfidence == PVMFRecognizerConfidenceCertain)
                    {
                        CompleteCurrentRecRegCommand(PVMFSuccess);
                        return;
                    }
                }
            }
        }

        // Complete the recognizer command
        CompleteCurrentRecRegCommand(PVMFSuccess);
    }
    else
    {
        //pending
        //wait for datastream call back
        oRecognizePending = true;
    }
}


void PVMFRecognizerRegistryImpl::DoCancelCommand(PVMFRecRegImplCommand& aCmd)
{
    // TEMP: For now only one command can be cancelled.
    // Since Recognize happens in one AO call, check in the pending cmd queue
    if (iRecognizerPendingCmdList.empty() == false)
    {
        iRecognizerCurrentCmd.push_front(iRecognizerPendingCmdList.top());
        iRecognizerPendingCmdList.pop();
        OSCL_ASSERT(iRecognizerCurrentCmd[0].GetCmdId() == aCmd.GetCmdId());
    }

    if (iRecognizerCurrentCmd.empty() == false)
    {
        // get the commandToCancelId
        PVMFRecRegImplCommandParamUnion paramval = aCmd.GetParam(0);
        PVMFCommandId commandToCancelId = paramval.int32_value;
        if (FindCommandByID(iRecognizerCurrentCmd, commandToCancelId) == false) return;
        CompleteCurrentRecRegCommand(PVMFErrCancelled, commandToCancelId);

        // close data stream object to avoid any memory leak in case of cancel command
        if (iDataStream) iDataStream->CloseSession(iDataStreamSessionID);
        if (iDataStreamFactory)
        {
            PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
            iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid,
                    OSCL_STATIC_CAST(PVInterface*, iDataStream));
            iDataStream = NULL;
        }
    }

}

bool PVMFRecognizerRegistryImpl::FindCommandByID(Oscl_Vector<PVMFRecRegImplCommand, OsclMemAllocator> &aCmdQueue, const PVMFCommandId aCmdId)
{
    if (aCmdQueue.empty()) return false;
    for (uint32 i = 0; i < aCmdQueue.size(); i++)
    {
        if (aCmdQueue[i].GetCmdId() == aCmdId) return true;
    }
    return false;
}

void PVMFRecognizerRegistryImpl::DataStreamCommandCompleted(const PVMFCmdResp& aResponse)
{
    if (aResponse.GetCmdId() == iRequestReadCapacityNotificationID)
    {
        iDataStreamCallBackStatus = aResponse.GetCmdStatus();
        RunIfNotReady();
    }
    else
    {
        LOGERROR((0, "PVMFRecognizerRegistryImpl::DataStreamCommandCompleted failed"));
        OSCL_ASSERT(false);
    }
}

void PVMFRecognizerRegistryImpl::DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    OSCL_LEAVE(OsclErrNotSupported);
}

void PVMFRecognizerRegistryImpl::DataStreamErrorEvent(const PVMFAsyncEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    OSCL_LEAVE(OsclErrNotSupported);
}








