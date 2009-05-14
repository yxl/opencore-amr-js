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
#include "pv_2way_datapath.h"
#include "pvmi_config_and_capability.h"
#include "pv_mime_string_utils.h"

bool CPV2WayPortPair::Connect()
{
    if ((iSrcPort.GetStatus() == EHasPort) &&
            (iDestPort.GetStatus() == EHasPort))
    {
        if (iSrcPort.GetPort()->Connect(iDestPort.GetPort()) == PVMFSuccess)
        {
            iIsConnected = true;
            return true;
        }
    }
    return false;
}

bool CPV2WayPortPair::Disconnect()
{
    if ((iSrcPort.GetStatus() == EHasPort) &&
            (iDestPort.GetStatus() == EHasPort))
    {
        if (iSrcPort.GetPort()->Disconnect() == PVMFSuccess)
        {
            iIsConnected = false;
            return true;
        }
    }
    return false;
}

CPVDatapathPort& CPVDatapathPort::operator=(const CPVDatapathPort & a)
{
    iRequestPortState = a.iRequestPortState;
    iCanCancelPort = a.iCanCancelPort;
    iPortSetType = a.iPortSetType;
    iFormatType = a.iFormatType;
    iDefaultFormatType = a.iDefaultFormatType;
    iPortTag = a.iPortTag;
    iPortPair = a.iPortPair;
    return *this;
}

CPVDatapathNode& CPVDatapathNode::operator=(const CPVDatapathNode & a)
{
    iNode = a.iNode;
    iConfigure = a.iConfigure;
    iConfigTime = a.iConfigTime;
    iCanNodePause = a.iCanNodePause;
    iLoggoffOnReset = a.iLoggoffOnReset;
    iOriginalState = a.iOriginalState;
    iInputPort = a.iInputPort;
    iOutputPort = a.iOutputPort;
    iCommandIssued = a.iCommandIssued;
    return *this;
}

bool CPV2WayDatapath::IsPortInDatapath(PVMFPortInterface *aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::IsPortInDatapath state %d, num port pairs %d\n", iState, iPortPairList.size()));
    if (aPort)
    {
        for (uint32 i = 0; i < iPortPairList.size(); i++)
        {
            if ((iPortPairList[i].iSrcPort.GetPort() == aPort) ||
                    (iPortPairList[i].iDestPort.GetPort() == aPort))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::IsPortInDatapath port found at idx %d\n", i));
                return true;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::IsPortInDatapath no node found\n"));
    return false;
}

bool CPV2WayDatapath::IsNodeInDatapath(PVMFNodeInterface *aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::IsNodeInDatapath state %d, num nodes %d\n", iState, iNodeList.size()));
    if (aNode)
    {
        for (uint32 i = 0; i < iNodeList.size(); i++)
        {
            if ((PVMFNodeInterface *)iNodeList[i].iNode == aNode)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::IsNodeInDatapath node found at idx %d\n", i));
                return true;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::IsNodeInDatapath no node found\n"));
    return false;
}

bool CPV2WayDatapath::ResetDatapath()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::ResetDatapath state %d, num nodes %d\n", iState, iNodeList.size()));

    if (iState == EClosed)
    {
        for (uint32 i = 0; i < iNodeList.size(); i++)
        {
            if (iNodeList[i].iLoggoffOnReset)
            {
                PVLOGGER_LOG_USE_ONLY(PVMFStatus status =)((PVMFNodeInterface *)iNodeList[i].iNode)->ThreadLogoff();
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::ResetDatapath thread logoff status %d\n", status));
            }
        }

        iPortPairList.clear();
        iNodeList.clear();
        return true;
    }
    else
    {
        return false;
    }
}

bool CPV2WayDatapath::AddNode(const CPVDatapathNode &aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::AddNode path state %d, num nodes %d\n", iState, iNodeList.size()));

    if ((iState == EClosed) &&
            (iNodeList.size() < MAX_DATAPATH_NODES))
    {
        CPVDatapathNode node(aNode);

        node.iOriginalState = ((PVMFNodeInterface *)(node.iNode))->GetState();
        node.iInputPort.iPortPair = NULL;
        node.iOutputPort.iPortPair = NULL;

        if (!iNodeList.empty())
        {
            CPV2WayPortPair portPair;
            iPortPairList.push_back(portPair);
            iNodeList.back().iOutputPort.iPortPair = &(iPortPairList.back());
            node.iInputPort.iPortPair = &(iPortPairList.back());
        }

        iNodeList.push_back(node);
        return true;
    }
    return false;
}

bool CPV2WayDatapath::Open()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath(%s)::Open path type %d, state %d, num nodes %d\n", iFormat.getMIMEStrPtr(), iType, iState, iNodeList.size()));
    if (SingleNodeOpen() || iNodeList.size() > 1)
    {
        switch (iState)
        {
            case EClosed:
                SetState(EOpening);
                CheckPath();
                //Fall through to next case.

            case EOpening:
                return true;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::Open invalid state to open\n"));
                break;
        }
    }
    return false;
}

bool CPV2WayDatapath::Close()
{
    uint32 i;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::Close path type %d, state %d\n", iType, iState));

    if (iFsi)
    {
        OSCL_DEFAULT_FREE(iFsi);
        iFsi = NULL;
    }
    iFsiLen = 0;

    switch (iState)
    {
        default:
            //Disconnect all port connections
            for (i = 0; i < iPortPairList.size(); i++)
            {
                iPortPairList[i].Disconnect();
            }

            iAllPortsConnected = false;

            iStateBeforeClose = iState;
            SetState(EClosing);

            //Close dependent paths
            for (i = 0; i < iDependentPathList.size(); i++)
            {
                iDependentPathList[i]->Close();
            }

            CheckPath();
            //Fall through to next case.

        case EClosing:
            return true;

        case EClosed:
            return false;
    }
}

bool CPV2WayDatapath::Pause()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::Pause path type %d, state %d\n", iType, iState));
    switch (iState)
    {
        case EOpened:
            SetState(EPausing);
            CheckPath();
            //Fall through to next case.

        case EPausing:
            return true;

        default:
            return false;
    }
}

bool CPV2WayDatapath::Resume()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::Resume path type %d, state %d\n", iType, iState));
    switch (iState)
    {
        case EPaused:
            SetState(EUnpausing);
            CheckPath();
            //Fall through to next case.

        case EUnpausing:
            return true;

        default:
            return false;
    }
}

bool CPV2WayDatapath::AddParentDatapath(CPV2WayDatapath &aDatapath)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::AddParentDatapath path state %d, num paths %d\n", iState, iParentPathList.size()));

    if ((iState == EClosed) &&
            (iParentPathList.size() < MAX_PARENT_PATHS))
    {
        iParentPathList.push_back(&aDatapath);
        return true;
    }
    return false;
}

bool CPV2WayDatapath::AddDependentDatapath(CPV2WayDatapath &aDatapath)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "CPV2WayDatapath::AddDependentDatapath path state %d, num paths %d\n", iState, iDependentPathList.size()));

    if ((iState == EClosed) &&
            (iDependentPathList.size() < MAX_PARENT_PATHS))
    {
        iDependentPathList.push_back(&aDatapath);
        return true;
    }
    return false;
}

void CPV2WayDatapath::ConstructL()
{
    iNodeList.reserve(MAX_DATAPATH_NODES);
    iPortPairList.reserve(MAX_DATAPATH_NODES);
    iParentPathList.reserve(MAX_PARENT_PATHS);
    iDependentPathList.reserve(MAX_DEPENDENT_PATHS);
    iFsi = NULL;
    iFsiLen = 0;
}

void CPV2WayDatapath::SetState(TPV2WayDatapathState aState)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath(%d)::SetState cur %d, new %d\n", iType, iState, aState));
    iState = aState;
}

bool CPV2WayDatapath::IsDatapathNodeClosed(CPVDatapathNode &aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::IsDatapathNodeClosed\n"));
    if (aNode.iCommandIssued) return false;

    if (aNode.iInputPort.iPortPair &&
            (aNode.iInputPort.iPortPair->iDestPort.GetStatus() != ENoPort)) return false;

    if (aNode.iOutputPort.iPortPair &&
            (aNode.iOutputPort.iPortPair->iSrcPort.GetStatus() != ENoPort)) return false;

    /* Gkl - We check for node states that are less than or equal to the state in which the node was added
    This is becoz the states are assymetric which sucks*/
    if (!aNode.iIgnoreNodeState &&
            (((PVMFNodeInterface *)(aNode.iNode))->GetState() > aNode.iOriginalState)) return false;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::IsDatapathNodeClosed node is closed\n"));

    return true;
}

bool CPV2WayDatapath::CheckNodePortsL(CPVDatapathNode &aNode)
{
    PVMFStatus status;
    TPV2WayNodeRequestPortParams reqPortParams;
    CPV2WayPortPair *inPortPair = aNode.iInputPort.iPortPair;
    CPV2WayPortPair *outPortPair = aNode.iOutputPort.iPortPair;
    CPVDatapathPort *dataPort;
    bool requestPort;
    bool portRequestDone = true;


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts in req port state %d, port %x\n", aNode.iInputPort.iRequestPortState, inPortPair));

    if (((PVMFNodeInterface *)(aNode.iNode))->GetState() >= aNode.iInputPort.iRequestPortState)
    {
        if (inPortPair)
        {
            dataPort = (CPVDatapathPort *) & aNode.iInputPort;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts in port state %d\n", inPortPair->iDestPort.GetStatus()));
            if (inPortPair->iDestPort.GetStatus() == ENoPort)
            {
                requestPort = false;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts in port request set type %d\n", aNode.iInputPort.iPortSetType));

                switch (dataPort->iPortSetType)
                {
                    case EUserDefined:
                        requestPort = true;
                        break;

                    case EConnectedPortFormat:
                        if (inPortPair->iSrcPort.GetStatus() == EHasPort)
                        {
                            dataPort->iFormatType = GetPortFormatType(*inPortPair->iSrcPort.GetPort(), false, inPortPair->iDestPort.GetPort());
                            OSCL_ASSERT(dataPort->iFormatType != PVMF_MIME_FORMAT_UNKNOWN);
                            requestPort = true;
                        }
                        break;

                    case EUseOtherNodePortFormat:
                        if (outPortPair->iSrcPort.GetStatus() == EHasPort)
                        {
                            dataPort->iFormatType = GetPortFormatType(*outPortPair->iSrcPort.GetPort(), false, outPortPair->iDestPort.GetPort());
                            OSCL_ASSERT(dataPort->iFormatType != PVMF_MIME_FORMAT_UNKNOWN);
                            requestPort = true;
                        }
                        break;

                    case EAppDefined:
                        requestPort = true;
                        break;

                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckNodePorts undefined set type\n"));
                        break;
                }

                if (requestPort)
                {
                    reqPortParams.portTag = dataPort->iPortTag;
                    reqPortParams.format = dataPort->iFormatType;

                    status = CheckConfig(EConfigBeforeReqInPort, aNode);
                    switch (status)
                    {
                        case PVMFPending:
                            break;

                        case PVMFSuccess:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts request input port, format %s, tag %d\n", reqPortParams.format.getMIMEStrPtr(), reqPortParams.portTag));
                            inPortPair->iDestPort.SetCmdId(SendNodeCmdL(PV2WAY_NODE_CMD_REQUESTPORT, aNode, &reqPortParams));
                            inPortPair->iDestPort.SetStatus(ERequestPort);
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckNodePorts node config failed\n"));
                            OSCL_LEAVE(status);
                            return false;
                    }
                }
            }

            if (inPortPair->iDestPort.GetStatus() != EHasPort)
            {
                portRequestDone = false;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts out req port state %d, port %x\n", aNode.iOutputPort.iRequestPortState, outPortPair));

    if (((PVMFNodeInterface *)(aNode.iNode))->GetState() >= aNode.iOutputPort.iRequestPortState)
    {
        if (outPortPair)
        {
            dataPort = (CPVDatapathPort *) & aNode.iOutputPort;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts out port state %d\n", outPortPair->iSrcPort.GetStatus()));
            if (outPortPair->iSrcPort.GetStatus() == ENoPort)
            {
                requestPort = false;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts out port request set type %d\n", aNode.iOutputPort.iPortSetType));

                switch (dataPort->iPortSetType)
                {
                    case EUserDefined:
                        requestPort = true;
                        break;

                    case EConnectedPortFormat:
                        if (outPortPair->iDestPort.GetStatus() == EHasPort)
                        {
                            dataPort->iFormatType = GetPortFormatType(*outPortPair->iDestPort.GetPort(), true, outPortPair->iSrcPort.GetPort());
                            OSCL_ASSERT(dataPort->iFormatType != PVMF_MIME_FORMAT_UNKNOWN);
                            requestPort = true;
                        }
                        break;

                    case EUseOtherNodePortFormat:
                        if (inPortPair->iDestPort.GetStatus() == EHasPort)
                        {
                            dataPort->iFormatType = GetPortFormatType(*inPortPair->iDestPort.GetPort(), true, inPortPair->iSrcPort.GetPort());
                            OSCL_ASSERT(dataPort->iFormatType != PVMF_MIME_FORMAT_UNKNOWN);
                            requestPort = true;
                        }
                        break;

                    case EAppDefined:
                        requestPort = true;
                        break;

                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckNodePorts undefined set type\n"));
                        break;
                }

                if (requestPort)
                {
                    reqPortParams.portTag = dataPort->iPortTag;
                    reqPortParams.format = dataPort->iFormatType;

                    status = CheckConfig(EConfigBeforeReqOutPort, aNode);
                    switch (status)
                    {
                        case PVMFPending:
                            break;

                        case PVMFSuccess:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckNodePorts request output port, format %s, type %d\n", reqPortParams.format.getMIMEStrPtr(), reqPortParams.portTag));
                            outPortPair->iSrcPort.SetCmdId(SendNodeCmdL(PV2WAY_NODE_CMD_REQUESTPORT, aNode, &reqPortParams));
                            outPortPair->iSrcPort.SetStatus(ERequestPort);
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckNodePorts node config failed\n"));
                            OSCL_LEAVE(status);
                            return false;
                    }
                }
            }

            if (outPortPair->iSrcPort.GetStatus() != EHasPort)
            {
                portRequestDone = false;
            }
        }
    }

    return portRequestDone;
}

PVMFStatus CPV2WayDatapath::PortStatusChange(PVMFNodeInterface *aNode, PVMFCommandId aId, PVMFPortInterface *aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::PortStatusChange id %d, port %x\n", aId, aPort));
    PVMFStatus status = PVMFFailure;
    int32 error = 0;
    CPV2WayPort *port = NULL;
    OsclAny *configPtr = NULL;
    PvmiKvp portParams;
    PvmiKvp *portParamsReturn = NULL;
    PVMFFormatType format = PVMF_MIME_FORMAT_UNKNOWN;
    CPV2WayPortPair *portPair = NULL;
    bool isInputPort = false;
    bool isAppDefined;

    for (uint32 i = 0; i < iNodeList.size(); i++)
    {
        if (((PVMFNodeInterface *)(iNodeList[i].iNode)) == aNode)
        {
            CPV2WayPortPair* inPortPair = iNodeList[i].iInputPort.iPortPair;
            CPV2WayPortPair* outPortPair = iNodeList[i].iOutputPort.iPortPair;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                            (0, "CPV2WayDatapath::PortStatusChange node %d\n", i));
            if (inPortPair &&
                    (inPortPair->iDestPort.GetCmdId() == aId))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                (0, "CPV2WayDatapath::PortStatusChange found input port\n"));
                format = iNodeList[i].iInputPort.iFormatType;
                port = &inPortPair->iDestPort;
                portPair = inPortPair;
                isInputPort = true;
                isAppDefined = (iNodeList[i].iInputPort.iPortSetType == EAppDefined);
            }
            else if (outPortPair &&
                     (outPortPair->iSrcPort.GetCmdId() == aId))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                (0, "CPV2WayDatapath::PortStatusChange found output port\n"));
                format = iNodeList[i].iOutputPort.iFormatType;
                port = &outPortPair->iSrcPort;
                portPair = outPortPair;
                isInputPort = false;
                isAppDefined = (iNodeList[i].iOutputPort.iPortSetType == EAppDefined);
            }
            break;
        }
    }

    if (port)
    {
        if (aPort && !isAppDefined)
        {
            aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, configPtr);

            //If config ptr exists, otherwise assume port has been configured.
            if (configPtr)
            {
                if (isInputPort)
                {
                    portParams.key = OSCL_CONST_CAST(char*, INPUT_FORMATS_VALTYPE);
                    portParams.length = oscl_strlen(INPUT_FORMATS_VALTYPE);
                    portParams.capacity = oscl_strlen(INPUT_FORMATS_VALTYPE);
                }
                else
                {
                    portParams.key = OSCL_CONST_CAST(char*, OUTPUT_FORMATS_VALTYPE);
                    portParams.length = oscl_strlen(OUTPUT_FORMATS_VALTYPE);
                    portParams.capacity = oscl_strlen(OUTPUT_FORMATS_VALTYPE);
                }

                portParams.value.pChar_value = OSCL_STATIC_CAST(mbchar*, format.getMIMEStrPtr());
                error = SetParametersSync((PvmiCapabilityAndConfig *)configPtr, &portParams, portParamsReturn);

                if (error || portParamsReturn != NULL)
                {
                    portParamsReturn = NULL;
                    portParams.key = OSCL_CONST_CAST(char*, "x-pvmf/port/formattype;valtype=char*");
                    portParams.length = oscl_strlen("x-pvmf/port/formattype;valtype=char*");
                    portParams.capacity = portParams.length;
                    portParams.value.pChar_value = OSCL_STATIC_CAST(mbchar*, format.getMIMEStrPtr());
                    error = SetParametersSync((PvmiCapabilityAndConfig *)configPtr,
                                              &portParams, portParamsReturn);
                }

                if ((error || portParamsReturn != NULL) && isInputPort)
                {
                    // this would be a file MIO component which requires explicit setting of audio and video formats
                    portParamsReturn = NULL;
                    if (format.isAudio())
                        portParams.key = OSCL_CONST_CAST(char*, MOUT_AUDIO_FORMAT_KEY);
                    else
                        portParams.key = OSCL_CONST_CAST(char*, MOUT_VIDEO_FORMAT_KEY);
                    portParams.length = oscl_strlen(portParams.key);
                    portParams.capacity = portParams.length;


                    /* This is for the MIO components having the convention
                       of returning uint32 for a query and requiring pChar for a setting
                       we don't know if we are talking to an MIO or a decoder node
                       (which will want a uint32), so we try both.  Try the pchar
                       first, because if its expecting pchar and gets uint32, it will
                       crash.
                    */

                    portParams.value.pChar_value = OSCL_STATIC_CAST(mbchar*, format.getMIMEStrPtr());

                    error = SetParametersSync((PvmiCapabilityAndConfig *)configPtr, &portParams, portParamsReturn);
                    if (error)
                    {
                        portParams.value.pChar_value = OSCL_STATIC_CAST(mbchar*, format.getMIMEStrPtr());
                        error = SetParametersSync((PvmiCapabilityAndConfig *)configPtr, &portParams, portParamsReturn);
                    }
                    portParamsReturn = NULL;
                    if (format.isAudio())
                    {
                        portParams.key = OSCL_CONST_CAST(char*, MOUT_AUDIO_SAMPLING_RATE_KEY);
                        portParams.value.uint32_value = 8000;
                    }
                    else
                    {
                        portParams.key = OSCL_CONST_CAST(char*, MOUT_VIDEO_HEIGHT_KEY);
                        portParams.value.uint32_value = 176;
                    }
                    portParams.length = oscl_strlen(portParams.key);
                    portParams.capacity = portParams.length;
                    error = SetParametersSync((PvmiCapabilityAndConfig *)configPtr,
                                              &portParams, portParamsReturn);
                    if (error && portParamsReturn != NULL)
                    {
                        error = 0;
                        portParamsReturn = NULL;
                    }
                }

                if (error || portParamsReturn != NULL)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::PortStatusChange setParametersSync failed %d at parameter %x!\n", error, portParamsReturn));
                }
                else
                {
                    status = PVMFSuccess;
                }
            }
            else
            {
                status = PVMFSuccess;
            }
        }
        else
        {
            status = isAppDefined ? PVMFSuccess : PVMFFailure;
        }

        SetPort(*port, aPort);
        if (status != PVMFSuccess)
        {
            if (portPair->iDestPort.GetPort() && portPair->iSrcPort.GetPort())
            {
                PVMFFormatType format2 = PVMF_MIME_FORMAT_UNKNOWN;
                // now try with two ports
                if (isInputPort)
                {
                    portParams.key = OSCL_CONST_CAST(char*, INPUT_FORMATS_VALTYPE);
                    portParams.length = oscl_strlen(INPUT_FORMATS_VALTYPE);
                    portParams.capacity = oscl_strlen(INPUT_FORMATS_VALTYPE);
                }
                else
                {
                    portParams.key = OSCL_CONST_CAST(char*, OUTPUT_FORMATS_VALTYPE);
                    portParams.length = oscl_strlen(OUTPUT_FORMATS_VALTYPE);
                    portParams.capacity = oscl_strlen(OUTPUT_FORMATS_VALTYPE);
                }
                format2 = GetPortFormatType(*portPair->iDestPort.GetPort(),
                                            true, portPair->iSrcPort.GetPort());
                portParams.value.pChar_value = OSCL_STATIC_CAST(mbchar*, format2.getMIMEStrPtr());
                error = SetParametersSync((PvmiCapabilityAndConfig *)configPtr, &portParams, portParamsReturn);
                if (!error)
                {
                    status = PVMFSuccess;
                }
            }
        }

        if (status == PVMFSuccess)
        {
            if ((portPair->iSrcPort.GetStatus() == EHasPort) &&
                    (portPair->iDestPort.GetStatus() == EHasPort))
            {
                if (!portPair->Connect())
                    return PVMFFailure;
            }
        }

        return status;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::PortStatusChange no port found!\n"));
        return status;
    }
}

int32 CPV2WayDatapath::SetParametersSync(PvmiCapabilityAndConfig * configPtr,
        PvmiKvp* portParams,
        PvmiKvp*& portParamsReturn)
{
    int32 error;
    OSCL_TRY(error, configPtr->setParametersSync(NULL, portParams, 1, portParamsReturn));
    return error;
}

void CPV2WayDatapath::CloseNodePorts(CPVDatapathNode &aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CloseNodePorts\n"));

    CPV2WayPortPair *portPair;

    portPair = aNode.iInputPort.iPortPair;
    if (portPair &&
            (portPair->iDestPort.GetStatus() == EHasPort))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CloseNodePorts closing input port\n"));
        portPair->iDestPort.SetPort(NULL);
    }

    portPair = aNode.iOutputPort.iPortPair;
    if (portPair &&
            (portPair->iSrcPort.GetStatus() == EHasPort))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CloseNodePorts closing output port\n"));
        portPair->iSrcPort.SetPort(NULL);
    }

    return;
}

PVMFCommandId CPV2WayDatapath::SendNodeCmdL(PV2WayNodeCmdType aCmd,
        CPVDatapathNode &aNode,
        void *aParam)
{
    PVMFCommandId id;
    id = i2Way->SendNodeCmdL(aCmd, &aNode.iNode, this, aParam);
    aNode.iCommandIssued = true;
    return id;
}

PVMFStatus CPV2WayDatapath::CheckConfig(TPVNodeConfigTimeType aConfigTime, CPVDatapathNode &aNode)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckConfig config %x, time %d\n", aNode.iConfigure, aConfigTime));
    if (aNode.iConfigure &&
            (aNode.iConfigTime == aConfigTime))
    {
        return aNode.iConfigure->ConfigureNode(&aNode);
    }

    return PVMFSuccess;
}

void CPV2WayDatapath::CheckOpen()
{
    uint32 i;
    bool checkPort = false;
    bool nodesStarted = true;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckOpen state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckOpen (0x%x) node %d state %d, cmd issued %d\n", this, i, iNodeList[i].iNode.iNode->GetState(), iNodeList[i].iCommandIssued));

        if (iNodeList[i].iCommandIssued)
        {
            nodesStarted = false;
            continue;
        }

        switch (iNodeList[i].iNode.iNode->GetState())
        {
            case EPVMFNodeIdle:
                nodesStarted = false;
                switch (CheckConfig(EConfigBeforeInit, iNodeList[i]))
                {
                    case PVMFPending:
                        continue;

                    case PVMFSuccess:
                        break;

                    default:
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen node config failed\n"));
                        DatapathError();
                        return;
                }

                if (!SendNodeCmd(PV2WAY_NODE_CMD_INIT, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen unable to initialize node\n"));
                    DatapathError();
                    return;
                }
                break;

            case EPVMFNodeInitialized:
                nodesStarted = false;

                if (!CheckNodePorts(checkPort, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen problem checking ports\n"));
                    DatapathError();
                    return;
                }

                if (checkPort)
                {
                    switch (CheckConfig(EConfigBeforeStart, iNodeList[i]))
                    {
                        case PVMFPending:
                            continue;

                        case PVMFSuccess:
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen node config failed\n"));
                            DatapathError();
                            return;
                    }

                    if (!SendNodeCmd(PV2WAY_NODE_CMD_PREPARE, i))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen unable to start node\n"));
                        DatapathError();
                        return;
                    }
                }
                break;

            case EPVMFNodePrepared:
                nodesStarted = false;
                //Make sure downstream node is started first.
                if ((i == iNodeList.size() - 1) || (iNodeList[i+1].iNode.iNode->GetState() == EPVMFNodeStarted))
                {
                    switch (CheckConfig(EConfigBeforeStart, iNodeList[i]))
                    {
                        case PVMFPending:
                            continue;

                        case PVMFSuccess:
                            break;

                        default:
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen node config failed\n"));
                            DatapathError();
                            return;
                    }

                    if (!CheckPathSpecificStart())
                    {
                        continue;
                    }

                    if (iAllPortsConnected || SingleNodeOpen())
                    {
                        if (!SendNodeCmd(PV2WAY_NODE_CMD_START, i))
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen unable to start node\n"));
                            DatapathError();
                            return;
                        }
                    }
                }
                break;

            case EPVMFNodeStarted:
                if (!CheckNodePorts(checkPort, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckOpen problem checking ports\n"));
                    DatapathError();
                    return;
                }
                break;

            default:
                nodesStarted = false;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckOpen transitional node state!\n"));
                break;
        }
    }

    //All nodes in path have not been started yet.
    if (!nodesStarted) return;

    //Check if all ports have been connected.
    if (!iAllPortsConnected && !SingleNodeOpen()) return;

    //Make path specific check
    if (!CheckPathSpecificOpen())
    {
        return;
    }

//Connect is done when both ports in a port pair are requested
//	//If reached this point then all ports have been allocated and datapath is deemed open, connect ports and notify upper layer.


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckOpen open complete\n"));

    SetState(EOpened);
    OpenComplete();
    return;
}

bool CPV2WayDatapath::CheckNodePorts(bool& aCheckPort, int32 i)
{
    int32 error;
    OSCL_TRY(error, aCheckPort = CheckNodePortsL(iNodeList[i]));
    OSCL_FIRST_CATCH_ANY(error,
                         return false;);
    return true;
}

void CPV2WayDatapath::CheckPause()
{
    uint32 i;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause node %d state %d, can pause %d, cmd issued %d\n", i, iNodeList[i].iNode.iNode->GetState(), iNodeList[i].iCanNodePause, iNodeList[i].iCommandIssued));

        if (!iNodeList[i].iCanNodePause) continue;
        if (iNodeList[i].iCommandIssued) continue;

        switch (iNodeList[i].iNode.iNode->GetState())
        {
            case EPVMFNodeStarted:
                if (!SendNodeCmd(PV2WAY_NODE_CMD_PAUSE, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckPause unable to pause node\n"));
                    DatapathError();
                    return;
                }

                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause transitional node state!\n"));
                break;
        }
    }

    for (i = 0; i < iNodeList.size(); i++)
    {
        //If possible pause node is not paused.
        if (iNodeList[i].iCanNodePause && (iNodeList[i].iNode.iNode->GetState() != EPVMFNodePaused))
        {
            return;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPause pause complete\n"));

    //If reached this point then the datapath is deemed paused, notify upper layer.
    SetState(EPaused);
    PauseComplete();
    return;
}

void CPV2WayDatapath::CheckResume()
{
    uint32 i;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume state %d\n", iState));

    for (i = 0; i < iNodeList.size(); i++)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume node %d state %d, can pause %d, cmd issued %d\n", i, iNodeList[i].iNode.iNode->GetState(), iNodeList[i].iCanNodePause, iNodeList[i].iCommandIssued));

        if (!iNodeList[i].iCanNodePause) continue;
        if (iNodeList[i].iCommandIssued) continue;

        switch (iNodeList[i].iNode.iNode->GetState())
        {
            case EPVMFNodePaused:
                if (!SendNodeCmd(PV2WAY_NODE_CMD_START, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckResume unable to pause node\n"));
                    DatapathError();
                    return;
                }
                break;

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume transitional node state!\n"));
                break;
        }
    }

    for (i = 0; i < iNodeList.size(); i++)
    {
        //If possible pause node is not started.
        if (iNodeList[i].iCanNodePause && (iNodeList[i].iNode.iNode->GetState() != EPVMFNodeStarted))
        {
            return;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckResume resume complete\n"));

    //If reached this point then the datapath is deemed resumed, notify upper layer.
    SetState(EOpened);
    ResumeComplete();
    return;
}

bool CPV2WayDatapath::SendNodeCmd(PV2WayNodeCmdType cmd, int32 i)
{
    int32 error;
    OSCL_TRY(error, SendNodeCmdL(cmd, iNodeList[i]));
    OSCL_FIRST_CATCH_ANY(error,
                         return false;);
    return true;
}

void CPV2WayDatapath::CheckClosed()
{
    int32 i;
    int32 error;
    PVMFCommandId id;
    CPV2WayPortPair *inPortPair;
    CPV2WayPortPair *outPortPair;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckClosed state %d\n", iState));

    if (!HaveAllDependentPathsClosed())
    {
        return;
    }

    if (!PathSpecificClose())
    {
        return;
    }

    // List must be closed one node at a time starting from the destination to the source to make sure all memory fragments are freed up to the correct node.
    for (i = (int32) iNodeList.size() - 1; i >= 0 ; i--)
    {

        if (IsDatapathNodeClosed(iNodeList[i])) continue;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckClosed node %d cur state %d, orig state %d, cmd issued %d\n", i, iNodeList[i].iNode.iNode->GetState(), iNodeList[i].iOriginalState, iNodeList[i].iCommandIssued));

        if (iNodeList[i].iCommandIssued) return;

        switch (iNodeList[i].iNode.iNode->GetState())
        {
            case EPVMFNodeCreated:
            case EPVMFNodeIdle:
                CloseNodePorts(iNodeList[i]);
                continue;

            case EPVMFNodeInitialized:
            case EPVMFNodePrepared:
            case EPVMFNodeError:
                CloseNodePorts(iNodeList[i]);
                if (iNodeList[i].CloseableState())
                    continue;

                if (!SendNodeCmd(PV2WAY_NODE_CMD_RESET, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckClosed unable to reset node\n"));
                    return;
                }
                return;

            case EPVMFNodeStarted:
            case EPVMFNodePaused:
            {
                inPortPair = iNodeList[i].iInputPort.iPortPair;
                outPortPair = iNodeList[i].iOutputPort.iPortPair;

                //Must at least release ports.

                if (inPortPair)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckClosed release in port, state %d, can cancel %d\n", inPortPair->iDestPort.GetStatus(), iNodeList[i].iInputPort.iCanCancelPort));
                    if (inPortPair->iDestPort.GetStatus() == EHasPort)
                    {
                        OSCL_TRY(error, inPortPair->iDestPort.SetCmdId(SendNodeCmdL(PV2WAY_NODE_CMD_RELEASEPORT, iNodeList[i], inPortPair->iDestPort.GetPort())));
                        OSCL_FIRST_CATCH_ANY(error,
                                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckClosed unable to release input port\n"));
                                             return;);

                        inPortPair->iDestPort.SetStatus(EReleasePort);
                        return;
                    }
                    else if ((inPortPair->iDestPort.GetStatus() == ERequestPort) &&
                             iNodeList[i].iInputPort.iCanCancelPort)
                    {
                        id = inPortPair->iDestPort.GetCmdId();
                        OSCL_TRY(error, inPortPair->iDestPort.SetCmdId(SendNodeCmdL(PV2WAY_NODE_CMD_CANCELCMD, iNodeList[i], &id)));
                        OSCL_FIRST_CATCH_ANY(error,
                                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckClosed unable to cancel request input port\n"));
                                             return;);

                        inPortPair->iDestPort.SetStatus(EReleasePort);
                        return;
                    }
                }

                if (outPortPair)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckClosed release out port, state %d, can cancel %d\n", outPortPair->iSrcPort.GetStatus(), iNodeList[i].iOutputPort.iCanCancelPort));
                    if (outPortPair->iSrcPort.GetStatus() == EHasPort)
                    {
                        OSCL_TRY(error, outPortPair->iSrcPort.SetCmdId(SendNodeCmdL(PV2WAY_NODE_CMD_RELEASEPORT, iNodeList[i], outPortPair->iSrcPort.GetPort())));
                        OSCL_FIRST_CATCH_ANY(error,
                                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckClosed unable to release output port\n"));
                                             return;);

                        outPortPair->iSrcPort.SetStatus(EReleasePort);
                        return;
                    }
                    else if ((outPortPair->iSrcPort.GetStatus() == EHasPort) &&
                             iNodeList[i].iOutputPort.iCanCancelPort)
                    {
                        id = outPortPair->iSrcPort.GetCmdId();
                        OSCL_TRY(error, outPortPair->iSrcPort.SetCmdId(SendNodeCmdL(PV2WAY_NODE_CMD_CANCELCMD, iNodeList[i], &id)));
                        OSCL_FIRST_CATCH_ANY(error,
                                             PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckClosed unable to cancel request output port\n"));
                                             return;);

                        outPortPair->iSrcPort.SetStatus(EReleasePort);
                        return;
                    }
                }

                if (inPortPair)
                {
                    switch (inPortPair->iDestPort.GetStatus())
                    {
                        case EHasPort:
                        case ENoPort:
                            break;

                        default:
                            return;
                    }
                }

                if (outPortPair)
                {
                    switch (outPortPair->iSrcPort.GetStatus())
                    {
                        case EHasPort:
                        case ENoPort:
                            break;

                        default:
                            return;
                    }
                }

                if (!SendNodeCmd(PV2WAY_NODE_CMD_STOP, i))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CheckClosed unable to stop node\n"));
                    return;
                }

                return;
            }

            default:
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckClosed transitional node state!\n"));
                return;
        }
    }

    for (i = 0; i < (int32) iNodeList.size(); i ++)
    {
        //If node is not in its original state when datapath was opened then not closed yet.
        if (!IsDatapathNodeClosed(iNodeList[i])) return;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckClosed close complete\n"));

    //If reached this point then all ports have been deallocated and datapath is deemed closed, notify upper layer.
    SetState(EClosed);
    CloseComplete();
    NotifyParentPaths();

    return;
}

void CPV2WayDatapath::CheckPath()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CheckPath path type %d, format %s, state %d\n", iType, iFormat.getMIMEStrPtr(), iState));

    switch (iState)
    {
        case EOpening:
            CheckOpen();
            break;
        case EPausing:
            CheckPause();
            break;
        case EUnpausing:
            CheckResume();
            break;
        case EClosing:
            CheckClosed();
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_WARNING, (0, "CPV2WayDatapath::CheckPath warning: static state\n"));
            break;
    }
}

void CPV2WayDatapath::CommandHandler(PV2WayNodeCmdType aType, const PVMFCmdResp& aResponse)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::CommandHandler path type %d, state %d, cmd type %d, status %d\n", iType, iState, aType, aResponse.GetCmdStatus()));
    CPV2WayNodeContextData *data = (CPV2WayNodeContextData *) aResponse.GetContext();
    uint16 i;

    for (i = 0; i < iNodeList.size(); i++)
    {
        if (iNodeList[i].iNode.iNode == data->iNode)
        {
            iNodeList[i].iCommandIssued = false;
            break;
        }
    }

    switch (aType)
    {
        case PV2WAY_NODE_CMD_REQUESTPORT:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                PortStatusChange(data->iNode, aResponse.GetCmdId(), NULL);
                DatapathError();
            }
            else
            {
                if (PortStatusChange(data->iNode, aResponse.GetCmdId(), (PVMFPortInterface *) aResponse.GetEventData()) != PVMFSuccess)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::CommandHandler PortStatusChange failed!\n"));
                    DatapathError();
                }
                else
                {

                    bool isConnected;
                    //Check if all ports have been allocated.
                    for (i = 0, isConnected = true; i < iPortPairList.size(); i++)
                    {
                        if (!iPortPairList[i].iIsConnected)
                        {
                            //If all ports have not been connected yet
                            isConnected = false;
                            break;
                        }
                        // right here- check for negotiated between the two!
                    }

                    iAllPortsConnected = isConnected;
                }
            }
            break;

        case PV2WAY_NODE_CMD_RELEASEPORT:
        case PV2WAY_NODE_CMD_CANCELCMD:
            PortStatusChange(data->iNode, aResponse.GetCmdId(), NULL);
            break;
        case PV2WAY_NODE_CMD_START:
            if (aResponse.GetCmdStatus() == PVMFSuccess)
            {
                if (i2Way->IsSinkNode(data->iNode))
                {
                    TPV2WayNode* sink_node = i2Way->GetTPV2WayNode(i2Way->iSinkNodes, data->iNode);
                    OSCL_ASSERT(sink_node);

                    i2Way->SendNodeCmdL(PV2WAY_NODE_CMD_SKIP_MEDIA_DATA, sink_node, i2Way);
                }
            }
            else
            {
                DatapathError();
            }
            break;
        default:
            if (aResponse.GetCmdStatus() != PVMFSuccess)
            {
                DatapathError();
            }
            break;
    }

    CheckPath();
    return;
}

void CPV2WayDatapath::DependentPathClosed(CPV2WayDatapath *aDependentPath)
{
    OSCL_UNUSED_ARG(aDependentPath);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::DependentPathClosed path state %d\n", iState));
    if (iState == EClosing)
    {
        //Start closing this path.
        CheckPath();
    }
    return;
}

bool CPV2WayDatapath::HaveAllDependentPathsClosed()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::HaveAllDependentPathsClosed path state %d\n", iState));
    for (uint32 i = 0; i < iDependentPathList.size(); i ++)
    {
        if (iDependentPathList[i]->GetState() != EClosed)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::DependentPathClosed not all dependent paths closed, %d\n", i));
            return false;
        }
    }

    return true;
}

bool CPV2WayDatapath::IsParentClosing()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::IsParentClosing path state %d\n", iState));
    for (uint32 i = 0; i < iParentPathList.size(); i++)
    {
        if (iParentPathList[i]->GetState() == EClosing)
        {
            //Parent datapath is closing
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::IsParentClosing parent path closing, %d\n", i));
            return true;
        }
    }

    //No parent datapath is closing
    return false;
}

void CPV2WayDatapath::NotifyParentPaths()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::NotifyParentPaths path state %d\n", iState));
    for (uint32 i = 0; i < iParentPathList.size(); i++)
    {
        iParentPathList[i]->DependentPathClosed(this);
    }
    return;
}

bool CPV2WayDatapath::ParentIsClosing()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "CPV2WayDatapath::ParentIsClosing path state %d\n", iState));
    return Close();
}

PVMFStatus CPV2WayDatapath::GetKvp(PVMFPortInterface &aPort,
                                   bool aInput,
                                   PvmiKvp*& aKvp,
                                   int32& aNumKvpElem,
                                   OsclAny*& aconfigPtr)
{
    PVMFStatus status = PVMFFailure;
    PvmiCapabilityContext context = NULL;
    PvmiKeyType keyType;

    aPort.QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, aconfigPtr);
    if (aconfigPtr)
    {
        if (aInput)
        {
            keyType = OSCL_CONST_CAST(char*, INPUT_FORMATS_CAP_QUERY);
        }
        else
        {
            keyType = OSCL_CONST_CAST(char*, OUTPUT_FORMATS_CAP_QUERY);
        }

        status = ((PvmiCapabilityAndConfig *)aconfigPtr)->getParametersSync(NULL,
                 keyType, aKvp, aNumKvpElem, context);

        if (status != PVMFSuccess && aInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV2WayDatapath::GetKvp 1st getParametersSync failed %d.  Trying another\n", status));

            keyType = OSCL_CONST_CAST(char*, "x-pvmf/video/decode/input_formats");
            status = ((PvmiCapabilityAndConfig *)aconfigPtr)->getParametersSync(NULL,
                     keyType, aKvp, aNumKvpElem, context);
        }

        if (status != PVMFSuccess && !aInput)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "CPV2WayDatapath::GetKvp 2nd getParametersSync failed %d.  Trying another\n", status));

            keyType = OSCL_CONST_CAST(char*, "x-pvmf/audio/decode/output_formats");
            status = ((PvmiCapabilityAndConfig *)aconfigPtr)->getParametersSync(NULL,
                     keyType, aKvp, aNumKvpElem, context);
        }
    }
    return status;
}

PVMFFormatType CPV2WayDatapath::GetPortFormatType(PVMFPortInterface &aPort,
        bool aInput,
        PVMFPortInterface *aOtherPort)
{
    PVMFStatus status = PVMFFailure;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                    (0, "CPV2WayDatapath::GetPortFormatType, is input %d\n", aInput));
    PvmiKvp *kvp = NULL;
    int32 numkvpElements = 0;
    PvmiKvp *kvpOther = NULL;
    int32 numkvpOtherElements = 0;
    OsclAny *configPtr = NULL;
    OsclAny *configOtherPtr = NULL;
    PVMFFormatType format = PVMF_MIME_FORMAT_UNKNOWN;
    PVMFFormatType format_datapath_media_type = PVMF_MIME_FORMAT_UNKNOWN;


    //If config ptr exists, otherwise assume port has been configured.
    status = GetKvp(aPort, aInput, kvp, numkvpElements, configPtr);
    if (status == PVMFSuccess && (aOtherPort != NULL))
    {
        status = GetKvp(*aOtherPort, !aInput, kvpOther, numkvpOtherElements, configOtherPtr);
    }

    if (status == PVMFSuccess)
    {
        for (int32 ii = 0; ii < numkvpElements; ++ii)
        {
            if ((pv_mime_strcmp(kvp[ii].key, "x-pvmf/port/formattype;valtype=char*") == 0) ||
                    (aInput && (pv_mime_strcmp(kvp[ii].key, INPUT_FORMATS_VALTYPE) == 0)) ||
                    (pv_mime_strcmp(kvp[ii].key, OUTPUT_FORMATS_VALTYPE) == 0))
            {
                format = kvp[ii].value.pChar_value;
                if ((format.isAudio() && iFormat.isAudio()) ||
                        (format.isVideo() && iFormat.isVideo()) ||
                        (format.isFile() && iFormat.isFile()))
                {
                    format_datapath_media_type = format;
                }
                // loop through other port, look for a match
                // if there is a match return it
                for (int jj = 0; jj < numkvpOtherElements; ++jj)
                {
                    if ((format == kvpOther[jj].value.pChar_value) &&
                            ((pv_mime_strcmp(kvpOther[jj].key, "x-pvmf/port/formattype;valtype=char*") == 0) ||
                             (pv_mime_strcmp(kvpOther[ii].key, INPUT_FORMATS_VALTYPE) == 0) ||
                             (pv_mime_strcmp(kvpOther[ii].key, OUTPUT_FORMATS_VALTYPE) == 0)))
                    {
                        ((PvmiCapabilityAndConfig *)configPtr)->releaseParameters(NULL,
                                kvp, numkvpElements);
                        if (configOtherPtr != NULL)
                        {
                            ((PvmiCapabilityAndConfig *)configOtherPtr)->releaseParameters(NULL,
                                    kvpOther, numkvpOtherElements);
                        }
                        return format;
                    }
                }
            }
        }
        ((PvmiCapabilityAndConfig *)configPtr)->releaseParameters(NULL, kvp, numkvpElements);
        if (configOtherPtr != NULL)
        {
            ((PvmiCapabilityAndConfig *)configOtherPtr)->releaseParameters(NULL, kvpOther, numkvpOtherElements);
        }
        return format_datapath_media_type;
    }
    else
    {
        if (configPtr != NULL)
        {
            ((PvmiCapabilityAndConfig *)configPtr)->releaseParameters(NULL, kvp, numkvpElements);
        }
        if (configOtherPtr != NULL)
        {
            ((PvmiCapabilityAndConfig *)configOtherPtr)->releaseParameters(NULL, kvpOther, numkvpOtherElements);
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "CPV2WayDatapath::GetPortFormatType 3rd getParametersSync failed %d, using configured format\n", status));
    }

    return format_datapath_media_type;
}

void CPV2WayDatapath::SetFormatSpecificInfo(uint8* fsi, uint16 fsi_len)
{
    if (iFsi)
    {
        OSCL_DEFAULT_FREE(iFsi);
        iFsi = NULL;
        iFsiLen = 0;
    }
    iFsi = (uint8*)OSCL_DEFAULT_MALLOC(fsi_len);
    iFsiLen = fsi_len;
    oscl_memcpy(iFsi, fsi, iFsiLen);
}


uint8* CPV2WayDatapath::GetFormatSpecificInfo(uint32* len)
{
    *len = iFsiLen;
    return iFsi;
}

void CPV2WayDatapath::SetSourceSinkFormat(PVMFFormatType aFormatType)
{
    iSourceSinkFormat = aFormatType;
}

PVMFFormatType CPV2WayDatapath::GetSourceSinkFormat() const
{
    return iSourceSinkFormat;
}




