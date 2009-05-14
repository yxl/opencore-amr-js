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
#include "h324utils.h"
#include "oscl_dll.h"
#include "oscl_map.h"

OSCL_DLL_ENTRY_POINT_DEFAULT()
static const uint32 g_num_ones[] =
{
    /* 0 -  0000 0000 */0, 1, 1, 2, 1, 2, 2, 3,
    /* 8 -  0000 1000 */1, 2, 2, 3, 2, 3, 3, 4,
    /*16 -  0001 0000 */1, 2, 2, 3, 2, 3, 3, 4,
    /*24 -  0001 1000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*32 -  0010 0000 */1, 2, 2, 3, 2, 3, 3, 4,
    /*40 -  0010 1000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*48 -  0011 0000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*56 -  0011 1000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*64 -  0100 0000 */1, 2, 2, 3, 2, 3, 3, 4,
    /*72 -  0100 1000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*80 -  0101 0000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*88 -  0101 1000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*96 -  0110 0000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*104 - 0110 1000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*112 - 0111 0000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*120 - 0111 1000 */4, 5, 5, 6, 5, 6, 6, 7,
    /*128 - 1000 0000 */1, 2, 2, 3, 2, 3, 3, 4,
    /*136 - 1000 1000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*144 - 1001 0000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*152 - 1001 1000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*160 - 1010 0000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*168 - 1010 1000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*176 - 1011 0000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*184 - 1011 1000 */4, 5, 5, 6, 5, 6, 6, 7,
    /*192 - 1100 0000 */2, 3, 3, 4, 3, 4, 4, 5,
    /*200 - 1100 1000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*208 - 1101 0000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*216 - 1101 1000 */4, 5, 5, 6, 5, 6, 6, 7,
    /*224 - 1110 0000 */3, 4, 4, 5, 4, 5, 5, 6,
    /*232 - 1110 1000 */4, 5, 5, 6, 5, 6, 6, 7,
    /*240 - 1111 0000 */4, 5, 5, 6, 5, 6, 6, 7,
    /*248 - 1111 1000 */5, 6, 6, 7, 6, 7, 7, 8
};

PVCodecType_t GetVidCodecTypeFromVideoCapability(PS_VideoCapability capability)
{
    switch (capability->index)
    {
        case 3:
            return PV_VID_TYPE_H263;
        case 5:
        {
            if (capability->genericVideoCapability->capabilityIdentifier.index == 0)
            {
                S_CapabilityIdentifierStandard capabilityIdentifierStandardInfo;
                ParseCapabilityIdentifierStandard(
                    capability->genericVideoCapability->capabilityIdentifier.standard->data,
                    capability->genericVideoCapability->capabilityIdentifier.standard->size,
                    capabilityIdentifierStandardInfo);
                return GetVidCodecTypeFromCapabilityIdentifier(
                           capabilityIdentifierStandardInfo);
            }
        }
        default:
            break;
    }
    return PV_CODEC_TYPE_NONE;
}

PVCodecType_t GetVidCodecTypeFromCapabilityIdentifier(S_CapabilityIdentifierStandard& identifier)
{
    if (identifier.itu != 0 ||
            identifier.spec_type != 0 ||
            identifier.series_letter != 8)
        return PV_CODEC_TYPE_NONE;
    if (identifier.spec_number == 245)
    {
        if (identifier.data[0] == 1 && // generic capabilty
                identifier.data[1] == 0 && // video
                identifier.data[2] == 0) // 14496-2
        {
            return PV_VID_TYPE_MPEG4;
        }
    }
    return PV_CODEC_TYPE_NONE;
}

PVCodecType_t GetAudCodecTypeFrom245Index(int32 index)
{
    switch (index)
    {
        case 8:
            return PV_AUD_TYPE_G723;
        default:
            break;
    }
    return PV_CODEC_TYPE_NONE;
};

PVCodecType_t GetAudCodecType(PS_GenericCapability audio_capability)
{
    S_CapabilityIdentifierStandard capabilityIdentifierStandardInfo;
    ParseCapabilityIdentifierStandard(
        audio_capability->capabilityIdentifier.standard->data,
        audio_capability->capabilityIdentifier.standard->size,
        capabilityIdentifierStandardInfo);
    if (capabilityIdentifierStandardInfo.itu != 0 ||
            capabilityIdentifierStandardInfo.spec_type != 0 ||
            capabilityIdentifierStandardInfo.series_letter != 8)
    {
        return PV_CODEC_TYPE_NONE;
    }
    if (capabilityIdentifierStandardInfo.spec_number != 245 ||
            capabilityIdentifierStandardInfo.data[0] != 1 || // generic capability
            capabilityIdentifierStandardInfo.data[1] != 1 || // AUDIO capability
            capabilityIdentifierStandardInfo.data[2] != 1)  // AMR capability
    {
        return PV_CODEC_TYPE_NONE;
    }
    return PV_AUD_TYPE_GSM;
};

PVCodecType_t GetAudCodecType(PS_AudioCapability audio_capability)
{
    if (audio_capability->index == 20)
    {
        return GetAudCodecType(audio_capability->genericAudioCapability);
    }
    return GetAudCodecTypeFrom245Index(audio_capability->index);
}

PVCodecType_t GetUiCodecTypeFrom245Index(int32 index)
{
    switch (index)
    {
        case 1:
            return PV_UI_BASIC_STRING;
        case 2:
            return PV_UI_IA5_STRING;
        case 3:
            return PV_UI_GENERAL_STRING;
        case 4:
            return PV_UI_DTMF;
        default:
            break;
    }
    return PV_CODEC_TYPE_NONE;
}

void GetCodecInfo(PS_Capability capability, CodecCapabilityInfo& info)
{
    info.codec = PV_CODEC_TYPE_NONE;
    switch (capability->index)
    {
        case 1:
            info.dir = INCOMING;
            info.codec = GetVidCodecTypeFromVideoCapability(
                             capability->receiveVideoCapability);
            break;
        case 2:
            info.dir = OUTGOING;
            info.codec = GetVidCodecTypeFromVideoCapability(
                             capability->transmitVideoCapability);
            break;
        case 3:
            info.dir = PV_DIRECTION_BOTH;
            info.codec = GetVidCodecTypeFromVideoCapability(
                             capability->receiveAndTransmitVideoCapability);
            break;
        case 4:
            info.dir = INCOMING;
            info.codec = GetAudCodecType(capability->receiveAudioCapability);
            break;
        case 5:
            info.dir = OUTGOING;
            info.codec = GetAudCodecType(capability->transmitAudioCapability);
            break;
        case 6:
            info.dir = PV_DIRECTION_BOTH;
            info.codec = GetAudCodecType(
                             capability->receiveAndTransmitAudioCapability);
            break;
        case 15:
            info.dir = INCOMING;
            info.codec = GetUiCodecTypeFrom245Index(
                             capability->receiveUserInputCapability->index);
            break;
        case 16:
            info.dir = OUTGOING;
            info.codec = GetUiCodecTypeFrom245Index(
                             capability->transmitUserInputCapability->index);
            break;
        case 17:
            info.dir = PV_DIRECTION_BOTH;
            info.codec = GetUiCodecTypeFrom245Index(
                             capability->receiveAndTransmitUserInputCapability->index);
            break;
        default:
            break;
    }
}

void FillCapability(CodecCapabilityInfo& codec_info,
                    PS_Capability capability)
{
    PV2WayMediaType media_type = GetMediaType(codec_info.codec);
    switch (media_type)
    {
        case PV_VIDEO:
        {
            PS_VideoCapability video_capability =
                (PS_VideoCapability)OSCL_DEFAULT_MALLOC(sizeof(S_VideoCapability));
            oscl_memset(video_capability, 0, sizeof(S_VideoCapability));

            if (codec_info.dir == INCOMING)
            {
                capability->index = 1;
                capability->receiveVideoCapability = video_capability;
            }
            else if (codec_info.dir == OUTGOING)
            {
                capability->index = 2;
                capability->transmitVideoCapability = video_capability;
            }
            else
            {
                capability->index = 3;
                capability->receiveAndTransmitVideoCapability =
                    video_capability;
            }
            FillVideoCapability((VideoCodecCapabilityInfo&)codec_info,
                                video_capability);
        }
        break;
        case PV_AUDIO:
        {
            PS_AudioCapability audio_capability =
                (PS_AudioCapability)OSCL_DEFAULT_MALLOC(sizeof(S_AudioCapability));
            oscl_memset(audio_capability, 0, sizeof(S_AudioCapability));

            if (codec_info.dir == INCOMING)
            {
                capability->index = 4;
                capability->receiveAudioCapability = audio_capability;
            }
            else if (codec_info.dir == OUTGOING)
            {
                capability->index = 5;
                capability->transmitAudioCapability = audio_capability;
            }
            else
            {
                capability->index = 6;
                capability->receiveAndTransmitAudioCapability = audio_capability;
            }
            FillAudioCapability(codec_info, audio_capability);
        }
        break;
        case PV_USER_INPUT:
        {
            PS_UserInputCapability userinput_capability =
                (PS_UserInputCapability)OSCL_DEFAULT_MALLOC(sizeof(S_UserInputCapability));
            oscl_memset(userinput_capability, 0, sizeof(S_UserInputCapability));
            if (codec_info.dir == INCOMING)
            {
                capability->index = 15;
                capability->receiveUserInputCapability = userinput_capability;
            }
            else if (codec_info.dir == OUTGOING)
            {
                capability->index = 16;
                capability->transmitUserInputCapability = userinput_capability;
            }
            else
            {
                capability->index = 15;
                capability->receiveAndTransmitUserInputCapability = userinput_capability;
            }
            FillUserInputCapability(codec_info, userinput_capability);
        }
        break;
        default:
            break;
    }
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_Capability capability)
{
    CodecCapabilityInfo* ret = NULL;
    TPVDirection dir = PV_DIRECTION_BOTH;
    switch (capability->index)
    {
        case 1:
            ret = GetCodecCapabilityInfo(capability->receiveVideoCapability);
            dir = INCOMING;
            break;
        case 2:
            ret = GetCodecCapabilityInfo(capability->transmitVideoCapability);
            dir = OUTGOING;
            break;
        case 3:
            ret = GetCodecCapabilityInfo(
                      capability->receiveAndTransmitVideoCapability);
            dir = PV_DIRECTION_BOTH;
            break;
        case 4:
            ret = GetCodecCapabilityInfo(capability->receiveAudioCapability);
            dir = INCOMING;
            break;
        case 5:
            ret = GetCodecCapabilityInfo(capability->transmitAudioCapability);
            dir = OUTGOING;
            break;
        case 6:
            ret = GetCodecCapabilityInfo(
                      capability->receiveAndTransmitAudioCapability);
            dir = PV_DIRECTION_BOTH;
            break;
        case 15:
            ret = GetCodecCapabilityInfo(capability->receiveUserInputCapability);
            dir = INCOMING;
            break;
        case 16:
            ret = GetCodecCapabilityInfo(
                      capability->transmitUserInputCapability);
            dir = OUTGOING;
            break;
        case 17:
            ret = GetCodecCapabilityInfo(
                      capability->receiveAndTransmitUserInputCapability);
            dir =  PV_DIRECTION_BOTH;
            break;
        default:
            break;
    }
    if (ret)
    {
        ret->dir = dir;
    }
    return ret;
}


void FillVideoCapability(VideoCodecCapabilityInfo& video_codec_info,
                         PS_VideoCapability video_capability)
{
    switch (video_codec_info.codec)
    {
        case PV_VID_TYPE_H263:
            video_capability->index = 3;
            video_capability->h263VideoCapability =
                (PS_H263VideoCapability)OSCL_DEFAULT_MALLOC(
                    sizeof(S_H263VideoCapability));
            oscl_memset(video_capability->h263VideoCapability,
                        0, sizeof(S_H263VideoCapability));
            FillH263Capability(video_codec_info,
                               video_capability->h263VideoCapability);
            break;
        case PV_VID_TYPE_MPEG4:
            video_capability->index = 5;
            video_capability->genericVideoCapability =
                (PS_GenericCapability)OSCL_DEFAULT_MALLOC(
                    sizeof(S_GenericCapability));
            oscl_memset(video_capability->genericVideoCapability,
                        0, sizeof(S_GenericCapability));
            FillM4vCapability(video_codec_info,
                              video_capability->genericVideoCapability);
            break;
        default:
            break;
    }
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_VideoCapability capability)
{
    switch (capability->index)
    {
        case 3:
            return GetCodecCapabilityInfo(capability->h263VideoCapability);
        case 5:
            return GetCodecCapabilityInfo(capability->genericVideoCapability);
        default:
            break;
    }
    return NULL;
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_GenericCapability capability)
{
    if (capability->capabilityIdentifier.index != 0)
        return NULL;
    S_CapabilityIdentifierStandard capabilityIdentifierStandardInfo;
    ParseCapabilityIdentifierStandard(
        capability->capabilityIdentifier.standard->data,
        capability->capabilityIdentifier.standard->size,
        capabilityIdentifierStandardInfo);
    PVCodecType_t codec_type
    = GetVidCodecTypeFromCapabilityIdentifier(capabilityIdentifierStandardInfo);
    switch (codec_type)
    {
        case PV_VID_TYPE_MPEG4:
            return GetCodecCapabilityInfoMpeg4(capability);
        default:
            break;
    }
    return NULL;
}

void FillM4vCapability(VideoCodecCapabilityInfo& video_codec_info,
                       PS_GenericCapability m4vcaps)
{
    OSCL_UNUSED_ARG(video_codec_info);
    // x15 is a set of 1 GenericParameter(s)
    unsigned num_generic_parameters = 1;
    PS_GenericParameter x15 = (PS_GenericParameter)OSCL_DEFAULT_MALLOC(num_generic_parameters *
                              sizeof(S_GenericParameter));
    oscl_memset(x15, 0, num_generic_parameters * sizeof(S_GenericParameter));

    // x13 is a GenericCapability (SEQUENCE)
    PS_CapabilityIdentifier x14 = &m4vcaps->capabilityIdentifier;
    m4vcaps->option_of_maxBitRate = ON;
    m4vcaps->maxBitRate = 521;
    m4vcaps->option_of_collapsing = OFF;
    m4vcaps->option_of_nonCollapsing = ON;
    m4vcaps->size_of_nonCollapsing = 1;
    m4vcaps->nonCollapsing = x15;
    m4vcaps->option_of_nonCollapsingRaw = OFF;
    m4vcaps->option_of_transport = OFF;

    // x14 is a CapabilityIdentifier (CHOICE)
    x14->index = 0;

    PS_OBJECTIDENT objident = (PS_OBJECTIDENT)OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
    oscl_memset(objident, 0, sizeof(S_OBJECTIDENT));
    x14->standard = objident;
    x14->standard->size = 7;
    x14->standard->data = (uint8*)OSCL_DEFAULT_MALLOC(x14->standard->size);
    x14->standard->data[0] = 0x00;
    x14->standard->data[1] = 0x08;
    x14->standard->data[2] = 0x81;
    x14->standard->data[3] = 0x75;
    x14->standard->data[4] = 0x01;
    x14->standard->data[5] = 0x00;
    x14->standard->data[6] = 0x00;

    // x15[0] is a GenericParameter (SEQUENCE)
    PS_ParameterIdentifier x16 = &x15[0].parameterIdentifier;
    PS_ParameterValue x17 = &x15[0].parameterValue;
    x15[0].option_of_supersedes = OFF;

    // x16 is a ParameterIdentifier (CHOICE)
    x16->index = 0;
    x16->standard = 0;

    // x17 is a ParameterValue (CHOICE)
    x17->index = 3;
    x17->unsignedMax = 8;  // Value changed from 3 to 8.  (RAN - PandL)
}

CodecCapabilityInfo* GetCodecCapabilityInfoMpeg4(PS_GenericCapability mpeg4caps)
{
    VideoCodecCapabilityInfo* cci = new VideoCodecCapabilityInfo();
    cci->codec = PV_VID_TYPE_MPEG4;
    cci->max_bitrate = mpeg4caps->maxBitRate;
    return cci;
}

void FillH263Capability(VideoCodecCapabilityInfo& video_codec_info,
                        PS_H263VideoCapability h263caps)
{
    if (IsResolutionSupported(PVMF_RESOLUTION_SQCIF,
                              video_codec_info.resolutions))
    {
        h263caps->option_of_sqcifMPI = ON;
        h263caps->sqcifMPI = 2;
    }
    if (IsResolutionSupported(PVMF_RESOLUTION_QCIF,
                              video_codec_info.resolutions))
    {
        h263caps->option_of_qcifMPI = ON;
        h263caps->qcifMPI = 2;
    }
    if (IsResolutionSupported(PVMF_RESOLUTION_CIF,
                              video_codec_info.resolutions))
    {
        h263caps->option_of_cifMPI = ON;
        h263caps->cifMPI = 2;
    }
    if (IsResolutionSupported(PVMF_RESOLUTION_4CIF,
                              video_codec_info.resolutions))
    {
        h263caps->option_of_cif4MPI = ON;
        h263caps->cif4MPI = 2;
    }
    if (IsResolutionSupported(PVMF_RESOLUTION_16CIF,
                              video_codec_info.resolutions))
    {
        h263caps->option_of_cif16MPI = ON;
        h263caps->cif16MPI = 2;
    }
    h263caps->maxBitRate = 521;
    h263caps->unrestrictedVector = OFF;
    h263caps->arithmeticCoding = OFF;
    h263caps->advancedPrediction = OFF;
    h263caps->pbFrames = OFF;
    h263caps->temporalSpatialTradeOffCapability = ON;
    h263caps->option_of_hrd_B = OFF;
    h263caps->option_of_bppMaxKb = OFF;
    h263caps->option_of_slowSqcifMPI = OFF;
    h263caps->option_of_slowQcifMPI = OFF;
    h263caps->option_of_slowCifMPI = OFF;
    h263caps->option_of_slowCif4MPI = OFF;
    h263caps->option_of_slowCif16MPI = OFF;
    h263caps->option_of_errorCompensation = ON;
    h263caps->errorCompensation = OFF;
    h263caps->option_of_enhancementLayerInfo = OFF;
    h263caps->option_of_h263Options = OFF;
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_H263VideoCapability h263caps)
{
    VideoCodecCapabilityInfo* cci = new VideoCodecCapabilityInfo();
    cci->codec = PV_VID_TYPE_H263;
    cci->max_bitrate = h263caps->maxBitRate;

    if (h263caps->option_of_sqcifMPI == ON && h263caps->sqcifMPI)
    {
        PVMFVideoResolutionRange range_sqcif(PVMF_RESOLUTION_SQCIF,
                                             PVMF_RESOLUTION_SQCIF);
        cci->resolutions.push_back(range_sqcif);
    }
    if (h263caps->option_of_qcifMPI == ON && h263caps->qcifMPI)
    {
        PVMFVideoResolutionRange range_qcif(PVMF_RESOLUTION_QCIF,
                                            PVMF_RESOLUTION_QCIF);
        cci->resolutions.push_back(range_qcif);
    }
    if (h263caps->option_of_cifMPI == ON && h263caps->cifMPI)
    {
        PVMFVideoResolutionRange range_cif(PVMF_RESOLUTION_CIF,
                                           PVMF_RESOLUTION_CIF);
        cci->resolutions.push_back(range_cif);
    }
    if (h263caps->option_of_cif4MPI == ON && h263caps->cif4MPI)
    {
        PVMFVideoResolutionRange range_4cif(PVMF_RESOLUTION_4CIF,
                                            PVMF_RESOLUTION_4CIF);
        cci->resolutions.push_back(range_4cif);
    }
    if (h263caps->option_of_cif16MPI == ON && h263caps->cif16MPI)
    {
        PVMFVideoResolutionRange range_16cif(PVMF_RESOLUTION_16CIF,
                                             PVMF_RESOLUTION_16CIF);
        cci->resolutions.push_back(range_16cif);
    }
    return cci;
}

void FillAudioCapability(CodecCapabilityInfo& codec_info,
                         PS_AudioCapability audio_capability)
{
    switch (codec_info.codec)
    {
        case PV_AUD_TYPE_GSM:
            audio_capability->index = 20;
            audio_capability->genericAudioCapability =
                (PS_GenericCapability)OSCL_DEFAULT_MALLOC(
                    sizeof(S_GenericCapability));
            FillAmrCapability(audio_capability->genericAudioCapability);
            break;
        case PV_AUD_TYPE_G723:
            audio_capability->index = 8;
            audio_capability->g7231 = (PS_G7231)OSCL_DEFAULT_MALLOC(
                                          sizeof(S_G7231));
            FillG723Capability(audio_capability->g7231);
            break;
        default:
            break;
    }
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_AudioCapability audio_capability)
{
    switch (audio_capability->index)
    {
        case 8:
            return GetCodecCapabilityInfo(audio_capability->g7231);
        case 20:
            return GetCodecCapabilityInfoAmr(
                       audio_capability->genericAudioCapability);
        default:
            break;
    }
    return NULL;
}

void FillG723Capability(PS_G7231 g723caps)
{
    g723caps->maxAl_sduAudioFrames = 1;
    g723caps->silenceSuppression = OFF;
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_G7231 g723caps)
{
    OSCL_UNUSED_ARG(g723caps);
    CodecCapabilityInfo* cci = new CodecCapabilityInfo();
    cci->codec = PV_AUD_TYPE_G723;
    cci->max_bitrate = 6300;
    return cci;
}

void FillAmrCapability(PS_GenericCapability amrCaps)
{
    unsigned sizeof_generic_params = 1;
    PS_GenericParameter x25 =
        (PS_GenericParameter)OSCL_DEFAULT_MALLOC(sizeof_generic_params *
                sizeof(S_GenericParameter));
    oscl_memset(x25, 0, sizeof_generic_params * sizeof(S_GenericParameter));

    // x23 is a GenericCapability (SEQUENCE)
    PS_CapabilityIdentifier x24 = &amrCaps->capabilityIdentifier;
    amrCaps->option_of_maxBitRate = ON;
    amrCaps->maxBitRate = 122;
    amrCaps->option_of_collapsing = ON;
    amrCaps->size_of_collapsing = 1;
    amrCaps->collapsing = x25;
    amrCaps->option_of_nonCollapsing = OFF;
    amrCaps->option_of_nonCollapsingRaw = OFF;
    amrCaps->option_of_transport = OFF;

    // x24 is a CapabilityIdentifier (CHOICE)
    x24->index = 0;
    // ----------- Define protocolID --------------
    PS_OBJECTIDENT objident2 = (PS_OBJECTIDENT)OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
    oscl_memset(objident2, 0, sizeof(S_OBJECTIDENT));

    x24->standard = objident2;
    x24->standard->size = 7;
    x24->standard->data = (uint8*)OSCL_DEFAULT_MALLOC(x24->standard->size);
    x24->standard->data[0] = 0x00;
    x24->standard->data[1] = 0x08;
    x24->standard->data[2] = 0x81;
    x24->standard->data[3] = 0x75;
    x24->standard->data[4] = 0x01;
    x24->standard->data[5] = 0x01;
    x24->standard->data[6] = 0x01;
    // ----------- End protocolID -----------------

    // x25 is a set of 1 GenericParameter(s)

    // x25[0] is a GenericParameter (SEQUENCE)
    PS_ParameterIdentifier x26 = &x25[0].parameterIdentifier;
    PS_ParameterValue x27 = &x25[0].parameterValue;
    x25[0].option_of_supersedes = OFF;

    // x26 is a ParameterIdentifier (CHOICE)
    x26->index = 0;
    x26->standard = 0;

    /* x27 is a ParameterValue (CHOICE) */
    x27->index = 2;
    x27->unsignedMin = 1;
}

CodecCapabilityInfo* GetCodecCapabilityInfoAmr(PS_GenericCapability amrCaps)
{
    CodecCapabilityInfo* cci = new CodecCapabilityInfo();
    cci->codec = PV_AUD_TYPE_GSM;
    cci->max_bitrate = amrCaps->maxBitRate;
    return cci;
}

void FillUserInputCapability(CodecCapabilityInfo& codec_info,
                             PS_UserInputCapability userinputCaps)
{
    switch (codec_info.codec)
    {
        case PV_UI_BASIC_STRING:
            userinputCaps->index = 1;
            break;
        case PV_UI_IA5_STRING:
            userinputCaps->index = 2;
            break;
        case PV_UI_GENERAL_STRING:
            userinputCaps->index = 3;
            break;
        case PV_UI_DTMF:
            userinputCaps->index = 4;
            break;
        default:
            break;
    }
}

CodecCapabilityInfo* GetCodecCapabilityInfo(PS_UserInputCapability uiCaps)
{
    CodecCapabilityInfo* cci = new CodecCapabilityInfo();
    switch (uiCaps->index)
    {
        case 1:
            cci->codec = PV_UI_BASIC_STRING;
            break;
        case 2:
            cci->codec = PV_UI_IA5_STRING;
            break;
        case 3:
            cci->codec = PV_UI_GENERAL_STRING;
            break;
        case 4:
            cci->codec = PV_UI_DTMF;
            break;
    }
    cci->max_bitrate = 0;
    return cci;
}

void ParseCapabilityIdentifierStandard(uint8* octet_string,
                                       uint16 octet_string_len,
                                       S_CapabilityIdentifierStandard& capabilityIdentifier)
{
    oscl_memset(&capabilityIdentifier, 0,
                sizeof(S_CapabilityIdentifierStandard));
    if (octet_string_len < 6)
        return;
    capabilityIdentifier.spec_type = (uint8)(octet_string[0] % 10);
    capabilityIdentifier.itu = (uint8)((octet_string[0] -
                                        capabilityIdentifier.spec_type) / 40);
    capabilityIdentifier.series_letter = octet_string[1];
    capabilityIdentifier.spec_number = 0;
    unsigned pos = 2;
    do
    {
        capabilityIdentifier.spec_number =
            capabilityIdentifier.spec_number << 7 | (octet_string[pos] & 0x7F);
    }
    while ((octet_string[pos++]&0x80) && (pos < octet_string_len));
    unsigned tmp_pos = 0;
    while (pos < octet_string_len)
    {
        capabilityIdentifier.data[tmp_pos++] = octet_string[pos++];
    }
}

bool PVCheckSH(uint8 *ptr, int32 size)
{
    int count = 0;
    int32 i = size - 4;

    if (size < 0)
    {
        return false;
    }
    while (i--)
    {
        if ((count > 1) && (ptr[0] == 0x01) && (ptr[1] & 0xF0) == 0x20)
        {
            return false;
        }

        if (*ptr++)
            count = 0;
        else
            count++;
    }
    return true;  // SH switch to h263
}

PVCodecType_t GetCodecType(PS_DataType pDataType)
{
    PVCodecType_t codecIndex = PV_INVALID_CODEC_TYPE;
    if (!pDataType)
        return PV_CODEC_TYPE_NONE;

    if (pDataType->index == 1)
    {  // null data
        codecIndex = PV_CODEC_TYPE_NONE;  // No codec
    }
    else if (pDataType->index == 2)
    {  // videoData
        if (pDataType->videoData->index == 3)
        {  // H263VideoCapability
            codecIndex = PV_VID_TYPE_H263;
        }
        else if (pDataType->videoData->index == 5)
        {  // GenericVideoCapability
            codecIndex = GetVidCodecTypeFromVideoCapability(pDataType->videoData);
        }
    }
    else if (pDataType->index == 3)
    {  // audioData
        if (pDataType->audioData->index == 8)
        {  // G.723
            codecIndex = PV_AUD_TYPE_G723;
        }
        else if (pDataType->audioData->index == 20)
        {  // GenericAudioCapability
            S_CapabilityIdentifierStandard capabilityIdentifierStandardInfo;
            ParseCapabilityIdentifierStandard(
                pDataType->audioData->genericAudioCapability->capabilityIdentifier.standard->data,
                pDataType->audioData->genericAudioCapability->capabilityIdentifier.standard->size,
                capabilityIdentifierStandardInfo);
            if (capabilityIdentifierStandardInfo.itu != 0 ||
                    capabilityIdentifierStandardInfo.spec_type != 0 ||
                    capabilityIdentifierStandardInfo.series_letter != 8)
            {
                return PV_CODEC_TYPE_NONE;
            }
            if (capabilityIdentifierStandardInfo.spec_number != 245 ||
                    capabilityIdentifierStandardInfo.data[0] != 1 || // generic capability
                    capabilityIdentifierStandardInfo.data[1] != 1 || // AUDIO capability
                    capabilityIdentifierStandardInfo.data[2] != 1)  // AMR capability
            {
                return PV_CODEC_TYPE_NONE;
            }
            codecIndex = PV_AUD_TYPE_GSM;
        }
    }
    return codecIndex;
}

PV2WayMediaType GetMediaType(PS_DataType pDataType)
{
    PV2WayMediaType mediaType = PV_MEDIA_NONE;
    if (!pDataType)
        return mediaType;

    if (pDataType->index == 2)
    {  // videoData
        mediaType = PV_VIDEO;
    }
    else if (pDataType->index == 3)
    {  // audioData
        mediaType = PV_AUDIO;
    }
    return mediaType;
}

// =======================================================
// GetSimpleAudioType()						(RAN-32K)
//
// This routine takes the value from p324->GetAudioType()
//   (an H324AudType_t enum) and maps it onto a
//   simpler, reduced codec type space (an H324AudTypeSimple_t
//   enum).  This is a convenience for the H.245 routines
//   which don't need to know the audio rate.
// =======================================================
PVAudTypeSimple_t GetSimpleAudioType(PVAudType_t audio)
{
    if ((audio >= PV_AUD_TYPE_GSM_475) &&
            (audio <= PV_AUD_TYPE_GSM_122))
    {
        return PV_AUD_TYPE_GSM;
    }
    else if (audio == PV_AUD_TYPE_G723_53 ||
             audio == PV_AUD_TYPE_G723_63)
    {
        return PV_AUD_TYPE_G723;
    }
    else
        return PV_SIMPLE_AUD_TYPE_NONE;
}

ErrorProtectionLevel_t GetEpl(uint16 al_index)
{
    switch (al_index)
    {
        case 1:
        case 2:
            return E_EP_LOW;
        case 3:
        case 4:
            return E_EP_MEDIUM;
        case 5:
            return E_EP_HIGH;
        default:
            return E_EP_LOW;
    }
}

void printBuffer(PVLogger* logger,
                 const uint8* buffer,
                 uint16 len)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                    (0, "printBuffer(%d)", len));

    const uint16 roundto = 10;
    int32 cpylen = len + roundto - (len + roundto) % roundto;
    uint8* cpy = (uint8*)OSCL_DEFAULT_MALLOC(cpylen);
    uint8* cpysave = cpy;
    oscl_memset(cpy, 0, cpylen);
    oscl_memcpy(cpy, buffer, len);
    int loops = cpylen / roundto;
    for (uint16 num = 0; num < loops; ++num)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                        (0,
                         "bytes(%d) %x %x %x %x %x %x %x %x %x %x",
                         num, cpy[0], cpy[1], cpy[2], cpy[3], cpy[4], cpy[5],
                         cpy[6], cpy[7], cpy[8], cpy[9]));
        cpy += roundto;
    }
    OSCL_DEFAULT_FREE(cpysave);
}

unsigned GetFormatSpecificInfo(PS_DataType dataType,
                               uint8*& fsi)
{
    uint32 size = 0;
    PS_GenericParameter parameter_list = NULL, parameter = NULL;
    PS_OCTETSTRING config = NULL;
    unsigned ret = 0;
    fsi = NULL;

    if (!dataType ||
            (dataType->index != 2) ||				// videoData
            (dataType->videoData->index != 5))	// genericVideoCapability
        return ret;

    if (dataType->videoData->genericVideoCapability->option_of_nonCollapsing)
    {
        size =
            dataType->videoData->genericVideoCapability->size_of_nonCollapsing;
        parameter_list =
            dataType->videoData->genericVideoCapability->nonCollapsing;
    }
    else if (dataType->videoData->genericVideoCapability->option_of_collapsing)
    {
        size =
            dataType->videoData->genericVideoCapability->size_of_collapsing;
        parameter_list =
            dataType->videoData->genericVideoCapability->collapsing;
    }
    if (size == 0 || parameter_list == NULL)
        return ret;

    for (uint32 ii = 0; ii < size; ++ii)
    {
        parameter = parameter_list + ii;
        if ((parameter->parameterIdentifier.index == 0) &&	// standard
                (parameter->parameterValue.index == 6)) // OctetString
        {
            config = parameter->parameterValue.octetString;
        }
    }

    bool is_filler_fsi = IsFillerFsi(config->data, config->size);
    if (config != NULL && !is_filler_fsi)
    {		// Found valid decoderConfig
        ret = config->size;
        fsi = config->data;
    }
    return ret;
}

PS_Capability LookupCapability(PS_TerminalCapabilitySet pTcs,
                               uint16 cap_entry_num)
{
    if (!(pTcs->option_of_capabilityTable &&
            pTcs->option_of_capabilityDescriptors))
    {
        return NULL;
    }
    for (unsigned cap_entry = 0;
            cap_entry < pTcs->size_of_capabilityTable;
            ++cap_entry)
    {
        PS_CapabilityTableEntry pCapEntry = pTcs->capabilityTable + cap_entry;
        if (pCapEntry->option_of_capability &&
                pCapEntry->capabilityTableEntryNumber == cap_entry_num)
        {
            return &pCapEntry->capability;
        }
    }
    return NULL;
}

bool IsTransmitOnlyAltCapSet(PS_TerminalCapabilitySet pTcs,
                             PS_AlternativeCapabilitySet pAltCapSet)
{
    for (uint32 ii = 0;ii < pAltCapSet->size; ++ii)
    {
        uint32 entry = pAltCapSet->item[ii];
        PS_Capability pCapability = LookupCapability(pTcs,
                                    OSCL_STATIC_CAST(uint16, entry));
        if (pCapability != NULL &&
                pCapability->index != 2 &&
                pCapability->index != 5)
        {
            return false;
        }
    }
    return true;
}

// =======================================================
// VerifyCodecs()							(RAN-32K)
//
// This routine checks an outgoing audio/video combination
//   against the capabilities of the remote terminal.
// Note:
//   audio, video are 'Simple' tags for outgoing codecs.
//   pTcs  = Pointer to received TerminalCapabilitySet
// The routine returns nonzero if the codec combo is OK.
// =======================================================
PVMFStatus VerifyCodecs(PS_TerminalCapabilitySet pTcs,
                        Oscl_Vector<OlcFormatInfo, OsclMemAllocator> codecs,
                        PVLogger *logger)
{
    PS_CapabilityTableEntry pCapEntry = NULL;
    PS_CapabilityDescriptor pCapDesc = NULL;
    PS_AlternativeCapabilitySet pAltCapSet = NULL;
    unsigned num_codecs = codecs.size();
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                    (0, "VerifyCodecs(%d)", num_codecs));

    if (!(pTcs->option_of_capabilityTable &&
            pTcs->option_of_capabilityDescriptors))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_WARNING,
                        (0,
                         "VerifyCodecs - No capabilities option_of_capabilityTable=%d,option_of_capabilityDescriptors=%d",
                         pTcs->option_of_capabilityTable,
                         pTcs->option_of_capabilityDescriptors));
        return PVMFErrNotSupported;
    }

    Oscl_Map<PVCodecType_t, uint32, OsclMemAllocator> codec_capentry_map;

    // verify if all the codecs are supported individually
    for (unsigned codec_num = 0; codec_num < codecs.size(); ++codec_num)
    {
        bool codec_verified = false;
        OlcFormatInfo& codec_info = codecs[codec_num];
        bool is_symmetric = codec_info.isSymmetric;
        if (codec_info.iCodec == PV_CODEC_TYPE_NONE)
        {
            num_codecs--;
            continue;
        }
        for (unsigned cap_entry = 0;
                cap_entry < pTcs->size_of_capabilityTable;
                ++cap_entry)
        {
            pCapEntry = pTcs->capabilityTable + cap_entry;
            if (pCapEntry->option_of_capability)
            {
                CodecCapabilityInfo cap_info;
                GetCodecInfo(&pCapEntry->capability, cap_info);
                if (((cap_info.dir == INCOMING) ||
                        (cap_info.dir == PV_DIRECTION_BOTH && is_symmetric)) &&
                        (codec_info.iCodec == cap_info.codec))
                {
                    codec_verified = true;
                    codec_capentry_map.insert(
                        Oscl_Map<PVCodecType_t, uint32, OsclMemAllocator>::value_type(
                            codec_info.iCodec, pCapEntry->capabilityTableEntryNumber));
                    break;
                }
            }
        }
        if (!codec_verified)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_WARNING,
                            (0, "VerifyCodecs Codec not present in cap table(%d)",
                             codec_info.iCodec));
            return PVMFErrNotSupported;
        }
    }

    if (num_codecs == 1)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                        (0, "VerifyCodecs - single codec ok.  No combo check reqd"));
        return PVMFSuccess;
    }

    for (uint32 ii = 0; ii < pTcs->size_of_capabilityDescriptors; ++ii)
    {
        pCapDesc = pTcs->capabilityDescriptors + ii;
        OSCL_ASSERT(pCapDesc->size_of_simultaneousCapabilities <=
                    MAX_NUM_SIMULTANEOUS_CAPABILITIES);
        // For each codec, create a bitwise OR of all
        // the simultaneous cap entries that include it
        Oscl_Map<PVCodecType_t, uint32, OsclMemAllocator> sim_caps_for_codec;
        uint32 numValidAltCapSets = 0;
        for (uint32 jj = 0; jj < pCapDesc->size_of_simultaneousCapabilities; ++jj)
        {
            // Does codec exist in this altCapSet?
            pAltCapSet = pCapDesc->simultaneousCapabilities + jj;

            if (IsTransmitOnlyAltCapSet(pTcs, pAltCapSet)) continue;

            for (unsigned codec_num = 0; codec_num < codecs.size(); codec_num++)
            {
                OlcFormatInfo& codec_info = codecs[codec_num];
                if (codec_info.iCodec != PV_CODEC_TYPE_NONE &&
                        CheckAltCapSet(pAltCapSet,
                                       codec_capentry_map[codec_info.iCodec]))
                {
                    uint32 mask = 1 << numValidAltCapSets;
                    uint32 val = 0;
                    Oscl_Map<PVCodecType_t, uint32, OsclMemAllocator>::iterator iter =
                        sim_caps_for_codec.find(codec_info.iCodec);
                    if (iter != sim_caps_for_codec.end())
                        val = (*iter).second;
                    val |= mask;
                    sim_caps_for_codec[codec_info.iCodec] = val;
                }
            }
            numValidAltCapSets++;
        }
        int32 sum = 0;
        Oscl_Map<PVCodecType_t, uint32, OsclMemAllocator>::iterator iter =
            sim_caps_for_codec.begin();
        if (sim_caps_for_codec.size() < num_codecs)
            continue;
        uint16 num_unsupported_codecs = 0;
        while (iter != sim_caps_for_codec.end())
        {
            if ((*iter).second == 0)
                num_unsupported_codecs++;
            sum |= (*iter++).second;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                        (0,
                         "VerifyCodecs TSC verified vs codecs sum(%d) num_codecs(%d),num_unsupported_codecs(%d)",
                         sum, num_codecs, num_unsupported_codecs));
        if (num_unsupported_codecs == 0 && (g_num_ones[sum] >= num_codecs))
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                            (0, "VerifyCodecs TSC verified vs codecs successfully"));
            // Found match
            return PVMFSuccess;
        }
    }

    // COMBINATION failed
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_WARNING,
                    (0, "VerifyCodecs Failed to find a single matching descriptor"));

    return PVMFErrNotSupported;
}

PVMFStatus GetCodecCapInfo(PVCodecType_t codec,
                           PS_TerminalCapabilitySet pTcs,
                           CodecCapabilityInfo& codec_info,
                           PVLogger *logger)
{
    OSCL_UNUSED_ARG(codec);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_STACK_TRACE,
                    (0, "GetCodecCapInfo(%d)", codec));
    PS_CapabilityTableEntry pCapEntry = NULL;

    if (!(pTcs->option_of_capabilityTable && pTcs->option_of_capabilityDescriptors))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_WARNING,
                        (0, "GetCodecCapInfo - Terminal is incapable of anything"));
        return PVMFErrNotSupported;
    }

    for (unsigned cap_entry = 0; cap_entry < pTcs->size_of_capabilityTable; ++cap_entry)
    {
        pCapEntry = pTcs->capabilityTable + cap_entry;
        if (pCapEntry->option_of_capability)
        {
            CodecCapabilityInfo cap_info;
            GetCodecInfo(&pCapEntry->capability, cap_info);
            if (cap_info.codec == codec)
            {
                codec_info = cap_info;
                return PVMFSuccess;
            }
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, logger, PVLOGMSG_WARNING,
                    (0, "GetCodecInfo - codec not suppported"));
    return PVMFErrNotSupported;
}

uint32 CheckAltCapSet(PS_AlternativeCapabilitySet pAltCapSet, uint32 entry)
{
    for (uint ii = 0; ii < pAltCapSet->size; ++ii)
    {
        if (pAltCapSet->item[ii] == entry)
            return true;
    }

    return false;
}


PS_TerminalCapabilitySet GenerateTcs(MultiplexCapabilityInfo& mux_cap_info,
                                     Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& outgoing_codecs,
                                     Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& incoming_codecs)
{
    // === Auto Codeword Generation ===
    PS_TerminalCapabilitySet ret =
        (PS_TerminalCapabilitySet)OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySet));
    oscl_memset(ret, 0, sizeof(S_TerminalCapabilitySet));

    PS_MultiplexCapability x3 = NULL;
    PS_H223Capability x4 =
        (PS_H223Capability)OSCL_DEFAULT_MALLOC(sizeof(S_H223Capability));
    oscl_memset(x4, 0, sizeof(S_H223Capability));

    PS_H223MultiplexTableCapability x5 = NULL;
    PS_MobileOperationTransmitCapability x6 = NULL;

    unsigned sizeof_cap_table = outgoing_codecs.size() + incoming_codecs.size();
    PS_CapabilityTableEntry x7 =
        (PS_CapabilityTableEntry)OSCL_DEFAULT_MALLOC(sizeof(S_CapabilityTableEntry) *
                sizeof_cap_table);
    oscl_memset(x7, 0, sizeof(S_CapabilityTableEntry) * sizeof_cap_table);

    unsigned sizeof_cap_descriptors = 1;
    PS_CapabilityDescriptor x28 =
        (PS_CapabilityDescriptor)OSCL_DEFAULT_MALLOC(sizeof(S_CapabilityDescriptor) *
                sizeof_cap_descriptors);
    oscl_memset(x28, 0, sizeof(S_CapabilityDescriptor) * sizeof_cap_descriptors);

    // ================== //
    // Build the Codeword //
    // ================== //

    // (Top two levels deleted -- RAN) //

    // ret is a TerminalCapabilitySet (SEQUENCE) //
    ret->sequenceNumber = 0;
    // ----------- Define protocolID -------------- //
    ret->protocolIdentifier.size = 6;
    uint8* x2ProtocolIdentifierData = (uint8*)OSCL_DEFAULT_MALLOC(ret->protocolIdentifier.size);
    x2ProtocolIdentifierData[0] = 0x00;
    x2ProtocolIdentifierData[1] = 0x08;
    x2ProtocolIdentifierData[2] = 0x81;
    x2ProtocolIdentifierData[3] = 0x75;
    x2ProtocolIdentifierData[4] = 0x00;
    x2ProtocolIdentifierData[5] = 0x06;
    ret->protocolIdentifier.data = x2ProtocolIdentifierData;

    ret->option_of_multiplexCapability = ON;
    ret->option_of_capabilityTable = ON;
    ret->size_of_capabilityTable = (uint16)sizeof_cap_table;
    ret->capabilityTable = x7;
    ret->option_of_capabilityDescriptors = ON;
    ret->size_of_capabilityDescriptors = (uint16)sizeof_cap_descriptors;
    ret->capabilityDescriptors = x28;

    // x3 is a MultiplexCapability (CHOICE) //
    x3 = &ret->multiplexCapability;
    x3->index = 2;
    x3->h223Capability = x4;

    // x4 is an H223Capability (SEQUENCE) //
    x4->transportWithI_frames = OFF; // transmitting contro msgs within LAPM I-frames //
    x4->videoWithAL1 = (mux_cap_info.iAllowAl1Video ? ON : OFF);
    x4->videoWithAL2 = (mux_cap_info.iAllowAl2Video ? ON : OFF);
    x4->videoWithAL3 = (mux_cap_info.iAllowAl3Video ? ON : OFF);
    x4->audioWithAL1 = (mux_cap_info.iAllowAl1Audio ? ON : OFF);
    x4->audioWithAL2 = (mux_cap_info.iAllowAl2Audio ? ON : OFF);
    x4->audioWithAL3 = (mux_cap_info.iAllowAl3Audio ? ON : OFF);
    x4->dataWithAL1 = OFF;
    x4->dataWithAL2 = OFF;
    x4->dataWithAL3 = OFF;
    x4->maximumAl2SDUSize = (uint16)mux_cap_info.iMaximumAl2SDUSize;
    x4->maximumAl3SDUSize = (uint16)mux_cap_info.iMaximumAl3SDUSize;
    x4->maximumDelayJitter = 10;
    x5 = &x4->h223MultiplexTableCapability;
    x4->option_of_maxMUXPDUSizeCapability = ON;
    x4->maxMUXPDUSizeCapability = ON;
    x4->option_of_nsrpSupport = ON;
    x4->nsrpSupport = ON;
    x4->option_of_mobileOperationTransmitCapability = ON;
    x6 = &x4->mobileOperationTransmitCapability;
    x4->option_of_h223AnnexCCapability = OFF;

    // x5 is an H223MultiplexTableCapability (CHOICE) //
    x5->index = 0;

    // x6 is a MobileOperationTransmitCapability (SEQUENCE) //
    x6->modeChangeCapability = OFF;
    x6->h223AnnexA = OFF;
    x6->h223AnnexADoubleFlag = OFF;
    x6->h223AnnexB = ON;
    x6->h223AnnexBwithHeader = OFF;

    // x7 is a set of  CapabilityTableEntry(s) //
    unsigned entry_num = 1, codec_num = 0;

    Oscl_Map<PV2WayMediaType, Oscl_Vector<unsigned, OsclMemAllocator>*, OsclMemAllocator>
    cap_entries_for_media_type;
    for (codec_num = 0; codec_num < incoming_codecs.size(); ++codec_num)
    {
        FillCapabilityTableEntry(x7 + entry_num - 1, *incoming_codecs[codec_num], entry_num);
        PV2WayMediaType media_type = GetMediaType(incoming_codecs[codec_num]->codec);
        Oscl_Vector<unsigned, OsclMemAllocator>* list = NULL;
        Oscl_Map<PV2WayMediaType, Oscl_Vector<unsigned, OsclMemAllocator>*, OsclMemAllocator>::iterator iter =
            cap_entries_for_media_type.find(media_type);
        if (iter == cap_entries_for_media_type.end())
        {
            list = new Oscl_Vector<unsigned, OsclMemAllocator>();
            cap_entries_for_media_type.insert(
                Oscl_Map < PV2WayMediaType,
                Oscl_Vector<unsigned, OsclMemAllocator>*,
                OsclMemAllocator >::value_type(media_type, list));
        }
        else
        {
            list = (*iter).second;
        }
        list->push_back(entry_num++);
    }

    for (codec_num = 0; codec_num < outgoing_codecs.size(); ++codec_num)
    {
        FillCapabilityTableEntry(x7 + entry_num - 1, *outgoing_codecs[codec_num], entry_num);
        entry_num++;
    }


    // x28 is a set of 1 CapabilityDescriptor(s) //
    // x29 is a set of num_alt_cap_sets AlternativeCapabilitySet(s) //
    unsigned num_alt_cap_sets = cap_entries_for_media_type.size();
    PS_AlternativeCapabilitySet x29 =
        (PS_AlternativeCapabilitySet)OSCL_DEFAULT_MALLOC(num_alt_cap_sets *
                sizeof(S_AlternativeCapabilitySet));
    oscl_memset(x29, 0, num_alt_cap_sets * sizeof(S_AlternativeCapabilitySet));
    Oscl_Map < PV2WayMediaType,
    Oscl_Vector<unsigned, OsclMemAllocator>*, OsclMemAllocator >::iterator iter =
        cap_entries_for_media_type.begin();

    // x28[0] is a CapabilityDescriptor (SEQUENCE) //
    x28[0].capabilityDescriptorNumber = 0;
    x28[0].option_of_simultaneousCapabilities = true;
    x28[0].size_of_simultaneousCapabilities = (uint16)num_alt_cap_sets;
    x28[0].simultaneousCapabilities = x29;

    for (unsigned acsnum = 0; acsnum < num_alt_cap_sets; ++acsnum)
    {
        Oscl_Vector<unsigned, OsclMemAllocator>* alternatives = (*iter++).second;
        x29[acsnum].item = (uint32*)OSCL_DEFAULT_MALLOC(sizeof(uint32) * alternatives->size());
        x29[acsnum].size = (uint16)alternatives->size();
        for (unsigned altnum = 0; altnum < alternatives->size(); ++altnum)
        {
            x29[acsnum].item[altnum] = (*alternatives)[altnum];
        }
        delete alternatives;
    }

    return ret;
}

void FillCapabilityTableEntry(PS_CapabilityTableEntry pCapEntry,
                              CodecCapabilityInfo& codec_info,
                              uint32 entry_num)
{
    pCapEntry->option_of_capability = 1;
    pCapEntry->capabilityTableEntryNumber = (uint16)entry_num;
    PS_Capability capability = &pCapEntry->capability;
    FillCapability(codec_info, capability);
}

unsigned GetMaxFrameRate(PS_DataType pDataType)
{
    if (!pDataType)
        return 0;
    switch (GetCodecType(pDataType))
    {
        case PV_VID_TYPE_H263:
            return GetMaxFrameRate_H263(pDataType->videoData->h263VideoCapability);
        case PV_VID_TYPE_MPEG4:
            return GetMaxFrameRate_M4V(
                       pDataType->videoData->genericVideoCapability);
        case PV_AUD_TYPE_GSM:
            return 50;
        case PV_AUD_TYPE_G723:
            return 34;
        default:
            break;
    }
    return 0;
}

unsigned GetVideoFrameSize(PS_DataType pDataType, bool width)
{
    if (!pDataType)
        return 0;
    switch (GetCodecType(pDataType))
    {
        case PV_VID_TYPE_H263:
            return GetVideoFrameSize_H263(pDataType->videoData->h263VideoCapability, width);
        case PV_VID_TYPE_MPEG4:
            return GetVideoFrameSize_M4V(
                       pDataType->videoData->genericVideoCapability, width);
        case PV_AUD_TYPE_GSM:
            return 0;
        case PV_AUD_TYPE_G723:
            return 0;
        default:
            break;
    }
    return 0;
}


unsigned GetMaxBitrate(PS_DataType pDataType)
{
    if (!pDataType)
        return 0;
    switch (GetCodecType(pDataType))
    {
        case PV_VID_TYPE_H263:
            return pDataType->videoData->h263VideoCapability->maxBitRate * 100;
        case PV_VID_TYPE_MPEG4:
            return pDataType->videoData->genericVideoCapability->maxBitRate * 100;
        case PV_VID_TYPE_H264:
            return pDataType->videoData->genericVideoCapability->maxBitRate * 100;
        case PV_AUD_TYPE_GSM:
            return pDataType->audioData->genericAudioCapability->maxBitRate * 100;
        case PV_AUD_TYPE_G723:
            return 6300;
        default:
            break;
    }
    return 0;
}

unsigned GetSampleInterval(PS_DataType pDataType)
{
    if (!pDataType)
        return 0;
    switch (GetCodecType(pDataType))
    {
        case PV_AUD_TYPE_GSM:
            return 20;
        case PV_AUD_TYPE_G723:
            return 30;
        default:
            break;
    }
    return 0;
}

unsigned GetMaxFrameRate_H263(PS_H263VideoCapability h263caps)
{
    const int MPIMAX = 31;
    unsigned mpi = MPIMAX;
    if (h263caps->option_of_cif16MPI && h263caps->cif16MPI)
    {
        mpi = h263caps->cif16MPI;
    }
    if (h263caps->option_of_cif4MPI && h263caps->cif4MPI)
    {
        mpi = (h263caps->cif4MPI < mpi) ? h263caps->cif4MPI : mpi;
    }
    if (h263caps->option_of_cifMPI && h263caps->cifMPI)
    {
        mpi = (h263caps->cifMPI < mpi) ? h263caps->cifMPI : mpi;
    }
    if (h263caps->option_of_qcifMPI && h263caps->qcifMPI)
    {
        mpi = (h263caps->qcifMPI < mpi) ? h263caps->qcifMPI : mpi;
    }
    return mpi ? (unsigned)(30.0 / mpi) : 0;
}

unsigned GetMaxFrameRate_M4V(PS_GenericCapability m4vcaps)
{
    OSCL_UNUSED_ARG(m4vcaps);
    return 15;
}

unsigned GetVideoFrameSize_H263(PS_H263VideoCapability h263caps, bool width)
{
    unsigned frame_width = 0;
    unsigned frame_height = 0;

    if (h263caps->option_of_cif16MPI && h263caps->cif16MPI)
    {
        //frame_width = 352;
        //frame_height = 288;
    }
    if (h263caps->option_of_cif4MPI && h263caps->cif4MPI)
    {
        //frame_width = 352;
        //frame_height = 288;
    }
    if (h263caps->option_of_cifMPI && h263caps->cifMPI)
    {
        frame_width = 352;
        frame_height = 288;
    }
    if (h263caps->option_of_qcifMPI && h263caps->qcifMPI)
    {
        frame_width = 176;
        frame_height = 144;
    }

    if (width == true)
    {
        return frame_width;
    }
    else
    {
        return frame_height;
    }

}

unsigned GetVideoFrameSize_M4V(PS_GenericCapability m4vcaps , bool width)
{
    OSCL_UNUSED_ARG(m4vcaps);
    if (width == true)
    {
        return 176;
    }
    else
    {
        return 144;
    }
}


bool FindCodecForMediaType(PV2WayMediaType media,
                           Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>&list,
                           int* index)
{
    for (unsigned ii = 0; ii < list.size(); ++ii)
    {
        if (GetMediaType(list[ii]->codec) == media)
        {
            *index = ii;
            return true;
        }
    }
    return false;
}

bool IsSupported(Oscl_Vector<H324ChannelParameters, PVMFTscAlloc>& list,
                 PV2WayMediaType media_type,
                 CodecCapabilityInfo& codec_info,
                 int* index)
{
    for (unsigned nn = 0; nn < list.size(); ++nn)
    {
        H324ChannelParameters& param = list[nn];
        if (param.GetMediaType() != media_type)
            continue;
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* codecs =
            param.GetCodecs();
        if (!codecs)
        {
            return false;
        }
        codec_info.codec = PVMFFormatTypeToPVCodecType((*codecs)[0].format);
        codec_info.dir = (*codecs)[0].dir;
        *index = nn;
        return true;
    }
    return false;
}

bool IsResolutionSupported(const PVMFVideoResolution& resolution,
                           const Oscl_Vector < PVMFVideoResolutionRange,
                           OsclMemAllocator > & resolutions)
{
    for (unsigned ii = 0; ii < resolutions.size(); ++ii)
    {
        if (resolution.height >= resolutions[ii].iFirst.height &&
                resolution.height <= resolutions[ii].iLast.height &&
                resolution.width >= resolutions[ii].iFirst.width &&
                resolution.width <= resolutions[ii].iLast.width)
        {
            return true;
        }
    }
    return false;
}

PS_H223LogicalChannelParameters
GetH223LogicalChannelParameters(uint8 al_index,
                                bool segmentable,
                                uint32 al_specific)
{
    PS_H223LogicalChannelParameters pParameter =
        (PS_H223LogicalChannelParameters)OSCL_DEFAULT_MALLOC(
            sizeof(S_H223LogicalChannelParameters));
    oscl_memset(pParameter , 0, sizeof(S_H223LogicalChannelParameters));

    pParameter->segmentableFlag = segmentable;
    pParameter->adaptationLayerType.index = (uint16)al_index;
    if (pParameter->adaptationLayerType.index == 5)
    {
        pParameter->adaptationLayerType.al3 = (PS_Al3) OSCL_DEFAULT_MALLOC(sizeof(S_Al3));
        oscl_memset(pParameter->adaptationLayerType.al3, 0, sizeof(S_Al3));
        pParameter->adaptationLayerType.al3->controlFieldOctets = (uint8)al_specific;
        pParameter->adaptationLayerType.al3->sendBufferSize = DEF_AL3_SEND_BUFFER_SIZE;
    }
    return pParameter;
}

uint16 GetCodecCapabilityInfo(PS_TerminalCapabilitySet pTcs,
                              PS_AlternativeCapabilitySet pAcs,
                              Oscl_Vector < CodecCapabilityInfo*,
                              OsclMemAllocator > & in_codecs_acs)
{
    uint16 num_media_types = 0;
    PV2WayMediaType cur_media_type = PV_MEDIA_NONE;
    for (int ll = 0; ll < pAcs->size; ++ll)
    {
        PS_Capability capability = LookupCapability(pTcs,
                                   (uint16)pAcs->item[ll]);
        if (!capability)
        {
            continue;
        }
        CodecCapabilityInfo* cci = GetCodecCapabilityInfo(capability);
        if (cci == NULL)
        {
            continue;
        }
        PV2WayMediaType mediaType = GetMediaType(cci->codec);
        if (mediaType == PV_MEDIA_NONE)
        {
            delete cci;
        }
        else
        {
            if (mediaType != cur_media_type)
            {
                num_media_types++;
                cur_media_type = mediaType;
            }
            in_codecs_acs.push_back(cci);
        }
    }
    return num_media_types;
}

void Deallocate(Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& cci_list)
{
    for (unsigned ii = 0; ii < cci_list.size(); ++ii)
    {
        delete cci_list[ii];
    }
    cci_list.clear();
}

CodecCapabilityInfo* IsSupported(CodecCapabilityInfo* codecInfo,
                                 Oscl_Vector<CodecCapabilityInfo*, OsclMemAllocator>& list)
{
    for (unsigned ii = 0; ii < list.size(); ++ii)
    {
        if (list[ii]->codec == codecInfo->codec)
            return list[ii];
    }
    return NULL;
}

PVMFStatus SetFormatSpecificInfo(PS_DataType pDataType, uint8* fsi, uint32 fsi_len)
{
    PVCodecType_t codec_type = GetCodecType(pDataType);
    if (codec_type != PV_VID_TYPE_MPEG4)
    {
        return PVMFFailure;
    }
    PS_OCTETSTRING octet_string = pDataType->videoData->genericVideoCapability->nonCollapsing[2].parameterValue.octetString;
    OSCL_ASSERT(octet_string != NULL);
    if (octet_string->data)
    {
        OSCL_DEFAULT_FREE(octet_string->data);
        octet_string->data = NULL;
        octet_string->size = 0;
    }
    octet_string->data = (uint8*)OSCL_DEFAULT_MALLOC(fsi_len);
    oscl_memcpy(octet_string->data,
                fsi,
                fsi_len);
    octet_string->size = fsi_len;
    return PVMFSuccess;
}

uint32 SetFillerFsi(uint8* dest, uint32 dest_len)
{
    if (dest_len != PV2WAY_FILLER_FSI_LEN)
        return 0;
    uint8 buf[PV2WAY_FILLER_FSI_LEN] = PV2WAY_FILLER_FSI;
    oscl_memcpy(dest, buf, dest_len);
    return dest_len;
}

bool IsFillerFsi(uint8* fsi, uint32 fsi_len)
{
    if (fsi_len != PV2WAY_FILLER_FSI_LEN)
        return false;
    uint8 buf[PV2WAY_FILLER_FSI_LEN] = PV2WAY_FILLER_FSI;
    if (oscl_memcmp(fsi, buf, fsi_len) == 0)
        return true;
    return false;
}


