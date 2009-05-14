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
 *
 * @file pvmf_videoparser_node.cpp
 * @brief Video parser node. Parses incoming video bitstream into frames.
 *
 */
#include "pvmf_videoparser_port.h"
#include "pvmf_videoparser_node.h"
#include "pvlogger.h"
#include "pvmf_media_cmd.h"
#include "pvmf_media_msg_format_ids.h"

#define VOP_START_BYTE_1 0x00
#define VOP_START_BYTE_2 0x00
#define VOP_START_BYTE_3 0x01
#define VOP_START_BYTE_4 0xB6
#define GOV_START_BYTE_4 0xB3

#define H263_START_BYTE_1 0x00
#define H263_START_BYTE_2 0x00
#define H263_START_BYTE_3 0x80
#define H263_START_BYTE_3_MASK 0xFC

#define VOL_HEADER_START_BYTE_1 0x00
#define VOL_HEADER_START_BYTE_2 0x00
#define VOL_HEADER_START_BYTE_3 0x01
#define VOL_HEADER_START_BYTE_4 0xB0

static const uint8 M4vScLlookup[] =
{
    0x18  /*00000=11000*/, 0x19/*00001=11001*/, 0x18/*00010=11000*/, 0x19/*00011=11001*/,
    0x0C  /*00100=01100*/, 0x0C/*00101=01100*/, 0x0C/*00110=01100*/, 0x0C/*00111=01100*/,
    0x0C  /*01000=01100*/, 0x19/*01001=11001*/, 0x18/*01010=11000*/, 0x19/*01011=11001*/,
    0x0C  /*01100=01100*/ , 0x0C/*01101=01100*/, 0x0C/*01110=01100*/, 0x0C/*01111=01100*/,
    0x18  /*10000=11000*/, 0x19/*10001=11001*/, 0x18/*10010=11000*/, 0x19/*10011=11001*/,
    0x18  /*10100=11000*/ , 0x19/*10101=11001*/, 0x18/*10110=11000*/, 0x19/*10111=11001*/,
    0x18  /*11000=11000*/, 0x19/*11001=11001*/, 0x18/*11010=11000*/ , 0x19/*11011=11001*/,
    0x18  /*11100=11000*/ , 0x19/*11101=11001*/, 0x18/*11110=11000*/, 0x19/*11111=11001*/
};

#include "oscl_dll.h"

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFNodeInterface* PVMFVideoParserNode::Create(uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen)
{
    PVMFVideoParserNode* aRet = OSCL_NEW(PVMFVideoParserNode, ());
    if (aRet)
    {
        aRet->ConstructL(aFormatSpecificInfo, aFormatSpecificInfoLen);
        aRet->AddToScheduler();
    }

    return aRet;
}


////////////////////////////////////////////////////////////////////////////
PVMFVideoParserNode::PVMFVideoParserNode() : OsclActiveObject(OsclActiveObject::EPriorityNominal, "VideoParserNode"),
        iCmdIdCounter(0),
        iInputPort(NULL),
        iOutputPort(NULL),
        iFormatType(PVMF_MIME_FORMAT_UNKNOWN),
        iFormatTypeInteger(PV_CODEC_TYPE_NONE),
        iFirstFrameFound(false),
        iVidParserMemoryPool(NULL),
        iMediaDataAlloc(NULL),
        iMediaDataMemPool(MAX_VIDEO_FRAMES, PVVIDEOPARSER_MEDIADATA_SIZE),
        iLogger(NULL),
        iFormatSpecificInfo(NULL),
        iFormatSpecificInfoLen(0)
{
    SetState(EPVMFNodeCreated);
    // Create a 256 element table with number of 1 bits in each index value
    // Ex: 0->0, 1->1, 2->1, 3->2, 4->1, 5->2, 6->2
    for (int n  = 0; n <= 0xFF; n++)
    {
        int m = n;
        int cnt = 0;
        while (m)
        {
            if (m & 0x1)
            {
                cnt++;
            }
            m >>= 1;
        }
        iNumOnes[n] = (uint8)cnt;
    }

}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFVideoParserNode::~PVMFVideoParserNode()
{
    if (iFormatSpecificInfo)
    {
        OSCL_FREE(iFormatSpecificInfo);
        iFormatSpecificInfo = NULL;
    }

    if (iMediaDataAlloc)
    {
        OSCL_DELETE(iMediaDataAlloc);
        iMediaDataAlloc = NULL;
    }

    if (iVidParserMemoryPool)
    {
        OSCL_DELETE(iVidParserMemoryPool);
        iVidParserMemoryPool = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserNode::ConstructL(uint8* aFormatSpecificInfo, uint32 aFormatSpecificInfoLen)
{
    iVidParserMemoryPool = OSCL_NEW(OsclMemPoolFixedChunkAllocator, (MAX_VIDEO_FRAMES));
    if (iVidParserMemoryPool == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }
    iVidParserMemoryPool->enablenullpointerreturn();

    iMediaDataAlloc = OSCL_NEW(PVMFSimpleMediaBufferCombinedAlloc, (iVidParserMemoryPool));
    if (iMediaDataAlloc == NULL)
    {
        OSCL_LEAVE(PVMFErrNoMemory);
    }

    if (aFormatSpecificInfo && aFormatSpecificInfoLen)
    {
        iFormatSpecificInfo = (uint8*)OSCL_MALLOC(aFormatSpecificInfoLen);
        if (iFormatSpecificInfo == NULL)
        {
            OSCL_LEAVE(PVMFErrNoMemory);
        }
        oscl_memcpy(iFormatSpecificInfo, aFormatSpecificInfo, aFormatSpecificInfoLen);
        iFormatSpecificInfoLen = aFormatSpecificInfoLen;
    }
}


////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserNode::DoCancel()
{
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFVideoParserNode::GetCapability(PVMFNodeCapability& aNodeCapability)
{
    aNodeCapability.iCanSupportMultipleInputPorts = false;
    aNodeCapability.iCanSupportMultipleOutputPorts = false;
    aNodeCapability.iHasMaxNumberOfPorts = true;
    aNodeCapability.iMaxNumberOfPorts = 2;
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_M4V);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H2631998);
    aNodeCapability.iInputFormatCapability.push_back(PVMF_MIME_H2632000);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_M4V);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_H2631998);
    aNodeCapability.iOutputFormatCapability.push_back(PVMF_MIME_H2632000);

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFPortIter* PVMFVideoParserNode::GetPorts(const PVMFPortFilter* aFilter)
{
    OSCL_UNUSED_ARG(aFilter);
    return NULL;
}
PVMFStatus PVMFVideoParserNode::ThreadLogon()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFVideoParserNode:ThreadLogon"));

    switch (iInterfaceState)
    {
        case EPVMFNodeCreated:
            if (!IsAdded())
            {
                AddToScheduler();
            }
            iLogger = PVLogger::GetLoggerObject("PVMFVideoParserNode");
            SetState(EPVMFNodeIdle);
            return PVMFSuccess;
        default:
            return PVMFErrInvalidState;
    }
}

PVMFStatus PVMFVideoParserNode::ThreadLogoff()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFVideoParserNode:ThreadLogoff"));

    switch (iInterfaceState)
    {
        case EPVMFNodeIdle:
            if (IsAdded())
            {
                RemoveFromScheduler();
            }
            iLogger = NULL;
            SetState(EPVMFNodeCreated);
            return PVMFSuccess;

        default:
            return PVMFErrInvalidState;
    }
}


////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::QueryUUID(PVMFSessionId s, const PvmfMimeString& aMimeType,
        Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids,
        bool aExactUuidsOnly,
        const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aMimeType);
    OSCL_UNUSED_ARG(aUuids);
    OSCL_UNUSED_ARG(aExactUuidsOnly);
    OSCL_UNUSED_ARG(aContext);
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_QUERY_UUID, s, NULL, aContext, NULL);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::QueryInterface(PVMFSessionId s, const PVUuid& aUuid,
        PVInterface*& aInterfacePtr,
        const OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aUuid);
    OSCL_UNUSED_ARG(aInterfacePtr);
    OSCL_UNUSED_ARG(aContext);
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_QUERY_INTERFACE, s, NULL, aContext, NULL);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::RequestPort(PVMFSessionId s, int32 aPortTag, const PvmfMimeString* aPortConfig, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::RequestPort called, format %s, port tag %d\n", aPortConfig ? aPortConfig->get_cstr() : "\"\"", aPortTag));
    // Copy property
    VideoParserPortProperty* property = OSCL_NEW(VideoParserPortProperty, ());
    if (property != NULL)
    {
        property->porttag = aPortTag;
        property->format = aPortConfig->get_str();
        //property->mimetype = *aPortConfig;
    }

    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_REQUESTPORT, s, NULL, aContext, property);

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::ReleasePort(PVMFSessionId s, PVMFPortInterface& aPort, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::ReleasePort\n"));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_RELEASEPORT, s, &aPort, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Init(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Init, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_INIT, s, NULL, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Start(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Start, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_START, s, NULL, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Prepare(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Prepare, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_PREPARE, s, NULL, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Flush(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Flush, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_FLUSH, s, NULL, aContext);
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Stop(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Stop, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_STOP, s, NULL, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Pause(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Pause, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_PAUSE, s, NULL, aContext);

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::Reset(PVMFSessionId s, const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::Reset, state %d\n", iInterfaceState));
    return AddCmdToQueue(PVMFVIDEOPARSER_NODE_CMD_RESET, s, NULL, aContext);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::CancelAllCommands(PVMFSessionId s, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::CancelAllCommands, state %d\n", iInterfaceState));
    OSCL_UNUSED_ARG(s);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMFVideoParserNode::CancelCommand(PVMFSessionId s, PVMFCommandId aCmdId, const OsclAny* aContextData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::CancelCommand, state %d\n", iInterfaceState));
    OSCL_UNUSED_ARG(s);
    OSCL_UNUSED_ARG(aCmdId);
    OSCL_UNUSED_ARG(aContextData);
    OSCL_LEAVE(PVMFErrNotSupported);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
PVMFCommandId PVMFVideoParserNode::AddCmdToQueue(PVMFVideoParserNodeCmdType aType, PVMFSessionId s, PVMFPortInterface* aPort, const OsclAny* aContext, OsclAny* aData)
{
    PVMFVideoParserNodeCmd cmd;
    cmd.iSession = s;
    cmd.iType = aType;
    cmd.iPort = aPort;
    cmd.iContext = (OsclAny *) aContext;
    cmd.iData = aData;
    cmd.iId = iCmdIdCounter;
    ++iCmdIdCounter;

    iCmdQueue.push(cmd);

    RunIfNotReady();
    return cmd.iId;
}

////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserNode::Run()
{
    PVMFPortInterface *port = NULL;
    OsclAny *eventData = NULL;

    if (!iCmdQueue.empty())
    {
        PVMFStatus status = PVMFSuccess;
        PVMFVideoParserNodeCmd cmd(iCmdQueue.top());
        iCmdQueue.pop();
        switch (cmd.iType)
        {
            case PVMFVIDEOPARSER_NODE_CMD_INIT:
            {
                if (iInterfaceState == EPVMFNodeIdle)
                {
                    OsclSharedPtr<PVMFMediaDataImpl> tempImpl;

                    tempImpl = iMediaDataAlloc->allocate(MAX_VIDEO_FRAME_PARSE_SIZE);

                    if (!tempImpl || !tempImpl->getCapacity())
                    {
                        SetState(EPVMFNodeIdle);
                        status = PVMFErrNoMemory;
                    }
                    else
                    {
                        tempImpl.Unbind();
                        SetState(EPVMFNodeInitialized);
                    }
                    tempImpl.Unbind();
                }
                else
                {
                    status = PVMFErrInvalidState;
                }
            }
            break;
            case PVMFVIDEOPARSER_NODE_CMD_PREPARE:

                if (iInterfaceState != EPVMFNodeInitialized)
                {
                    status = PVMFErrInvalidState;
                }
                else
                {
                    SetState(EPVMFNodePrepared);
                }
                break;

            case PVMFVIDEOPARSER_NODE_CMD_START:
                if (iInterfaceState == EPVMFNodePaused || iInterfaceState == EPVMFNodePrepared)
                {
                    status = HandleCmdStart(&cmd);
                }
                else
                {
                    status = PVMFErrInvalidState;
                }
                break;

            case PVMFVIDEOPARSER_NODE_CMD_STOP:
                if (iInterfaceState == EPVMFNodeStarted)
                {
                    SetState(EPVMFNodePrepared);
                }
                else
                {
                    status = PVMFErrInvalidState;
                }
                break;

            case PVMFVIDEOPARSER_NODE_CMD_RESET:
                switch (iInterfaceState)
                {
                    case EPVMFNodeCreated:
                    case EPVMFNodeIdle:
                    case EPVMFNodeInitialized:
                    case EPVMFNodePrepared:
                    {
                        if (iInputPort)
                        {
                            OSCL_DELETE(iInputPort);
                            iInputPort = NULL;
                        }

                        if (iOutputPort)
                        {
                            OSCL_DELETE(iOutputPort);
                            iOutputPort = NULL;
                        }

                        iVideoFrame.Unbind();

                        while (iPortActivityQueue.size())
                        {

                            PVMFPortActivity activity(iPortActivityQueue.front());
                            iPortActivityQueue.erase(&iPortActivityQueue.front());
                        }
                        while (iCmdQueue.size())
                        {

                            PVMFVideoParserNodeCmd cmd(iCmdQueue.top());
                            iCmdQueue.pop();
                            PVMFCmdResp resp(cmd.iId, cmd.iContext, PVMFFailure, eventData);
                            ReportCmdCompleteEvent(cmd.iSession, resp);
                        }


                        SetState(EPVMFNodeIdle);
                        status = PVMFSuccess;
                    }
                    break;
                    default:
                        status = PVMFErrInvalidState;
                        break;
                }
                break;

            case PVMFVIDEOPARSER_NODE_CMD_PAUSE:
                if (iInterfaceState == EPVMFNodeStarted)
                {
                    SetState(EPVMFNodePaused);
                }
                else
                {
                    status = PVMFErrInvalidState;
                }
                break;

            case PVMFVIDEOPARSER_NODE_CMD_REQUESTPORT:
                switch (iInterfaceState)
                {

                    case EPVMFNodeInitialized:
                    case EPVMFNodePrepared:
                    case EPVMFNodeStarted:
                    {
                        port = HandleCmdRequestPort(&cmd, status);
                        eventData = (OsclAny*)port;
                        if (cmd.iData != NULL)
                        {
                            OSCL_DELETE(((VideoParserPortProperty*)cmd.iData)); // Allocated dynamically in RequestPort()
                        }
                    }
                    break;
                    default:
                        status = PVMFErrInvalidState;
                        break;
                }
                break;

            case PVMFVIDEOPARSER_NODE_CMD_RELEASEPORT:
            {
                status = HandleCmdReleasePort(&cmd);
            }
            break;
            case PVMFVIDEOPARSER_NODE_CMD_FLUSH:
                break;
            case PVMFVIDEOPARSER_NODE_CMD_QUERY_INTERFACE:
            case PVMFVIDEOPARSER_NODE_CMD_QUERY_UUID:
            default:
            {
                status = PVMFErrNotSupported;
            }
            break;
        }
        PVMFCmdResp resp(cmd.iId, cmd.iContext, status, eventData);
        ReportCmdCompleteEvent(cmd.iSession, resp);
    }

    if (!iPortActivityQueue.empty() && (iInterfaceState == EPVMFNodeStarted))
    {
        // If the port activity cannot be processed because a port is
        // busy, discard the activity and continue to process the next
        // activity in queue until getting to one that can be processed.
        while (!iPortActivityQueue.empty())
        {
            if (ProcessPortActivity())
            {
                break; //processed a port
            }
        }
    }

    if (!iPortActivityQueue.empty() || !iCmdQueue.empty())
    {
        // Run again if there are more commands to process
        RunIfNotReady();
    }

}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFVideoParserNode::HandleCmdStart(PVMFVideoParserNodeCmd* aCmd)
{
    OSCL_UNUSED_ARG(aCmd);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::HandleCmdStart, input port %x, output port %x\n", iInputPort, iOutputPort));

    SetState(EPVMFNodeStarted);
    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////
PVMFPortInterface *PVMFVideoParserNode::HandleCmdRequestPort(PVMFVideoParserNodeCmd* aCmd, PVMFStatus &aStatus)
{
    VideoParserPortProperty *property = (VideoParserPortProperty*)aCmd->iData;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::HandleCmdRequestPort\n"));

    aStatus  = PVMFSuccess;

    if (!property)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, no port property\n"));
        aStatus = PVMFErrNoMemory;
        return NULL;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::HandleCmdRequestPort, port type %d, format %s\n", property->porttag, (property->format).getMIMEStrPtr()));

    if (!((property->format == PVMF_MIME_M4V) || (property->format == PVMF_MIME_H2631998) || (property->format == PVMF_MIME_H2632000)))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, unknown format\n"));
        aStatus = PVMFErrArgument;
        return NULL;
    }

    switch (property->porttag)
    {
        case PVMF_VIDEOPARSER_NODE_PORT_TYPE_SINK:
            if (iOutputPort && iFormatType != property->format)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, output format (%s) doesn't match input format (%s)\n", iFormatType.getMIMEStrPtr(), (property->format).getMIMEStrPtr()));
                //Output format doesn't match output format.
                aStatus = PVMFFailure;
                return NULL;
            }

            if (iInputPort)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, input port already exists\n"));
                // it's been taken for now, so reject this request
                aStatus = PVMFFailure;
                return NULL;
            }

            // Create and configure output port
            iInputPort = OSCL_NEW(PVMFVideoParserPort, (property->porttag,
                                  property->format,
                                  this,
                                  iFormatSpecificInfo, iFormatSpecificInfoLen,
                                  OSCL_STATIC_CAST(char*, "PVMFVideoParserInputPort")));
            if (!iInputPort)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, unable to allocate input port\n"));
                aStatus = PVMFErrNoMemory;
                return NULL;
            }

            iFormatType = property->format;
            if ((iFormatType == PVMF_MIME_H2631998) || (iFormatType == PVMF_MIME_H2632000))
            {
                iFormatTypeInteger = PV_VID_TYPE_H263;
            }
            else if (iFormatType == PVMF_MIME_M4V)
            {
                iFormatTypeInteger = PV_VID_TYPE_MPEG4;
            }
            return iInputPort;

        case PVMF_VIDEOPARSER_NODE_PORT_TYPE_SOURCE:

            if (iInputPort)
            {
                if (iFormatType != property->format)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, input format (%s) doesn't match output format (%s)\n", iFormatType.getMIMEStrPtr(), (property->format).getMIMEStrPtr()));
                    //Input format doesn't match output format.
                    aStatus = PVMFFailure;
                    return NULL;
                }
            }


            if (iOutputPort)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, output port already exists\n"));
                // it's been taken for now, so reject this request
                aStatus = PVMFFailure;
                return NULL;
            }

            iOutputPort = OSCL_NEW(PVMFVideoParserPort, (property->porttag,
                                   property->format,
                                   this,
                                   iFormatSpecificInfo, iFormatSpecificInfoLen,
                                   OSCL_STATIC_CAST(char*, "PVMFVideoParserOutputPort")));
            if (!iOutputPort)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, unable to allocate output port\n"));
                aStatus = PVMFErrNoMemory;
                return NULL;
            }

            iFormatType = property->format;
            if ((iFormatType == PVMF_MIME_H2631998) || (iFormatType == PVMF_MIME_H2632000))
            {
                iFormatTypeInteger = PV_VID_TYPE_H263;
            }
            else if (iFormatType == PVMF_MIME_M4V)
            {
                iFormatTypeInteger = PV_VID_TYPE_MPEG4;
            }

            return iOutputPort;

        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdStart, unknown port type\n"));
            aStatus = PVMFErrNotSupported;
            return NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFVideoParserNode::HandleCmdReleasePort(PVMFVideoParserNodeCmd* aCmd)
{
    if (aCmd->iPort == iInputPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFVideoParserNode::HandleCmdReleasePort, release input port\n"));
        iInputPort->Disconnect();
        OSCL_DELETE(iInputPort);
        iInputPort = NULL;
        return PVMFSuccess;
    }
    else if (aCmd->iPort == iOutputPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFVideoParserNode::HandleCmdReleasePort, release output port\n"));
        iOutputPort->Disconnect();
        OSCL_DELETE(iOutputPort);
        iOutputPort = NULL;
        return PVMFSuccess;
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::HandleCmdReleasePort, unknown port to release\n"));
        return PVMFFailure;
    }
}

////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserNode::DataReceived(OsclSharedPtr<PVMFMediaMsg>& aMsg)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::DataReceived, state %d\n", iInterfaceState));
    if (iInterfaceState == EPVMFNodeStarted)
    {
        PVMFSharedMediaDataPtr mediaData;
        OsclRefCounterMemFrag memFrag;
        OsclRefCounterMemFrag curVideoFrag;

        convertToPVMFMediaData(mediaData, aMsg);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::DataReceived, Num fragments=%d, filled size=%d\n", mediaData->getNumFragments(), mediaData->getFilledSize()));


        for (uint32 i = 0; i < mediaData->getNumFragments(); i++)
        {
            mediaData->getMediaFragment(i, memFrag);

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFVideoParserNode::DataReceived, frag size %d, idx %d\n", memFrag.getMemFragSize(), i));

            //Make sure it can fit in the video buffer.
            if (memFrag.getMemFragSize() > MAX_VIDEO_FRAME_PARSE_SIZE)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::DataReceived, frag cannot fit into video buffer\n"));
                continue;
            }

            if (FrameMarkerExists((uint8 *)memFrag.getMemFragPtr(), memFrag.getMemFragSize(), mediaData->getErrorsFlag()))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFVideoParserNode::DataReceived, frame found\n"));
                //If buffer exists then send it.
                if (iVideoFrame.GetRep())
                {
                    SendFrame();
                }
            }

            //If data cannot fit in remaining buffer space, send current buffer regardless
            if (iVideoFrame.GetRep() && (iVideoFrame->getFilledSize() + memFrag.getMemFragSize()) > iVideoFrame->getCapacity())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFVideoParserNode::DataReceived, frag data cannot fit into remaining buffer space\n"));
                SendFrame();
            }

            if (!iVideoFrame.GetRep())
            {
                OsclRefCounterMemFrag formatSpecificInfo;
                OsclSharedPtr<PVMFMediaDataImpl> tempImpl;

                // Drop rest of data if unable to allocate next buffer
                tempImpl = iMediaDataAlloc->allocate(MAX_VIDEO_FRAME_PARSE_SIZE)
                           ;

                if (!tempImpl)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFVideoParserNode::DataReceived, unable to allocate media impl\n"));
                    return;
                }
                iVideoFrame = PVMFMediaData::createMediaData(tempImpl, &iMediaDataMemPool);

                if (!iVideoFrame)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "PVMFVideoParserNode::DataReceived, unable to allocate media data\n"));
                    tempImpl.Unbind();
                    return;
                }
                iVideoFrame->setTimestamp(mediaData->getTimestamp());
                iVideoFrame->setSeqNum(1);
                mediaData->getFormatSpecificInfo(formatSpecificInfo);
                iVideoFrame->setFormatSpecificInfo(formatSpecificInfo);
            }
            if (memFrag.getMemFragSize() > 0)
            {
                iVideoFrame->getMediaFragment(0, curVideoFrag);
                oscl_memcpy((uint8 *)curVideoFrag.getMemFragPtr() + iVideoFrame->getFilledSize(), memFrag.getMemFragPtr(), memFrag.getMemFragSize());
                iVideoFrame->setMediaFragFilledLen(0, iVideoFrame->getFilledSize() + memFrag.getMemFragSize());
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////
void PVMFVideoParserNode::SendFrame()
{
    OsclSharedPtr<PVMFMediaMsg> mediaMsg;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImpl;
    if (iVideoFrame->getMediaDataImpl(mediaDataImpl))
    {
        mediaDataImpl->setMarkerInfo(1);
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFVideoParserNode::SendFrame() cannot get media data impl\n"));
    }

    convertToPVMFMediaMsg(mediaMsg, iVideoFrame);


    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFVideoParserNode::SendFrame, size %d\n", iVideoFrame->getFilledSize()));

    if (iOutputPort) iOutputPort->QueueOutgoingMsg(mediaMsg);
    iVideoFrame.Unbind();
}

////////////////////////////////////////////////////////////////////////////
bool PVMFVideoParserNode::FrameMarkerExists(uint8* aDataPtr,
        int32 aDataSize,
        uint32 aCrcError)
{
    uint16 match_cnt = 0;
    if (iFormatTypeInteger == PV_VID_TYPE_H263)
    {
        //If start of new frame.
        if ((aDataSize > 2) &&
                (aDataPtr[0] == H263_START_BYTE_1) &&
                (aDataPtr[1] == H263_START_BYTE_2) &&
                ((aDataPtr[2] & H263_START_BYTE_3_MASK) == H263_START_BYTE_3))
        {
            return true;
        }
    }

    else if (iFormatTypeInteger == PV_VID_TYPE_MPEG4)
    {
        if (!aCrcError)
        {
            if (aDataSize >= 3)
            {
                if ((aDataPtr[0] == VOP_START_BYTE_1) &&
                        (aDataPtr[1] == VOP_START_BYTE_2) &&
                        (aDataPtr[2] == VOP_START_BYTE_3))
                {
                    return true;
                }
            }
        }
        else
        {
            if (aDataSize > 4)
            {
                int16 nBitsCorrected = 0;
                //If start of new frame.
                match_cnt = (uint16)(iNumOnes[(aDataPtr[0] ^ VOP_START_BYTE_1)] +
                                     iNumOnes[(aDataPtr[1] ^ VOP_START_BYTE_2)]);
                if (match_cnt > 2)
                {
                    return false;
                }

                nBitsCorrected = match_cnt;

                if (aDataPtr[2] & 0x80)
                {
                    return false;
                }

                // Match the next 12 bits
                match_cnt = (uint16)(iNumOnes[(aDataPtr[2] ^ VOP_START_BYTE_3)] +
                                     iNumOnes[((aDataPtr[3] & 0xF8) ^ 0xB0)]);
                if (match_cnt > 1)
                {
                    return false;
                }
                nBitsCorrected = (int16)(nBitsCorrected + match_cnt);

                uint8 c3 = 0, c4 = 0;
                uint8 c = (uint8)(aDataPtr[3] & 0x07);
                c3 = VOP_START_BYTE_4 &  0xF0;
                c = (uint8)(c << 2 | aDataPtr[4] >> 6);
                c = M4vScLlookup[c];
                c3 |= (c >> 2);
                c4 = (uint8)(aDataPtr[4] & 0x3F);
                c4 |= (c << 6);

                nBitsCorrected = (int16)(nBitsCorrected + iNumOnes[((c3^aDataPtr[3]) + (c4 ^ aDataPtr[4]))]);

                aDataPtr[0] = VOP_START_BYTE_1;
                aDataPtr[1] = VOP_START_BYTE_2;
                aDataPtr[2] = VOP_START_BYTE_3;
                aDataPtr[3] = c3;
                aDataPtr[4] = c4;

                return true;
            }
        }
    }

    else
    {
        // Unhandled codec type. Error
        return false;
    }

    return false;
}

void PVMFVideoParserNode::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFVideoParserNode::PortActivity: port=0x%x, type=%d",
                     this, aActivity.iPort, aActivity.iType));

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            if (ProcessOutgoingMsg(aActivity.iPort) != PVMFSuccess)
            {
                if (aActivity.iPort->OutgoingMsgQueueSize() == 1)
                    QueuePortActivity(aActivity);
            }
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:

            if (ProcessIncomingMsg(aActivity.iPort) != PVMFSuccess)
            {
                if (aActivity.iPort->IncomingMsgQueueSize() == 1)
                    QueuePortActivity(aActivity);
            }

            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            if (aActivity.iPort->OutgoingMsgQueueSize() > 0)
            {
                RunIfNotReady();
            }
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
            //nothing needed.
            break;

        case PVMF_PORT_ACTIVITY_DISCONNECT:
            //clear the node input queue when either port is disconnected.
            break;
        default:
            break;
    }
}

int PVMFVideoParserNode::AddPortActivity(const PVMFPortActivity& activity)
{
    int32 err = OsclErrNone;
    OSCL_TRY(err, iPortActivityQueue.push_back(activity););
    return err;
}

bool PVMFVideoParserNode::ProcessPortActivity()
{//called by the AO to process a port activity message

    //Pop the queue...
    PVMFPortActivity activity(iPortActivityQueue.front());
    iPortActivityQueue.erase(&iPortActivityQueue.front());

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFVideoParserNode::ProcessPortActivity: port=0x%x, type=%d",
                     this, activity.iPort, activity.iType));

    int32 err = OsclErrNone;

    PVMFStatus status = PVMFSuccess;
    switch (activity.iType)
    {
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            status = ProcessOutgoingMsg(activity.iPort);
            //Re-queue the port activity event as long as there's
            //more data to process and it isn't in a Busy state.
            if (status != PVMFErrBusy
                    && activity.iPort->OutgoingMsgQueueSize() > 0)
            {
                err = AddPortActivity(activity);
            }
            break;
        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            status = ProcessIncomingMsg(activity.iPort);
            //Re-queue the port activity event as long as there's
            //more data to process and it isn't in a Busy state.
            if (status != PVMFErrBusy
                    && activity.iPort->IncomingMsgQueueSize() > 0)
            {
                err = AddPortActivity(activity);
            }
            break;
        default:
            break;
    }

    //report a failure in queueing new activity...
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFVideoParserNode::Run: Error - queue port activity failed. port=0x%x, type=%d", this, activity.iPort, activity.iType));
        ReportErrorEvent(PVMFErrPortProcessing);
    }

    //return true if we processed an activity...
    return (status != PVMFErrBusy);
}

void PVMFVideoParserNode::QueuePortActivity(const PVMFPortActivity &aActivity)
{
    //queue a new port activity event
    int32 err;
    err = AddPortActivity(aActivity);
    if (err != OsclErrNone)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMFVideoParserNode::PortActivity: Error - iPortActivityQueue.push_back() failed", this));
        ReportErrorEvent(PVMFErrPortProcessing, (OsclAny*)(aActivity.iPort));
    }
    else
    {
        //wake up the AO to process the port activity event.
        RunIfNotReady();
    }
}


PVMFStatus PVMFVideoParserNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    //Called by the AO to process one message off the outgoing
    //message queue for the given port.  This routine will
    //try to send the data to the connected port.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMFVideoParserNode::ProcessOutgoingMsg: aPort=0x%x", this, aPort));

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "0x%x PVMFVideoParserNode::ProcessOutgoingMsg: Connected port goes into busy state", this));
    }

    return status;
}

PVMFStatus PVMFVideoParserNode::ProcessIncomingMsg(PVMFPortInterface* aPort)
{
    PVMFSharedMediaMsgPtr msg;
    if (iInterfaceState != EPVMFNodeStarted)
        return PVMFFailure;
    PVMFStatus status = aPort->DequeueIncomingMsg(msg);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "0x%x PVMFVideoParserNode::ProcessIncomingMsg: Error - DequeueIncomingMsg failed", this));
    }
    else if (msg->getFormatID() == PVMF_MEDIA_CMD_BOS_FORMAT_ID)
    {
        PVMFSharedMediaCmdPtr sharedMediaCmdPtr = PVMFMediaCmd::createMediaCmd();
        // Set the formatID, timestamp, sequenceNumber and streamID for the media message
        sharedMediaCmdPtr->setFormatID(PVMF_MEDIA_CMD_BOS_FORMAT_ID);
        sharedMediaCmdPtr->setTimestamp(msg->getTimestamp());
        sharedMediaCmdPtr->setSeqNum(msg->getSeqNum());
        sharedMediaCmdPtr->setStreamID(msg->getStreamID());

        PVMFSharedMediaMsgPtr mediaMsgOut;
        convertToPVMFMediaCmdMsg(mediaMsgOut, sharedMediaCmdPtr);
        PVMFStatus retVal = iOutputPort->QueueOutgoingMsg(mediaMsgOut);

        if (retVal != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFVideoParserNode::SendBeginOfMediaStreamCommand: Outgoing queue busy. "));
        }
        msg.Unbind();
        return retVal;
    }
    else
    {
        DataReceived(msg);
    }
    return status;
}
