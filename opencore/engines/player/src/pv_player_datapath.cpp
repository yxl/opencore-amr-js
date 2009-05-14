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
#include "pv_player_datapath.h"

#include "pvlogger.h"

#include "pvmi_config_and_capability_utils.h"

// Temporary until port tag query is ready
#define DEFAULT_INPUT_PORTTAG 0
#define DEFAULT_OUTPUT_PORTTAG 1
#define PORT_CONFIG_INPUT_FORMATS_VALTYPE "x-pvmf/port/formattype;valtype=int32"

//
// PVPlayerDatapath Section
//
PVPlayerDatapath::PVPlayerDatapath() :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVPlayerDatapath"),
        iState(PVPDP_IDLE),
        iSourceNode(NULL), iSourceSessionId(0),
        iDecNode(NULL), iDecSessionId(0),
        iSinkNode(NULL), iSinkSessionId(0),
        iSourceOutPort(NULL), iDecInPort(NULL), iDecOutPort(NULL), iSinkInPort(NULL),
        iObserver(NULL),
        iErrorObserver(NULL),
        iInfoObserver(NULL),
        iContext(NULL),
        iSourceDecFormatType(PVMF_MIME_FORMAT_UNKNOWN),
        iDecSinkFormatType(PVMF_MIME_FORMAT_UNKNOWN),
        iSourceSinkFormatType(PVMF_MIME_FORMAT_UNKNOWN),
        iSourceTrackInfo(NULL),
        iDatapathConfig(CONFIG_NONE),
        iErrorCondition(false),
        iErrorOccurredDuringErrorCondition(false)
{
    AddToScheduler();

    // Retrieve the logger object
    iLogger = PVLogger::GetLoggerObject("PVPlayerEngine");
}


PVPlayerDatapath::~PVPlayerDatapath()
{
    if (IsBusy())
    {
        Cancel();
    }
}


PVMFStatus PVPlayerDatapath::Prepare(OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Prepare() In"));

    // Check that source and sink nodes are set
    if (!iSourceNode || !iSinkNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Prepare() Source and/or sink node not set"));
        return PVMFFailure;
    }

    // Check that source track info is set
    if (iSourceTrackInfo == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Prepare() Source track info not set"));
        return PVMFFailure;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Prepare() Track MIME type %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    // Determine the datapath configuration
    // from the nodes that have been set
    if (iDecNode)
    {
        iDatapathConfig = CONFIG_DEC;
    }
    else
    {
        iDatapathConfig = CONFIG_NONE;
    }

    iContext = aContext;

    // Connect to the nodes and get session IDs
    PVMFNodeSessionInfo sessioninfo(this, this, NULL, this, NULL);
    int32 leavecode = 0;

    iSourceNode->ThreadLogon();
    sessioninfo.iErrorContext = (OsclAny*)iSourceNode;
    sessioninfo.iInfoContext = (OsclAny*)iSourceNode;
    leavecode = 0;
    OSCL_TRY(leavecode, iSourceSessionId = iSourceNode->Connect(sessioninfo));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Prepare() Connect on iSourceNode did a leave!"));
                         return PVMFFailure);

    iSinkNode->ThreadLogon();
    sessioninfo.iErrorContext = (OsclAny*)iSinkNode;
    sessioninfo.iInfoContext = (OsclAny*)iSinkNode;
    leavecode = 0;
    OSCL_TRY(leavecode, iSinkSessionId = iSinkNode->Connect(sessioninfo));
    OSCL_FIRST_CATCH_ANY(leavecode,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Prepare() Connect on iSinkNode did a leave!"));
                         return PVMFFailure);

    if (iDatapathConfig == CONFIG_DEC)
    {
        iDecNode->ThreadLogon();
        sessioninfo.iErrorContext = (OsclAny*)iDecNode;
        sessioninfo.iInfoContext = (OsclAny*)iDecNode;
        leavecode = 0;
        OSCL_TRY(leavecode, iDecSessionId = iDecNode->Connect(sessioninfo));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Prepare() Connect on iDecNode did a leave!"));
                             return PVMFFailure);
    }

    iState = PREPARE_INIT;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Prepare() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerDatapath::Start(OsclAny* aContext)
{
    OSCL_ASSERT(iSourceTrackInfo != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Start() In %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    iContext = aContext;
    iState = START_START;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Start() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerDatapath::Pause(OsclAny* aContext, bool aSinkPaused)
{
    OSCL_ASSERT(iSourceTrackInfo != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Pause() In %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    iContext = aContext;
    iSinkPaused = aSinkPaused;
    if (iSinkPaused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Pause() Sink is already paused"));
    }
    iState = PAUSE_PAUSE;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Pause() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerDatapath::Stop(OsclAny* aContext, bool aErrorCondition)
{
    OSCL_ASSERT(iSourceTrackInfo != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Stop() In %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    iContext = aContext;
    iErrorCondition = aErrorCondition;
    iErrorOccurredDuringErrorCondition = false;
    iState = STOP_STOP;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Stop() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerDatapath::Teardown(OsclAny* aContext, bool aErrorCondition)
{
    OSCL_ASSERT(iSourceTrackInfo != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Teardown() In %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    // Disconnect the ports
    if (!iSourceOutPort || !iSinkInPort ||
            !iSourceNode || !iSinkNode)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Teardown() Source/sink nodes and/or ports are not set!"));
        return PVMFFailure;
    }

    iSourceOutPort->Disconnect();

    if (iDatapathConfig == CONFIG_DEC)
    {
        if (!iDecInPort || !iDecOutPort || !iDecNode)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Teardown() Decoder node and/or ports are not set"));
            return PVMFFailure;
        }

        iDecOutPort->Disconnect();
    }

    iContext = aContext;
    iErrorCondition = aErrorCondition;
    iErrorOccurredDuringErrorCondition = false;
    iState = TEARDOWN_RELEASEPORT1;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Teardown() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerDatapath::Reset(OsclAny* aContext, bool aErrorCondition)
{
    OSCL_ASSERT(iSourceTrackInfo != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Reset() In %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    iContext = aContext;
    iErrorCondition = aErrorCondition;
    iErrorOccurredDuringErrorCondition = false;
    iState = RESET_RESET;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Reset() Out"));
    return PVMFSuccess;
}


PVMFStatus PVPlayerDatapath::CancelCommand(OsclAny* aContext)
{
    OSCL_ASSERT(iSourceTrackInfo != NULL);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::CancelCommand() In %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

    iContext = aContext;

    // Need to cancel node command
    iState = PVPDP_CANCEL;
    RunIfNotReady();

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::CancelCommand() Out"));
    return PVMFSuccess;
}


void PVPlayerDatapath::DisconnectNodeSession(void)
{
    int32 leavecode = 0;

    // Emergency case only to disconnect from nodes
    if (iSourceNode)
    {
        leavecode = 0;
        OSCL_TRY(leavecode, iSourceNode->Disconnect(iSourceSessionId));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::DisconnectNodeSession() Disconnect on iSourceNode did a leave")));
    }

    if (iSinkNode)
    {
        leavecode = 0;
        OSCL_TRY(leavecode, iSinkNode->Disconnect(iSinkSessionId));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::DisconnectNodeSession() Disconnect on iSinkNode did a leave")));
    }

    if (iDatapathConfig == CONFIG_DEC && iDecNode)
    {
        leavecode = 0;
        OSCL_TRY(leavecode, iDecNode->Disconnect(iDecSessionId));
        OSCL_FIRST_CATCH_ANY(leavecode,
                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::DisconnectNodeSession() Disconnect on iDecNode did a leave")));
    }
}


void PVPlayerDatapath::Run()
{
    int32 leavecode = 0;
    PVMFCommandId cmdid = 0;

    switch (iState)
    {
        case PREPARE_INIT:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing PREPARE_INIT case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));
            iPendingCmds = 0;
            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Init() on dec node"));
                leavecode = IssueDatapathInit(iDecNode, iDecSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Init on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Init() on sink node"));
            leavecode = IssueDatapathInit(iSinkNode, iSinkSessionId, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Init on iSinkNode did a leave"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }
            break;

        case PREPARE_REQPORT:
        {
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing PREPARE_REQPORT case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling RequestPort() on source node"));
            leavecode = IssueDatapathRequestPort(iSourceNode, iSourceSessionId, iSourceTrackInfo->getPortTag(),
                                                 &(iSourceTrackInfo->getTrackMimeType()),
                                                 (OsclAny*)iSourceNode, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() RequestPort on iSourceNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling RequestPort() on dec node(input)"));
                leavecode = IssueDatapathRequestPort(iDecNode, iDecSessionId, DEFAULT_INPUT_PORTTAG,
                                                     &(iSourceTrackInfo->getTrackMimeType()),
                                                     (OsclAny*)iSourceTrackInfo, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() RequestPort on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling RequestPort() on dec node(output)"));
                leavecode = IssueDatapathRequestPort(iDecNode, iDecSessionId, DEFAULT_OUTPUT_PORTTAG,
                                                     &iDecSinkFormatString, (OsclAny*) & iDecSinkFormatString, cmdid);


                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() RequestPort on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling RequestPort() on sink node"));
                leavecode = IssueDatapathRequestPort(iSinkNode, iSinkSessionId, DEFAULT_INPUT_PORTTAG,
                                                     &iDecSinkFormatString, (OsclAny*)iSinkNode, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() RequestPort on iSinkNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling RequestPort() on sink node"));
                leavecode = IssueDatapathRequestPort(iSinkNode, iSinkSessionId, DEFAULT_INPUT_PORTTAG,
                                                     &(iSourceTrackInfo->getTrackMimeType()),
                                                     (OsclAny*)iSinkNode, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() RequestPort on iSinkNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
            }
        }
        break;

        case PREPARE_CONNECT:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing PREPARE_CONNECT case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            if (iDatapathConfig == CONFIG_DEC)
            {
                OsclAny* temp = NULL;
                iSourceOutPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                PvmiCapabilityAndConfig *portconfigif = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (portconfigif)
                {
                    pvmiSetPortFormatSync(portconfigif, PORT_CONFIG_INPUT_FORMATS_VALTYPE, iSourceDecFormatType);
                }
                else
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Port config IF for source node's outport not available"));
                    break;
                }

                temp = NULL;
                iDecInPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                portconfigif = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (portconfigif)
                {
                    pvmiSetPortFormatSync(portconfigif, PORT_CONFIG_INPUT_FORMATS_VALTYPE, iSourceDecFormatType);
                }
                else
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Port config IF for dec node's inport not available"));
                    break;
                }

                if (iSourceOutPort->Connect(iDecInPort) != PVMFSuccess)
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Connect on source and dec ports failed"));
                    break;
                }

                temp = NULL;
                iDecOutPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                portconfigif = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (portconfigif)
                {
                    pvmiSetPortFormatSync(portconfigif, PORT_CONFIG_INPUT_FORMATS_VALTYPE, iDecSinkFormatType);
                }
                else
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Port config IF for dec node's outport not available"));
                    break;
                }

                temp = NULL;
                iSinkInPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                portconfigif = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (portconfigif)
                {
                    pvmiSetPortFormatSync(portconfigif, PORT_CONFIG_INPUT_FORMATS_VALTYPE, iDecSinkFormatType);
                }
                else
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Port config IF for sink node's inport not available"));
                    break;
                }

                if (iDecOutPort->Connect(iSinkInPort) != PVMFSuccess)
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Connect on dec and sink ports failed"));
                    break;
                }
            }
            else
            {
                OsclAny* temp;
                iSourceOutPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                PvmiCapabilityAndConfig *portconfigif = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (portconfigif)
                {
                    pvmiSetPortFormatSync(portconfigif, PORT_CONFIG_INPUT_FORMATS_VALTYPE, iSourceSinkFormatType);
                }
                else
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Port config IF for source node's outport not available"));
                    break;
                }

                temp = NULL;
                iSinkInPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
                portconfigif = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);
                if (portconfigif)
                {
                    pvmiSetPortFormatSync(portconfigif, PORT_CONFIG_INPUT_FORMATS_VALTYPE, iSourceSinkFormatType);
                }
                else
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Port config IF for sink node's inport not available"));
                    break;
                }

                if (iSourceOutPort->Connect(iSinkInPort) != PVMFSuccess)
                {
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Connect on source and sink ports failed"));
                    break;
                }
            }

            iState = PREPARE_PREPARE;
            RunIfNotReady();
            break;

        case PREPARE_PREPARE:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing PREPARE_PREPARE case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Prepare() on sink node"));
            leavecode = IssueDatapathPrepare(iSinkNode, iSinkSessionId, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
                if (iDatapathConfig == CONFIG_DEC)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Prepare() on dec node"));
                    leavecode = IssueDatapathPrepare(iDecNode, iDecSessionId, cmdid);

                    if (cmdid != -1 && leavecode == 0)
                    {
                        ++iPendingCmds;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Prepare on iDecNode did a leave or failed"));
                        iState = PVPDP_ERROR;
                        RunIfNotReady();
                        break;
                    }
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Prepare on iSinkNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }
            break;

        case PREPARED:
            break;

        case START_START:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing START_START case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Start() on sink node"));
            leavecode = IssueDatapathStart(iSinkNode, iSinkSessionId, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Start on iSinkNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Start() on dec node"));
                leavecode = IssueDatapathStart(iDecNode, iDecSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Start on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
            }
            break;

        case STARTED:
            break;

        case PAUSE_PAUSE:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing PAUSE_PAUSE case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            if (iSinkPaused == false)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Pause() on sink node"));
                leavecode = IssueDatapathPause(iSinkNode, iSinkSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Pause on iSinkNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Sink already paused so skipping Pause() on sink"));
                iSinkPaused = false;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Pause() on dec node"));
                leavecode = IssueDatapathPause(iDecNode, iDecSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Pause on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
            }

            if (iPendingCmds == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() No pending node command in PAUSE_PAUSE so going to PAUSED"));
                iState = PAUSED;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Pause() command completed successfully"));
                iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
            }
            break;

        case PAUSED:
            break;

        case STOP_STOP:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing STOP_STOP case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Stop() on sink node"));
            leavecode = IssueDatapathStop(iSinkNode, iSinkSessionId, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else if (!iErrorCondition)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Stop on iSinkNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }
            else
            {
                iErrorOccurredDuringErrorCondition = true;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Stop() on dec node"));
                leavecode = IssueDatapathStop(iDecNode, iDecSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else if (!iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Stop on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
                else
                {
                    iErrorOccurredDuringErrorCondition = true;
                }
            }

            if (iPendingCmds == 0 && iErrorCondition)
            {
                iState = PVPDP_CANCELLED;
                if (iErrorCondition && iErrorOccurredDuringErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Stop() command completed with errors during error condition"));
                    iErrorCondition = false;
                    iObserver->HandlePlayerDatapathEvent(0, PVMFFailure, iContext);
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Stop() command completed successfully"));
                    iErrorCondition = false;
                    iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                }
            }
            break;

        case STOPPED:
            break;

            // Note: ReleasePort must be done in two stages with dest ports first
            // so the media data is freed in the dest port before the source port
        case TEARDOWN_RELEASEPORT1:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing TEARDOWN_RELEASEPORT1 case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling ReleasePort() on sink node"));
            leavecode = IssueDatapathReleasePort(iSinkNode, iSinkSessionId, iSinkInPort, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else if (!iErrorCondition)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() ReleasePort on iSinkNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }
            else
            {
                iErrorOccurredDuringErrorCondition = true;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling ReleasePort() on dec node(input)"));
                leavecode = IssueDatapathReleasePort(iDecNode, iDecSessionId, iDecInPort, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else if (!iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() ReleasePort on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
                else
                {
                    iErrorOccurredDuringErrorCondition = true;
                }
            }

            if (iPendingCmds == 0 && iErrorCondition)
            {
                iState = TEARDOWN_RELEASEPORT2;
                RunIfNotReady();
            }
            break;

        case TEARDOWN_RELEASEPORT2:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing TEARDOWN_RELEASEPORT2 case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling ReleasePort() on source node"));
            leavecode = IssueDatapathReleasePort(iSourceNode, iSourceSessionId, iSourceOutPort, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else if (!iErrorCondition)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() ReleasePort on iSourceNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }
            else
            {
                iErrorOccurredDuringErrorCondition = true;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling ReleasePort() on dec node(output)"));
                leavecode = IssueDatapathReleasePort(iDecNode, iDecSessionId, iDecOutPort, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else if (!iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() ReleasePort on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
                else
                {
                    iErrorOccurredDuringErrorCondition = true;
                }
            }

            if (iPendingCmds == 0 && iErrorCondition)
            {
                iState = TEARDOWNED;
                if (iErrorCondition && iErrorOccurredDuringErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Teardown() command completed with errors during error condition"));
                    iErrorCondition = false;
                    iObserver->HandlePlayerDatapathEvent(0, PVMFFailure, iContext);
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Teardown() command completed successfully"));
                    iErrorCondition = false;
                    iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                }
            }
            break;

        case TEARDOWNED:
            break;

        case RESET_RESET:
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing RESET_RESET case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            iPendingCmds = 0;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Reset() on sink node"));
            leavecode = IssueDatapathReset(iSinkNode, iSinkSessionId, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else if (!iErrorCondition)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Reset on iSinkNode did a leave or failed"));
                iState = PVPDP_ERROR;
                RunIfNotReady();
                break;
            }
            else
            {
                iErrorOccurredDuringErrorCondition = true;
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Reset() on dec node"));
                leavecode = IssueDatapathReset(iDecNode, iDecSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else if (!iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Reset on iDecNode did a leave or failed"));
                    iState = PVPDP_ERROR;
                    RunIfNotReady();
                    break;
                }
                else
                {
                    iErrorOccurredDuringErrorCondition = true;
                }
            }

            if (iPendingCmds == 0 && iErrorCondition)
            {
                iState = RESETTED;
                RunIfNotReady();
            }
            break;

        case RESETTED:
        {
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing RESETTED case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            PVMFStatus retval;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling Disconnect() on nodes"));

            retval = iSourceNode->Disconnect(iSourceSessionId);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Disconnect on iSourceNode failed, asserting"));
                OSCL_ASSERT(false);
            }

            retval = iSinkNode->Disconnect(iSinkSessionId);
            if (retval != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Disconnect on iSinkNode failed, asserting"));
                OSCL_ASSERT(false);
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                retval = iDecNode->Disconnect(iDecSessionId);
                if (retval != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Disconnect on iDecNode failed, asserting"));
                    OSCL_ASSERT(false);
                }
            }

            iState = PVPDP_IDLE;
            if (iErrorCondition && iErrorOccurredDuringErrorCondition)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Reset() command completed with errors during error condition"));
                iErrorCondition = false;
                iObserver->HandlePlayerDatapathEvent(0, PVMFFailure, iContext);
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Reset() command completed successfully"));
                iErrorCondition = false;
                iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
            }
        }
        break;

        case PVPDP_CANCEL:
        {
            OSCL_ASSERT(iSourceTrackInfo != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Processing PVPDP_CANCEL case for %s", iSourceTrackInfo->getTrackMimeType().get_cstr()));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling CancelAllCommands() on sink node"));
            leavecode = IssueDatapathCancel(iSinkNode, iSinkSessionId, cmdid);

            if (cmdid != -1 && leavecode == 0)
            {
                ++iPendingCmds;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() CancelAllCommands on iSinkNode did a leave or failed"));
            }

            if (iDatapathConfig == CONFIG_DEC)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Calling CancelAllCommands() on dec node"));
                leavecode = IssueDatapathCancel(iDecNode, iDecSessionId, cmdid);

                if (cmdid != -1 && leavecode == 0)
                {
                    ++iPendingCmds;
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() CancelAllCommands on iDecNode did a leave or failed"));
                }
            }

            if (iPendingCmds == 0)
            {
                // If all CancelAllCommands() weren't accepted, assume everything is cancelled so report cancel complete
                iState = PVPDP_CANCELLED;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Cancel() command completed successfully"));
                iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
            }
        }
        break;

        case PVPDP_CANCELLED:
            break;

        case PVPDP_ERROR:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::Run() Report command failed"));
            iState = PVPDP_CANCELLED;
            iObserver->HandlePlayerDatapathEvent(0, PVMFFailure, iContext);
            break;

        default:
            break;
    }
}


void PVPlayerDatapath::NodeCommandCompleted(const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() In"));

    switch (iState)
    {
        case PREPARE_INIT:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node Init() requests completed successfully"));
                    iState = PREPARE_REQPORT;
                    RunIfNotReady();
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in PREPARE_INIT state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case PREPARE_REQPORT:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (aResponse.GetContext() == (OsclAny*)iSourceNode)
                {
                    iSourceOutPort = (PVMFPortInterface*)(aResponse.GetEventData());
                }
                else if (aResponse.GetContext() == (OsclAny*)iSinkNode)
                {
                    iSinkInPort = (PVMFPortInterface*)(aResponse.GetEventData());
                }
                else if (aResponse.GetContext() == (OsclAny*) iSourceTrackInfo)
                {
                    iDecInPort = (PVMFPortInterface*)(aResponse.GetEventData());
                }
                else if (aResponse.GetContext() == (OsclAny*) &iDecSinkFormatString)
                {
                    iDecOutPort = (PVMFPortInterface*)(aResponse.GetEventData());
                }
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node RequestPort() requests completed successfully"));
                    iState = PREPARE_CONNECT;
                    RunIfNotReady();
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in PREPARE_REQPORT state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case PREPARE_PREPARE:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node Prepare() requests completed successfully"));
                    iState = PREPARED;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Prepare() command completed successfully"));
                    iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in PREPARE_PREPARE state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case START_START:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node Start() requests completed successfully"));
                    iState = STARTED;
                    // Send event immediately- iContext might get changed before its next Run
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Start() command completed successfully"));
                    iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in START_START state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case PAUSE_PAUSE:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node Pause() requests completed successfully"));
                    iState = PAUSED;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Pause() command completed successfully"));
                    iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in PAUSE_PAUSE state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case STOP_STOP:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess || iErrorCondition)
            {
                if (aResponse.GetCmdStatus() != PVMFSuccess && iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node reported error in Stop() during error condition"));
                    iErrorOccurredDuringErrorCondition = true;
                }
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node Stop() requests completed"));
                    iState = PVPDP_IDLE;
                    if (iErrorCondition && iErrorOccurredDuringErrorCondition)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Stop() command completed with errors during error condition"));
                        iErrorCondition = false;
                        iObserver->HandlePlayerDatapathEvent(0, PVMFFailure, iContext);
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Stop() command completed successfully"));
                        iErrorCondition = false;
                        iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                    }
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in STOP_STOP state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case RESET_RESET:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess || iErrorCondition)
            {
                if (aResponse.GetCmdStatus() != PVMFSuccess && iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node reported error in Reset() during error condition"));
                    iErrorOccurredDuringErrorCondition = true;
                }
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node Reset() requests completed"));
                    iState = RESETTED;
                    RunIfNotReady();
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in RESET_RESET state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case TEARDOWN_RELEASEPORT1:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess || iErrorCondition)
            {
                if (aResponse.GetCmdStatus() != PVMFSuccess && iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node reported error in ReleasePort() during error condition(1st set)"));
                    iErrorOccurredDuringErrorCondition = true;
                }
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node ReleasePort() requests completed(1st set)"));
                    iState = TEARDOWN_RELEASEPORT2;
                    RunIfNotReady();
                    break;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in TEARDOWN_RELEASEPORT1 state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case TEARDOWN_RELEASEPORT2:
            --iPendingCmds;
            if (aResponse.GetCmdStatus() == PVMFSuccess || iErrorCondition)
            {
                if (aResponse.GetCmdStatus() != PVMFSuccess && iErrorCondition)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node reported error in ReleasePort() during error condition(2nd set)"));
                    iErrorOccurredDuringErrorCondition = true;
                }
                if (iPendingCmds == 0)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node ReleasePort() requests completed(2nd set)"));
                    iState = TEARDOWNED;
                    if (iErrorCondition && iErrorOccurredDuringErrorCondition)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Teardown() command completed with errors during error condition"));
                        iErrorCondition = false;
                        iObserver->HandlePlayerDatapathEvent(0, PVMFFailure, iContext);
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::Run() Report Teardown() command completed successfully"));
                        iErrorCondition = false;
                        iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
                    }
                    break;
                }
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVPlayerDatapath::NodeCommandCompleted() Node command failed in TEARDOWN_RELEASEPORT2 state"));
                iState = PVPDP_CANCELLED;
                iObserver->HandlePlayerDatapathEvent(0, aResponse.GetCmdStatus(), iContext, (PVMFCmdResp*)&aResponse);
            }
            break;

        case PVPDP_CANCEL:
            // When cancelling, don't care about the command status
            --iPendingCmds;
            if (iPendingCmds == 0)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() All node CancelAllCommands() requests completed"));
                iState = PVPDP_CANCELLED;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() Report Cancel() command completed successfully"));
                iObserver->HandlePlayerDatapathEvent(0, PVMFSuccess, iContext);
            }
            break;

        case PVPDP_CANCELLED:
        case PVPDP_IDLE:
            --iPendingCmds;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() Report %d Pending command in NodeCommandcompleted ", iPendingCmds));
            break;

        default:
            break;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVPlayerDatapath::NodeCommandCompleted() Out"));
}


void PVPlayerDatapath::HandleNodeInformationalEvent(const PVMFAsyncEvent& /*aEvent*/)
{
    // Ignore node info events since the engine will receive it directly from the nodes
}


void PVPlayerDatapath::HandleNodeErrorEvent(const PVMFAsyncEvent& /*aEvent*/)
{
    // Ignore node error events since the engine will receive it directly from the nodes
}

PVMFStatus PVPlayerDatapath::IssueDatapathInit(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->Init(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathRequestPort(PVMFNodeInterface* aNode, PVMFSessionId aSessionId,
        int32 aPortTag, PvmfMimeString* aPortConfig,
        OsclAny* aContext, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->RequestPort(aSessionId, aPortTag, aPortConfig, aContext));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathPrepare(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->Prepare(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathStart(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->Start(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathPause(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->Pause(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathStop(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->Stop(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathReleasePort(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFPortInterface* aPort, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->ReleasePort(aSessionId, *aPort));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathReset(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->Reset(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}

PVMFStatus PVPlayerDatapath::IssueDatapathCancel(PVMFNodeInterface* aNode, PVMFSessionId aSessionId, PVMFCommandId &aCmdId)
{
    PVMFStatus leavecode;
    OSCL_TRY(leavecode, aCmdId = aNode->CancelAllCommands(aSessionId));
    OSCL_FIRST_CATCH_ANY(leavecode,;);
    return leavecode;
}


