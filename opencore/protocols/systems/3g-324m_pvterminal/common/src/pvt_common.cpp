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
#include "oscl_base.h"
#include "oscl_mem.h"

#include "pvt_common.h"
#include "pvt_params.h"


/* CPVMediaParam */
OSCL_EXPORT_REF CPVMediaParam::CPVMediaParam(PVCodecType_t aCodecType)
        : iCodecType(aCodecType)
{
}

OSCL_EXPORT_REF CPVMediaParam::~CPVMediaParam()
{

}

OSCL_EXPORT_REF PVCodecType_t CPVMediaParam::GetCodecType()
{
    return iCodecType;
}



/* CPVAudioParam */
OSCL_EXPORT_REF CPVParam* CPVAudioParam::Copy()
{
    return new CPVAudioParam(GetCodecType());
}

OSCL_EXPORT_REF PV2WayMediaType CPVAudioParam::GetMediaType()
{
    return PV_AUDIO;
}

CPVAudioParam::CPVAudioParam(PVCodecType_t aCodecType) : CPVMediaParam(aCodecType)
{
}

/* CPVVideoParam */
OSCL_EXPORT_REF CPVVideoParam::~CPVVideoParam()
{
}

OSCL_EXPORT_REF PV2WayMediaType CPVVideoParam::GetMediaType()
{
    return PV_VIDEO;
}

OSCL_EXPORT_REF CPVParam* CPVVideoParam::Copy()
{
    return new CPVVideoParam(iWidth, iHeight, GetCodecType());
}

OSCL_EXPORT_REF uint16 CPVVideoParam::GetWidth()
{
    return iWidth;
}

OSCL_EXPORT_REF uint16 CPVVideoParam::GetHeight()
{
    return iHeight;
}

CPVVideoParam::CPVVideoParam(uint16 aWidth, uint16 aHeight, PVCodecType_t aCodecType)
        : CPVMediaParam(aCodecType), iWidth(aWidth), iHeight(aHeight)
{
}


OSCL_EXPORT_REF CPVM4vVideoParam::CPVM4vVideoParam(uint16 w, uint16 h, uint16 sz, uint8 *cfg)
        : CPVVideoParam(w, h, PV_VID_TYPE_MPEG4), iSz(sz), iCfg(NULL)
{
    if (iSz)
    {
        iCfg = (uint8 *)OSCL_DEFAULT_MALLOC(iSz);
        oscl_memcpy(iCfg, cfg, iSz);
    }
}

OSCL_EXPORT_REF CPVM4vVideoParam::~CPVM4vVideoParam()
{
    if (iCfg)
    {
        OSCL_DEFAULT_FREE(iCfg);
        iCfg = NULL;
    }
}

OSCL_EXPORT_REF uint16 CPVM4vVideoParam::GetDecoderConfigSize()
{
    return iSz;
}

OSCL_EXPORT_REF uint8 *CPVM4vVideoParam::GetDecoderConfig()
{
    return iCfg;
}

OSCL_EXPORT_REF OsclAny CPVM4vVideoParam::Set(uint16 config_sz, uint8* cfg)
{
    iSz = config_sz;
    if (iCfg)
    {
        OSCL_DEFAULT_FREE(iCfg);
        iCfg = NULL;
    }

    if (iSz)
    {
        iCfg = (uint8 *)OSCL_DEFAULT_MALLOC(iSz);
        oscl_memcpy(iCfg, cfg, iSz);
    }
}

OSCL_EXPORT_REF CPVParam* CPVM4vVideoParam::Copy()
{
    return new CPVM4vVideoParam(GetWidth(), GetHeight(), iSz, iCfg);
}

/* CPVAMRAudioParam */
OSCL_EXPORT_REF CPVAMRAudioParam::CPVAMRAudioParam() : CPVAudioParam(PV_AUD_TYPE_GSM)
{
}

OSCL_EXPORT_REF CPVAMRAudioParam::~CPVAMRAudioParam()
{
}

/* CPVG723AudioParam */
OSCL_EXPORT_REF CPVG723AudioParam::CPVG723AudioParam() : CPVAudioParam(PV_AUD_TYPE_G723)
{
}

OSCL_EXPORT_REF CPVG723AudioParam::~CPVG723AudioParam()
{

}

/* CPVH263VideoParam */
OSCL_EXPORT_REF CPVH263VideoParam::CPVH263VideoParam(uint16 w, uint16 h) : CPVVideoParam(w, h, PV_VID_TYPE_H263)
{
}

OSCL_EXPORT_REF CPVH263VideoParam::~CPVH263VideoParam()
{

}



/* CPVTrackInfo */
OSCL_EXPORT_REF bool operator==(CPVTrackInfo &a, CPVTrackInfo &b)
{
    return ((a.GetDirection() == b.GetDirection()) && (a.GetChannelId() == b.GetChannelId()));
}

/* CPVUserInputDtmf */
OSCL_EXPORT_REF CPVUserInputDtmf::CPVUserInputDtmf(uint8 input, bool update, uint16 duration)
{
    iInput = input;
    iIsUpdate = update;
    iDuration = duration;
}

OSCL_EXPORT_REF TPVUserInputType CPVUserInputDtmf::GetType()
{
    return EDtmf;
}

OSCL_EXPORT_REF uint8 CPVUserInputDtmf::GetInput()
{
    return iInput;
}

OSCL_EXPORT_REF bool CPVUserInputDtmf::IsUpdate()
{
    return iIsUpdate;
}

OSCL_EXPORT_REF uint16 CPVUserInputDtmf::GetDuration()
{
    return iDuration;
}
OSCL_EXPORT_REF CPVUserInput* CPVUserInputDtmf::Copy()
{
    return new CPVUserInputDtmf(iInput, iIsUpdate, iDuration);
}

/* CPVUserInputAlphanumeric */
OSCL_EXPORT_REF CPVUserInputAlphanumeric::CPVUserInputAlphanumeric(uint8* input, uint16 len): iInput(NULL), iLength(len)
{
    if ((len > 0) && (input != NULL))
    {
        iInput = (uint8*) OSCL_DEFAULT_MALLOC(len);
        oscl_memcpy(iInput, input, len);
    }
}

OSCL_EXPORT_REF CPVUserInputAlphanumeric::~CPVUserInputAlphanumeric()
{
    if (iInput)
    {
        OSCL_DEFAULT_FREE(iInput);
    }
}

OSCL_EXPORT_REF TPVUserInputType CPVUserInputAlphanumeric::GetType()
{
    return EAlphanumeric ;
}

OSCL_EXPORT_REF uint8* CPVUserInputAlphanumeric::GetInput()
{
    return iInput;
}

OSCL_EXPORT_REF uint16 CPVUserInputAlphanumeric::GetLength()
{
    return iLength;
}
OSCL_EXPORT_REF CPVUserInput* CPVUserInputAlphanumeric::Copy()
{
    return new CPVUserInputAlphanumeric(iInput, iLength);
}

/* TPVH245VendorObjectIdentifier */
OSCL_EXPORT_REF TPVH245VendorObjectIdentifier::TPVH245VendorObjectIdentifier(uint8* vendor, uint16 vendorLength)
        : iVendor(NULL), iVendorLength(vendorLength)
{
    iVendor = (uint8*)OSCL_DEFAULT_MALLOC(iVendorLength);
    oscl_memcpy(iVendor, vendor, iVendorLength);
}

OSCL_EXPORT_REF TPVH245VendorObjectIdentifier::~TPVH245VendorObjectIdentifier()
{
    if (iVendor)
    {
        OSCL_DEFAULT_FREE(iVendor);
    }
}

OSCL_EXPORT_REF TPVH245VendorType TPVH245VendorObjectIdentifier::GetVendorType()
{
    return EObjectIdentifier;
}

OSCL_EXPORT_REF uint8* TPVH245VendorObjectIdentifier::GetVendor(uint16* length)
{
    *length = iVendorLength;
    return iVendor;
}

OSCL_EXPORT_REF TPVH245Vendor* TPVH245VendorObjectIdentifier::Copy()
{
    return OSCL_NEW(TPVH245VendorObjectIdentifier, (iVendor, iVendorLength));
}


/* TPVVendorH221NonStandard */
OSCL_EXPORT_REF TPVVendorH221NonStandard::TPVVendorH221NonStandard(uint8 t35countryCode, uint8 t35extension, uint32 manufacturerCode):
        iT35CountryCode(t35countryCode), iT35Extension(t35extension), iManufacturerCode(manufacturerCode)
{
}

OSCL_EXPORT_REF TPVVendorH221NonStandard::~TPVVendorH221NonStandard()
{
}

OSCL_EXPORT_REF TPVH245VendorType TPVVendorH221NonStandard::GetVendorType()
{
    return EH221NonStandard;
}

OSCL_EXPORT_REF TPVH245Vendor* TPVVendorH221NonStandard::Copy()
{
    return OSCL_NEW(TPVVendorH221NonStandard, (iT35CountryCode, iT35Extension, iManufacturerCode));
}

OSCL_EXPORT_REF uint8 TPVVendorH221NonStandard::GetT35CountryCode()
{
    return iT35CountryCode;
}

OSCL_EXPORT_REF uint8 TPVVendorH221NonStandard::GetT35Extension()
{
    return iT35Extension;
}

OSCL_EXPORT_REF uint32 TPVVendorH221NonStandard::GetManufacturerCode()
{
    return iManufacturerCode;
}

/* TPVVendorIdentification */
OSCL_EXPORT_REF TPVVendorIdentification::TPVVendorIdentification() : iVendor(NULL), iProductNumber(NULL), iProductNumberLen(0),
        iVersionNumber(NULL), iVersionNumberLen(0)
{
}

OSCL_EXPORT_REF TPVVendorIdentification::TPVVendorIdentification(TPVH245Vendor* vendor,
        uint8* pn, uint16 pn_len,
        uint8* vn, uint16 vn_len)
{
    if (vendor)
    {
        iVendor = vendor->Copy();
    }
    if (pn_len)
    {
        iProductNumber = (uint8*)OSCL_DEFAULT_MALLOC(pn_len);
        oscl_memcpy(iProductNumber, pn, pn_len);
        iProductNumberLen = pn_len;
    }
    if (vn_len)
    {
        iVersionNumber = (uint8*)OSCL_DEFAULT_MALLOC(vn_len);
        oscl_memcpy(iVersionNumber, vn, vn_len);
        iVersionNumberLen = vn_len;
    }
}

OSCL_EXPORT_REF TPVVendorIdentification::~TPVVendorIdentification()
{
    if (iVendor)
    {
        OSCL_DELETE(iVendor);
    }
    if (iProductNumber)
    {
        OSCL_DEFAULT_FREE(iProductNumber);
    }
    if (iVersionNumber)
    {
        OSCL_DEFAULT_FREE(iVersionNumber);
    }
}


/* CPvtDiagnosticIndication */
OSCL_EXPORT_REF CPvtDiagnosticIndication::CPvtDiagnosticIndication(TPVTerminalEvent aEvent, int aParam1, int aParam2, int aParam3)
        : iEvent(aEvent), iParam1(aParam1), iParam2(aParam2), iParam3(aParam3)
{
}

OSCL_EXPORT_REF CPvtDiagnosticIndication::~CPvtDiagnosticIndication()
{
}

/* CPVGenericMuxParam */
OSCL_EXPORT_REF CPVGenericMuxParam::CPVGenericMuxParam(TPVMuxType aMuxType):
        discard_corrupt_video(PARAM_DEFAULT),
        max_discard_video_sdu_size(0),
        err_rate_threshold_to_req_I_frame(0),
        audio_err_rate_update_interval(0),
        audio_encode_frame_delay(0),
        audio_decode_frame_delay(0),
        iMuxType(aMuxType)
{
}

OSCL_EXPORT_REF TPVMuxType CPVGenericMuxParam::GetMuxType()
{
    return iMuxType;
}

OSCL_EXPORT_REF CPVParam* CPVGenericMuxParam::Copy()
{
    CPVGenericMuxParam* ret = new CPVGenericMuxParam(iMuxType);
    ret->discard_corrupt_video = discard_corrupt_video;
    ret->max_discard_video_sdu_size = max_discard_video_sdu_size;
    ret->err_rate_threshold_to_req_I_frame = err_rate_threshold_to_req_I_frame;
    ret->audio_err_rate_update_interval = audio_err_rate_update_interval;
    ret->audio_encode_frame_delay = audio_encode_frame_delay;
    ret->audio_decode_frame_delay = audio_decode_frame_delay;
    return ret;
}

OSCL_EXPORT_REF OsclAny CPVGenericMuxParam::Copy(CPVGenericMuxParam* param)
{
    param->discard_corrupt_video = discard_corrupt_video;
    param->max_discard_video_sdu_size = max_discard_video_sdu_size;
    param->err_rate_threshold_to_req_I_frame = err_rate_threshold_to_req_I_frame;
    param->audio_err_rate_update_interval = audio_err_rate_update_interval;
    param->audio_encode_frame_delay = audio_encode_frame_delay;
    param->audio_decode_frame_delay = audio_decode_frame_delay;
    return;
}


/* CPVH223MuxParam */
OSCL_EXPORT_REF CPVH223MuxParam::CPVH223MuxParam() : CPVGenericMuxParam(MUX_H223),
        iBitrate(PVT_NOT_SET),
        iLevel(H223_LEVEL_UNKNOWN),
        iMaxAl1SduSize(PVT_NOT_SET),
        iMaxAl2SduSize(PVT_NOT_SET),
        iMaxAl3SduSize(PVT_NOT_SET),
        iMaxAl1SduSizeR(PVT_NOT_SET),
        iMaxAl2SduSizeR(PVT_NOT_SET),
        iMaxAl3SduSizeR(PVT_NOT_SET),
        iParseOnResyncMarkers(false),
        iOutgoingPduType(H223_PDU_COMBINED),
        iMaxPduSize(0),
        iIdleSyncType(H223_IDLE_SYNC_NONE),
        iIdleSyncByte(0x00)
{
}

OSCL_EXPORT_REF CPVH223MuxParam::~CPVH223MuxParam()
{
}

OSCL_EXPORT_REF CPVParam* CPVH223MuxParam::Copy()
{
    CPVH223MuxParam* ret = new CPVH223MuxParam();
    CPVGenericMuxParam::Copy(ret);
    ret->iBitrate = iBitrate;
    ret->iLevel = iLevel;
    ret->iMaxAl1SduSize = iMaxAl1SduSize;
    ret->iMaxAl2SduSize = iMaxAl2SduSize;
    ret->iMaxAl3SduSize = iMaxAl3SduSize;
    ret->iMaxAl1SduSizeR = iMaxAl1SduSizeR;
    ret->iMaxAl2SduSizeR = iMaxAl2SduSizeR;
    ret->iMaxAl3SduSizeR = iMaxAl3SduSizeR;
    ret->iParseOnResyncMarkers = iParseOnResyncMarkers;
    ret->iOutgoingPduType = iOutgoingPduType;
    ret->iMaxPduSize = iMaxPduSize;
    ret->iIdleSyncType = iIdleSyncType;
    ret->iIdleSyncByte = iIdleSyncByte;
    return ret;
}

/* CPVH245Param */
OSCL_EXPORT_REF CPVH245Param::CPVH245Param()
{
}

OSCL_EXPORT_REF CPVParam* CPVH245Param::Copy()
{
    CPVH245Param* ret = new CPVH245Param();
    return ret;
}

/* CPVSrpParam */
OSCL_EXPORT_REF CPVSrpParam::CPVSrpParam()
{

}

OSCL_EXPORT_REF CPVParam* CPVSrpParam::Copy()
{
    CPVSrpParam* ret = new CPVSrpParam();
    return ret;
}

/* CPVTerminalParam */
OSCL_EXPORT_REF CPVTerminalParam::CPVTerminalParam(CPVGenericMuxParam* muxParam): iMuxParam(NULL)
{
    if (muxParam)
    {
        iMuxParam = (CPVGenericMuxParam*)muxParam->Copy();
    }
}

OSCL_EXPORT_REF CPVTerminalParam::CPVTerminalParam(CPVTerminalParam& that): CPVParam(that), iMuxParam(NULL)
{
    if (that.iMuxParam)
    {
        iMuxParam = (CPVGenericMuxParam*)that.iMuxParam->Copy();
    }
}


CPVTerminalParam::~CPVTerminalParam()
{
    if (iMuxParam)
    {
        delete iMuxParam;
    }
}


OSCL_EXPORT_REF OsclAny CPVTerminalParam::SetMuxParam(CPVGenericMuxParam* muxParam)
{
    if (iMuxParam)
    {
        delete iMuxParam;
    }
    iMuxParam = (CPVGenericMuxParam*)muxParam->Copy();
}

OSCL_EXPORT_REF CPVGenericMuxParam* CPVTerminalParam::GetMuxParam()
{
    return iMuxParam;
}

/* CPVH324MParam */
OSCL_EXPORT_REF CPVH324MParam::CPVH324MParam(CPVH223MuxParam* h223param) : CPVTerminalParam(h223param), iH245Param(NULL), iSrpParam(NULL)
{
    iAllowAl1Video = false;
    iAllowAl2Video = true;
    iAllowAl3Video = true;
    iUseAl1Video = true;
    iUseAl2Video = true;
    iUseAl3Video = true;
    iVideoLayer = PVT_AL_UNKNOWN;
    iMasterSlave = PVT_MSD_INDETERMINATE;
    iForceVideoLayerIfMaster = PVT_AL_UNKNOWN;
    iForceVideoLayerIfSlave = PVT_AL_UNKNOWN;
    iSpecifyReceiveAndTransmitCapability = false;
    iSendRme = false;
    iSkipMsd = false;
    iRequestMaxMuxPduSize = 0;
}

OSCL_EXPORT_REF CPVH324MParam::CPVH324MParam(const CPVH324MParam& that) : CPVTerminalParam((CPVTerminalParam&)that)
{
    iAllowAl1Video = that.iAllowAl1Video;
    iAllowAl2Video = that.iAllowAl2Video;
    iAllowAl3Video = that.iAllowAl3Video;
    iUseAl1Video = that.iUseAl1Video;
    iUseAl2Video = that.iUseAl2Video;
    iUseAl3Video = that.iUseAl3Video;
    iVideoLayer = that.iVideoLayer;
    iMasterSlave = that.iMasterSlave;
    iForceVideoLayerIfMaster = that.iForceVideoLayerIfMaster;
    iForceVideoLayerIfSlave = that.iForceVideoLayerIfSlave;
    iSpecifyReceiveAndTransmitCapability = that.iSpecifyReceiveAndTransmitCapability;
    iSendRme = that.iSendRme;
    iSkipMsd = that.iSkipMsd;
    iRequestMaxMuxPduSize = that.iRequestMaxMuxPduSize;
    iH245Param = NULL;
    if (that.iH245Param)
        iH245Param = (CPVH245Param*)that.iH245Param->Copy();
    iSrpParam = NULL;
    if (that.iSrpParam)
        iSrpParam = (CPVSrpParam*)that.iSrpParam->Copy();
}

OSCL_EXPORT_REF CPVH324MParam::~CPVH324MParam()
{

}

OSCL_EXPORT_REF TPVTerminalType CPVH324MParam::GetTerminalType()
{
    return PV_324M;
}

OSCL_EXPORT_REF CPVParam* CPVH324MParam::Copy()
{
    return new CPVH324MParam(*this);
}

OSCL_EXPORT_REF OsclAny CPVH324MParam::SetH223Param(CPVH223MuxParam* h223Param)
{
    SetMuxParam(h223Param);
}

OSCL_EXPORT_REF CPVH223MuxParam* CPVH324MParam::GetH223Param()
{
    return (CPVH223MuxParam*)GetMuxParam();
}

OSCL_EXPORT_REF OsclAny CPVH324MParam::SetSRPParam(CPVSrpParam* srpParam)
{
    iSrpParam = (CPVSrpParam*)srpParam->Copy();
}

OSCL_EXPORT_REF OsclAny CPVH324MParam::SetH245Param(CPVH245Param* h245Param)
{
    iH245Param = (CPVH245Param*)h245Param->Copy();
}

OSCL_EXPORT_REF TPVVideoEncoderParam::TPVVideoEncoderParam():
        video_bitrate(KPVDefaultVideoBitRate),
        codec(PV_VID_TYPE_MPEG4),
        video_frame_rate(KPVDefaultFrameRate),
        air(0),
        intra_refresh(1),
        data_partitioning(1),
        advanced(0),
        use_gov(0),
        rvlc(0),
        use_resync(480),
        use_hec(0),
        use_gobsync(0),
        vos(0),
        ref_frame_rate(15),
        orig_frameskip(1),
        chosen_frame_skip(3),
        qp(10),
        qpi(10),
        cam_low_light_mode(1),
        cam_saturation_level(55),
        cam_contrast_level(90),
        cam_edge_enhance(85),
        cam_brightness(100),
        deblocking_filter(0),
        prefilter(0),
        prefilter_coeff1(0),
        prefilter_coeff2(0),
        prefilter_coeff3(0),
        buffer_backlog(-55000),
        qp_max(31),
        qp_min(1),
        qp_delta_frame(30),
        qp_delta_slice(30),
        iframe_interval(KPVDefaultIFrameInterval),
        iframe_request_interval(KPVDefaultIFrameRequestInterval)
{};


OSCL_EXPORT_REF PVMFFormatType PVCodecTypeToPVMFFormatType(PVCodecType_t aCodecType)
{
    PVMFFormatType aFormatType = PVMF_MIME_FORMAT_UNKNOWN;
    switch (aCodecType)
    {
        case PV_AUD_TYPE_G723:
            aFormatType = PVMF_MIME_G723;
            break;
        case PV_AUD_TYPE_GSM:
            aFormatType = PVMF_MIME_AMR_IF2;
            break;
        case PV_VID_TYPE_H263:
            aFormatType = PVMF_MIME_H2632000;
            break;
        case PV_VID_TYPE_MPEG4:
            aFormatType = PVMF_MIME_M4V;
            break;
        default:
            break;
    }
    return aFormatType;
}

OSCL_EXPORT_REF PVCodecType_t PVMFFormatTypeToPVCodecType(PVMFFormatType aFormatType)
{
    PVCodecType_t aCodecType = PV_CODEC_TYPE_NONE;

    if (aFormatType == PVMF_MIME_G723)
    {
        aCodecType = PV_AUD_TYPE_G723;
    }
    else if (aFormatType == PVMF_MIME_AMR_IF2)
    {
        aCodecType = PV_AUD_TYPE_GSM;
    }
    else if ((aFormatType == PVMF_MIME_H2632000) || (aFormatType == PVMF_MIME_H2631998))
    {
        aCodecType = PV_VID_TYPE_H263;
    }
    else if (aFormatType == PVMF_MIME_M4V)
    {
        aCodecType = PV_VID_TYPE_MPEG4;
    }
    else if (aFormatType == PVMF_MIME_USERINPUT_BASIC_STRING)
    {
        aCodecType = PV_UI_BASIC_STRING;
    }
    else if (aFormatType == PVMF_MIME_USERINPUT_IA5_STRING)
    {
        aCodecType = PV_UI_IA5_STRING;
    }
    else if (aFormatType == PVMF_MIME_USERINPUT_GENERAL_STRING)
    {
        aCodecType = PV_UI_GENERAL_STRING;
    }
    else if (aFormatType == PVMF_MIME_USERINPUT_DTMF)
    {
        aCodecType = PV_UI_DTMF;
    }

    return aCodecType;
}

OSCL_EXPORT_REF PV2WayMediaType PVMFFormatTypeToPVMediaType(PVMFFormatType aFormatType)
{
    PV2WayMediaType aMediaType = PV_MEDIA_NONE;
    if (aFormatType.isAudio())
    {
        aMediaType = PV_AUDIO;
    }
    if (aFormatType.isVideo())
    {
        aMediaType = PV_VIDEO;
    }
    if (aFormatType.isUserInput())
    {
        aMediaType = PV_USER_INPUT;
    }
    return aMediaType;
}


OSCL_EXPORT_REF int IndexForAdaptationLayer(TPVAdaptationLayer al)
{
    int ret = -1;
    switch (al)
    {
        case PVT_AL1:
            ret = 2;
            break;
        case PVT_AL2:
            ret = 3;
            break;
        case PVT_AL3:
            ret = 5;
            break;
        default:
            break;
    }
    return ret;
}

OSCL_EXPORT_REF TPVAdaptationLayer AdaptationLayerForIndex(int al_index)
{
    TPVAdaptationLayer ret = PVT_AL1;
    switch (al_index)
    {
        case 1:
        case 2:
            ret = PVT_AL1;
            break;
        case 3:
        case 4:
            ret = PVT_AL2;
            break;
        case 5:
            ret = PVT_AL3;
            break;
    }
    return ret;
}

OSCL_EXPORT_REF ErrorProtectionLevel_t EplForAdaptationLayer(TPVAdaptationLayer al)
{
    ErrorProtectionLevel_t epl = E_EP_LOW;
    switch (al)
    {
        case PVT_AL1:
            epl = E_EP_LOW;
            break;
        case PVT_AL2:
            epl = E_EP_MEDIUM;
            break;
        case PVT_AL3:
            epl = E_EP_HIGH;
            break;
        default:
            break;
    }
    return epl;
}

OSCL_EXPORT_REF PV2WayMediaType GetMediaType(PVCodecType_t codec)
{
    PV2WayMediaType media_type = PV_MEDIA_NONE;
    switch (codec)
    {
        case PV_AUD_TYPE_G723:
        case PV_AUD_TYPE_GSM:
            media_type = PV_AUDIO;
            break;
        case PV_VID_TYPE_H263:
        case PV_VID_TYPE_MPEG4:
            media_type = PV_VIDEO;
            break;
        case PV_UI_BASIC_STRING:
        case PV_UI_IA5_STRING:
        case PV_UI_GENERAL_STRING:
        case PV_UI_DTMF:
            media_type = PV_USER_INPUT;
            break;
        default:
            break;
    }
    return media_type;
}

OSCL_EXPORT_REF H324ChannelParameters::H324ChannelParameters(TPVDirection dir, PVMFFormatType mediaType, unsigned bandwidth)
        : iCodecs(NULL)
{
    OSCL_UNUSED_ARG(mediaType);
    iBandwidth = bandwidth;
    iCodecs = new Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>();
    FormatCapabilityInfo codec_info;
    codec_info.dir = dir;
}

OSCL_EXPORT_REF H324ChannelParameters::H324ChannelParameters(const H324ChannelParameters& that)
        : iCodecs(NULL)
{
    iBandwidth = that.iBandwidth;

    if (that.iCodecs)
    {
        iCodecs = new Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>(*that.iCodecs);
    }
}

OSCL_EXPORT_REF H324ChannelParameters::~H324ChannelParameters()
{
    if (iCodecs)
        OSCL_DELETE(iCodecs);
}

OSCL_EXPORT_REF PV2WayMediaType H324ChannelParameters::GetMediaType()
{
    if (iCodecs == NULL)
    {
        return PV_MEDIA_NONE;
    }
    return PVMFFormatTypeToPVMediaType((*iCodecs)[0].format);
}

OSCL_EXPORT_REF unsigned H324ChannelParameters::GetBandwidth()
{
    return iBandwidth;
}

OSCL_EXPORT_REF void H324ChannelParameters::SetCodecs(Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& codecs)
{
    if (iCodecs)
    {
        OSCL_DELETE(iCodecs);
        iCodecs = NULL;
    }
    if (!codecs.size())
        return;

    iCodecs = new Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>(codecs);
}

OSCL_EXPORT_REF Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>* H324ChannelParameters::GetCodecs()
{
    return iCodecs;
}

OSCL_EXPORT_REF CodecCapabilityInfo::CodecCapabilityInfo()
        : codec(PV_CODEC_TYPE_NONE),
        dir(PV_DIRECTION_NONE),
        max_bitrate(0),
        min_sample_size(0),
        max_sample_size(0)
{
}

OSCL_EXPORT_REF CodecCapabilityInfo* CodecCapabilityInfo::Copy()
{
    CodecCapabilityInfo* ret = new CodecCapabilityInfo();
    ret->codec = codec;
    ret->dir = dir;
    ret->max_bitrate = max_bitrate;
    return ret;
}

OSCL_EXPORT_REF VideoCodecCapabilityInfo::VideoCodecCapabilityInfo()
        : codec_specific_info(NULL),
        codec_specific_info_len(0)
{
}

OSCL_EXPORT_REF CodecCapabilityInfo* VideoCodecCapabilityInfo::Copy()
{
    VideoCodecCapabilityInfo* ret = new VideoCodecCapabilityInfo();
    ret->codec = codec;
    ret->dir = dir;
    ret->max_bitrate = max_bitrate;
    ret->resolutions = resolutions;
    ret->codec_specific_info_len = codec_specific_info_len;
    ret->codec_specific_info = (uint8*)OSCL_DEFAULT_MALLOC(codec_specific_info_len);
    oscl_memcpy(ret->codec_specific_info, codec_specific_info, codec_specific_info_len);
    return ret;
}

OSCL_EXPORT_REF const char* GetFormatsString(TPVDirection aDir, PV2WayMediaType aMediaType)
{
    switch (aDir)
    {
        case OUTGOING:
            switch (aMediaType)
            {
                case PV_AUDIO:
                    return PV_H324_AUDIO_INPUT_FORMATS;
                case PV_VIDEO:
                    return PV_H324_VIDEO_INPUT_FORMATS;
                case PV_MULTIPLEXED:
                    return PV_H324_MUX_INPUT_FORMATS;
                default:
                    break;
            }
            break;
        case INCOMING:
            switch (aMediaType)
            {
                case PV_AUDIO:
                    return PV_H324_AUDIO_OUTPUT_FORMATS;
                case PV_VIDEO:
                    return PV_H324_VIDEO_OUTPUT_FORMATS;
                case PV_MULTIPLEXED:
                    return PV_H324_MUX_OUTPUT_FORMATS;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return NULL;
}

OSCL_EXPORT_REF const char* GetFormatsValtypeString(TPVDirection aDir, PV2WayMediaType aMediaType)
{
    switch (aDir)
    {
        case OUTGOING:
            switch (aMediaType)
            {
                case PV_AUDIO:
                    return PV_H324_AUDIO_INPUT_FORMATS_VALTYPE;
                case PV_VIDEO:
                    return PV_H324_VIDEO_INPUT_FORMATS_VALTYPE;
                case PV_MULTIPLEXED:
                    return PV_H324_MUX_INPUT_FORMATS_VALTYPE;
                default:
                    break;
            }
            break;
        case INCOMING:
            switch (aMediaType)
            {
                case PV_AUDIO:
                    return PV_H324_AUDIO_OUTPUT_FORMATS_VALTYPE;
                case PV_VIDEO:
                    return PV_H324_VIDEO_OUTPUT_FORMATS_VALTYPE;
                case PV_MULTIPLEXED:
                    return PV_H324_MUX_OUTPUT_FORMATS_VALTYPE;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return NULL;
}

OSCL_EXPORT_REF void GetSampleSize(PVMFFormatType aFormatType, uint32* aMin, uint32* aMax)
{
    *aMin = *aMax = 0;
    if (aFormatType == PVMF_MIME_G723)
    {
        *aMin = 20;
        *aMax = 24;
    }
    else if (aFormatType == PVMF_MIME_AMR_IF2)
    {
        *aMin = 13;
        *aMax = 31;
    }
}


OSCL_EXPORT_REF bool CodecRequiresFsi(PVCodecType_t codec)
{
    bool ret = false;
    switch (codec)
    {
        case PV_VID_TYPE_MPEG4:
            ret = true;
            break;
        default:
            break;
    }
    return ret;
}


