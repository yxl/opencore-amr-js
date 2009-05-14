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
#include "pvmf_omx_basedec_port.h"
#include "pvmf_omx_basedec_node.h"


OSCL_EXPORT_REF PVMFOMXDecPort::PVMFOMXDecPort(int32 aTag, PVMFNodeInterface* aNode, const char*name)
        : PvmfPortBaseImpl(aTag, aNode, name)
{
    iOMXNode = (PVMFOMXBaseDecNode *) aNode;
    Construct();
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXDecPort::Construct()
{
    iLogger = PVLogger::GetLoggerObject(PortName());
    oscl_memset(&iStats, 0, sizeof(PvmfPortBaseImplStats));
    iNumFramesGenerated = 0;
    iNumFramesConsumed = 0;
    iTrackConfig = NULL;
    iTrackConfigSize = 0;
    if ((oscl_strcmp(PortName(), PVMF_OMX_VIDEO_DEC_INPUT_PORT_NAME) == 0) || (oscl_strcmp(PortName(), PVMF_OMX_VIDEO_DEC_OUTPUT_PORT_NAME) == 0))
    {
        PvmiCapabilityAndConfigPortFormatImpl::Construct(
            PVMF_OMX_VIDEO_DEC_PORT_INPUT_FORMATS
            , PVMF_OMX_VIDEO_DEC_PORT_INPUT_FORMATS_VALTYPE);
    }
    else if ((oscl_strcmp(PortName(), PVMF_OMX_AUDIO_DEC_INPUT_PORT_NAME) == 0) || (oscl_strcmp(PortName(), PVMF_OMX_AUDIO_DEC_OUTPUT_PORT_NAME) == 0))
    {
        PvmiCapabilityAndConfigPortFormatImpl::Construct(
            PVMF_OMX_AUDIO_DEC_PORT_INPUT_FORMATS
            , PVMF_OMX_AUDIO_DEC_PORT_INPUT_FORMATS_VALTYPE);
    }
}


OSCL_EXPORT_REF PVMFOMXDecPort::~PVMFOMXDecPort()
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
OSCL_EXPORT_REF bool PVMFOMXDecPort::IsFormatSupported(PVMFFormatType aFmt)
{
    bool isSupported = false;

    if ((oscl_strcmp(PortName(), PVMF_OMX_VIDEO_DEC_INPUT_PORT_NAME) == 0) || (oscl_strcmp(PortName(), PVMF_OMX_VIDEO_DEC_OUTPUT_PORT_NAME) == 0))
    {
        isSupported = ((aFmt == PVMF_MIME_YUV420) ||
                       (aFmt == PVMF_MIME_H264_VIDEO_RAW) ||
                       (aFmt == PVMF_MIME_H264_VIDEO_MP4) ||
                       (aFmt == PVMF_MIME_H264_VIDEO) ||
                       (aFmt == PVMF_MIME_M4V) ||
                       (aFmt == PVMF_MIME_H2631998) ||
                       (aFmt == PVMF_MIME_H2632000) ||
                       (aFmt == PVMF_MIME_WMV)
                      );
    }
    else if ((oscl_strcmp(PortName(), PVMF_OMX_AUDIO_DEC_INPUT_PORT_NAME) == 0) || (oscl_strcmp(PortName(), PVMF_OMX_AUDIO_DEC_OUTPUT_PORT_NAME) == 0))
    {
        isSupported = ((aFmt == PVMF_MIME_PCM16) ||
                       (aFmt == PVMF_MIME_LATM) ||
                       (aFmt == PVMF_MIME_MPEG4_AUDIO) ||
                       (aFmt == PVMF_MIME_3640) ||
                       (aFmt == PVMF_MIME_ADIF) ||
                       (aFmt == PVMF_MIME_ASF_MPEG4_AUDIO) ||
                       (aFmt == PVMF_MIME_AAC_SIZEHDR) ||
                       (aFmt == PVMF_MIME_AMR_IF2) ||
                       (aFmt == PVMF_MIME_AMR_IETF) ||
                       (aFmt == PVMF_MIME_AMR) ||
                       (aFmt == PVMF_MIME_AMRWB_IETF) ||
                       (aFmt == PVMF_MIME_AMRWB) ||
                       (aFmt == PVMF_MIME_MP3) ||
                       (aFmt == PVMF_MIME_WMA)
                      );
    }

    return isSupported;
}

////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void PVMFOMXDecPort::FormatUpdated()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG, iLogger, PVLOGMSG_INFO
                    , (0, "%s::FormatUpdated %s", PortName(), iFormat.getMIMEStrPtr()));
}

OSCL_EXPORT_REF bool
PVMFOMXDecPort::releaseParametersSync(PvmiKvp*& aParameters, int& aNumParamElements)
{
    if ((iConnectedPort) && (iTag == PVMF_OMX_DEC_NODE_PORT_TYPE_OUTPUT))
    {
        OsclAny* temp = NULL;
        iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);

        PvmiCapabilityAndConfig *config = (PvmiCapabilityAndConfig*)temp;

        if (config != NULL)
        {
            config->releaseParameters(NULL, aParameters, aNumParamElements);

        }
        return true;
    }
    return false;
}

OSCL_EXPORT_REF bool
PVMFOMXDecPort::pvmiGetBufferAllocatorSpecificInfoSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements)
{
    if ((iConnectedPort) && (iTag == PVMF_OMX_DEC_NODE_PORT_TYPE_OUTPUT))
    {
        OsclAny* temp = NULL;
        iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);

        PvmiCapabilityAndConfig *config = (PvmiCapabilityAndConfig*)temp;

        if (config != NULL)
        {
            PVMFStatus status =
                config->getParametersSync(NULL, (PvmiKeyType)aIdentifier, aParameters, aNumParamElements, NULL);

            if (PVMFSuccess == status)
            {
                // config->releaseParameters(NULL, aParameters, aNumParamElements);
            }
        }
        return true;
    }
    return false;
}

OSCL_EXPORT_REF bool
PVMFOMXDecPort::pvmiSetPortFormatSpecificInfoSync(OsclRefCounterMemFrag& aMemFrag, PvmiKeyType KvpKey)
{
    if ((iConnectedPort) &&
            (iTag == PVMF_OMX_DEC_NODE_PORT_TYPE_OUTPUT))
    {
        OsclAny* temp = NULL;
        iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);

        PvmiCapabilityAndConfig *config = (PvmiCapabilityAndConfig*)temp;

        /*
         * Create PvmiKvp for capability settings
         */
        if ((config) && (aMemFrag.getMemFragSize() > 0))
        {
            PvmiKvp kvp;

            kvp.length = oscl_strlen(KvpKey) + 1; // +1 for \0
            kvp.key = KvpKey;
            if (kvp.key == NULL)
            {
                return false;
            }

            kvp.value.key_specific_value = (OsclAny*)(aMemFrag.getMemFragPtr());
            kvp.capacity = aMemFrag.getMemFragSize();
            PvmiKvp* retKvp = NULL; // for return value
            int32 err;
            OSCL_TRY(err, config->setParametersSync(NULL, &kvp, 1, retKvp););
            OSCL_FIRST_CATCH_ANY(err, OSCL_LEAVE(-1));
        }
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PVMFStatus PVMFOMXDecPort::Connect(PVMFPortInterface* aPort)
{
    if (!aPort)
    {
        return PVMFErrArgument;
    }

    if (iConnectedPort)
    {
        return PVMFFailure;
    }

    OsclAny* temp;
    aPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    PvmiCapabilityAndConfig *config = (PvmiCapabilityAndConfig*)temp;

    if (config != NULL)
    {
        PvmiKvp* kvp;
        int numKvp = 0;
        PVMFStatus status =
            config->getParametersSync(NULL, (PvmiKeyType)PVMF_FORMAT_SPECIFIC_INFO_KEY, kvp, numKvp, NULL);
        if (status == PVMFSuccess)
        {
            if (iTrackConfig != NULL)
            {
                OSCL_FREE(iTrackConfig);
                iTrackConfigSize = 0;
            }
            if (kvp)
            {
                iTrackConfigSize = kvp->capacity;
                iTrackConfig = (uint8*)(OSCL_MALLOC(sizeof(uint8) * iTrackConfigSize));
                oscl_memcpy(iTrackConfig, kvp->value.key_specific_value, iTrackConfigSize);
            }
            config->releaseParameters(NULL, kvp, numKvp);
        }

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
OSCL_EXPORT_REF void PVMFOMXDecPort::setParametersSync(PvmiMIOSession aSession,
        PvmiKvp* aParameters,
        int num_elements,
        PvmiKvp * & aRet_kvp)
{

    // if port connect needs format specific info
    if (aParameters && pv_mime_strcmp(aParameters->key, PVMF_FORMAT_SPECIFIC_INFO_KEY) == 0)
    {
        if (iTrackConfig != NULL)
        {
            OSCL_FREE(iTrackConfig);
            iTrackConfigSize = 0;
        }
        iTrackConfigSize = aParameters->capacity;
        iTrackConfig = (uint8*)(OSCL_MALLOC(sizeof(uint8) * iTrackConfigSize));
        oscl_memcpy(iTrackConfig, aParameters->value.key_specific_value, iTrackConfigSize);
        return;
    }
    // call the base class function
    PvmiCapabilityAndConfigPortFormatImpl::setParametersSync(aSession, aParameters, num_elements, aRet_kvp);

}




OSCL_EXPORT_REF PVMFStatus PVMFOMXDecPort::verifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements)
{

    OSCL_UNUSED_ARG(aSession);



    // in case of WMV or WMA call the Node verify to make sure that the parameters are OK
    // (the node will also make sure that the downstream node is OK with the change
    // if necessary)
    if (iFormat == PVMF_MIME_WMV || iFormat == PVMF_MIME_WMA)
    {
        if (iOMXNode->VerifyParametersSync(aSession, aParameters, num_elements))
        {
            return PVMFSuccess;
        }
        else
        {
            return PVMFErrNotSupported;
        }
    }
    else
    {
        return PVMFSuccess;
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFOMXDecPort::verifyConnectedPortParametersSync(const char* aFormatValType,
        OsclAny* aConfig)
{
    PVMFStatus status = PVMFErrNotSupported;
    OsclAny* temp;
    if (iConnectedPort)
    {
        iConnectedPort->QueryInterface(PVMI_CAPABILITY_AND_CONFIG_PVUUID, temp);
    }
    else
        return PVMFFailure;

    PvmiCapabilityAndConfig *capConfig = (PvmiCapabilityAndConfig*)temp;

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




