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
#include "pv_player_node_registry.h"

// For decoder node registry
#if BUILD_OMX_VIDEO_DEC_NODE
#include "pvmf_omx_videodec_factory.h"
#endif
#if BUILD_OMX_AUDIO_DEC_NODE
#include "pvmf_omx_audiodec_factory.h"
#endif
#if BUILD_VIDEO_DEC_NODE
#include "pvmf_videodec_factory.h"
#endif
#if BUILD_AVC_DEC_NODE
#include "pvmf_avcdec_factory.h"
#endif
#if BUILD_WMV_DEC_NODE
#include "pvmf_wmvdec_factory.h"
#endif
#if BUILD_RV_DEC_NODE
#include "pvmf_rvdec_factory.h"
#endif
#if BUILD_WMA_DEC_NODE
#include "pvmf_wmadec_factory.h"
#endif
#if BUILD_G726_DEC_NODE
#include "pvmf_g726dec_node_factory.h"
#endif
#if BUILD_GSMAMR_DEC_NODE
#include "pvmf_gsmamrdec_factory.h"
#endif
#if BUILD_AAC_DEC_NODE
#include "pvmf_aacdec_factory.h"
#endif
#if BUILD_MP3_DEC_NODE
#include "pvmf_mp3dec_factory.h"
#endif
#if BUILD_RA8_DEC_NODE
#include "pvmf_ra8dec_factory.h"
#endif

// For parser node registry
#if BUILD_MP4_FF_PARSER_NODE
#include "pvmf_mp4ffparser_factory.h"
#endif
#if BUILD_AMR_FF_PARSER_NODE
#include "pvmf_amrffparser_factory.h"
#endif
#if BUILD_AAC_FF_PARSER_NODE
#include "pvmf_aacffparser_factory.h"
#endif
#if BUILD_MP3_FF_PARSER_NODE
#include "pvmf_mp3ffparser_factory.h"
#endif
#if BUILD_WAV_FF_PARSER_NODE
#include "pvmf_wavffparser_factory.h"
#endif
#if BUILD_ASF_FF_PARSER_NODE
#include "pvmf_asfffparser_factory.h"
#endif
#if BUILD_RM_FF_PARSER_NODE
#include "pvmf_rmffparser_factory.h"
#endif
#if BUILD_STREAMING_MANAGER_NODE
#include "pvmf_sm_node_factory.h"
#endif
#if BUILD_DOWNLOAD_MANAGER_NODE
#include "pvmf_downloadmanager_factory.h"
#endif
#if BUILD_STILL_IMAGE_NODE
#include "pvmf_stillimage_factory.h"
#endif

// For recognizer registry
#if BUILD_MP4_FF_REC
#include "pvmp4ffrec_factory.h"
#endif
#if BUILD_ASF_FF_REC
#include "pvasfffrec_factory.h"
#endif
#if BUILD_OMA1_FF_REC
#include "pvoma1ffrec_factory.h"
#endif
#if BUILD_AAC_FF_REC
#include "pvaacffrec_factory.h"
#endif
#if BUILD_RM_FF_REC
#include "pvrmffrec_factory.h"
#endif
#if BUILD_MP3_FF_REC
#include "pvmp3ffrec_factory.h"
#endif
#if BUILD_WAV_FF_REC
#include "pvwavffrec_factory.h"
#endif
#if BUILD_AMR_FF_REC
#include "pvamrffrec_factory.h"
#endif
#ifdef USE_LOADABLE_MODULES
#include "oscl_shared_library.h"

#include "pvmf_node_shared_lib_interface.h"
#endif

#include "pvmf_recognizer_registry.h"

#include "pvmi_datastreamsyncinterface_ref_factory.h"

#include "pvmf_recognizer_plugin.h"

//#include "pvsdcffrec_factory.h"
//#include "pvmf_videodec_node.h"
//#include "pvmf_mp4ffparser_node.h"
//#include "pvmf_pvciwmvdec_factory.h"
//#include "pv_devvideoplay_factory.h"
//#include "pvmf_gsmamrdec_factory_dpi.h"
//#include "pvmf_aacdec_factory_dpi.h"
//#include "pvmf_videodec_factory_dpi.h"

void PVPlayerRegistryPopulator::RegisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny*& aContext)
{
    OSCL_UNUSED_ARG(aContext);
    PVPlayerNodeInfo nodeinfo;
#if BUILD_OMX_VIDEO_DEC_NODE
    //For PVMFOMXVideoDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H2631998);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H2632000);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_M4V);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H264_VIDEO);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H264_VIDEO_RAW);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H264_VIDEO_MP4);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_WMV);
    nodeinfo.iNodeUUID = KPVMFOMXVideoDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_YUV420);
    nodeinfo.iNodeCreateFunc = PVMFOMXVideoDecNodeFactory::CreatePVMFOMXVideoDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFOMXVideoDecNodeFactory::DeletePVMFOMXVideoDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_OMX_AUDIO_DEC_NODE
    //For PVMFOMXAudioDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMR_IETF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMR);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMRWB_IETF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMRWB);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMR_IF2);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_MPEG4_AUDIO);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_3640);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_ADIF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_ADTS);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_LATM);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_WMA);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_MP3);
    nodeinfo.iNodeUUID = KPVMFOMXAudioDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFOMXAudioDecNodeFactory::CreatePVMFOMXAudioDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFOMXAudioDecNodeFactory::DeletePVMFOMXAudioDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_VIDEO_DEC_NODE
    //For PVMFVideoDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H2631998);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H2632000);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_M4V);
    nodeinfo.iNodeUUID = KPVMFVideoDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_YUV420);
    nodeinfo.iNodeCreateFunc = PVMFVideoDecNodeFactory::CreatePVMFVideoDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFVideoDecNodeFactory::DeletePVMFVideoDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_AVC_DEC_NODE
    //For PVMFAVCDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H264_VIDEO);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H264_VIDEO_RAW);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_H264_VIDEO_MP4);
    nodeinfo.iNodeUUID = KPVMFAVCDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_YUV420);
    nodeinfo.iNodeCreateFunc = PVMFAVCDecNodeFactory::CreatePVMFAVCDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFAVCDecNodeFactory::DeletePVMFAVCDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_WMV_DEC_NODE
    //For PVMFWmvDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_WMV);
    nodeinfo.iNodeUUID = KPVMFWmvDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_YUV420);
    nodeinfo.iNodeCreateFunc = PVMFWmvDecNodeFactory::CreatePVMFWmvDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFWmvDecNodeFactory::DeletePVMFWmvDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_RV_DEC_NODE
    //For PVMFRVDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_REAL_VIDEO);
    nodeinfo.iNodeUUID = KPVMFRvDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_YUV420);
    nodeinfo.iNodeCreateFunc = PVMFRvDecNodeFactory::CreatePVMFRvDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFRvDecNodeFactory::DeletePVMFRvDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_WMA_DEC_NODE
    //For PVMFWmaDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_WMA);
    nodeinfo.iNodeUUID = KPVMFWMADecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFWMADecNodeFactory::CreatePVMFWMADecNode;
    nodeinfo.iNodeReleaseFunc = PVMFWMADecNodeFactory::DeletePVMFWMADecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_G726_DEC_NODE
    //For PVMFG726DecoderNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_G726);
    nodeinfo.iNodeUUID = PVMFG726DecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFG726DecNodeFactory::Create;
    nodeinfo.iNodeReleaseFunc = PVMFG726DecNodeFactory::Delete;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_GSMAMR_DEC_NODE
    //For PVMFGSMAMRDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMR_IETF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMR);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMRWB_IETF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMRWB);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMR_IF2);
    nodeinfo.iNodeUUID = KPVMFGSMAmrNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFGSMAMRDecNodeFactory::CreatePVMFGSMAMRDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFGSMAMRDecNodeFactory::DeletePVMFGSMAMRDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_AAC_DEC_NODE
    //For PVMFAACDecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_MPEG4_AUDIO);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_3640);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_ADIF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_ADTS);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_LATM);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_ASF_MPEG4_AUDIO);
    nodeinfo.iNodeUUID = KPVMFAACDecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFAACDecNodeFactory::CreatePVMFAACDecNode;
    nodeinfo.iNodeReleaseFunc = PVMFAACDecNodeFactory::DeletePVMFAACDecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_MP3_DEC_NODE
    //For PVMFMP3DecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_MP3);
    nodeinfo.iNodeUUID = KPVMFMP3DecNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFMP3DecNodeFactory::CreatePVMFMP3DecNode;
    nodeinfo.iNodeReleaseFunc = PVMFMP3DecNodeFactory::DeletePVMFMP3DecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_RA8_DEC_NODE
    //For PVMFRA8DecNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_REAL_AUDIO);
    nodeinfo.iNodeUUID = KPVMFRA8NodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_PCM16);
    nodeinfo.iNodeCreateFunc = PVMFRA8DecNodeFactory::CreatePVMFRA8DecNode;
    nodeinfo.iNodeReleaseFunc = PVMFRA8DecNodeFactory::DeletePVMFRA8DecNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_MP4_FF_PARSER_NODE
    //For PVMFMP4FFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_MPEG4FF);
    nodeinfo.iNodeUUID = KPVMFMP4FFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFMP4FFParserNodeFactory::CreatePVMFMP4FFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFMP4FFParserNodeFactory::DeletePVMFMP4FFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_AMR_FF_PARSER_NODE
    //For PVMFAMRFFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AMRFF);
    nodeinfo.iNodeUUID = KPVMFAmrFFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFAMRFFParserNodeFactory::CreatePVMFAMRFFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFAMRFFParserNodeFactory::DeletePVMFAMRFFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_AAC_FF_PARSER_NODE
    //For PVMFAACFFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_AACFF);
    nodeinfo.iNodeUUID = KPVMFAacFFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFAACFFParserNodeFactory::CreatePVMFAACFFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFAACFFParserNodeFactory::DeletePVMFAACFFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_MP3_FF_PARSER_NODE
    //For PVMFMP3FFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_MP3FF);
    nodeinfo.iNodeUUID = KPVMFMP3FFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFMP3FFParserNodeFactory::CreatePVMFMP3FFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFMP3FFParserNodeFactory::DeletePVMFMP3FFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_WAV_FF_PARSER_NODE
    //For PVMFWAVFFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_WAVFF);
    nodeinfo.iNodeUUID = KPVMFWavFFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFWAVFFParserNodeFactory::CreatePVMFWAVFFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFWAVFFParserNodeFactory::DeletePVMFWAVFFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_ASF_FF_PARSER_NODE
    //For PVMFASFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_ASFFF);
    nodeinfo.iNodeUUID = KPVMFASFFFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFASFParserNodeFactory::CreatePVMFASFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFASFParserNodeFactory::DeletePVMFASFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_RM_FF_PARSER_NODE
    //For PVMFRMFFParserNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_RMFF);
    nodeinfo.iNodeUUID = KPVMFRMFFParserNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFRMFFParserNodeFactory::CreatePVMFRMFFParserNode;
    nodeinfo.iNodeReleaseFunc = PVMFRMFFParserNodeFactory::DeletePVMFRMFFParserNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_STREAMING_MANAGER_NODE
    //For PVMFStreamingManagerNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_MS_HTTP_STREAMING_URL);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_REAL_HTTP_CLOAKING_URL);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_RTSP_URL);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_SDP_FILE);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_SDP_PVR_FCS_FILE);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_RTSP_PVR_FCS_URL);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_SDP_BROADCAST);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_PVRFF);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_RTP_PACKET_SOURCE);
    nodeinfo.iNodeUUID = KPVMFStreamingManagerNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFStreamingManagerNodeFactory::CreateStreamingManagerNode;
    nodeinfo.iNodeReleaseFunc = PVMFStreamingManagerNodeFactory::DeleteStreamingManagerNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_DOWNLOAD_MANAGER_NODE
    //For PVMFDownloadManagerNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_HTTP_URL);
    nodeinfo.iInputTypes.push_back(PVMF_MIME_DATA_SOURCE_PVX_FILE);
    nodeinfo.iNodeUUID = KPVMFDownloadManagerNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFDownloadManagerNodeFactory::CreatePVMFDownloadManagerNode;
    nodeinfo.iNodeReleaseFunc = PVMFDownloadManagerNodeFactory::DeletePVMFDownloadManagerNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
#if BUILD_STILL_IMAGE_NODE
    //For PVMFStillImageNode
    nodeinfo.iInputTypes.clear();
    nodeinfo.iInputTypes.push_back(PVMF_MIME_IMAGE_FORMAT);
    nodeinfo.iNodeUUID = KPVMFStillImageNodeUuid;
    nodeinfo.iOutputType.clear();
    nodeinfo.iOutputType.push_back(PVMF_MIME_FORMAT_UNKNOWN);
    nodeinfo.iNodeCreateFunc = PVMFStillImageNodeFactory::CreateStillImageNode;
    aRegistry->RegisterNode(nodeinfo);
#endif
}

void PVPlayerRegistryPopulator::UnregisterAllNodes(PVPlayerNodeRegistryInterface* aRegistry, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aRegistry);
    OSCL_UNUSED_ARG(aContext);
    //nothing needed currently.
}




// Player engine with recognizers
void PVPlayerRegistryPopulator::RegisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny*& aContext)
{
    //Keep a list of all factories allocated by this populator for later cleanup.
    typedef Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator> nodelistType;
    nodelistType* nodeList = OSCL_NEW(nodelistType, ());
    aContext = nodeList;

    PVMFRecognizerPluginFactory* tmpfac = NULL;

#if BUILD_MP4_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVMP4FFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVMP4FFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_ASF_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVASFFFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVASFFFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_OMA1_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVOMA1FFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVOMA1FFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_AAC_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVAACFFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVAACFFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_RM_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVRMFFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVRMFFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_MP3_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVMP3FFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVMP3FFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_WAV_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVWAVFFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVWAVFFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
#if BUILD_AMR_FF_REC
    tmpfac = OSCL_STATIC_CAST(PVMFRecognizerPluginFactory*, OSCL_NEW(PVAMRFFRecognizerFactory, ()));
    if (PVMFRecognizerRegistry::RegisterPlugin(*tmpfac) == PVMFSuccess)
    {
        aRegistry->RegisterRecognizer(tmpfac);
        nodeList->push_back(tmpfac);
    }
    else
    {
        OSCL_DELETE(((PVAMRFFRecognizerFactory*)tmpfac));
        tmpfac = NULL;
        return;
    }
#endif
}


void PVPlayerRegistryPopulator::UnregisterAllRecognizers(PVPlayerRecognizerRegistryInterface* aRegistry, OsclAny* aContext)
{
    OSCL_UNUSED_ARG(aRegistry);

    //find the nodes added by this populator & delete them.
    Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator>* nodeList = (Oscl_Vector<PVMFRecognizerPluginFactory*, OsclMemAllocator>*) aContext;
    if (nodeList)
    {
        PVMFRecognizerPluginFactory* tmpfac = NULL;
        while (nodeList->size())
        {
            tmpfac = nodeList->front();;
            nodeList->erase(nodeList->begin());
            PVMFRecognizerRegistry::RemovePlugin(*tmpfac);
            OSCL_DELETE(tmpfac);
        }
        OSCL_DELETE(nodeList);
    }
}

