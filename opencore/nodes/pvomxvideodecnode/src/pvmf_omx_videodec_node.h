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
#ifndef PVMF_OMX_VIDEODEC_NODE_H_INCLUDED
#define PVMF_OMX_VIDEODEC_NODE_H_INCLUDED

#ifndef PVMF_VIDEO_H_INCLUDED
#include "pvmf_video.h"
#endif

#ifndef PVMF_OMX_BASEDEC_NODE_H_INCLUDED
#include "pvmf_omx_basedec_node.h"
#endif

#ifndef PVMF_OMX_VIDEODEC_PORT_H_INCLUDED
#include "pvmf_omx_basedec_port.h"
#endif

#define PVMFOMXVIDEODECNODE_NUM_CMD_IN_POOL 8
#define PVOMXVIDEODECMAXNUMDPBFRAMESPLUS1 17

#define PVOMX_VIDEO_DECODER_H264 "video_decoder.avc"
#define PVOMX_VIDEO_DECODER_MPEG4 "video_decoder.mpeg4"
#define PVOMX_VIDEO_DECODER_H263 "video_decoder.h263"
#define PVOMX_VIDEO_DECODER_WMV "video_decoder.wmv"

#define PVOMXVIDEODECNODECONFIG_KEYSTRING_SIZE 128

// Key string info at the base level ("x-pvmf/video/decoder")
#define PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS 6
const PVOMXBaseDecNodeKeyStringData PVOMXVideoDecNodeConfigBaseKeys[PVOMXVIDEODECNODECONFIG_BASE_NUMKEYS] =
{
    {"postproc_enable", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL},
    {"postproc_type", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BITARRAY32},
    {"dropframe_enable", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_BOOL},
    {"h263", PVMI_KVPTYPE_AGGREGATE, PVMI_KVPVALTYPE_KSV},
    {"m4v", PVMI_KVPTYPE_AGGREGATE, PVMI_KVPVALTYPE_KSV},
    {"format-type", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_CHARPTR}
};

// Key string info at the h263 level ("x-pvmf/video/decoder/h263")
#define PVOMXVIDEODECNODECONFIG_H263_NUMKEYS 2
const PVOMXBaseDecNodeKeyStringData PVOMXVideoDecNodeConfigH263Keys[PVOMXVIDEODECNODECONFIG_H263_NUMKEYS] =
{
    {"maxbitstreamframesize", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"maxdimension", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_RANGE_UINT32}
};

// Key string info at the m4v level ("x-pvmf/video/decoder/m4v")
#define PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS 2
const PVOMXBaseDecNodeKeyStringData PVOMXVideoDecNodeConfigM4VKeys[PVOMXVIDEODECNODECONFIG_M4V_NUMKEYS] =
{
    {"maxbitstreamframesize", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"maxdimension", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_RANGE_UINT32}
};

// Key string info at the render level ("x-pvmf/video/render")
#define PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS 2
const PVOMXBaseDecNodeKeyStringData PVOMXVideoDecNodeConfigRenderKeys[PVOMXVIDEODECNODECONFIG_RENDER_NUMKEYS] =
{
    {"width", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32},
    {"height", PVMI_KVPTYPE_VALUE, PVMI_KVPVALTYPE_UINT32}
};


typedef enum
{
    PV_MPEG_VIDEO_SIMPLE_PROFILE = 0,
    PV_MPEG_VIDEO_SIMPLE_SCALABLE_PROFILE,
    PV_MPEG_VIDEO_CORE_PROFILE,
    PV_MPEG_VIDEO_MAIN_PROFILE,
    PV_MPEG_VIDEO_N_BIT_PROFILE,
    PV_MPEG_VIDEO_SCALABLE_TEXTURE_PROFILE,
    PV_MPEG_VIDEO_SIMPLE_FACE_ANIMATION_PROFILE,
    PV_MPEG_VIDEO_SIMPLE_FBA_PROFILE,
    PV_MPEG_VIDEO_BASIC_ANIMATED_TEXTURE_PROFILE,
    PV_MPEG_VIDEO_HYBRID_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_REAL_TIME_SIMPLE_PROFILE,
    PV_MPEG_VIDEO_CORE_SCALABLE_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_CODING_EFFICIENCY_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_CORE_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_SCALABLE_TEXTURE_PROFILE,
    PV_MPEG_VIDEO_SIMPLE_STUDIO_PROFILE,
    PV_MPEG_VIDEO_CORE_STUDIO_PROFILE,
    PV_MPEG_VIDEO_ADVANCE_SIMPLE_PROFILE,
    PV_MPEG_VIDEO_FINE_GRANUALITY_SCALABLE_PROFILE,
    PV_MPEG_VIDEO_RESERVED_PROFILE
}PVMF_MPEGVideoProfileType;

typedef enum
{
    PV_MPEG_VIDEO_LEVEL0 = 0,
    PV_MPEG_VIDEO_LEVEL1,
    PV_MPEG_VIDEO_LEVEL2,
    PV_MPEG_VIDEO_LEVEL3,
    PV_MPEG_VIDEO_LEVEL4,
    PV_MPEG_VIDEO_LEVEL5,
    PV_MPEG_VIDEO_LEVEL_UNKNOWN

}PVMF_MPEGVideoLevelType;

/////////////////////////////////////////////////////////////////////////////////////////





//Mimetypes for the custom interface
#define PVMF_OMX_VIDEO_DEC_NODE_MIMETYPE "pvxxx/OMXVideoDecNode"
#define PVMF_BASEMIMETYPE "pvxxx"

class PVMFOMXVideoDecNode
            : public PVMFOMXBaseDecNode
{
    public:
        PVMFOMXVideoDecNode(int32 aPriority);
        ~PVMFOMXVideoDecNode();

        // From PVMFNodeInterface
        PVMFStatus ThreadLogon();

        //**********begin PVMFMetadataExtensionInterface
        uint32 GetNumMetadataKeys(char* query_key = NULL);
        uint32 GetNumMetadataValues(PVMFMetadataList& aKeyList);
        //**********End PVMFMetadataExtensionInterface


        //==============================================================================

        OMX_ERRORTYPE EventHandlerProcessing(OMX_OUT OMX_HANDLETYPE aComponent,
                                             OMX_OUT OMX_PTR aAppData,
                                             OMX_OUT OMX_EVENTTYPE aEvent,
                                             OMX_OUT OMX_U32 aData1,
                                             OMX_OUT OMX_U32 aData2,
                                             OMX_OUT OMX_PTR aEventData);

        // for WMV params
        bool VerifyParametersSync(PvmiMIOSession aSession, PvmiKvp* aParameters, int num_elements);
    protected:

        void DoQueryUuid(PVMFOMXBaseDecNodeCommand&);
        void DoRequestPort(PVMFOMXBaseDecNodeCommand&);
        void DoReleasePort(PVMFOMXBaseDecNodeCommand&);
        PVMFStatus DoGetNodeMetadataKey(PVMFOMXBaseDecNodeCommand&);
        PVMFStatus DoGetNodeMetadataValue(PVMFOMXBaseDecNodeCommand&);
        PVMFStatus HandlePortReEnable();

        bool InitDecoder(PVMFSharedMediaDataPtr&);

        bool NegotiateComponentParameters(OMX_PTR aOutputParameters);
        bool GetSetCodecSpecificInfo();
        bool QueueOutputBuffer(OsclSharedPtr<PVMFMediaDataImpl> &mediadataimplout, uint32 aDataLen);

        bool ReleaseAllPorts();
        // Capability And Config Helper Methods
        PVMFStatus DoCapConfigGetParametersSync(PvmiKeyType aIdentifier, PvmiKvp*& aParameters, int& aNumParamElements, PvmiCapabilityContext aContext);
        PVMFStatus DoCapConfigReleaseParameters(PvmiKvp* aParameters, int aNumElements);
        void DoCapConfigSetParameters(PvmiKvp* aParameters, int aNumElements, PvmiKvp* &aRetKVP);
        PVMFStatus DoCapConfigVerifyParameters(PvmiKvp* aParameters, int aNumElements);
        PVMFStatus DoGetVideoDecNodeParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);
        PVMFStatus DoGetH263DecoderParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);
        PVMFStatus DoGetM4VDecoderParameter(PvmiKvp*& aParameters, int& aNumParamElements, int32 aIndex, PvmiKvpAttr reqattr);
        PVMFStatus DoVerifyAndSetVideoDecNodeParameter(PvmiKvp& aParameter, bool aSetParam);
        PVMFStatus DoVerifyAndSetH263DecoderParameter(PvmiKvp& aParameter, bool aSetParam);
        PVMFStatus DoVerifyAndSetM4VDecoderParameter(PvmiKvp& aParameter, bool aSetParam);

        PVMFStatus GetProfileAndLevel(PVMF_MPEGVideoProfileType& aProfile, PVMF_MPEGVideoLevelType& aLevel);
        int32 GetNAL_OMXNode(uint8** bitstream, int* size);

        OMX_COLOR_FORMATTYPE iOMXVideoColorFormat;
        OMX_VIDEO_CODINGTYPE iOMXVideoCompressionFormat;
        PVMFFormatType iYUVFormat;
        // Width of decoded frame
        int32 iYUVWidth;
        // Height of decoded frame
        int32 iYUVHeight;
        // Last stored value of width
        int32 iLastYUVWidth;
        // Last stored value of hight
        int32 iLastYUVHeight;

        uint32 iH263MaxBitstreamFrameSize;
        uint32 iH263MaxWidth;
        uint32 iH263MaxHeight;
        uint32 iM4VMaxBitstreamFrameSize;
        uint32 iM4VMaxWidth;
        uint32 iM4VMaxHeight;

        uint32 iNewWidth , iNewHeight;

};


#endif // PVMF_OMXVIDEODEC_NODE_H_INCLUDED

