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
 * @file pvmp4ffcn_port.cpp
 * @brief Port for PVMp4FFComposerNode
 */

#ifndef PVMP4FFCN_PORT_H_INCLUDED
#include "pvmp4ffcn_port.h"
#endif
#ifndef PVMP4FFCN_NODE_H_INCLUDED
#include "pvmp4ffcn_node.h"
#endif
#ifndef PVMP4FFCN_TYPES_H_INCLUDED
#include "pvmp4ffcn_types.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef OSCL_MIME_STRING_UTILS_H
#include "pv_mime_string_utils.h"
#endif

#define LOG_STACK_TRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);
#define LOG_DEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, m);
#define LOG_ERR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_REL,iLogger,PVLOGMSG_ERR,m);

////////////////////////////////////////////////////////////////////////////
PVMp4FFComposerPort::PVMp4FFComposerPort(int32 aTag, PVMp4FFComposerNode* aNode, int32 aPriority, const char* aName)
        :	PvmfPortBaseImpl(aTag, this,
                           PVMF_MP4FFCN_PORT_CAPACITY, PVMF_MP4FFCN_PORT_RESERVE, PVMF_MP4FFCN_PORT_THRESHOLD,
                           PVMF_MP4FFCN_PORT_CAPACITY, PVMF_MP4FFCN_PORT_RESERVE, PVMF_MP4FFCN_PORT_THRESHOLD, aName),
        OsclActiveObject(aPriority, "PVMp4FFComposerPort"),
        iNode(aNode),
        iTrackId(0),
        iFormat(PVMF_MIME_FORMAT_UNKNOWN),
        iCodecType(0),
        iReferencePort(NULL),
        iLastTS(0),
        iEndOfDataReached(false)
{
    AddToScheduler();
    iLogger = PVLogger::GetLoggerObject("PVMp4FFComposerPort");
    oscl_memset((OsclAny*)&iFormatSpecificConfig, 0, sizeof(PVMP4FFCNFormatSpecificConfig));
    memfrag_pps = NULL;
}

////////////////////////////////////////////////////////////////////////////
PVMp4FFComposerPort::~PVMp4FFComposerPort()
{
    if (memfrag_sps)
    {
        for (uint32 i = 0;i < iNode->memvector_sps.size();i++)
        {
            OSCL_FREE(iNode->memvector_sps[i]->ptr);
        }

        while (!iNode->memvector_sps.empty())
        {
            if (iNode->memvector_sps.front())
            {
                OSCL_FREE(iNode->memvector_sps.front());
            }
            iNode->memvector_sps.erase(&iNode->memvector_sps.front());
        }
    }

    if (memfrag_pps)
    {
        for (uint32 i = 0;i < iNode->memvector_pps.size();i++)
        {
            OSCL_FREE(iNode->memvector_pps[i]->ptr);
        }

        while (!iNode->memvector_pps.empty())
        {
            if (iNode->memvector_pps.front())
            {
                OSCL_FREE(iNode->memvector_pps.front());
            }
            iNode->memvector_pps.erase(&iNode->memvector_pps.front());
        }
    }

    if (iNode->textdecodervector.size() > 0)
    {

        while (!iNode->textdecodervector.empty())
        {
            if (iNode->textdecodervector.front())
            {
                OSCL_DELETE(iNode->textdecodervector.front());
            }

            iNode->textdecodervector.erase(&iNode->textdecodervector.front());
        }
    }
    Disconnect();
    ClearMsgQueues();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerPort::QueryInterface(const PVUuid& aUuid, OsclAny*& aPtr)
{
    if (aUuid == PVMI_CAPABILITY_AND_CONFIG_PVUUID)
        aPtr = (PvmiCapabilityAndConfig*)this;
    else
        aPtr = NULL;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerPort::Connect(PVMFPortInterface* aPort)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::Connect: aPort=0x%x", aPort));

    if (!aPort)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::Connect: Error - Connecting to invalid port"));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::Connect: Error - Already connected"));
        return PVMFFailure;
    }

    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (!config)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::Connect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }

    PVMFStatus status = PVMFSuccess;
    switch (iTag)
    {
        case PVMF_MP4FFCN_PORT_TYPE_SINK:
            status = NegotiateInputSettings(config);
            break;
        default:
            LOG_ERR((0, "PVMp4FFComposerPort::Connect: Error - Invalid port tag"));
            status = PVMFFailure;
    }

    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::Connect: Error - Settings negotiation failed. status=%d", status));
        return status;
    }

    //Automatically connect the peer.
    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::Connect: Error - Peer Connect failed"));
        return PVMFFailure;
    }

    iConnectedPort = aPort;

#if PVMF_PORT_BASE_IMPL_STATS
    // Reset statistics
    oscl_memset((OsclAny*)&(PvmfPortBaseImpl::iStats), 0, sizeof(PvmfPortBaseImplStats));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerPort::PeerConnect(PVMFPortInterface* aPort)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "0x%x PVMp4FFComposerPort::PeerConnect: aPort=0x%x", this, aPort));

    if (!aPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMp4FFComposerPort::PeerConnect: Error - Connecting to invalid port", this));
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_REL, iLogger, PVLOGMSG_ERR,
                        (0, "0x%x PVMp4FFComposerPort::PeerConnect: Error - Already connected", this));
        return PVMFFailure;
    }

    // When connection is initiated by peer, this port still needs to query the peer
    // for input format configuration data
    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *config = OSCL_STATIC_CAST(PvmiCapabilityAndConfig*, temp);

    if (!config)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::PeerConnect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }

    PVMFStatus status = GetInputParametersFromPeer(config);
    if (status != PVMFSuccess)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::PeerConnect: Error - GetVideoInputParametersFromPeer failed. status=%d",
                 status));
        return status;
    }

    iConnectedPort = aPort;

#if PVMF_PORT_BASE_IMPL_STATS
    // Reset statistics
    oscl_memset((OsclAny*)&(PvmfPortBaseImpl::iStats), 0, sizeof(PvmfPortBaseImplStats));
#endif

    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
//                  PvmiCapabilityAndConfig
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerPort::setObserver(PvmiConfigAndCapabilityCmdObserver* aObserver)
{
    // Not supported
    OSCL_UNUSED_ARG(aObserver);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerPort::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::getParametersSync"));
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);

    parameters = NULL;
    num_parameter_elements = 0;
    PVMFStatus status = PVMFFailure;

    //identifier is a key and is assumed to be null terminated
    if (oscl_strcmp(identifier, INPUT_FORMATS_CAP_QUERY) == 0)
    {
        num_parameter_elements = 8;
        status = AllocateKvp(parameters, (PvmiKeyType)INPUT_FORMATS_VALTYPE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMp4FFComposerPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.pChar_value = (char*)PVMF_MIME_AMR_IETF;
            parameters[1].value.pChar_value = (char*)PVMF_MIME_MPEG4_AUDIO;
            parameters[2].value.pChar_value = (char*)PVMF_MIME_M4V;
            parameters[3].value.pChar_value = (char*)PVMF_MIME_H2631998;
            parameters[4].value.pChar_value = (char*)PVMF_MIME_H2632000;
            parameters[5].value.pChar_value = (char*)PVMF_MIME_H264_VIDEO_MP4;
            parameters[6].value.pChar_value = (char*)PVMF_MIME_3GPP_TIMEDTEXT;
            parameters[7].value.pChar_value = (char*)PVMF_MIME_AMRWB_IETF;
        }
    }
    else if (oscl_strcmp(identifier, INPUT_FORMATS_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (PvmiKeyType)INPUT_FORMATS_VALTYPE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            LOG_ERR((0, "PVMp4FFComposerPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
        }
        else
        {
            parameters[0].value.pChar_value = (char*)iFormat.getMIMEStrPtr();
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerPort::releaseParameters(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(num_elements);

    if (parameters)
    {
        iAlloc.deallocate((OsclAny*)parameters);
        return PVMFSuccess;
    }
    else
    {
        return PVMFFailure;
    }
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerPort::createContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerPort::setContextParameters(PvmiMIOSession session,
        PvmiCapabilityContext& context,
        PvmiKvp* parameters, int num_parameter_elements)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_parameter_elements);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerPort::DeleteContext(PvmiMIOSession session, PvmiCapabilityContext& context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMp4FFComposerPort::setParametersSync(PvmiMIOSession session, PvmiKvp* parameters,
        int num_elements, PvmiKvp*& ret_kvp)
{
    OSCL_UNUSED_ARG(session);

    ret_kvp = NULL;
    if (iFormat == PVMF_MIME_H264_VIDEO_MP4)
    {
        //this code is specific to H264 file format
        for (int32 i = 0;i < num_elements;i++)//assuming the memory is allocated for key
        {
            if (pv_mime_strcmp(parameters->key, VIDEO_AVC_OUTPUT_SPS_CUR_VALUE) == 0)
            {
                memfrag_sps = (OsclMemoryFragment *)(OSCL_MALLOC(sizeof(OsclMemoryFragment)));
                memfrag_sps->len = parameters->capacity;
                memfrag_sps->ptr = (uint8*)(OSCL_MALLOC(sizeof(uint8) * memfrag_sps->len));
                oscl_memcpy((void*)memfrag_sps->ptr, (const void*)parameters->value.key_specific_value, memfrag_sps->len);
                iNode->memvector_sps.push_back(memfrag_sps);	//storing SPS in the vector
                iNode->iNum_SPS_Set += 1;
            }
            if (pv_mime_strcmp(parameters->key, VIDEO_AVC_OUTPUT_PPS_CUR_VALUE) == 0)
            {
                memfrag_pps = (OsclMemoryFragment *)(OSCL_MALLOC(sizeof(OsclMemoryFragment)));
                memfrag_pps->len = parameters->capacity;
                memfrag_pps->ptr = (uint8*)(OSCL_MALLOC(sizeof(uint8) * memfrag_pps->len));
                oscl_memcpy((void*)memfrag_pps->ptr, (const void*)parameters->value.key_specific_value, memfrag_pps->len);
                iNode->memvector_pps.push_back(memfrag_pps);	//storing PPS in the vector
                iNode->iNum_PPS_Set += 1;
            }
        }
    }
    if (iFormat == PVMF_MIME_3GPP_TIMEDTEXT)
    {
        for (int32 i = 0;i < num_elements;i++)//assuming the memory is allocated for keys
        {
            if (pv_mime_strcmp(parameters->key, TIMED_TEXT_OUTPUT_CONFIG_INFO_CUR_VALUE) == 0)
            {
                PVA_FF_TextSampleDescInfo* ptempDecoderinfo =
                    OSCL_STATIC_CAST(PVA_FF_TextSampleDescInfo*, parameters->value.key_specific_value);

                PVA_FF_TextSampleDescInfo* pDecoderinfo = OSCL_NEW(PVA_FF_TextSampleDescInfo, (*ptempDecoderinfo));
                iNode->textdecodervector.push_back(pDecoderinfo);
            }
        }
    }

}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFCommandId PVMp4FFComposerPort::setParametersAsync(PvmiMIOSession session,
        PvmiKvp* parameters,
        int num_elements,
        PvmiKvp*& ret_kvp,
        OsclAny* context)
{
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(parameters);
    OSCL_UNUSED_ARG(num_elements);
    OSCL_UNUSED_ARG(ret_kvp);
    OSCL_UNUSED_ARG(context);
    return -1;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF uint32 PVMp4FFComposerPort::getCapabilityMetric(PvmiMIOSession session)
{
    OSCL_UNUSED_ARG(session);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMp4FFComposerPort::verifyParametersSync(PvmiMIOSession session,
        PvmiKvp* parameters, int num_elements)
{
    OSCL_UNUSED_ARG(session);

    PVMFStatus status = PVMFSuccess;
    for (int32 i = 0; (i < num_elements) && (status == PVMFSuccess); i++)
        status = VerifyAndSetParameter(&(parameters[i]));

    return status;
}

////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerPort::ProcessIncomingMsgReady()
{
    if (IncomingMsgQueueSize() > 0)
        RunIfNotReady();
}

////////////////////////////////////////////////////////////////////////////
//           Pure virtuals from PVMFPortActivityHandler
////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerPort::HandlePortActivity(const PVMFPortActivity& aActivity)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::HandlePortActivity: type=%d", aActivity.iType));

    if (aActivity.iPort != this)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::HandlePortActivity: Error - Activity is not on this port"));
        return;
    }

    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CREATED:
            //Report port created info event to the node.
            iNode->ReportInfoEvent(PVMFInfoPortCreated,
                                   (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_DELETED:
            //Report port deleted info event to the node.
            iNode->ReportInfoEvent(PVMFInfoPortDeleted,
                                   (OsclAny*)aActivity.iPort);
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            // Not supported
            break;

        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            //Wakeup the AO on the first message only. After that it re-schedules itself as needed.
            if (IncomingMsgQueueSize() == 1)
            {
                RunIfNotReady();
            }
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
            // Not supported
            break;

        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            // Not supported
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            // Not supported
            break;

        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            // Not supported
            break;

        case PVMF_PORT_ACTIVITY_CONNECT:
        case PVMF_PORT_ACTIVITY_DISCONNECT:
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////
//           Pure virtuals from OsclActiveObject
////////////////////////////////////////////////////////////////////////////
void PVMp4FFComposerPort::Run()
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::Run"));
    PVMFStatus status = PVMFSuccess;

    if (!iEndOfDataReached && (IncomingMsgQueueSize() > 0))
    {
        //dispatch the incoming data.
#ifdef _TEST_AE_ERROR_HANDLING

        if (1 == iNode->iErrorDataPathStall)
        {
            status = PVMFSuccess;
        }
        else
        {
            status = iNode->ProcessIncomingMsg(this);
        }
#else
        status = iNode->ProcessIncomingMsg(this);
#endif
        switch (status)
        {
            case PVMFSuccess:
                // Reschedule if there is more data and the node did not become busy
                // after processing the current msg
                if (IncomingMsgQueueSize() > 0 && iNode->IsProcessIncomingMsgReady())
                {
                    RunIfNotReady();
                }
                break;

            case PVMFErrBusy:
                // Node busy. Don't schedule next data
                break;

            default:
                LOG_ERR((0, "PVMp4FFComposerPort::Run: Error - ProcessIncomingMsg failed. status=%d", status));
                break;
        }
    }

    if (iNode->IsFlushPending())
    {
        if (IncomingMsgQueueSize() == 0 && OutgoingMsgQueueSize() == 0)
            iNode->FlushComplete();
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerPort::AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::AllocateKvp"));
    uint8* buf = NULL;
    uint32 keyLen = oscl_strlen(aKey) + 1;
    int32 err = 0;

    OSCL_TRY(err,
             buf = (uint8*)iAlloc.allocate(aNumParams * (sizeof(PvmiKvp) + keyLen));
             if (!buf)
             OSCL_LEAVE(OsclErrNoMemory);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVMp4FFComposerPort::AllocateKvp: Error - kvp allocation failed"));
                         return PVMFErrNoMemory;
                        );

    int32 i = 0;
    PvmiKvp* curKvp = aKvp = OSCL_PLACEMENT_NEW(buf, PvmiKvp);
    buf += sizeof(PvmiKvp);
    for (i = 1; i < aNumParams; i++)
    {
        curKvp += i;
        curKvp = OSCL_PLACEMENT_NEW(buf, PvmiKvp);
        buf += sizeof(PvmiKvp);
    }

    for (i = 0; i < aNumParams; i++)
    {
        aKvp[i].key = (char*)buf;
        oscl_strncpy(aKvp[i].key, aKey, keyLen);
        buf += keyLen;
    }

    return PVMFSuccess;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerPort::VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::VerifyAndSetParameter: aKvp=0x%x, aSetParam=%d", aKvp, aSetParam));

    if (!aKvp)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::VerifyAndSetParameter: Error - Invalid key-value pair"));
        return PVMFFailure;
    }

    if (pv_mime_strcmp(aKvp->key, INPUT_FORMATS_VALTYPE) == 0)
    {
        if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_3GPP_TIMEDTEXT) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_AMR_IETF) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_AMRWB_IETF) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H264_VIDEO_MP4) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_M4V) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H2631998) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H2632000) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_MPEG4_AUDIO) == 0)
        {
            if (aSetParam)
                iFormat = aKvp->value.pChar_value;
            return PVMFSuccess;
        }
        else
        {
            LOG_ERR((0, "PVMp4FFComposerPort::VerifyAndSetParameter: Error - Unsupported format %d",
                     aKvp->value.uint32_value));
            return PVMFFailure;
        }
    }

    LOG_ERR((0, "PVMp4FFComposerPort::VerifyAndSetParameter: Error - Unsupported parameter"));
    return PVMFFailure;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerPort::NegotiateInputSettings(PvmiCapabilityAndConfig* aConfig)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::NegotiateInputSettings: aConfig=0x%x", aConfig));
    if (!aConfig)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::NegotiateInputSettings: Error - Invalid config object"));
        return PVMFFailure;
    }

    PvmiKvp* kvp = NULL;
    PvmiKvp* retKvp = NULL;
    int numParams = 0;
    int32 err = 0;

    // Get current output formats from peer
    PVMFStatus status = aConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_FORMATS_CUR_QUERY, kvp, numParams, NULL);
    if (status != PVMFSuccess || numParams != 1)
    {
        LOG_ERR((0, "PVMp4FFComposerPort::NegotiateInputSettings: Error - config->getParametersSync(output_formats) failed"));
        return status;
    }

    // Check if data format from peer is supported
    if (pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_3GPP_TIMEDTEXT) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_AMR_IETF) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_AMRWB_IETF) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_H264_VIDEO_MP4) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_M4V) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_H2631998) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_H2632000) == 0 ||
            pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_MPEG4_AUDIO) == 0)
    {
        // do nothing
    }
    else
    {
        return PVMFErrNotSupported;
    }

    // Set format of this port, peer port and container node
    iFormat = kvp->value.pChar_value;
    OSCL_TRY(err, aConfig->setParametersSync(NULL, kvp, 1, retKvp););
    OSCL_FIRST_CATCH_ANY(err,
                         LOG_ERR((0, "PVMp4FFComposerPort::NegotiateInputSettings: Error - aConfig->setParametersSync failed. err=%d", err));
                         return PVMFFailure;
                        );

    // Release parameters back to peer and reset for the next query
    aConfig->releaseParameters(NULL, kvp, numParams);
    kvp = NULL;
    numParams = 0;

    return GetInputParametersFromPeer(aConfig);
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMp4FFComposerPort::GetInputParametersFromPeer(PvmiCapabilityAndConfig* aConfig)
{
    LOG_STACK_TRACE((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: aConfig=0x%x", aConfig));

    PvmiKvp* kvp = NULL;
    int numParams = 0;

    // Get data bitrate from peer
    PVMFStatus status = aConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_BITRATE_CUR_QUERY, kvp, numParams, NULL);
    if (status != PVMFSuccess || !kvp || numParams != 1)
    {
        LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Bitrate info not available. Use default"));
        if (iFormat == PVMF_MIME_3GPP_TIMEDTEXT)
        {
            iFormatSpecificConfig.iBitrate = PVMF_MP4FFCN_TEXT_BITRATE;
        }
        else if (iFormat == PVMF_MIME_AMR_IETF ||
                 iFormat == PVMF_MIME_AMRWB_IETF ||
                 iFormat == PVMF_MIME_MPEG4_AUDIO)
        {
            iFormatSpecificConfig.iBitrate = PVMF_MP4FFCN_AUDIO_BITRATE;
        }
        else if (iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
                 iFormat == PVMF_MIME_M4V ||
                 iFormat == PVMF_MIME_H2631998 ||
                 iFormat == PVMF_MIME_H2632000)
        {
            iFormatSpecificConfig.iBitrate = PVMF_MP4FFCN_VIDEO_BITRATE;
        }
        else
        {
            return PVMFErrNotSupported;
        }
    }
    else
    {
        iFormatSpecificConfig.iBitrate = kvp[0].value.uint32_value;
        aConfig->releaseParameters(NULL, kvp, numParams);
    }
    kvp = NULL;
    numParams = 0;

    // Get timescale from peer
    if (iFormat == PVMF_MIME_AMR_IETF ||
            iFormat == PVMF_MIME_AMRWB_IETF ||
            iFormat == PVMF_MIME_MPEG4_AUDIO)
    {
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_TIMESCALE_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || !kvp || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Sampling rate info not available. Use default"));
            iFormatSpecificConfig.iTimescale = PVMF_MP4FFCN_AUDIO_TIMESCALE;
        }
        else
        {
            iFormatSpecificConfig.iTimescale = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;
    }
    else if (iFormat == PVMF_MIME_H264_VIDEO_MP4 ||
             iFormat == PVMF_MIME_M4V ||
             iFormat == PVMF_MIME_H2631998 ||
             iFormat == PVMF_MIME_H2632000)
    {
        if (iFormat == PVMF_MIME_H2631998 ||
                iFormat == PVMF_MIME_H2632000)
        {
            iFormatSpecificConfig.iH263Profile = PVMF_MP4FFCN_VIDEO_H263_PROFILE;
            iFormatSpecificConfig.iH263Level = PVMF_MP4FFCN_VIDEO_H263_LEVEL;
            // Do not break here. Continue to configure the other video parameters
        }

        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_WIDTH_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Frame width not available. Use default"));
            iFormatSpecificConfig.iWidth = PVMF_MP4FFCN_VIDEO_FRAME_WIDTH;
        }
        else
        {
            iFormatSpecificConfig.iWidth = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;

        iFormatSpecificConfig.iRateControlType = PVMP4FFCN_RATE_CONTROL_CBR;

        // Get size (in pixels) of video data from peer
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_IFRAME_INTERVAL_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Frame width not available. Use default"));
            iFormatSpecificConfig.iIFrameInterval = PVMF_MP4FFCN_VIDEO_IFRAME_INTERVAL;
        }
        else
        {
            iFormatSpecificConfig.iIFrameInterval = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;

        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_HEIGHT_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Frame height info not available. Use default"));
            iFormatSpecificConfig.iHeight = PVMF_MP4FFCN_VIDEO_FRAME_HEIGHT;
        }
        else
        {
            iFormatSpecificConfig.iHeight = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;

        // Get video frame rate from peer
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Frame rate not available. Use default"));
            iFormatSpecificConfig.iFrameRate = PVMF_MP4FFCN_VIDEO_FRAME_RATE;
        }
        else
        {
            // Set input frame rate of container node
            iFormatSpecificConfig.iFrameRate = kvp[0].value.float_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;

        status = aConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_TIMESCALE_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || !kvp || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Sampling rate info not available. Use default"));
            iFormatSpecificConfig.iTimescale = PVMF_MP4FFCN_VIDEO_TIMESCALE;
        }
        else
        {
            iFormatSpecificConfig.iTimescale = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;
    }
    else if (iFormat == PVMF_MIME_3GPP_TIMEDTEXT)
    {
        // Get size (in pixels) of Text data from peer
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)TEXT_INPUT_WIDTH_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Frame width not available. Use default"));
            iFormatSpecificConfig.iWidth = PVMF_MP4FFCN_TEXT_FRAME_WIDTH;
        }
        else
        {
            iFormatSpecificConfig.iWidth = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;

        status = aConfig->getParametersSync(NULL, (PvmiKeyType)TEXT_INPUT_HEIGHT_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Frame height info not available. Use default"));
            iFormatSpecificConfig.iHeight = PVMF_MP4FFCN_TEXT_FRAME_HEIGHT;
        }
        else
        {
            iFormatSpecificConfig.iHeight = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;

        status = aConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_TIMESCALE_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || !kvp || numParams != 1)
        {
            LOG_DEBUG((0, "PVMp4FFComposerPort::GetInputParametersFromPeer: Sampling rate info not available. Use default"));
            iFormatSpecificConfig.iTimescale = PVMF_MP4FFCN_TEXT_TIMESCALE;
        }
        else
        {
            iFormatSpecificConfig.iTimescale = kvp[0].value.uint32_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
        }
        kvp = NULL;
        numParams = 0;
    }
    else
    {
        return PVMFErrNotSupported;
    }

    return PVMFSuccess;
}







