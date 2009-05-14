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
#include "pvmf_omx_enc_port.h"
#include "pvmf_omx_enc_node.h"


PVMFOMXEncPort::PVMFOMXEncPort(int32 aTag, PVMFNodeInterface* aNode, const char*name)
        : PvmfPortBaseImpl(aTag, aNode, name)
{
    iOMXNode = (PVMFOMXEncNode *) aNode;
    Construct();
}

////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject("PVMFOMXEncPort");
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    iNumFramesConsumed = 0;
    iTrackConfig = NULL;
    iTrackConfigSize = 0;
    iTimescale = 0;

}


PVMFOMXEncPort::~PVMFOMXEncPort()
{
    if (iTrackConfig != NULL)
    {
        OSCL_FREE(iTrackConfig);
        iTrackConfigSize = 0;
    }

    Disconnect();
    ClearMsgQueues();
}

////////////////////////////////////////////////////////////////////////////
bool PVMFOMXEncPort::IsFormatSupported(PVMFFormatType aFmt)
{
    if ((aFmt == PVMF_MIME_YUV420) ||
            (aFmt == PVMF_MIME_YUV422) ||
            (aFmt == PVMF_MIME_RGB12) ||
            (aFmt == PVMF_MIME_RGB24) ||
            (aFmt == PVMF_MIME_PCM16) ||
            (aFmt == PVMF_MIME_H264_VIDEO_RAW) ||
            (aFmt == PVMF_MIME_H264_VIDEO_MP4) ||
            //(aFmt==PVMF_MIME_H264_VIDEO) ||
            (aFmt == PVMF_MIME_M4V) ||
            (aFmt == PVMF_MIME_H2631998) ||
            (aFmt == PVMF_MIME_H2632000) ||
            (aFmt == PVMF_MIME_AMR_IETF) ||
            (aFmt == PVMF_MIME_AMRWB_IETF) ||
            (aFmt == PVMF_MIME_AMR_IF2) ||
            (aFmt == PVMF_MIME_ADTS) ||
            (aFmt == PVMF_MIME_ADIF) ||
            (aFmt == PVMF_MIME_MPEG4_AUDIO))
    {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////
void PVMFOMXEncPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "PVMFOMXEncPort::FormatUpdated %s", iFormat.getMIMEStrPtr()));
}

bool
PVMFOMXEncPort::pvmiSetPortFormatSpecificInfoSync(OsclRefCounterMemFrag& aMemFrag)
{
    if ((iConnectedPort) &&
            (iTag == PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT))
    {
        OsclAny* temp = NULL;
        iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
        PvmiCapabilityAndConfig *config = (PvmiCapabilityAndConfig*) temp;

        /*
         * Create PvmiKvp for capability settings
         */
        if ((config) && (aMemFrag.getMemFragSize() > 0))
        {
            OsclMemAllocator alloc;
            PvmiKvp kvp;
            kvp.key = NULL;
            kvp.length = oscl_strlen(PVMF_FORMAT_SPECIFIC_INFO_KEY) + 1; // +1 for \0
            kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
            if (kvp.key == NULL)
            {
                return false;
            }
            oscl_strncpy(kvp.key, PVMF_FORMAT_SPECIFIC_INFO_KEY, kvp.length);

            kvp.value.key_specific_value = (OsclAny*)(aMemFrag.getMemFragPtr());
            kvp.capacity = aMemFrag.getMemFragSize();
            kvp.length = aMemFrag.getMemFragSize();
            PvmiKvp* retKvp = NULL; // for return value
            int32 err;
            OSCL_TRY(err, config->setParametersSync(NULL, &kvp, 1, retKvp););
            /* ignore the error for now */
            alloc.deallocate((OsclAny*)(kvp.key));
        }
        return true;
    }
    return false;
}

PVMFStatus PVMFOMXEncPort::Connect(PVMFPortInterface* aPort)
{
    if (!aPort)
    {
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        return PVMFFailure;
    }

    OsclAny* temp = NULL;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig* config = (PvmiCapabilityAndConfig*) temp;
    if (!config)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - Peer port does not support capability interface"));
        return PVMFFailure;
    }

    PVMFStatus status = PVMFSuccess;
    switch (iTag)
    {
        case PVMF_OMX_ENC_NODE_PORT_TYPE_INPUT:
            status = NegotiateInputSettings(config);
            break;
        case PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT:
            status = NegotiateOutputSettings(config);
            break;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - Invalid port tag"));
            status = PVMFFailure;
    }

    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - Settings negotiation failed. status=%d", status));
        return status;
    }



    // Automatically connect the peer.

    if (aPort->PeerConnect(this) != PVMFSuccess)
    {
        return PVMFFailure;
    }

    iConnectedPort = aPort;
    PortActivity(PVMF_PORT_ACTIVITY_CONNECT);
    return PVMFSuccess;
}

// needed for WMV port exchange
void PVMFOMXEncPort::setParametersSync(PvmiMIOSession aSession,
                                       PvmiKvp* aParameters,
                                       int num_elements,
                                       PvmiKvp * & aRet_kvp)
{

    OSCL_UNUSED_ARG(aSession);
    PVMFStatus status = PVMFSuccess;
    aRet_kvp = NULL;

    for (int32 i = 0; i < num_elements; i++)
    {
        status = VerifyAndSetParameter(&(aParameters[i]), true);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFOMXEncPort::setParametersSync: Error - VerifiyAndSetParameter failed on parameter #%d", i));
            aRet_kvp = &(aParameters[i]);
            OSCL_LEAVE(OsclErrArgument);
        }
    }


}




PVMFStatus PVMFOMXEncPort::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{

    OSCL_UNUSED_ARG(aSession);

    PVMFStatus status = PVMFSuccess;
    for (int32 i = 0; (i < num_elements) && (status == PVMFSuccess); i++)
        status = VerifyAndSetParameter(&(aParameters[i]));

    return status;


}

PVMFStatus PVMFOMXEncPort::verifyConnectedPortParametersSync(const char* aFormatValType,
        OsclAny* aConfig)
{
    PVMFStatus status = PVMFErrNotSupported;
    PvmiCapabilityAndConfig *capConfig;
    if (iConnectedPort)
    {
        OsclAny* temp = NULL;
        iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
        capConfig = (PvmiCapabilityAndConfig*) temp;
    }
    else
        return PVMFFailure;

    if (capConfig != NULL)
    {
        if (pv_mime_strcmp(aFormatValType, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
        {
            OsclRefCounterMemFrag* aFormatValue = (OsclRefCounterMemFrag*)aConfig;
            if (aFormatValue->getMemFragSize() > 0)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, aFormatValType, kvp.length);

                kvp.value.key_specific_value = (OsclAny*)(aFormatValue->getMemFragPtr());
                kvp.length = aFormatValue->getMemFragSize();
                kvp.capacity = aFormatValue->getMemFragSize();
                int32 err;
                OSCL_TRY(err, status = capConfig->verifyParametersSync(NULL, &kvp, 1););
                /* ignore the error for now */
                alloc.deallocate((OsclAny*)(kvp.key));

                return status;
            }
        }
        else if (pv_mime_strcmp(aFormatValType, PVMF_BITRATE_VALUE_KEY) == 0 ||
                 pv_mime_strcmp(aFormatValType, PVMF_FRAMERATE_VALUE_KEY) == 0)
        {
            if (aConfig != NULL)
            {
                OsclMemAllocator alloc;
                PvmiKvp kvp;
                kvp.key = NULL;
                kvp.length = oscl_strlen(aFormatValType) + 1; // +1 for \0
                kvp.key = (PvmiKeyType)alloc.ALLOCATE(kvp.length);
                if (kvp.key == NULL)
                {
                    return PVMFErrNoMemory;
                }
                oscl_strncpy(kvp.key, aFormatValType, kvp.length);
                uint32* bitrate = (uint32*)aConfig;
                kvp.value.uint32_value = *bitrate;

                int32 err;
                OSCL_TRY(err, status = capConfig->verifyParametersSync(NULL, &kvp, 1););
                /* ignore the error for now */
                alloc.deallocate((OsclAny*)(kvp.key));

                return status;
            }
        }
        return PVMFErrArgument;
    }
    return PVMFFailure;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////// ENCODER SPECIFIC EXCHANGE TO SET PARAMETERS
////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncPort::GetInputParametersSync(PvmiKeyType identifier, PvmiKvp*& parameters,
        int& num_parameter_elements)
{
    if (iTag != PVMF_OMX_ENC_NODE_PORT_TYPE_INPUT)
        return PVMFFailure;

    PVMFStatus status = PVMFSuccess;

    // this comparison only goes up to delimiter (i.e. ; or /)
    if (pv_mime_strcmp(identifier, INPUT_FORMATS_CAP_QUERY) == 0)
    {
        // get the first parameter as well to compare
        char *param1 = NULL;
        char *param2 = NULL;
        pv_mime_string_extract_param(0, identifier, param1);
        pv_mime_string_extract_param(0, (char*)INPUT_FORMATS_CAP_QUERY, param2);

        if (pv_mime_strcmp(param1, param2) == 0)
        {
            num_parameter_elements = 5;
            status = AllocateKvp(parameters, (OMX_STRING)INPUT_FORMATS_VALTYPE, num_parameter_elements);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetInputParametersSync: Error - AllocateKvp failed. status=%d", status));
                return status;
            }

            parameters[0].value.pChar_value = (char*)PVMF_MIME_YUV420;
            parameters[1].value.pChar_value = (char*)PVMF_MIME_YUV422;
            parameters[2].value.pChar_value = (char*)PVMF_MIME_RGB12;
            parameters[3].value.pChar_value = (char*)PVMF_MIME_RGB24;
            parameters[4].value.pChar_value = (char*)PVMF_MIME_PCM16;
        }
    }

    if (pv_mime_strcmp(identifier, INPUT_FORMATS_CUR_QUERY) == 0)
    {
        // get the first parameter as well to compare
        char *param1 = NULL;
        char *param2 = NULL;
        pv_mime_string_extract_param(0, identifier, param1);
        pv_mime_string_extract_param(0, (char*)INPUT_FORMATS_CUR_QUERY, param2);

        if (pv_mime_strcmp(param1, param2) == 0)
        {
            num_parameter_elements = 1;
            status = AllocateKvp(parameters, (PvmiKeyType)INPUT_FORMATS_VALTYPE, num_parameter_elements);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetInputParametersSync: Error - AllocateKvp failed. status=%d", status));
                return status;
            }
            parameters[0].value.pChar_value = (char*)iFormat.getMIMEStrPtr();
        }
    }
    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncPort::GetOutputParametersSync(PvmiKeyType identifier, PvmiKvp*& parameters,
        int& num_parameter_elements)
{
    if (iTag != PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT)
        return PVMFFailure;

    PVMFStatus status = PVMFSuccess;

    // this comparison only goes up to the first delimiter i.e. ; or /
    if (pv_mime_strcmp(identifier, (OMX_STRING)OUTPUT_FORMATS_CAP_QUERY) == 0)
    {
        // get the first parameter as well to compare
        char *param1 = NULL;
        char *param2 = NULL;
        pv_mime_string_extract_param(0, identifier, param1);
        pv_mime_string_extract_param(0, (char*)OUTPUT_FORMATS_CAP_QUERY, param2);

        if (pv_mime_strcmp(param1, param2) == 0)
        {
            num_parameter_elements = 11;
            status = AllocateKvp(parameters, (OMX_STRING)OUTPUT_FORMATS_VALTYPE, num_parameter_elements);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            }
            else
            {
                parameters[0].value.pChar_value = (char*)PVMF_MIME_H2631998;
                parameters[1].value.pChar_value = (char*)PVMF_MIME_H2632000;
                parameters[2].value.pChar_value = (char*)PVMF_MIME_M4V;
                parameters[3].value.pChar_value = (char*)PVMF_MIME_H264_VIDEO_RAW;
                parameters[4].value.pChar_value = (char*)PVMF_MIME_H264_VIDEO_MP4;
                parameters[5].value.pChar_value = (char*)PVMF_MIME_AMR_IETF;
                parameters[6].value.pChar_value = (char*)PVMF_MIME_AMRWB_IETF;
                parameters[7].value.pChar_value = (char*)PVMF_MIME_AMR_IF2;
                parameters[8].value.pChar_value = (char*)PVMF_MIME_ADTS;
                parameters[9].value.pChar_value = (char*)PVMF_MIME_ADIF;
                parameters[10].value.pChar_value = (char*)PVMF_MIME_MPEG4_AUDIO;

            }
        }
    }

    if (pv_mime_strcmp(identifier, OUTPUT_FORMATS_CUR_QUERY) == 0)
    {
        // get the first parameter as well to compare
        char *param1 = NULL;
        char *param2 = NULL;
        pv_mime_string_extract_param(0, identifier, param1);
        pv_mime_string_extract_param(0, (char*)OUTPUT_FORMATS_CUR_QUERY, param2);

        if (pv_mime_strcmp(param1, param2) == 0)
        {
            num_parameter_elements = 1;
            status = AllocateKvp(parameters, (OMX_STRING)OUTPUT_FORMATS_VALTYPE, num_parameter_elements);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            }
            else
            {
                parameters[0].value.pChar_value = (char*)iOMXNode->GetCodecType().getMIMEStrPtr();
            }
        }
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_WIDTH_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (OMX_STRING)VIDEO_OUTPUT_WIDTH_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }

        uint32 width, height;
        status = iOMXNode->GetOutputFrameSize(0, width, height);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error -iOMXNode->GetOutputFrameSize failed. status=%d", status));
        }
        else
        {
            parameters[0].value.uint32_value = width;
        }
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_HEIGHT_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (OMX_STRING)VIDEO_OUTPUT_HEIGHT_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {
            uint32 width, height;
            status = iOMXNode->GetOutputFrameSize(0, width, height);
            if (status != PVMFSuccess)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - iOMXNode->GetOutputFrameSize failed. status=%d", status));
            }
            else
            {
                parameters[0].value.uint32_value = height;
            }
        }
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (OMX_STRING)VIDEO_OUTPUT_FRAME_RATE_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {
            parameters[0].value.float_value = iOMXNode->GetOutputFrameRate(0);
        }
    }
    else if (pv_mime_strcmp(identifier, OUTPUT_BITRATE_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (OMX_STRING)OUTPUT_BITRATE_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {

            // NOTE: we assume that port format will be set before this call
            if ((iFormat == PVMF_MIME_AMR_IETF) || (iFormat == PVMF_MIME_AMRWB_IETF) || (iFormat == PVMF_MIME_AMR_IF2) ||
                    (iFormat == PVMF_MIME_ADTS) || (iFormat == PVMF_MIME_ADIF) || (iFormat == PVMF_MIME_MPEG4_AUDIO))
            {
                parameters[0].value.uint32_value = iOMXNode->GetOutputBitRate(); // use audio version - void arg
            }
            else if (iFormat == PVMF_MIME_H2631998 ||
                     iFormat == PVMF_MIME_H2632000 ||
                     iFormat == PVMF_MIME_M4V ||
                     iFormat == PVMF_MIME_H264_VIDEO_RAW ||
                     iFormat == PVMF_MIME_H264_VIDEO_MP4
                    )

            {
                // use the video version
                parameters[0].value.uint32_value = iOMXNode->GetOutputBitRate(0);
            }
            else
            {
                status = PVMFFailure;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - status=%d", status));
                return status;
            }

        }
    }
    else if (pv_mime_strcmp(identifier, VIDEO_OUTPUT_IFRAME_INTERVAL_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (OMX_STRING)VIDEO_OUTPUT_IFRAME_INTERVAL_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {
            parameters[0].value.uint32_value = iOMXNode->GetIFrameInterval();
        }
    }
    else if (pv_mime_strcmp(identifier, AUDIO_OUTPUT_SAMPLING_RATE_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (PvmiKeyType)AUDIO_OUTPUT_SAMPLING_RATE_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }

        parameters[0].value.uint32_value = (uint32) iOMXNode->GetOutputSamplingRate();

    }
    else if (pv_mime_strcmp(identifier, AUDIO_OUTPUT_NUM_CHANNELS_CUR_QUERY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (PvmiKeyType)AUDIO_OUTPUT_NUM_CHANNELS_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }

        parameters[0].value.uint32_value = (uint32) iOMXNode->GetOutputNumChannels();
    }
    else if ((pv_mime_strcmp(identifier, OUTPUT_TIMESCALE_CUR_QUERY) == 0) &&
             ((iFormat == PVMF_MIME_AMR_IETF) ||
              (iFormat == PVMF_MIME_AMRWB_IETF) ||
              (iFormat == PVMF_MIME_AMR_IF2) ||
              (iFormat == PVMF_MIME_ADTS) ||
              (iFormat == PVMF_MIME_ADIF) ||
              (iFormat == PVMF_MIME_MPEG4_AUDIO)
             ))
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (PvmiKeyType)OUTPUT_TIMESCALE_CUR_VALUE, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {

            parameters[0].value.uint32_value = (uint32) iOMXNode->GetOutputSamplingRate();

        }
    }
    else if (pv_mime_strcmp(identifier, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        num_parameter_elements = 1;
        status = AllocateKvp(parameters, (PvmiKeyType)PVMF_FORMAT_SPECIFIC_INFO_KEY, num_parameter_elements);
        if (status != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::GetOutputParametersSync: Error - AllocateKvp failed. status=%d", status));
            return status;
        }
        else
        {
            OsclRefCounterMemFrag refMemFrag;
            if (iOMXNode->GetVolHeader(refMemFrag))
            {
                parameters[0].value.key_specific_value = refMemFrag.getMemFragPtr();
                parameters[0].capacity = refMemFrag.getMemFragSize();
                parameters[0].length = refMemFrag.getMemFragSize();
            }
            else
            {
                return PVMFFailure;
            }
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncPort::AllocateKvp(PvmiKvp*& aKvp, PvmiKeyType aKey, int32 aNumParams)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncPort::AllocateKvp"));
    uint8* buf = NULL;
    uint32 keyLen = oscl_strlen(aKey) + 1;
    int32 err = 0;

    OSCL_TRY(err,
             buf = (uint8*)iAlloc.allocate(aNumParams * (sizeof(PvmiKvp) + keyLen));
             if (!buf)
             OSCL_LEAVE(OsclErrNoMemory);
            );
    OSCL_FIRST_CATCH_ANY(err,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::AllocateKvp: Error - kvp allocation failed"));
                         return PVMFErrNoMemory;
                        );

    int32 i = 0;
    PvmiKvp* curKvp = aKvp = new(buf) PvmiKvp;
    buf += sizeof(PvmiKvp);
    for (i = 1; i < aNumParams; i++)
    {
        curKvp += i;
        curKvp = new(buf) PvmiKvp;
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
PVMFStatus PVMFOMXEncPort::VerifyAndSetParameter(PvmiKvp* aKvp, bool aSetParam)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncPort::VerifyAndSetParameter: aKvp=0x%x, aSetParam=%d", aKvp, aSetParam));

    if (!aKvp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::VerifyAndSetParameter: Error - Invalid key-value pair"));
        return PVMFFailure;
    }

    if (pv_mime_strcmp(aKvp->key, INPUT_FORMATS_VALTYPE) == 0 &&
            iTag == PVMF_OMX_ENC_NODE_PORT_TYPE_INPUT)
    {
        if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_YUV420) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_YUV422) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_RGB12) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_RGB24) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_PCM16) == 0)
        {
            if (aSetParam)
            {
                iFormat = aKvp->value.pChar_value;
                if (iOMXNode->SetInputFormat(iFormat) != PVMFSuccess)
                    return PVMFFailure;
            }
            return PVMFSuccess;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::VerifyAndSetParameter: Error - Input format %d not supported",
                            aKvp->value.uint32_value));
            return PVMFFailure;
        }
    }
    else if (pv_mime_strcmp(aKvp->key, OUTPUT_FORMATS_VALTYPE) == 0 &&
             iTag == PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT)
    {
        if (pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H2631998) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H2632000) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_M4V) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H264_VIDEO_RAW) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_H264_VIDEO_MP4) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_AMR_IETF) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_AMRWB_IETF) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_AMR_IF2) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_ADIF) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_ADTS) == 0 ||
                pv_mime_strcmp(aKvp->value.pChar_value, PVMF_MIME_MPEG4_AUDIO) == 0)
        {
            if (aSetParam)
            {
                iFormat = aKvp->value.pChar_value;
                if (iOMXNode->SetCodecType(iFormat) != PVMFSuccess)
                    return PVMFFailure;
            }
            return PVMFSuccess;
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::VerifyAndSetParameter: Error - Output format %d not supported",
                            aKvp->value.pChar_value));
            return PVMFFailure;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::VerifyAndSetParameter: Error - Unsupported parameter"));
    return PVMFFailure;
}


////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncPort::NegotiateInputSettings(PvmiCapabilityAndConfig* aConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncPort::NegotiateInputSettings: aConfig=0x%x", aConfig));
    if (!aConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: Error - Invalid config object"));
        return PVMFFailure;
    }

    PvmiKvp* kvp = NULL;
    int numParams = 0;
    int32 err = 0;
    PVMFFormatType videoFormat = 0;
    // Get supported output formats from peer
    PVMFStatus status = aConfig->getParametersSync(NULL, (OMX_STRING)OUTPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
    if (status != PVMFSuccess || numParams == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: Error - config->getParametersSync(output_formats) failed"));
        return status;
    }

    OsclPriorityQueue < PvmiKvp*, OsclMemAllocator,
    Oscl_Vector<PvmiKvp*, OsclMemAllocator>,
    PVMFOMXEncInputFormatCompareLess > sortedKvp;

    // Using a priority queue, sort the kvp's returned from aConfig->getParametersSync
    // according to the preference of this port. Formats that are not supported are
    // not pushed to the priority queue and hence dropped from consideration.
    PvmiKvp* selectedAudioKvp = NULL;
    for (int32 i = 0; i < numParams; i++)
    {
        if (pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_YUV420) == 0 ||
                pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_YUV422) == 0 ||
                pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_RGB12) == 0 ||
                pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_RGB24) == 0)
        {
            videoFormat = kvp[i].value.pChar_value;
            err = PushKVP(sortedKvp, &(kvp[i]));
            OSCL_FIRST_CATCH_ANY(err,
                                 PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: Error - sortedKvp.push failed"));
                                 return PVMFErrNoMemory;
                                );
        }
        else if (pv_mime_strcmp(kvp->value.pChar_value, PVMF_MIME_PCM16) == 0)
        {
            selectedAudioKvp = &kvp[i];
        }
    }

    if (sortedKvp.size() == 0 && (selectedAudioKvp == NULL))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: Error - No matching supported input format"));
        // Release parameters back to peer
        aConfig->releaseParameters(NULL, kvp, numParams);
        kvp = NULL;
        numParams = 0;
        return PVMFFailure;
    }

    PvmiKvp* selectedKvp = NULL;
    if (sortedKvp.size() != 0)
    {
        selectedKvp = sortedKvp.top();
    }
    else
    {
        selectedKvp = selectedAudioKvp;
    }
    PvmiKvp* retKvp = NULL;

    // Set format of this port, peer port and container node
    iFormat = selectedKvp->value.pChar_value;
    if (iOMXNode->SetInputFormat(iFormat) != PVMFSuccess)
        return PVMFFailure;

    err = Config_ParametersSync(aConfig, selectedKvp, retKvp);
    OSCL_FIRST_CATCH_ANY(err,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: Error - aConfig->setParametersSync failed. err=%d", err));
                         return PVMFFailure;
                        );

    // Release parameters back to peer and reset for the next query
    aConfig->releaseParameters(NULL, kvp, numParams);
    kvp = NULL;
    numParams = 0;

    // audio/video negotiation differ
    if (iFormat == PVMF_MIME_PCM16)
    {
        // audio i.e. AMR for now

        // first set bitspersample to 16
        status = iOMXNode->SetInputBitsPerSample(PVMF_AMRENC_DEFAULT_BITSPERSAMPLE);
        if (status != PVMFSuccess)
            return status;


        status = aConfig->getParametersSync(NULL, (PvmiKeyType)AUDIO_OUTPUT_SAMPLING_RATE_CUR_QUERY, kvp, numParams, NULL);
        uint32 samplingRate = 0;
        if (status != PVMFSuccess || !kvp || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: config->getParametersSync(sampling rate) not supported. Use default."));
            samplingRate = PVMF_AMRENC_DEFAULT_SAMPLING_RATE;
        }
        else
        {
            samplingRate = kvp[0].value.uint32_value;

            aConfig->releaseParameters(NULL, kvp, numParams);
        }


        // Forward settings to encoder
        status = iOMXNode->SetInputSamplingRate(samplingRate);
        if (status != PVMFSuccess)
            return status;

        kvp = NULL;
        numParams = 0;

        status = aConfig->getParametersSync(NULL, (PvmiKeyType)AUDIO_OUTPUT_NUM_CHANNELS_CUR_QUERY, kvp, numParams, NULL);
        uint32 numChannels = 0;
        if (status != PVMFSuccess || !kvp || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: config->getParametersSync(num channels) not supported. Use default"));
            numChannels = PVMF_AMRENC_DEFAULT_NUM_CHANNELS;
        }
        else
        {
            numChannels = kvp[0].value.uint32_value;

            aConfig->releaseParameters(NULL, kvp, numParams);
        }

        // Forward settings to encoder
        status = iOMXNode->SetInputNumChannels(numChannels);
        if (status != PVMFSuccess)
            return status;

        kvp = NULL;
        numParams = 0;

        // do the "TIMESCALE" query
        status = aConfig->getParametersSync(NULL, (PvmiKeyType)OUTPUT_TIMESCALE_CUR_QUERY, kvp, numParams, NULL);
        iTimescale = 0;
        if (status != PVMFSuccess || !kvp || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateInputSettings: config->getParametersSync(sampling rate) not supported. Use default."));
            iTimescale = PVMF_AMRENC_DEFAULT_SAMPLING_RATE;
        }
        else
        {
            iTimescale = kvp[0].value.uint32_value;

            aConfig->releaseParameters(NULL, kvp, numParams);
        }

        kvp = NULL;
        numParams = 0;

        return PVMFSuccess;

    }
    else
    {
        // video
        // Get size (in pixels) of video data from peer
        uint32 width = 0;
        uint32 height = 0;
        uint8 orientation = 0;
        status = aConfig->getParametersSync(NULL, (OMX_STRING)VIDEO_OUTPUT_WIDTH_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - config->getParametersSync(current width) failed"));
            return status;
        }
        width = kvp[0].value.uint32_value;
        aConfig->releaseParameters(NULL, kvp, numParams);
        kvp = NULL;
        numParams = 0;

        status = aConfig->getParametersSync(NULL, (OMX_STRING)VIDEO_OUTPUT_HEIGHT_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - config->getParametersSync(current height) failed"));
            return status;
        }
        height = kvp[0].value.uint32_value;
        aConfig->releaseParameters(NULL, kvp, numParams);
        kvp = NULL;
        numParams = 0;

        if ((videoFormat == PVMF_MIME_RGB12) || (videoFormat == PVMF_MIME_RGB24))
        {
            status = aConfig->getParametersSync(NULL, (OMX_STRING)VIDEO_FRAME_ORIENTATION_CUR_QUERY, kvp, numParams, NULL);
            if (status != PVMFSuccess || numParams != 1)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - config->getParametersSync(current height) failed"));
                return status;
            }

            orientation = kvp[0].value.uint8_value;
            aConfig->releaseParameters(NULL, kvp, numParams);
            kvp = NULL;
            numParams = 0;

        }
        // Set input frame size of container node
        status = iOMXNode->SetInputFrameSize(width, height, orientation);
        if (status != PVMFSuccess)
            return status;


        // Get video frame rate from peer
        status = aConfig->getParametersSync(NULL, (OMX_STRING)VIDEO_OUTPUT_FRAME_RATE_CUR_QUERY, kvp, numParams, NULL);
        if (status != PVMFSuccess || numParams != 1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::Connect: Error - config->getParametersSync(current frame rate) failed"));
            return status;
        }

        // Set input frame rate of container node
        status = iOMXNode->SetInputFrameRate(kvp[0].value.float_value);
        aConfig->releaseParameters(NULL, kvp, numParams);
        kvp = NULL;
        numParams = 0;
        return status;
    }
}

////////////////////////////////////////////////////////////////////////////
PVMFStatus PVMFOMXEncPort::NegotiateOutputSettings(PvmiCapabilityAndConfig* aConfig)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncPort::NegotiateOutputSettings: aConfig=0x%x", aConfig));
    if (!aConfig)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateOutputSettings: Error - Invalid config object"));
        return PVMFFailure;
    }

    PvmiKvp* kvp = NULL;
    int numParams = 0;
    int32 i = 0;
    int32 err = 0;

    // Get supported input formats from peer
    PVMFStatus status = aConfig->getParametersSync(NULL, (OMX_STRING)INPUT_FORMATS_CAP_QUERY, kvp, numParams, NULL);
    if (status != PVMFSuccess || numParams == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateOutputSettings: Error - config->getParametersSync(input_formats) failed"));
        return status;
    }

    PvmiKvp* selectedKvp = NULL;
    PvmiKvp* retKvp = NULL;
    for (i = 0; i < numParams && !selectedKvp; i++)
    {
        if (pv_mime_strcmp(kvp[i].value.pChar_value, (char*)iFormat.getMIMEStrPtr()) == 0)
            selectedKvp = &(kvp[i]);
    }

    if (!selectedKvp)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateOutputSettings: Error - Output format not supported by peer"));
        return PVMFFailure;
    }

    err = Config_ParametersSync(aConfig, selectedKvp, retKvp);
    OSCL_FIRST_CATCH_ANY(err,
                         PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::NegotiateOutputSettings: Error - aConfig->setParametersSync failed. err=%d", err));
                         return PVMFFailure;
                        );

    aConfig->releaseParameters(NULL, kvp, numParams);
    kvp = NULL;
    numParams = 0;

    return PVMFSuccess;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncPort::getParametersSync(PvmiMIOSession session,
        PvmiKeyType identifier,
        PvmiKvp*& parameters,
        int& num_parameter_elements,
        PvmiCapabilityContext context)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFOMXEncPort::getParametersSync"));
    OSCL_UNUSED_ARG(session);
    OSCL_UNUSED_ARG(context);

    parameters = NULL;
    num_parameter_elements = 0;
    PVMFStatus status = PVMFFailure;

    switch (iTag)
    {
        case PVMF_OMX_ENC_NODE_PORT_TYPE_INPUT:
            return GetInputParametersSync(identifier, parameters, num_parameter_elements);
        case PVMF_OMX_ENC_NODE_PORT_TYPE_OUTPUT:
            return GetOutputParametersSync(identifier, parameters, num_parameter_elements);
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFOMXEncPort::getParametersSync: Error - Invalid port tag"));
            break;
    }

    return status;
}
////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMFStatus PVMFOMXEncPort::releaseParameters(PvmiMIOSession session,
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


// AVC Encoder specific call to send set of SPS and PPS nals to the composer
void PVMFOMXEncPort::SendSPS_PPS(OsclMemoryFragment *aSPSs, int aNumSPSs, OsclMemoryFragment *aPPSs, int aNumPPSs)
{
    int ii;

    OsclAny* temp = NULL;
    iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig* config = (PvmiCapabilityAndConfig*) temp;
    if (config)
    {
        for (ii = 0; ii < aNumSPSs; ii++)
        {
            // send SPS
            PvmiKvp* sps, *ret;
            AllocateKvp(sps, (OMX_STRING)VIDEO_AVC_OUTPUT_SPS_CUR_VALUE, 1);

            sps->value.key_specific_value = aSPSs[ii].ptr;
            sps->capacity = aSPSs[ii].len;
            sps->length = aSPSs[ii].len;
            config->setParametersSync(NULL, sps, 1, ret);
            if (ret)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOmxEncPort::SendSPS_PPS: Error returned Kvp is not null."));
            }
            // Release parameters back to peer and reset for the next query
            config->releaseParameters(NULL, sps, 1);
        }
        for (ii = 0; ii < aNumPPSs; ii++)
        {
            // send PPS
            PvmiKvp* pps, *ret;
            AllocateKvp(pps, (OMX_STRING)VIDEO_AVC_OUTPUT_PPS_CUR_VALUE, 1);

            pps->value.key_specific_value = aPPSs[ii].ptr;
            pps->capacity = aPPSs[ii].len;
            pps->length = aPPSs[ii].len;
            config->setParametersSync(NULL, pps, 1, ret);
            if (ret)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "PVMFOmxEncPort::SendSPS_PPS: Error returned Kvp is not null."));
            }
            // Release parameters back to peer and reset for the next query
            config->releaseParameters(NULL, pps, 1);
        }
    }

    return ;
}

int32 PVMFOMXEncPort::Config_ParametersSync(PvmiCapabilityAndConfig *&aConfig, PvmiKvp *&aSelectedKvp, PvmiKvp *&aRetKvp)
{
    int32 err = 0;
    OSCL_TRY(err, aConfig->setParametersSync(NULL, aSelectedKvp, 1, aRetKvp););
    return err;
}

int32 PVMFOMXEncPort::PushKVP(OsclPriorityQueue<PvmiKvp*, OsclMemAllocator, Oscl_Vector<PvmiKvp*, OsclMemAllocator>, PVMFOMXEncInputFormatCompareLess> &aSortedKvp, PvmiKvp *aKvp)
{
    int32 err = 0;
    OSCL_TRY(err, aSortedKvp.push(aKvp););
    return err;
}

