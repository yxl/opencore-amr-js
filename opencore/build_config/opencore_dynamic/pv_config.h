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
//
// Automatically generated, don't edit
//
// At: Thu, 26 Feb 2009 19:56:21 +0000
// Linux version 2.6.22-16-generic (buildd@rothera) (gcc version 4.1.3 20070929 (prerelease) (Ubuntu 4.1.2-16ubuntu2)) //1 SMP Mon Nov 24 18:28:27 GMT 2008

//
// PV Code Base Configuration System
//

//
// Menu for selecting supported features
//
#define module_support 1
#define shoutcast_support 0


//
// Menu for configuring runtime loadable modules
//
#define pv_so 0
#define pvsplit_so 1
#define streaming_support 1

//
// Menu for selecting streaming features
//
#define rtsp_support 1
#define asf_streaming_support 0

#define download_support 1
#define fasttrack_download_support 0
#define mp4local_support 1
#define asflocal_support 0
#define janus_support 0
#define wmdrmplat_support 0
#define wmdrmdev_support 0
#define wmdrmsysclk_support 0
#define mtp_db_support 0


//
// Menu for configuring Baselibs
//
#define csprng_lib 0
#define pvcrypto_lib 0
#define pventropysrc_lib 0
#define pvgendatastruct_lib m
#define pvmediadatastruct_lib m
#define pvmimeutils_lib m
#define threadsafe_callback_ao_lib m
#define pvthreadmessaging_lib m


//
// Menu for configuring File Formats
//
#define pvasfff_lib 0
#define pvmp3ff_lib m
#define pvmp4ffcomposer_lib 0
#define pvmp4ffcomposeropencore_lib m
#define pvmp4ff_lib 0
#define pvmp4ffopencore_lib m
#define mp4recognizer_utility_lib m
#define pvaacparser_lib m
#define pvgsmamrparser_lib m
#define pvrmff_lib 0
#define pvrmffparser_lib 0
#define pvfileparserutils_lib m
#define pvid3parcom_lib m
#define pvpvxparser_lib m
#define pvwav_lib m
#define pvasxparser_lib 0
#define pvavifileparser_lib m
#define pvpvrff_lib 0
#define asfrecognizer_utility_lib 0
#define pv_divxfile_parser_lib 0


//
// Menu for configuring Codecs
//

//
// Menu for configuring OMX Support
//
#define omx_mastercore_lib m
#define MAX_NUMBER_OF_OMX_CORES 10
#define MAX_NUMBER_OF_OMX_COMPONENTS 50
#define pv_omx 1
#define omx_avc_component_lib m
#define omx_common_lib m
#define omx_m4v_component_lib m
#define omx_queue_lib m
#define omx_wmv_component_lib 0
#define pvomx_proxy_lib m
#define omx_aac_component_lib m
#define omx_amr_component_lib m
#define omx_mp3_component_lib m
#define omx_wma_component_lib 0
#define omx_amrenc_component_lib m
#define omx_m4venc_component_lib m
#define omx_avcenc_component_lib m
#define omx_baseclass_lib m


//
// Menu for configuring audio codecs
//
#define pv_aac_dec_lib m
#define getactualaacconfig_lib m
#define pv_amr_nb_common_lib m
#define pvdecoder_gsmamr_lib m
#define pvencoder_gsmamr_lib m
#define pvamrwbdecoder_lib m
#define gsm_amr_headers_lib m
#define pvmp3_lib m
#define pvra8decoder_lib 0
#define wmadecoder_lib 0


//
// Menu for configuring video codecs
//
#define pv_avc_common_lib m
#define pvavcdecoder_lib m
#define wmvdecoder_lib 0
#define pvmp4decoder_lib m
#define rvdecoder_lib 0
#define pvm4vencoder_lib m
#define pvavch264enc_lib m


//
// Menu for configuring codecs utilities
//
#define m4v_config_lib m
#define pv_config_parser_lib m
#define colorconvert_lib m



//
// Menu for configuring Nodes
//

//
// Menu for configuring Streaming
//
#define pvstreamingmanagernode_segments_lib m

//
// Menu for configuring Streaming Features
//
#define mshttp_support 0
#define rtspunicast_support 1
#define unicastpvr_support 0
#define broadcastpvr_support 0
#define pvrfileplayback_support 0

#define pvstreamingmanagernode_lib 0
#define pvstreamingmanagernode_3gpp_lib 0
#define pvmedialayernode_lib m
#define pvmedialayernode_opencore_lib 0
#define pvjitterbuffernode_lib m

//
// Menu for configuring Jitter Buffer Node
//
#define rtpjitterbuffer_support m
#define asfjitterbuffer_support 0



//
// Menu for configuring Download
//
#define pvdownloadmanagernode_lib m

//
// Menu for configuring downloadmanager features
//
#define PVMF_DOWNLOADMANAGER_SUPPORT_PPB 1
#define PVMF_DOWNLOADMANAGER_MIN_TCP_BUFFERS_FOR_PPB 39
#define PVMF_DOWNLOADMANAGER_CACHE_SIZE_FOR_SC_IN_SECONDS 6
#define PVMF_DOWNLOADMANAGER_MAX_BITRATE_FOR_SC 128



//
// Menu for adding NON-OMX Nodes
//
#define aacdecnode_lib 0
#define mp3decnode_lib 0
#define pvavcdecnode_lib 0
#define pvwmadecnode_lib 0
#define pvwmvdecnode_lib 0
#define gsmamrdecnode_lib 0
#define pvvideodecnode_lib 0
#define pvvideoencnode_lib 0
#define pvamrencnode_lib 0
#define pvavcencnode_lib 0


//
// Menu for configuring ProtocolEngine
//
#define pvprotocolenginenode_segments_lib m
#define pvprotocolenginenode_lib 0
#define pvprotocolenginenode_pdl_plugin_lib 0
#define pvprotocolenginenode_ps_plugin_lib 0
#define pvprotocolenginenode_ftdl_plugin_lib 0
#define pvprotocolenginenode_wmhttpstreaming_plugin_lib 0
#define pvprotocolenginenode_shoutcast_plugin_lib 0

//
// Menu for configuring protocol engine node plugin registry
//
#define BUILD_PROGRESSIVE_DOWNLOAD_PLUGIN 1
#define BUILD_PROGRESSIVE_STREAMING_PLUGIN 1
#define BUILD_FASTTRACK_DOWNLOAD_PLUGIN 0
#define BUILD_WMHTTPSTREAMING_PLUGIN 0
#define BUILD_SHOUTCAST_PLUGIN 0


#define pvfileoutputnode_lib m
#define pvmediaoutputnode_lib m
#define pvsocketnode_lib m
#define pvwavffparsernode_lib m
#define pvomxencnode_lib m
#define pvomxaudiodecnode_lib m
#define pvomxbasedecnode_lib m
#define pvomxvideodecnode_lib m
#define pvaacffparsernode_lib m
#define pvamrffparsernode_lib m
#define pvasfffparsernode_lib 0
#define pvmp3ffparsernode_lib m
#define pvmp4ffparsernode_lib 0
#define pvmp4ffparsernodeopencore_lib m
#define pvrvdecnode_lib 0
#define pvra8decnode_lib 0
#define pvrmffparsernode_lib 0
#define pvrtppacketsourcenode_lib 0
#define nodes_common_headers_lib m
#define pvmediainputnode_lib m
#define pvmp4ffcomposernode_lib 0
#define pvmp4ffcomposernodeopencore_lib m
#define pvpvr_lib 0
#define pvpvrnode_lib 0
#define pvcommsionode_lib m
#define pvclientserversocketnode_lib m
#define pvloopbacknode_lib m
#define pvvideoparsernode_lib m
#define pvdummyinputnode_lib m
#define pvdummyoutputnode_lib m
#define pvdivxffparsernode_lib 0


//
// Menu for configuring Oscl
//
#define build_oscl m
#define unit_test_lib 1


//
// Menu for configuring Protocols
//

//
// Menu for configuring Value Adds for 2way
//
#define twoway_value_add_config 1
#define PV_2WAY_VALUE_ADD_NONE 1

#define pv_http_parcom_lib m
#define pvlatmpayloadparser_lib m
#define sdp_common m

//
// Menu for configuring SDPParser
//
#define sdp_default 0
#define sdp_opencore m

#define rdt_parser_lib 0
#define pv_rtsp_parcom_lib m
#define pvrtsp_cli_eng_node_lib 0
#define pvrtsp_cli_eng_node_3gpp_lib 0
#define pvrtsp_cli_eng_node_opencore_lib m
#define rtppayloadparser_lib m

//
// Menu for rtppayload parser plugins
//
#define rfc_2429 1
#define rfc_3016 1
#define rfc_3267 1
#define rfc_3640 1
#define rfc_3984 1
#define asf_payload 0
#define realmedia_payload 0

#define rtprtcp_lib m
#define pv324m_lib m
#define pv324m_common_headers_lib m
#define pvgeneraltools_lib m


//
// Menu for configuring Pvmi
//

//
// Menu for configuring Recognizers
//
#define pvmfrecognizer_lib m
#define pvaacffrecognizer_lib m
#define pvamrffrecognizer_lib m
#define pvoma1ffrecognizer_lib 0
#define pvasfffrecognizer_lib 0
#define pvmp3ffrecognizer_lib m
#define pvmp4ffrecognizer_lib m
#define pvwavffrecognizer_lib m
#define pvrmffrecognizer_lib 0
#define pvdivxffrecognizer_lib 0


//
// Menu for configuring Content Policy Manager
//
#define cpm_lib m
#define passthru_oma1_lib m
#define pvjanusplugin_lib 0
#define cpm_headers_lib m
#define pvoma1lockstream_lib 0


//
// Menu for configuring Media IO
//
#define pvmiofileinput_lib m
#define pvmiofileoutput_lib m
#define pvmioaviwavfileinput_lib m
#define pvmio_comm_loopback_lib m


//
// Menu for configuring PacketSources
//
#define packetsources_default_lib 0

//
// Menu for configuring PacketSource Plugins
//
#define optimized_bcast_ps_support 0
#define standard_bcast_ps_support 0


#define pvmf_lib m
#define realaudio_deinterleaver_lib 0
#define pvdbmanager_lib 0


//
// Menu for configuring Engines
//

//
// Menu for configuring Player
//
#define pvplayer_engine_lib m

//
// Menu for player engine tunables
//
#define PVPLAYERENGINE_CONFIG_SKIPTOREQUESTEDPOS_DEF 1
#define PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF -200
#define PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF 200
#define VIDEO_DEC_NODE_LOW_PRIORITY 1


//
// Menu for configuring player registry
//
#define BUILD_OMX_VIDEO_DEC_NODE 1
#define BUILD_OMX_AUDIO_DEC_NODE 1
#define BUILD_VIDEO_DEC_NODE 0
#define BUILD_AVC_DEC_NODE 0
#define BUILD_WMV_DEC_NODE 0
#define BUILD_RV_DEC_NODE 0
#define BUILD_WMA_DEC_NODE 0
#define BUILD_G726_DEC_NODE 0
#define BUILD_GSMAMR_DEC_NODE 0
#define BUILD_AAC_DEC_NODE 0
#define BUILD_MP3_DEC_NODE 0
#define BUILD_RA8_DEC_NODE 0
#define BUILD_MP4_FF_PARSER_NODE 0
#define BUILD_AMR_FF_PARSER_NODE 1
#define BUILD_AAC_FF_PARSER_NODE 1
#define BUILD_MP3_FF_PARSER_NODE 1
#define BUILD_WAV_FF_PARSER_NODE 1
#define BUILD_ASF_FF_PARSER_NODE 0
#define BUILD_RM_FF_PARSER_NODE 0
#define BUILD_STREAMING_MANAGER_NODE 0
#define BUILD_DOWNLOAD_MANAGER_NODE 0
#define BUILD_STILL_IMAGE_NODE 0
#define BUILD_MP4_FF_REC 0
#define BUILD_ASF_FF_REC 0
#define BUILD_OMA1_FF_REC 0
#define BUILD_AAC_FF_REC 1
#define BUILD_RM_FF_REC 0
#define BUILD_MP3_FF_REC 1
#define BUILD_WAV_FF_REC 1
#define BUILD_AMR_FF_REC 1
#define BUILD_DIVX_FF_PARSER_NODE 0
#define BUILD_DIVX_FF_REC 0



//
// Menu for configuring Author
//
#define pvauthorengine_lib m


//
// Menu for configuring pv2way
//
#define pv2wayengine_lib m

#define engines_common_headers_lib m
#define pvframemetadatautility_lib m


//
// Menu for configuring Extern_libs
//
#define pvmtp_engine_lib 0
#define pvmtpip_engine_lib 0
#define pviptransport_lib 0
#define pvsqlite_lib 0
#define pvwmdrm_lib 0
#define wmdrm_config 0


//
// That's all, folks!
//
// Derived symbols
//
#define pvrtspinterface_m_mk "/modules/linux_rtsp/core/build/make"
#define pvpvxparser_m_lib "-lpvpvxparser"
#define pvprotocolenginenode_ftdl_plugin_in_registry_y_mk ""
#define pvavcencnode_y_lib ""
#define pvmedialayernode_m_mk "/nodes/streaming/medialayernode/build/make"
#define pvpvr_y_mk ""
#define LIBDIR_engines_shared "/engines/player/build/make /engines/author/build/make /engines/2way/build/make /engines/common/build/make /engines/adapters/player/framemetadatautility/build/make"
#define LIBDIR_oscl_static " /oscl/unit_test/build/make"
#define SOLIBDIRS_omx_m4venc_sharedlibrary "/codecs_v2/omx/omx_m4venc/build/make_multithreaded /codecs_v2/video/m4v_h263/enc/build/make"
#define pvprotocolenginenode_shoutcast_plugin_m_lib ""
#define pvprotocolenginenode_ftdl_plugin_in_registry_m_lib ""
#define pvrmffparser_m_lib ""
#define pvm4vencoder_m_mk "/codecs_v2/video/m4v_h263/enc/build/make"
#define SOLIBS_omx_avcdec_sharedlibrary "-lomx_avc_component_lib -lpvavcdecoder"
#define rvdecoder_m_lib ""
#define pvmf_m_mk "/pvmi/pvmf/build/make"
#define opencore_downloadreg_so_name "opencore_downloadreg"
#define LIBS_codecs_v2_static "                                   "
#define pvmp3ffrecognizer_m_lib "-lpvmp3ffrecognizer"
#define pvmediainputnode_m_lib "-lpvmediainputnode"
#define pvmp4ff_m_mk ""
#define pvavifileparser_y_mk ""
#define pvprotocolenginenode_shoutcast_plugin_in_registry_m_mk ""
#define pvprotocolenginenode_wmhttpstreaming_plugin_m_lib ""
#define pvasflocalpbreg_so_name ""
#define pv_avc_common_lib_y_mk ""
#define pvavcdecoder_m_mk "/codecs_v2/video/avc_h264/dec/build/make"
#define LIBDIR_extern_libs_shared "    "
#define pvprotocolenginenode_pdl_plugin_y_mk ""
#define pvmio_comm_loopback_y_lib ""
#define pv_amr_nb_common_lib_y_lib ""
#define pventropysrc_m_mk ""
#define SOLIBDIRS_pvmtpip " "
#define cpm_m_mk "/pvmi/content_policy_manager/build/make"
#define pvprotocolenginenode_wmhttpstreaming_plugin_y_mk ""
#define pvmtp_engine_m_mk ""
#define realaudio_deinterleaver_y_lib ""
#define MODS_pvfasttrack_downloadreg "-lopencore_player -lopencore_common"
#define omx_amr_component_imp_m_lib ""
#define pvframemetadatautility_y_mk ""
#define pvwmdrmsysclk_so_name ""
#define pvavcdecoder_m_lib "-lpvavcdecoder"
#define SOLIBDIRS_opencore_mp4localreg "/modules/linux_mp4/node_registry/build/make /pvmi/recognizer/plugins/pvmp4ffrecognizer/build/make"
#define pviptransport_y_lib ""
#define SOLIBS_opencore_rtsp "-lpvrtspinterface -lpvstreamingmanagernode -lpv_rtsp_parcom -lpvrtsp_cli_eng_node -lrtppayloadparser -lrtprtcp -lpvjitterbuffer -lpvjitterbufferrtp -lpvjitterbuffernode -lpvmedialayernode  -lpvsdpparser"
#define pvsdpparser_y_lib ""
#define pvwmadecnode_y_mk ""
#define pvrtppacketsourcenode_y_mk ""
#define pvprotocolenginenode_shoutcast_plugin_y_mk ""
#define pvmp4ffcomposer_y_lib ""
#define pvdecoder_gsmamr_m_mk "/codecs_v2/audio/gsm_amr/amr_nb/dec/build/make"
#define pvrtppacketsourcenode_y_lib ""
#define gsm_amr_headers_m_mk "/codecs_v2/audio/gsm_amr/common/dec/build/make"
#define pvasfcommon_so_name ""
#define pvmp3ffrecognizer_y_mk ""
#define LIBDIR_pvmi_shared "/pvmi/content_policy_manager/build/make /pvmi/content_policy_manager/plugins/oma1/passthru/build/make  /pvmi/content_policy_manager/plugins/common/build/make  /pvmi/media_io/pvmiofileoutput/build/make /pvmi/media_io/pvmi_mio_fileinput/build/make_pvauthor /pvmi/media_io/pvmi_mio_avi_wav_fileinput/build/make /pvmi/media_io/pvmio_comm_loopback/build/make /pvmi/recognizer/build/make /pvmi/recognizer/plugins/pvaacffrecognizer/build/make /pvmi/recognizer/plugins/pvamrffrecognizer/build/make   /pvmi/recognizer/plugins/pvmp3ffrecognizer/build/make /pvmi/recognizer/plugins/pvmp4ffrecognizer/build/make /pvmi/recognizer/plugins/pvwavffrecognizer/build/make    /pvmi/pvmf/build/make  "
#define csprng_y_mk ""
#define pvdivxffparsernode_y_lib ""
#define pvrtspinterface_m_lib "-lpvrtspinterface"
#define pvavcencnode_m_mk ""
#define pvprotocolenginenode_ftdl_plugin_m_lib ""
#define rdt_parser_y_lib ""
#define pvmp4ff_y_lib ""
#define pvamrffparsernode_y_mk ""
#define protocolenginenode_download_common_y_mk ""
#define LIBS_omxenc_static "  "
#define pvstreamingmanagernode_y_mk ""
#define SOLIBS_pvwmdrmsysclk "n"
#define MODS_pvdb "-lopencore_player -lopencore_common -lpvsqlite -lpvwmdrm -lpvasfcommon -lpvasflocalpbreg"
#define pvmio_comm_loopback_m_mk "/pvmi/media_io/pvmio_comm_loopback/build/make"
#define DYNAMIC_LOAD_OMX_WMV_COMPONENT 0
#define pvdummyinputnode_y_lib ""
#define omx_avc_component_imp_m_lib ""
#define pvjitterbufferasf_y_mk ""
#define protocolenginenode_common_y_mk ""
#define pvomxbasedecnode_y_mk ""
#define nodes_common_headers_y_mk ""
#define omx_avcenc_component_m_mk "/codecs_v2/omx/omx_h264enc/build/make_multithreaded"
#define cml2_support_flag "USE_CML2_CONFIG"
#define MODS_omx_mp3dec_sharedlibrary "-lomx_sharedlibrary -lopencore_common"
#define pvplayer_engine_y_lib ""
#define SOLIBS_pvmtp "n"
#define pvasfstreamingreginterface_m_mk ""
#define pvprotocolenginenode_pdl_plugin_m_mk ""
#define pvrtsp_cli_eng_node_m_mk ""
#define pvprotocolenginenode_wmhttpstreaming_plugin_in_registry_y_lib ""
#define standard_bcast_ps_mk ""
#define pvid3parcom_y_lib ""
#define pv324m_y_mk ""
#define pvprotocolenginenode_ftdl_plugin_in_registry_m_mk ""
#define pvmp4ffrecognizer_y_mk ""
#define pvstreamingmanagernode_3gpp_y_mk ""
#define LIBDIR_omxencimp_shared ""
#define SOLIBS_omx_m4venc_sharedlibrary "-lomx_m4venc_component_lib -lpvm4vencoder"
#define pvdownloadinterface_m_mk "/modules/linux_download/core/build/make"
#define pvthreadmessaging_y_mk ""
#define rtspunicast_support_mk "rtspunicast.mk"
#define pvmfrecognizer_m_mk "/pvmi/recognizer/build/make"
#define LIBS_omxdecimp_shared ""
#define pvjitterbuffercommon_y_lib ""
#define pvmedialayernode_m_lib "-lpvmedialayernode"
#define pvclientserversocketnode_y_mk ""
#define pvdownloadmanagernode_y_lib ""
#define protocolenginenode_base_y_mk ""
#define omx_amr_component_imp_m_mk ""
#define pvgeneraltools_y_lib ""
#define pv_avc_common_imp_lib_m_lib ""
#define pvdivxffrecognizer_y_mk ""
#define pvdbmanager_m_mk ""
#define pvstreamingmanagernode_segments_y_mk ""
#define pvprotocolenginenode_ps_plugin_m_lib ""
#define USE_LOADABLE_MODULES 1
#define SOLIBS_pvwmdrm "   "
#define pvrmffparser_m_mk ""
#define pvrmff_y_mk ""
#define pvgendatastruct_m_mk "/baselibs/gen_data_structures/build/make"
#define pv_aac_dec_m_lib "-lpv_aac_dec"
#define pvsocketnode_y_mk ""
#define MODS_pvasflocalpb "-lopencore_player -lopencore_common -lpvasfcommon"
#define SOLIBDIRS_pv "/oscl   /codecs_v2/audio/aac/dec/build/make /codecs_v2/audio/mp3/dec/build/make  /codecs_v2/audio/gsm_amr/amr_nb/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/enc/build/make /codecs_v2/audio/gsm_amr/amr_nb/common/build/make /codecs_v2/audio/gsm_amr/amr_wb/dec/build/make /codecs_v2/video/avc_h264/dec/build/make /codecs_v2/video/avc_h264/common/build/make /codecs_v2/video/avc_h264/enc/build/make  /codecs_v2/video/m4v_h263/dec/build/make /codecs_v2/video/m4v_h263/enc/build/make  /codecs_v2/audio/aac/dec/util/getactualaacconfig/build/make /codecs_v2/utilities/colorconvert/build/make /codecs_v2/utilities/m4v_config_parser/build/make /codecs_v2/utilities/pv_config_parser/build/make /codecs_v2/omx/omx_h264/build/make_multithreaded /codecs_v2/omx/omx_m4v/build/make_multithreaded  /codecs_v2/omx/omx_aac/build/make_multithreaded /codecs_v2/omx/omx_amr/build/make_multithreaded /codecs_v2/omx/omx_mp3/build/make_multithreaded  /codecs_v2/omx/omx_common/build/make_multithreaded /codecs_v2/omx/omx_queue/build/make /codecs_v2/omx/omx_proxy/build/make /codecs_v2/omx/omx_baseclass/build/make /codecs_v2/omx/omx_mastercore/build/make_multithreaded /codecs_v2/omx/omx_sharedlibrary/interface/build/make /baselibs/threadsafe_callback_ao/build/make /baselibs/media_data_structures/build/make /baselibs/pv_mime_utils/build/make /baselibs/gen_data_structures/build/make /pvmi/pvmf/build/make /pvmi/content_policy_manager/build/make /pvmi/content_policy_manager/plugins/oma1/passthru/build/make /pvmi/media_io/pvmiofileoutput/build/make /fileformats/common/parser/build/make /fileformats/id3parcom/build/make  /fileformats/mp4/parser/build_opencore/make /fileformats/mp4/parser/utils/mp4recognizer/build/make  /fileformats/mp4/composer/build_opencore/make  /nodes/pvmp4ffcomposernode/build_opencore/make  /fileformats/pvx/parser/build/make /nodes/pvmediainputnode/build/make_pvauthor /nodes/pvmediaoutputnode/build/make /nodes/pvfileoutputnode/build/make /fileformats/rawgsmamr/parser/build/make /nodes/pvamrffparsernode/build/make /pvmi/recognizer/plugins/pvamrffrecognizer/build/make /fileformats/rawaac/parser/build/make /nodes/pvaacffparsernode/build/make  /pvmi/recognizer/plugins/pvaacffrecognizer/build/make /fileformats/mp3/parser/build/make  /nodes/pvmp3ffparsernode/build/make /pvmi/recognizer/plugins/pvmp3ffrecognizer/build/make    /nodes/pvomxvideodecnode/build/make /nodes/pvomxaudiodecnode/build/make /nodes/pvomxbasedecnode/build/make        /nodes/pvwavffparsernode/build/make /pvmi/recognizer/plugins/pvwavffrecognizer/build/make     /pvmi/recognizer/build/make /pvmi/media_io/pvmi_mio_fileinput/build/make_pvauthor /pvmi/media_io/pvmi_mio_avi_wav_fileinput/build/make /engines/adapters/player/framemetadatautility/build/make /engines/player/build/make /engines/author/build/make /protocols/systems/3g-324m_pvterminal/build/make/ /engines/2way/build/make /fileformats/avi/parser/build/make /baselibs/thread_messaging/build/make /protocols/rtp_payload_parser/util/build/latmparser/make  "
#define pv_config_parser_m_mk "/codecs_v2/utilities/pv_config_parser/build/make"
#define pv_amr_nb_common_imp_lib_m_lib ""
#define pvsocketnode_y_lib ""
#define SOLIBDIRS_opencore_downloadreg "/modules/linux_download/node_registry/build/make"
#define pvmimeutils_m_lib "-lpvmimeutils"
#define pvra8decnode_y_mk ""
#define pvwmdrmdevinterface_m_mk ""
#define pvprotocolenginenode_wmhttpstreaming_plugin_in_registry_m_mk ""
#define LIBS_tools_v2_shared "-lpvrtspinterface -lpvrtspreginterface   -lpvdownloadinterface -lpvdownloadreginterface -lpvmp4interface -lpvmp4reginterface      "
#define SOLIBS_pvasflocalpb " "
#define config_3gpp_asf_mk ""
#define download_common_enabled "n"
#define pvthreadmessaging_m_lib "-lpvthreadmessaging"
#define pvmiofileinput_m_lib "-lpvmiofileinput"
#define omx_avcenc_sharedlibrary_so_name "omx_avcenc_sharedlibrary"
#define rtppayloadparser_plugins_opencore_rtsp "rfc_2429.mk rfc_3016.mk rfc_3267.mk rfc_3640.mk rfc_3984.mk"
#define omx_common_y_lib ""
#define pvprotocolenginewmhttpstreamingpluginreginterface_m_lib ""
#define pvpvrnode_m_mk ""
#define gsm_amr_headers_y_mk ""
#define pvmp3_imp_m_lib ""
#define LIBDIR_cpm_shared "/pvmi/content_policy_manager/build/make /pvmi/content_policy_manager/plugins/oma1/passthru/build/make  /pvmi/content_policy_manager/plugins/common/build/make "
#define threadsafe_callback_ao_y_mk ""
#define csprng_y_lib ""
#define LIBS_oscl_shared "-losclbase -losclerror -losclmemory -losclproc -losclregcli -losclregserv -losclutil -losclio -lunit_test  -loscllib"
#define pvframemetadatautility_m_lib "-lpvframemetadatautility"
#define omx_mastercore_y_lib ""
#define pvmfrecognizer_y_lib ""
#define rtppayloadparser_m_mk "/protocols/rtp_payload_parser/build/make"
#define pvwmdrmsysclkinterface_m_lib ""
#define pvsdpparser_m_lib ""
#define rtprtcp_m_mk "/protocols/rtp/build/make"
#define pvmp3_imp_m_mk ""
#define MODS_omx_amrdec_sharedlibrary "-lomx_sharedlibrary -lopencore_common"
#define csprng_m_mk ""
#define LIBS_video_static "      "
#define omx_avc_component_y_mk ""
#define pvdbmanager_y_lib ""
#define pvasxparser_m_lib ""
#define pvvideodecnode_y_mk ""
#define pvamrffparsernode_m_lib "-lpvamrffparsernode"
#define pvmp4ffcomposer_y_mk ""
#define LIBS_nodes_shared "-lpvfileoutputnode -lpvmediaoutputnode -lpvsocketnode  -lprotocolenginenode_base -lprotocolenginenode_common -lprotocolenginenode_download_common -lprotocolenginenode_pdl -lprotocolenginenode_ps      -lpvwavffparsernode   -lpvomxencnode -lpvomxbasedecnode -lpvomxaudiodecnode -lpvomxvideodecnode  -lpvaacffparsernode  -lpvamrffparsernode   -lpvmp3ffparsernode  -lpvmp4ffparsernode      -lpvmediainputnode  -lpvmp4ffcomposernode     -lpvdownloadmanagernode   -lpvstreamingmanagernode   -lpvrtspinterface -lpvrtspreginterface -lpvmedialayernode  -lpvjitterbuffer -lpvjitterbufferrtp  -lpvjitterbuffernode -lpvcommsionode -lpvclientserversocketnode -lpvloopbacknode -lpvvideoparsernode -lpvdummyinputnode -lpvdummyoutputnode "
#define pvloopbacknode_m_mk "/nodes/pvloopbacknode/build/make"
#define pvprotocolenginenode_pdl_plugin_y_lib ""
#define pvjitterbuffernode_m_mk "/nodes/streaming/jitterbuffernode/build/make"
#define wmadecoder_m_lib ""
#define omx_aac_component_y_mk ""
#define pvavcdecoder_y_lib ""
#define pv_rtsp_parcom_m_lib "-lpv_rtsp_parcom"
#define pvvideoencnode_m_mk ""
#define pvdecoder_gsmamr_y_lib ""
#define omx_amrdec_sharedlibrary_so_name "omx_amrdec_sharedlibrary"
#define pvoma1lockstream_m_mk ""
#define pv_amr_nb_common_lib_y_mk ""
#define REGISTER_OMX_AMRENC_COMPONENT 1
#define pvmp4ffrecognizer_m_mk "/pvmi/recognizer/plugins/pvmp4ffrecognizer/build/make"
#define pvrtsp_cli_eng_node_y_mk ""
#define pvmioaviwavfileinput_y_mk ""
#define pvmp3_y_mk ""
#define pvasflocalpb_so_name ""
#define pvmp4decoder_y_lib ""
#define pvprotocolenginenode_wmhttpstreaming_plugin_y_lib ""
#define pvmtp_engine_y_lib ""
#define omx_mp3_component_y_lib ""
#define pvmedialayernode_plugins "ml_default.mk"
#define oscl_y_mk ""
#define pv_http_parcom_y_mk ""
#define rtppayloadparser_plugins "rfc_2429.mk rfc_3016.mk rfc_3267.mk rfc_3640.mk rfc_3984.mk  "
#define pvsdpparser_y_mk ""
#define pvfileoutputnode_y_lib ""
#define SOLIBDIRS_omx_amrenc_sharedlibrary "/codecs_v2/omx/omx_amrenc/build/make_multithreaded /codecs_v2/audio/gsm_amr/amr_nb/enc/build/make"
#define pvamrwbdecoder_imp_m_mk ""
#define pvwavffrecognizer_y_mk ""
#define LIBS_protocols_shared "-lpv_http_parcom -lpvlatmpayloadparser  -lpvsdpparser  -lpv_rtsp_parcom   -lpvrtsp_cli_eng_node -lrtppayloadparser -lrtprtcp -lpv324m -lpvgeneraltools"
#define DYNAMIC_LOAD_OMX_WMA_COMPONENT 0
#define pvwmvdecnode_y_mk ""
#define pvjitterbufferrtp_y_mk ""
#define pvrtspreginterface_m_mk "/modules/linux_rtsp/node_registry/build/make"
#define pvomxencnode_m_mk "/nodes/pvomxencnode/build/make"
#define pvfileparserutils_m_mk "/fileformats/common/parser/build/make"
#define SOLIBDIRS_pvprotocolengine_asfstreamingreg "n"
#define pvwmdrmdevinterface_m_lib ""
#define pvcommsionode_y_mk ""
#define pvprotocolenginenode_shoutcast_plugin_in_registry_y_lib ""
#define nodes_common_headers_m_mk "/nodes/common/build/make"
#define pvoma1lockstream_y_lib ""
#define pvprotocolenginenode_pdl_plugin_in_registry_m_lib "-lprotocolenginenode_pdl"
#define LIBDIR_protocols_shared "/protocols/http_parcom/build/make /protocols/rtp_payload_parser/util/build/latmparser/make /protocols/sdp/parser/build/make /protocols/sdp/common/build/make  /protocols/rtsp_parcom/build/make   /protocols/rtsp_client_engine/build_opencore/make /protocols/rtp_payload_parser/build/make /protocols/rtp/build/make /protocols/systems/3g-324m_pvterminal/build/make/ /protocols/systems/common/build/make/ /protocols/systems/tools/general/build/make"
#define pvmfrecognizer_m_lib "-lpvmfrecognizer"
#define pvencoder_gsmamr_y_mk ""
#define pvoma1ffrecognizer_y_lib ""
#define pvrtsp_cli_eng_node_3gpp_m_mk ""
#define wmadecoder_m_mk ""
#define wmvdecoder_y_lib ""
#define SOLIBDIRS_omx_amrdec_sharedlibrary "/codecs_v2/omx/omx_amr/build/make_multithreaded /codecs_v2/audio/gsm_amr/amr_nb/dec/build/make /codecs_v2/audio/gsm_amr/amr_wb/dec/build/make"
#define pvpvr_m_mk ""
#define SOLIBDIRS_pvdb "n"
#define omx_wma_component_m_mk ""
#define pvwmadecnode_y_lib ""
#define pvmedialayernode_opencore_m_mk ""
#define LIBDIR_audio_static "         "
#define SOLIBS_pvfasttrack_download "-lpvsdpparser  "
#define rfc_3984_mk "rfc_3984.mk"
#define pvprotocolenginenode_pdl_plugin_in_registry_y_mk ""
#define pv2wayengine_y_lib ""
#define LIBDIR_omxjoint_shared "/codecs_v2/omx/omx_common/build/make_multithreaded /codecs_v2/omx/omx_queue/build/make /codecs_v2/omx/omx_proxy/build/make /codecs_v2/omx/omx_baseclass/build/make /codecs_v2/omx/omx_mastercore/build/make_multithreaded /codecs_v2/omx/omx_sharedlibrary/interface/build/make"
#define LIBS_shared "-losclbase -losclerror -losclmemory -losclproc -losclregcli -losclregserv -losclutil -losclio -lunit_test  -loscllib    -lpvgendatastruct -lpvmediadatastruct -lpvmimeutils -lthreadsafe_callback_ao -lpvthreadmessaging -lomx_avc_component_lib -lomx_m4v_component_lib  -lomx_aac_component_lib -lomx_amr_component_lib -lomx_mp3_component_lib  -lomx_amrenc_component_lib -lomx_m4venc_component_lib -lomx_avcenc_component_lib -lomx_common_lib -lomx_queue_lib -lpvomx_proxy_lib -lomx_baseclass_lib -lomx_mastercore_lib -lpv_omx_interface   -lpv_aac_dec -lpv_amr_nb_common_lib -lpvamrwbdecoder -lpvdecoder_gsmamr -lpvmp3  -lpvencoder_gsmamr -lpv_avc_common_lib -lpvavcdecoder  -lpvmp4decoder  -lpvm4vencoder -lpvavch264enc -lm4v_config -lpv_config_parser -lcolorconvert -lpvfileparserutils -lpvid3parcom -lpvpvxparser -lpvwav  -lpvavifileparser  -lpvmp3ff -lpvaacparser -lpvgsmamrparser   -lmp4recognizer_utility -lpvmp4ff  -lpvmp4ffcomposer     -lpv_http_parcom -lpvlatmpayloadparser  -lpvsdpparser  -lpv_rtsp_parcom   -lpvrtsp_cli_eng_node -lrtppayloadparser -lrtprtcp -lpv324m -lpvgeneraltools -lcpm -lpassthru_oma1   -lpvmiofileoutput -lpvmiofileinput -lpvmioaviwavfileinput -lpvmio_comm_loopback -lpvmfrecognizer -lpvaacffrecognizer -lpvamrffrecognizer   -lpvmp3ffrecognizer -lpvmp4ffrecognizer -lpvwavffrecognizer    -lpvmf        -lpvfileoutputnode -lpvmediaoutputnode -lpvsocketnode  -lprotocolenginenode_base -lprotocolenginenode_common -lprotocolenginenode_download_common -lprotocolenginenode_pdl -lprotocolenginenode_ps      -lpvwavffparsernode   -lpvomxencnode -lpvomxbasedecnode -lpvomxaudiodecnode -lpvomxvideodecnode  -lpvaacffparsernode  -lpvamrffparsernode   -lpvmp3ffparsernode  -lpvmp4ffparsernode      -lpvmediainputnode  -lpvmp4ffcomposernode     -lpvdownloadmanagernode   -lpvstreamingmanagernode   -lpvrtspinterface -lpvrtspreginterface -lpvmedialayernode  -lpvjitterbuffer -lpvjitterbufferrtp  -lpvjitterbuffernode -lpvcommsionode -lpvclientserversocketnode -lpvloopbacknode -lpvvideoparsernode -lpvdummyinputnode -lpvdummyoutputnode  -lpvplayer_engine -lpvauthorengine -lpv2wayengine -lpvframemetadatautility -lpvrtspinterface -lpvrtspreginterface   -lpvdownloadinterface -lpvdownloadreginterface -lpvmp4interface -lpvmp4reginterface      "
#define REGISTER_OMX_AAC_COMPONENT 1
#define pvwavffparsernode_m_lib "-lpvwavffparsernode"
#define rtppayloadparser_m_lib "-lrtppayloadparser"
#define DYNAMIC_LOAD_OMX_M4V_COMPONENT 1
#define MODS_omx_avcenc_sharedlibrary "-lomx_sharedlibrary -lopencore_common "
#define pvaacparser_m_mk "/fileformats/rawaac/parser/build/make"
#define pvgsmamrparser_y_mk ""
#define protocolenginenode_download_common_m_lib "-lprotocolenginenode_download_common"
#define pvmediadatastruct_y_lib ""
#define SOLIBS_pvprotocolengine_asfstreamingreg "n"
#define LIBDIR_codecs_v2_static "                                 "
#define MODS_omx_m4vdec_sharedlibrary "-lomx_sharedlibrary -lopencore_common "
#define pvmp4ff_m_lib ""
#define rvdecoder_y_lib ""
#define pvauthorengine_y_mk ""
#define omx_avc_component_y_lib ""
#define pvmfrecognizer_y_mk ""
#define pvmediadatastruct_m_lib "-lpvmediadatastruct"
#define pvstreamingmanagernode_plugins "  3gpp.mk rtspunicast.mk    "
#define pvmp4ffcomposernode_y_mk ""
#define pvprotocolenginewmhttpstreaminginterface_m_lib ""
#define pdl_enabled "y"
#define omx_avc_component_m_lib "-lomx_avc_component_lib"
#define pvplayer_engine_m_lib "-lpvplayer_engine"
#define passthru_oma1_y_mk ""
#define USING_OMX 1
#define wmvdecoder_m_lib ""
#define pvprotocolenginenode_pdl_plugin_in_registry_y_lib ""
#define CONFIG_FLAGS "USE_CML2_CONFIG "
#define mp4recognizer_utility_y_mk ""
#define REGISTER_OMX_H263_COMPONENT 1
#define pvmp4reginterface_m_lib "-lpvmp4reginterface"
#define pv324m_m_lib "-lpv324m"
#define pvpvrnode_y_lib ""
#define pvpvrff_y_lib ""
#define omx_common_y_mk ""
#define unicastpvr_support_mk ""
#define getactualaacconfig_imp_m_mk "/codecs_v2/audio/aac/dec/util/getactualaacconfig/build/make"
#define getactualaacconfig_for_static_m_mk ""
#define sdp_parser_mksegment_default ""
#define pvstreamingmanagernode_3gpp_m_lib ""
#define omx_wmv_component_y_mk ""
#define LIBS_video_shared "-lpv_avc_common_lib -lpvavcdecoder  -lpvmp4decoder  -lpvm4vencoder -lpvavch264enc"
#define pvmiofileinput_y_mk ""
#define omx_queue_y_mk ""
#define pvrvdecnode_m_mk ""
#define pvmp4ffopencore_m_mk "/fileformats/mp4/parser/build_opencore/make"
#define DYNAMIC_LOAD_OMX_AMRENC_COMPONENT 1
#define omx_wma_component_imp_m_mk "n"
#define opencore_mp4local_so_name "opencore_mp4local"
#define MODS_pvsqlite "-lopencore_player -lopencore_common"
#define ftdl_enabled "n"
#define pvamrwbdecoder_m_lib "-lpvamrwbdecoder"
#define pvfileparserutils_m_lib "-lpvfileparserutils"
#define pvgendatastruct_m_lib "-lpvgendatastruct"
#define SOLIBS_opencore_rtspreg "-lpvrtspreginterface"
#define pvjitterbuffercommon_y_mk ""
#define LIBS_omxenc_shared "-lomx_amrenc_component_lib -lomx_m4venc_component_lib -lomx_avcenc_component_lib"
#define pvjitterbuffernode_y_mk ""
#define aacdecnode_m_mk ""
#define protocolenginenode_base_m_mk "/nodes/pvprotocolenginenode/base/build/make"
#define pvencoder_gsmamr_y_lib ""
#define omx_amrenc_component_m_mk "/codecs_v2/omx/omx_amrenc/build/make_multithreaded"
#define sdp_parser_mksegment_opencore "sdp_opencore.mk"
#define scsp_m_lib ""
#define pvauthorengine_y_lib ""
#define omx_amrenc_component_m_lib "-lomx_amrenc_component_lib"
#define pvfileparserutils_y_mk ""
#define pv_aac_dec_y_mk ""
#define pvamrencnode_m_lib ""
#define pvmimeutils_y_lib ""
#define mp4recognizer_utility_m_lib "-lmp4recognizer_utility"
#define MODS_opencore_download "-lopencore_net_support -lopencore_player -lopencore_common"
#define omx_avcenc_component_m_lib "-lomx_avcenc_component_lib"
#define omx_m4v_component_y_lib ""
#define pvomxbasedecnode_m_lib "-lpvomxbasedecnode"
#define pvprotocolenginenode_ftdl_plugin_y_lib ""
#define SOLIBDIRS_pvasfstreaming " /nodes/streaming/streamingmanager/build/make_segments /nodes/streaming/jitterbuffernode/jitterbuffer/common/build/make  /nodes/streaming/jitterbuffernode/build/make /nodes/streaming/medialayernode/build/make /protocols/rtp_payload_parser/build/make /protocols/rtp/build/make"
#define m4v_config_m_mk "/codecs_v2/utilities/m4v_config_parser/build/make"
#define pvrtsp_cli_eng_node_3gpp_y_mk ""
#define DRMCONFIG ""
#define asfrecognizer_utility_y_mk ""
#define LIBDIR_codecs_v2_shared "/codecs_v2/omx/omx_h264/build/make_multithreaded /codecs_v2/omx/omx_m4v/build/make_multithreaded  /codecs_v2/omx/omx_aac/build/make_multithreaded /codecs_v2/omx/omx_amr/build/make_multithreaded /codecs_v2/omx/omx_mp3/build/make_multithreaded  /codecs_v2/omx/omx_amrenc/build/make_multithreaded /codecs_v2/omx/omx_m4venc/build/make_multithreaded /codecs_v2/omx/omx_h264enc/build/make_multithreaded /codecs_v2/omx/omx_common/build/make_multithreaded /codecs_v2/omx/omx_queue/build/make /codecs_v2/omx/omx_proxy/build/make /codecs_v2/omx/omx_baseclass/build/make /codecs_v2/omx/omx_mastercore/build/make_multithreaded /codecs_v2/omx/omx_sharedlibrary/interface/build/make   /codecs_v2/audio/aac/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/common/build/make /codecs_v2/audio/gsm_amr/amr_wb/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/dec/build/make /codecs_v2/audio/mp3/dec/build/make  /codecs_v2/audio/gsm_amr/common/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/enc/build/make /codecs_v2/video/avc_h264/common/build/make /codecs_v2/video/avc_h264/dec/build/make  /codecs_v2/video/m4v_h263/dec/build/make  /codecs_v2/video/m4v_h263/enc/build/make /codecs_v2/video/avc_h264/enc/build/make /codecs_v2/utilities/m4v_config_parser/build/make /codecs_v2/utilities/pv_config_parser/build/make /codecs_v2/utilities/colorconvert/build/make"
#define MODS_opencore_player "-lopencore_common -lopencore_net_support"
#define omx_amrenc_component_imp_m_mk ""
#define pvdecoder_gsmamr_m_lib "-lpvdecoder_gsmamr"
#define pvfileoutputnode_y_mk ""
#define SOLIBDIRS_opencore_2way "/engines/2way/build/make /protocols/systems/3g-324m_pvterminal/build/make/ /nodes/pvvideoparsernode/build/make /nodes/pvcommsionode/build/make /pvmi/media_io/pvmio_comm_loopback/build/make /protocols/systems/common/build/make/ /protocols/systems/tools/general/build/make "
#define pvmp4ffcomposeropencore_m_mk "/fileformats/mp4/composer/build_opencore/make"
#define SOLIBS_opencore_author " -lpvmp4ffcomposer  -lpvmp4ffcomposernode -lpvauthorengine"
#define SOLIBS_opencore_2way "-lpv2wayengine -lpv324m -lpvvideoparsernode -lpvcommsionode -lpvmio_comm_loopback -lpvgeneraltools "
#define MODS_pvprotocolengine_asfstreamingreg "-lopencore_player -lopencore_common"
#define pvamrffrecognizer_y_mk ""
#define pventropysrc_m_lib ""
#define LIBDIR_baselibs_shared "   /baselibs/gen_data_structures/build/make /baselibs/media_data_structures/build/make /baselibs/pv_mime_utils/build/make /baselibs/threadsafe_callback_ao/build/make /baselibs/thread_messaging/build/make"
#define pvaacffrecognizer_y_lib ""
#define LIBS_omxencimp_shared ""
#define pvdivxffrecognizer_m_lib ""
#define pvmp4ffopencore_m_lib "-lpvmp4ff"
#define pvprotocolenginefasttrackdownloadpluginreginterface_m_lib ""
#define sdp_common_y_mk ""
#define pvasfstreaminginterface_m_mk ""
#define pvavifileparser_m_mk "/fileformats/avi/parser/build/make"
#define pvaacparser_y_mk ""
#define LIBS_extern_libs_shared "    "
#define getactualaacconfig_m_mk "/codecs_v2/audio/aac/dec/util/getactualaacconfig/build/make"
#define pvmp3ff_y_mk ""
#define pv_aac_dec_imp_m_lib ""
#define rfc_3016_mk "rfc_3016.mk"
#define pvwmdrm_so_name ""
#define pvasxparser_y_lib ""
#define pvvideodecnode_m_mk ""
#define pvmimeutils_y_mk ""
#define pvwavffrecognizer_m_lib "-lpvwavffrecognizer"
#define pvrtsp_cli_eng_node_y_lib ""
#define pvencoder_gsmamr_imp_m_lib ""
#define SOLIBS_pvjanus " "
#define pvprotocolenginenode_ftdl_plugin_in_registry_y_lib ""
#define pvmp4decoder_m_mk "/codecs_v2/video/m4v_h263/dec/build/make"
#define pvomxvideodecnode_m_lib "-lpvomxvideodecnode"
#define pvgeneraltools_y_mk ""
#define SOLIBDIRS_pvasfstreamingreg "n"
#define pvjitterbufferasf_m_lib ""
#define pvasxparser_m_mk ""
#define gsmamrdecnode_m_mk ""
#define pvrvdecnode_y_lib ""
#define pvfileoutputnode_m_lib "-lpvfileoutputnode"
#define pvdownloadinterface_m_lib "-lpvdownloadinterface"
#define omx_queue_m_lib "-lomx_queue_lib"
#define pvavcdecoder_imp_m_lib ""
#define SOLIBS_pvdb "n"
#define omx_aac_component_y_lib ""
#define pvoma1lockstream_y_mk ""
#define pvauthorengine_m_lib "-lpvauthorengine"
#define pvmediaoutputnode_y_lib ""
#define pvwavffparsernode_y_lib ""
#define protocolenginenode_download_common_m_mk "/nodes/pvprotocolenginenode/download_protocols/common/build/make"
#define DYNAMIC_LOAD_OMX_M4VENC_COMPONENT 1
#define pvstreamingmanagernode_plugins_pvasfstreaming "asf.mk mshttp.mk"
#define protocolenginenode_segments_y_lib "       "
#define protocolenginenode_common_m_lib "-lprotocolenginenode_common"
#define LIBDIR_omxdec_static "      "
#define MODS_omx_wmvdec_sharedlibrary "-lomx_sharedlibrary -lopencore_common"
#define pvwmdrmplatinterface_m_mk ""
#define pvprotocolenginenode_shoutcast_plugin_y_lib ""
#define pvaacffparsernode_y_mk ""
#define pvasfff_y_lib ""
#define pvwavffrecognizer_y_lib ""
#define mp3decnode_y_lib ""
#define pvomx_proxy_m_mk "/codecs_v2/omx/omx_proxy/build/make"
#define pvdecoder_gsmamr_imp_m_mk ""
#define pvomxencnode_m_lib "-lpvomxencnode"
#define pvsdpparser_m_mk "/protocols/sdp/parser/build/make"
#define pvsqlite_y_mk ""
#define pvmp4interface_m_lib "-lpvmp4interface"
#define MODS_pvasfstreaming "-lopencore_net_support -lopencore_player -lopencore_common -lpvasfcommon"
#define MODS_pv ""
#define LIBS_audio_static "        "
#define pvrtsp_cli_eng_node_m_lib ""
#define pvavch264enc_m_mk "/codecs_v2/video/avc_h264/enc/build/make"
#define pvm4vencoder_y_lib ""
#define pvrtsp_cli_eng_node_opencore_m_lib "-lpvrtsp_cli_eng_node"
#define USE_DYNAMIC_LOAD_OMX_COMPONENTS 1
#define LIBDIR_omxenc_static "  "
#define rtppayloadparser_y_mk ""
#define pvid3parcom_m_mk "/fileformats/id3parcom/build/make"
#define pvprotocolenginenode_ps_plugin_in_registry_m_mk "/nodes/pvprotocolenginenode/download_protocols/progressive_streaming/build/make"
#define opencore_author_so_name "opencore_author"
#define pvdownloadreginterface_m_mk "/modules/linux_download/node_registry/build/make"
#define omx_amr_component_m_mk "/codecs_v2/omx/omx_amr/build/make_multithreaded"
#define pvcrypto_m_mk ""
#define pvfasttrack_downloadreg_so_name ""
#define realaudio_deinterleaver_y_mk ""
#define pvdummyinputnode_m_mk "/nodes/pvdummyinputnode/build/make"
#define pvaacffparsernode_m_lib "-lpvaacffparsernode"
#define pvprotocolenginenode_wmhttpstreaming_plugin_m_mk ""
#define pvstreamingmanagernode_3gpp_y_lib ""
#define pvjitterbufferrtp_y_lib ""
#define pvomxencnode_y_mk ""
#define omx_wmv_component_imp_m_mk "n"
#define pvmtp_so_name ""
#define pvrtppacketsourcenode_m_mk ""
#define MODS_omx_avcdec_sharedlibrary "-lomx_sharedlibrary -lopencore_common"
#define pvasfffrecognizer_m_mk ""
#define pvlatmpayloadparser_y_lib ""
#define pvmp4ffparsernode_m_mk ""
#define pvpvxparser_y_mk ""
#define rfc_3640_mk "rfc_3640.mk"
#define pv_divxfile_parser_y_mk ""
#define pvprotocolenginewmhttpstreaminginterface_m_mk ""
#define SOLIBDIRS_opencore_mp4local "/modules/linux_mp4/core/build/make  /nodes/pvmp4ffparsernode/build_opencore/make"
#define omx_m4venc_component_m_lib "-lomx_m4venc_component_lib"
#define pvmp4ffcomposernodeopencore_m_lib "-lpvmp4ffcomposernode"
#define rfc_3267_mk "rfc_3267.mk"
#define LIBS_media_io_shared "-lpvmiofileoutput -lpvmiofileinput -lpvmioaviwavfileinput -lpvmio_comm_loopback"
#define LIBS_audio_shared "  -lpv_aac_dec -lpv_amr_nb_common_lib -lpvamrwbdecoder -lpvdecoder_gsmamr -lpvmp3  -lpvencoder_gsmamr"
#define MODS_pvfasttrack_download "-lopencore_net_support -lopencore_player -lopencore_common"
#define pvmioaviwavfileinput_y_lib ""
#define pvmp4ffparsernode_m_lib ""
#define SOLIBS_pvasfcommon " "
#define SOLIBS_omx_m4vdec_sharedlibrary "-lomx_m4v_component_lib -lpvmp4decoder"
#define omx_avcenc_component_imp_m_lib ""
#define pvmp4ffcomposernodeopencore_y_mk ""
#define pvsocketnode_m_lib "-lpvsocketnode"
#define SOLIBS_opencore_common "-losclbase -losclerror -losclmemory -losclproc -losclregcli -losclregserv -losclutil -losclio -lunit_test  -loscllib -lomx_mastercore_lib             -lpv_avc_common_lib -lpv_amr_nb_common_lib -lpvgsmamrparser -lgetactualaacconfig -lm4v_config -lpv_config_parser -lcolorconvert -lthreadsafe_callback_ao -lpvmediadatastruct -lpvmimeutils -lpvgendatastruct -lpvmf -lpvfileoutputnode -lpvmediainputnode -lpvomxencnode -lpvmiofileinput -lpvmioaviwavfileinput -lpvavifileparser -lpvthreadmessaging -lpvmiofileoutput -lpvmediaoutputnode -lpvomxvideodecnode -lpvomxaudiodecnode -lpvomxbasedecnode -lpvlatmpayloadparser -lpvwav -lpvfileparserutils"
#define pvgsmamrparser_m_mk "/fileformats/rawgsmamr/parser/build/make"
#define rvdecoder_m_mk ""
#define pvmp4ffparsernodeopencore_m_mk "/nodes/pvmp4ffparsernode/build_opencore/make"
#define REGISTER_OMX_MP3_COMPONENT 1
#define scsp_y_lib ""
#define pvstreamingmanagernode_segments_m_lib "-lpvstreamingmanagernode"
#define LIBS_omxdec_shared "-lomx_avc_component_lib -lomx_m4v_component_lib  -lomx_aac_component_lib -lomx_amr_component_lib -lomx_mp3_component_lib "
#define pvmp4decoder_y_mk ""
#define pvmp4interface_m_mk "/modules/linux_mp4/core/build/make"
#define REGISTER_OMX_AVC_COMPONENT 1
#define pvaacffparsernode_y_lib ""
#define omx_mp3_component_imp_m_mk ""
#define pvasfffparsernode_m_mk ""
#define m4v_config_y_lib ""
#define mp3decnode_m_mk ""
#define SOLIBS_opencore_downloadreg "-lpvdownloadreginterface"
#define pvmp4ffparsernodeopencore_m_lib "-lpvmp4ffparsernode"
#define pvamrencnode_m_mk ""
#define pvavch264enc_m_lib "-lpvavch264enc"
#define mp3decnode_m_lib ""
#define pvwavffparsernode_y_mk ""
#define omx_wmv_component_m_lib ""
#define pvaacffrecognizer_m_mk "/pvmi/recognizer/plugins/pvaacffrecognizer/build/make"
#define pvwmdrmdev_so_name ""
#define SOLIBDIRS_pvmtp "n"
#define omx_amr_component_m_lib "-lomx_amr_component_lib"
#define LIBDIR_omxdecimp_static ""
#define pvwmdrm_y_lib ""
#define pvcommsionode_m_lib "-lpvcommsionode"
#define pvmiofileoutput_m_mk "/pvmi/media_io/pvmiofileoutput/build/make"
#define aacdecnode_m_lib ""
#define pvmediainputnode_y_lib ""
#define omx_aac_component_m_mk "/codecs_v2/omx/omx_aac/build/make_multithreaded"
#define pv_aac_dec_y_lib ""
#define SOLIBS_pvasflocalpbreg " "
#define pvgsmamrparser_y_lib ""
#define pvoma1ffrecognizer_m_lib ""
#define rtprtcp_y_lib ""
#define pvasfffparsernode_m_lib ""
#define LIBDIR_shared "/oscl     /baselibs/gen_data_structures/build/make /baselibs/media_data_structures/build/make /baselibs/pv_mime_utils/build/make /baselibs/threadsafe_callback_ao/build/make /baselibs/thread_messaging/build/make /codecs_v2/omx/omx_h264/build/make_multithreaded /codecs_v2/omx/omx_m4v/build/make_multithreaded  /codecs_v2/omx/omx_aac/build/make_multithreaded /codecs_v2/omx/omx_amr/build/make_multithreaded /codecs_v2/omx/omx_mp3/build/make_multithreaded  /codecs_v2/omx/omx_amrenc/build/make_multithreaded /codecs_v2/omx/omx_m4venc/build/make_multithreaded /codecs_v2/omx/omx_h264enc/build/make_multithreaded /codecs_v2/omx/omx_common/build/make_multithreaded /codecs_v2/omx/omx_queue/build/make /codecs_v2/omx/omx_proxy/build/make /codecs_v2/omx/omx_baseclass/build/make /codecs_v2/omx/omx_mastercore/build/make_multithreaded /codecs_v2/omx/omx_sharedlibrary/interface/build/make   /codecs_v2/audio/aac/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/common/build/make /codecs_v2/audio/gsm_amr/amr_wb/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/dec/build/make /codecs_v2/audio/mp3/dec/build/make  /codecs_v2/audio/gsm_amr/common/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/enc/build/make /codecs_v2/video/avc_h264/common/build/make /codecs_v2/video/avc_h264/dec/build/make  /codecs_v2/video/m4v_h263/dec/build/make  /codecs_v2/video/m4v_h263/enc/build/make /codecs_v2/video/avc_h264/enc/build/make /codecs_v2/utilities/m4v_config_parser/build/make /codecs_v2/utilities/pv_config_parser/build/make /codecs_v2/utilities/colorconvert/build/make /fileformats/common/parser/build/make /fileformats/id3parcom/build/make /fileformats/pvx/parser/build/make /fileformats/wav/parser/build/make  /fileformats/avi/parser/build/make  /fileformats/mp3/parser/build/make /fileformats/rawaac/parser/build/make /fileformats/rawgsmamr/parser/build/make    /fileformats/mp4/parser/utils/mp4recognizer/build/make /fileformats/mp4/parser/build_opencore/make  /fileformats/mp4/composer/build_opencore/make     /protocols/http_parcom/build/make /protocols/rtp_payload_parser/util/build/latmparser/make /protocols/sdp/parser/build/make /protocols/sdp/common/build/make  /protocols/rtsp_parcom/build/make   /protocols/rtsp_client_engine/build_opencore/make /protocols/rtp_payload_parser/build/make /protocols/rtp/build/make /protocols/systems/3g-324m_pvterminal/build/make/ /protocols/systems/common/build/make/ /protocols/systems/tools/general/build/make /pvmi/content_policy_manager/build/make /pvmi/content_policy_manager/plugins/oma1/passthru/build/make  /pvmi/content_policy_manager/plugins/common/build/make  /pvmi/media_io/pvmiofileoutput/build/make /pvmi/media_io/pvmi_mio_fileinput/build/make_pvauthor /pvmi/media_io/pvmi_mio_avi_wav_fileinput/build/make /pvmi/media_io/pvmio_comm_loopback/build/make /pvmi/recognizer/build/make /pvmi/recognizer/plugins/pvaacffrecognizer/build/make /pvmi/recognizer/plugins/pvamrffrecognizer/build/make   /pvmi/recognizer/plugins/pvmp3ffrecognizer/build/make /pvmi/recognizer/plugins/pvmp4ffrecognizer/build/make /pvmi/recognizer/plugins/pvwavffrecognizer/build/make    /pvmi/pvmf/build/make        /nodes/pvfileoutputnode/build/make /nodes/pvmediaoutputnode/build/make /nodes/pvsocketnode/build/make  /nodes/pvprotocolenginenode/base/build/make /nodes/pvprotocolenginenode/protocol_common/build/make /nodes/pvprotocolenginenode/download_protocols/common/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_download/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_streaming/build/make      /nodes/pvwavffparsernode/build/make   /nodes/pvomxencnode/build/make /nodes/pvomxbasedecnode/build/make /nodes/pvomxaudiodecnode/build/make /nodes/pvomxvideodecnode/build/make  /nodes/pvaacffparsernode/build/make  /nodes/pvamrffparsernode/build/make   /nodes/pvmp3ffparsernode/build/make  /nodes/pvmp4ffparsernode/build_opencore/make     /nodes/common/build/make   /nodes/pvmediainputnode/build/make_pvauthor  /nodes/pvmp4ffcomposernode/build_opencore/make    /nodes/pvdownloadmanagernode/build/make   /nodes/streaming/streamingmanager/build/make_segments   /modules/linux_rtsp/core/build/make /modules/linux_rtsp/node_registry/build/make /nodes/streaming/medialayernode/build/make  /nodes/streaming/jitterbuffernode/jitterbuffer/common/build/make /nodes/streaming/jitterbuffernode/jitterbuffer/rtp/build/make  /nodes/streaming/jitterbuffernode/build/make /nodes/pvcommsionode/build/make /nodes/pvclientserversocketnode/build/make /nodes/pvloopbacknode/build/make /nodes/pvvideoparsernode/build/make /nodes/pvdummyinputnode/build/make /nodes/pvdummyoutputnode/build/make  /engines/player/build/make /engines/author/build/make /engines/2way/build/make /engines/common/build/make /engines/adapters/player/framemetadatautility/build/make /modules/linux_rtsp/core/build/make /modules/linux_rtsp/node_registry/build/make   /modules/linux_download/core/build/make /modules/linux_download/node_registry/build/make /modules/linux_mp4/core/build/make /modules/linux_mp4/node_registry/build/make      "
#define omx_aac_component_imp_m_lib ""
#define pvprotocolenginenode_wmhttpstreaming_plugin_in_registry_y_mk ""
#define pvmp3ffparsernode_y_lib ""
#define config_asf_mk ""
#define MODS_pvwmdrmplat "-lopencore_player -lopencore_common"
#define omx_wmadec_sharedlibrary_so_name ""
#define SOLIBDIRS_opencore_net_support "/nodes/pvsocketnode/build/make /protocols/http_parcom/build/make /nodes/pvprotocolenginenode/base/build/make /nodes/pvprotocolenginenode/protocol_common/build/make /nodes/pvprotocolenginenode/download_protocols/common/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_download/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_streaming/build/make   "
#define omx_aac_component_m_lib "-lomx_aac_component_lib"
#define pvmtpip_engine_y_lib ""
#define pvmp4decoder_m_lib "-lpvmp4decoder"
#define LIBS_oscl_static " -lunit_test"
#define omx_avcenc_component_y_mk ""
#define aacdecnode_y_mk ""
#define pvasfffrecognizer_y_mk ""
#define wmadecoder_y_lib ""
#define oscllib_lib "-loscllib"
#define pvclientserversocketnode_y_lib ""
#define pvrmff_m_lib ""
#define pvmedialayernode_opencore_m_lib ""
#define DYNAMIC_LOAD_OMX_AMR_COMPONENT 1
#define SOLIBS_opencore_mp4local "-lpvmp4interface  -lpvmp4ffparsernode"
#define pvavch264enc_imp_m_lib ""
#define pvasfff_m_lib ""
#define LIBS_omxdec_static "         "
#define DYNAMIC_LOAD_OMX_AVCENC_COMPONENT 1
#define REGISTER_OMX_WMA_COMPONENT 0
#define SOLIBS_opencore_net_support "-lpvsocketnode -lpv_http_parcom -lprotocolenginenode_base -lprotocolenginenode_common -lprotocolenginenode_download_common -lprotocolenginenode_pdl -lprotocolenginenode_ps   "
#define pv_http_parcom_m_lib "-lpv_http_parcom"
#define pv_rtsp_parcom_m_mk "/protocols/rtsp_parcom/build/make"
#define omx_wma_component_y_lib ""
#define csprng_m_lib ""
#define LIBS_fileformats_shared "-lpvfileparserutils -lpvid3parcom -lpvpvxparser -lpvwav  -lpvavifileparser  -lpvmp3ff -lpvaacparser -lpvgsmamrparser   -lmp4recognizer_utility -lpvmp4ff  -lpvmp4ffcomposer    "
#define LIBS_engines_static "   "
#define pvmp3_m_lib "-lpvmp3"
#define SOLIBDIRS_opencore_player "        /pvmi/content_policy_manager/build/make /pvmi/content_policy_manager/plugins/oma1/passthru/build/make /fileformats/pvx/parser/build/make /fileformats/id3parcom/build/make /nodes/pvamrffparsernode/build/make /pvmi/recognizer/plugins/pvamrffrecognizer/build/make /fileformats/mp3/parser/build/make  /nodes/pvmp3ffparsernode/build/make /pvmi/recognizer/plugins/pvmp3ffrecognizer/build/make  /fileformats/mp4/parser/build_opencore/make /fileformats/mp4/parser/utils/mp4recognizer/build/make /fileformats/rawaac/parser/build/make /nodes/pvaacffparsernode/build/make  /pvmi/recognizer/plugins/pvaacffrecognizer/build/make /nodes/pvwavffparsernode/build/make /pvmi/recognizer/plugins/pvwavffrecognizer/build/make           /pvmi/recognizer/build/make   /engines/adapters/player/framemetadatautility/build/make /engines/player/build/make"
#define SOLIBS_pvprotocolengine_asfstreaming " "
#define pvmimeutils_m_mk "/baselibs/pv_mime_utils/build/make"
#define SOLIBS_omx_amrenc_sharedlibrary "-lomx_amrenc_component_lib -lpvencoder_gsmamr"
#define pvprotocolenginenode_ps_plugin_y_mk ""
#define pvsqlite_so_name ""
#define pvomxbasedecnode_m_mk "/nodes/pvomxbasedecnode/build/make"
#define pvamrwbdecoder_imp_m_lib ""
#define oscl_m_lib "-losclbase -losclerror -losclmemory -losclproc -losclregcli -losclregserv -losclutil -losclio -lunit_test"
#define pvasfffrecognizer_y_lib ""
#define pvmediainputnode_y_mk ""
#define omx_common_m_lib "-lomx_common_lib"
#define pvprotocolenginefasttrackdownloadinterface_m_mk ""
#define omx_wma_component_m_lib ""
#define protocolenginenode_base_y_lib ""
#define pvwav_m_mk "/fileformats/wav/parser/build/make"
#define pvjitterbufferasf_m_mk ""
#define pvmp4ffcomposeropencore_y_lib ""
#define pvmp4ffparsernodeopencore_y_lib ""
#define LIBS_baselibs_shared "   -lpvgendatastruct -lpvmediadatastruct -lpvmimeutils -lthreadsafe_callback_ao -lpvthreadmessaging"
#define MODS_pvmtp "-lopencore_player -lopencore_common -lpvsqlite -lpvdb -lpvwmdrm -lpvasfcommon -lpvasflocalpbreg"
#define pvlatmpayloadparser_m_mk "/protocols/rtp_payload_parser/util/build/latmparser/make"
#define pvid3parcom_m_lib "-lpvid3parcom"
#define pvprotocolenginenode_wmhttpstreaming_plugin_in_registry_m_lib ""
#define LIBDIR_fileformats_static "                    "
#define LIBS_cpm_shared "-lcpm -lpassthru_oma1  "
#define pvra8decoder_m_mk ""
#define LIBS_pvmi_static "                     "
#define pv_amr_nb_common_lib_m_mk "/codecs_v2/audio/gsm_amr/amr_nb/common/build/make"
#define pvavcdecnode_y_lib ""
#define pvcrypto_y_lib ""
#define pvavcdecnode_y_mk ""
#define LIBDIR_cpm_static "    "
#define LIBDIR_omxjointimp_static "n"
#define pvjitterbuffernode_y_lib ""
#define MODS_opencore_common ""
#define pvdownloadmanagernode_m_mk "/nodes/pvdownloadmanagernode/build/make"
#define pviptransport_m_mk ""
#define pvmediaoutputnode_y_mk ""
#define pvwmvdecnode_m_lib ""
#define pvjitterbufferrtp_m_mk "/nodes/streaming/jitterbuffernode/jitterbuffer/rtp/build/make"
#define MODS_omx_aacdec_sharedlibrary "-lomx_sharedlibrary -lopencore_common"
#define LIBS_packetsources_static "n"
#define pvstreamingmanagernode_m_lib ""
#define pvwmdrmplatinterface_m_lib ""
#define sdp_common_m_mk "/protocols/sdp/common/build/make"
#define LIBDIR_omxdecimp_shared ""
#define pvlatmpayloadparser_m_lib "-lpvlatmpayloadparser"
#define pv2wayengine_m_lib "-lpv2wayengine"
#define pvrtspreginterface_m_lib "-lpvrtspreginterface"
#define pvamrffrecognizer_m_lib "-lpvamrffrecognizer"
#define omx_wmv_component_m_mk ""
#define mp3decnode_y_mk ""
#define pvjitterbufferasf_y_lib ""
#define protocolenginenode_segments_m_mk "/nodes/pvprotocolenginenode/base/build/make /nodes/pvprotocolenginenode/protocol_common/build/make /nodes/pvprotocolenginenode/download_protocols/common/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_download/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_streaming/build/make   "
#define asfrecognizer_utility_m_lib ""
#define pvdecoder_gsmamr_y_mk ""
#define pviptransport_m_lib ""
#define LIBS_codecs_v2_shared "-lomx_avc_component_lib -lomx_m4v_component_lib  -lomx_aac_component_lib -lomx_amr_component_lib -lomx_mp3_component_lib  -lomx_amrenc_component_lib -lomx_m4venc_component_lib -lomx_avcenc_component_lib -lomx_common_lib -lomx_queue_lib -lpvomx_proxy_lib -lomx_baseclass_lib -lomx_mastercore_lib -lpv_omx_interface   -lpv_aac_dec -lpv_amr_nb_common_lib -lpvamrwbdecoder -lpvdecoder_gsmamr -lpvmp3  -lpvencoder_gsmamr -lpv_avc_common_lib -lpvavcdecoder  -lpvmp4decoder  -lpvm4vencoder -lpvavch264enc -lm4v_config -lpv_config_parser -lcolorconvert"
#define pvdecoder_gsmamr_imp_m_lib ""
#define omx_mastercore_m_mk "/codecs_v2/omx/omx_mastercore/build/make_multithreaded"
#define pvprotocolenginenode_ftdl_plugin_m_mk ""
#define omx_avc_component_m_mk "/codecs_v2/omx/omx_h264/build/make_multithreaded"
#define pvmf_y_mk ""
#define getactualaacconfig_imp_m_lib "-lgetactualaacconfig"
#define pvencoder_gsmamr_imp_m_mk ""
#define LIBDIR_recognizer_static "         "
#define cpm_m_lib "-lcpm"
#define pvmp4ffcomposeropencore_m_lib "-lpvmp4ffcomposer"
#define MODS_opencore_author "-lopencore_common"
#define wmvdecoder_imp_m_mk "n"
#define pvamrffparsernode_y_lib ""
#define pvmioaviwavfileinput_m_lib "-lpvmioaviwavfileinput"
#define pv324m_plugins "default_support.mk"
#define MODS_omx_wmadec_sharedlibrary "-lomx_sharedlibrary -lopencore_common"
#define REGISTER_OMX_WMV_COMPONENT 0
#define omx_m4v_component_m_lib "-lomx_m4v_component_lib"
#define pvprotocolenginenode_ps_plugin_in_registry_y_mk ""
#define pvsocketnode_m_mk "/nodes/pvsocketnode/build/make"
#define omx_avcdec_sharedlibrary_so_name "omx_avcdec_sharedlibrary"
#define pvdownloadmanagernode_m_lib "-lpvdownloadmanagernode"
#define packetsources_default_m_mk ""
#define pvmp4ffrecognizer_m_lib "-lpvmp4ffrecognizer"
#define pvprotocolenginefasttrackdownloadpluginreginterface_m_mk ""
#define pvmp4ffcomposer_m_lib ""
#define rdt_parser_m_mk ""
#define pvencoder_gsmamr_m_mk "/codecs_v2/audio/gsm_amr/amr_nb/enc/build/make"
#define pv_rtsp_parcom_y_mk ""
#define pvmio_comm_loopback_m_lib "-lpvmio_comm_loopback"
#define omx_wmv_component_y_lib ""
#define omx_m4vdec_sharedlibrary_so_name "omx_m4vdec_sharedlibrary"
#define LIBDIR_media_io_static "   "
#define SOLIBDIRS_omx_wmvdec_sharedlibrary " "
#define MODS_pvwmdrmdev "-lopencore_player -lopencore_common"
#define pvprotocolenginenode_ps_plugin_y_lib ""
#define pvasflocalpbinterface_m_mk ""
#define SOLIBDIRS_opencore_download "/nodes/pvdownloadmanagernode/build/make /modules/linux_download/core/build/make"
#define pvmp4ffparsernode_y_mk ""
#define pvwmdrmplat_so_name ""
#define omx_m4venc_component_imp_m_mk ""
#define SOLIBDIRS_omx_wmadec_sharedlibrary " "
#define LIBDIR_protocols_static "             "
#define LIBDIR_codecs_utilities_shared "/codecs_v2/utilities/m4v_config_parser/build/make /codecs_v2/utilities/pv_config_parser/build/make /codecs_v2/utilities/colorconvert/build/make"
#define pv_avc_common_lib_m_lib "-lpv_avc_common_lib"
#define pvrmffparsernode_y_mk ""
#define LIBS_extern_libs_static "    "
#define pvmp3ff_m_lib "-lpvmp3ff"
#define pvstreamingmanagernode_segments_y_lib ""
#define pvmp3_y_lib ""
#define opencore_download_so_name "opencore_download"
#define pvaacparser_y_lib ""
#define MODS_opencore_rtsp "-lopencore_net_support -lopencore_player -lopencore_common"
#define pv324m_common_headers_m_mk "/protocols/systems/common/build/make/"
#define REGISTER_OMX_AMR_COMPONENT 1
#define pvrtsp_cli_eng_node_opencore_y_mk ""
#define pv_avc_common_imp_lib_m_mk ""
#define engines_common_headers_m_mk "/engines/common/build/make"
#define SOLIBS_pvasfstreamingreg "n"
#define pvrtsp_cli_eng_node_opencore_y_lib ""
#define LIBDIR_omxencimp_static ""
#define pvrmffparser_y_mk ""
#define SOLIBS_pvfasttrack_downloadreg "n"
#define pvprotocolenginenode_pdl_plugin_m_lib ""
#define MODS_omx_m4venc_sharedlibrary "-lomx_sharedlibrary -lopencore_common "
#define pvjitterbufferrtp_m_lib "-lpvjitterbufferrtp"
#define pvmp4ff_y_mk ""
#define pvvideoparsernode_m_mk "/nodes/pvvideoparsernode/build/make"
#define LIBDIR_oscl_shared "/oscl "
#define cpm_headers_y_mk ""
#define pv_aac_dec_imp_m_mk ""
#define omx_wmv_component_imp_m_lib "n"
#define DYNAMIC_LOAD_OMX_H263ENC_COMPONENT 1
#define omx_m4v_component_imp_m_mk ""
#define LIBDIR_media_io_shared "/pvmi/media_io/pvmiofileoutput/build/make /pvmi/media_io/pvmi_mio_fileinput/build/make_pvauthor /pvmi/media_io/pvmi_mio_avi_wav_fileinput/build/make /pvmi/media_io/pvmio_comm_loopback/build/make"
#define pvamrffparsernode_m_mk "/nodes/pvamrffparsernode/build/make"
#define pvasfffparsernode_y_mk ""
#define pv_avc_common_lib_m_mk "/codecs_v2/video/avc_h264/common/build/make"
#define pv_aac_dec_plugins "aacdec_util.mk"
#define pvomxbasedecnode_y_lib ""
#define pvmedialayernode_opencore_y_lib ""
#define colorconvert_m_mk "/codecs_v2/utilities/colorconvert/build/make"
#define threadsafe_callback_ao_m_lib "-lthreadsafe_callback_ao"
#define opencore_rtsp_so_name "opencore_rtsp"
#define LIBDIR_codecs_utilities_static "  "
#define pvauthorengine_m_mk "/engines/author/build/make"
#define SOLIBDIRS_pvasfcommon " "
#define pvomxvideodecnode_y_mk ""
#define pvrtsp_cli_eng_node_3gpp_y_lib ""
#define pvmediadatastruct_y_mk ""
#define pvasfffrecognizer_m_lib ""
#define pvrmffrecognizer_y_mk ""
#define SOLIBDIRS_opencore_rtsp "/modules/linux_rtsp/core/build/make /nodes/streaming/streamingmanager/build/make_segments /protocols/rtsp_parcom/build/make /protocols/rtsp_client_engine/build_opencore/make /protocols/rtp_payload_parser/build/make /protocols/rtp/build/make /nodes/streaming/jitterbuffernode/jitterbuffer/common/build/make /nodes/streaming/jitterbuffernode/jitterbuffer/rtp/build/make /nodes/streaming/jitterbuffernode/build/make /nodes/streaming/medialayernode/build/make /protocols/sdp/parser/build/make /protocols/sdp/common/build/make"
#define omx_avcenc_component_y_lib ""
#define rtprtcp_y_mk ""
#define pvmp4reginterface_m_mk "/modules/linux_mp4/node_registry/build/make"
#define omx_m4venc_component_y_mk ""
#define LIBDIR_module "/modules"
#define pvpvxparser_m_mk "/fileformats/pvx/parser/build/make"
#define pv_omx_interface_m_lib "-lpv_omx_interface"
#define SOLIBS_pvwmdrmplat "n"
#define pvrmffrecognizer_m_lib ""
#define mp4recognizer_utility_m_mk "/fileformats/mp4/parser/utils/mp4recognizer/build/make"
#define pvamrffrecognizer_m_mk "/pvmi/recognizer/plugins/pvamrffrecognizer/build/make"
#define pvasflocalpbreginterface_m_lib ""
#define colorconvert_m_lib "-lcolorconvert"
#define pvprotocolengine_asfstreaming_so_name ""
#define wmvdecoder_imp_m_lib "n"
#define pvmtp_engine_y_mk ""
#define LIBDIR_video_shared "/codecs_v2/video/avc_h264/common/build/make /codecs_v2/video/avc_h264/dec/build/make  /codecs_v2/video/m4v_h263/dec/build/make  /codecs_v2/video/m4v_h263/enc/build/make /codecs_v2/video/avc_h264/enc/build/make"
#define pvasfff_m_mk ""
#define omx_queue_m_mk "/codecs_v2/omx/omx_queue/build/make"
#define DYNAMIC_LOAD_OMX_AAC_COMPONENT 1
#define DYNAMIC_LOAD_OMX_AVC_COMPONENT 1
#define pvgendatastruct_y_mk ""
#define pvmiofileoutput_y_lib ""
#define pvdownloadreginterface_m_lib "-lpvdownloadreginterface"
#define pvrvdecnode_y_mk ""
#define pvjanusplugin_m_mk ""
#define LIBS_static " -lunit_test                                                                                                                                                                       "
#define pvmedialayernode_plugins_opencore_rtsp "ml_rtsp.mk"
#define REGISTER_OMX_M4V_COMPONENT 1
#define MODS_opencore_net_support "-lopencore_common"
#define pvcrypto_y_mk ""
#define pvdummyoutputnode_y_lib ""
#define pvm4vencoder_y_mk ""
#define asfrecognizer_utility_y_lib ""
#define pvmp3ff_m_mk "/fileformats/mp3/parser/build/make"
#define pvomx_proxy_y_lib ""
#define pvmiofileinput_m_mk "/pvmi/media_io/pvmi_mio_fileinput/build/make_pvauthor"
#define pvasfstreaminginterface_m_lib ""
#define oscl_y_lib ""
#define SOLIBDIRS_pvsqlite "n"
#define LIBDIR_audio_shared "  /codecs_v2/audio/aac/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/common/build/make /codecs_v2/audio/gsm_amr/amr_wb/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/dec/build/make /codecs_v2/audio/mp3/dec/build/make  /codecs_v2/audio/gsm_amr/common/dec/build/make /codecs_v2/audio/gsm_amr/amr_nb/enc/build/make"
#define SOLIBS_pv "-losclbase -losclerror -losclmemory -losclproc -losclregcli -losclregserv -losclutil -losclio -lunit_test  -loscllib  -lpv_aac_dec -lpvmp3  -lpvdecoder_gsmamr -lpvencoder_gsmamr -lpv_amr_nb_common_lib -lpvamrwbdecoder -lpvavcdecoder -lpv_avc_common_lib -lpvavch264enc  -lpvmp4decoder -lpvm4vencoder  -lgetactualaacconfig -lcolorconvert -lm4v_config -lpv_config_parser -lomx_avc_component_lib -lomx_m4v_component_lib  -lomx_aac_component_lib -lomx_amr_component_lib -lomx_mp3_component_lib  -lomx_common_lib -lomx_queue_lib -lpvomx_proxy_lib -lomx_baseclass_lib -lomx_mastercore_lib -lpv_omx_interface -lthreadsafe_callback_ao -lpvmediadatastruct -lpvmimeutils -lpvgendatastruct -lpvmf -lcpm -lpassthru_oma1 -lpvmiofileoutput -lpvfileparserutils -lpvid3parcom  -lpvmp4ff -lmp4recognizer_utility  -lpvmp4ffcomposer  -lpvmp4ffcomposernode  -lpvpvxparser -lpvmediainputnode -lpvmediaoutputnode -lpvfileoutputnode -lpvgsmamrparser -lpvamrffparsernode -lpvamrffrecognizer -lpvaacparser -lpvaacffparsernode  -lpvaacffrecognizer -lpvmp3ff  -lpvmp3ffparsernode -lpvmp3ffrecognizer    -lpvomxvideodecnode -lpvomxaudiodecnode -lpvomxbasedecnode        -lpvwavffparsernode -lpvwavffrecognizer     -lpvmfrecognizer -lpvmiofileinput -lpvmioaviwavfileinput -lpvframemetadatautility -lpvplayer_engine -lpvauthorengine -lpv324m -lpv2wayengine -lpvavifileparser -lpvthreadmessaging -lpvlatmpayloadparser  "
#define pvsdpparser_opencore_m_lib "-lpvsdpparser"
#define protocolenginenode_m_lib ""
#define omx_amr_component_y_mk ""
#define SOLIBS_omx_wmadec_sharedlibrary " "
#define pvomxaudiodecnode_m_lib "-lpvomxaudiodecnode"
#define pv324m_y_lib ""
#define pvprotocolenginenode_ftdl_plugin_y_mk ""
#define unit_test_y_mk "/oscl/unit_test/build/make"
#define getactualaacconfig_y_mk ""
#define pvvideoparsernode_y_lib ""
#define pvoma1lockstream_m_lib ""
#define pvprotocolenginenode_shoutcast_plugin_m_mk ""
#define pvmf_y_lib ""
#define pv_config_parser_y_lib ""
#define m4v_config_m_lib "-lm4v_config"
#define pvaacffrecognizer_m_lib "-lpvaacffrecognizer"
#define cpm_y_mk ""
#define pvwmdrm_m_lib ""
#define wmvdecoder_y_mk ""
#define opencore_mp4localreg_so_name "opencore_mp4localreg"
#define SOLIBDIRS_pvfasttrack_download "/protocols/sdp/parser/build/make  "
#define pvomxvideodecnode_m_mk "/nodes/pvomxvideodecnode/build/make"
#define pvmp4ffcomposernodeopencore_m_mk "/nodes/pvmp4ffcomposernode/build_opencore/make"
#define pvwmdrm_y_mk ""
#define pvomxaudiodecnode_y_mk ""
#define pvra8decoder_y_mk ""
#define m4v_config_y_mk ""
#define pvthreadmessaging_y_lib ""
#define pvthreadmessaging_m_mk "/baselibs/thread_messaging/build/make"
#define pvjitterbuffercommon_m_lib "-lpvjitterbuffer"
#define SOLIBS_pvasfstreaming " -lpvstreamingmanagernode -lpvjitterbuffer  -lpvjitterbuffernode -lpvmedialayernode -lrtppayloadparser -lrtprtcp"
#define LIBS_pvmi_shared "-lcpm -lpassthru_oma1   -lpvmiofileoutput -lpvmiofileinput -lpvmioaviwavfileinput -lpvmio_comm_loopback -lpvmfrecognizer -lpvaacffrecognizer -lpvamrffrecognizer   -lpvmp3ffrecognizer -lpvmp4ffrecognizer -lpvwavffrecognizer    -lpvmf  "
#define pvcommsionode_m_mk "/nodes/pvcommsionode/build/make"
#define pvm4vencoder_imp_m_mk ""
#define omx_mp3_component_imp_m_lib ""
#define pvstreamingmanagernode_m_mk ""
#define DYNAMIC_LOAD_OMX_H263_COMPONENT 1
#define pvomxencnode_y_lib ""
#define omx_wma_component_imp_m_lib "n"
#define pvvideoencnode_y_lib ""
#define pvpvxparser_y_lib ""
#define pvra8decoder_y_lib ""
#define MODS_pvasflocalpbreg "-lopencore_common -lpvasfcommon"
#define pvrmffparsernode_m_lib ""
#define pv_divxfile_parser_m_mk ""
#define LIBS_media_io_static "   "
#define pvdummyoutputnode_m_lib "-lpvdummyoutputnode"
#define pv_amr_nb_common_imp_lib_m_mk ""
#define pvrmff_y_lib ""
#define SOLIBS_opencore_player "        -lcpm -lpassthru_oma1 -lpvpvxparser -lpvid3parcom -lpvamrffparsernode -lpvamrffrecognizer -lpvmp3ff  -lpvmp3ffparsernode -lpvmp3ffrecognizer  -lpvmp4ff -lmp4recognizer_utility -lpvaacparser -lpvaacffparsernode  -lpvaacffrecognizer -lpvwavffparsernode -lpvwavffrecognizer           -lpvmfrecognizer   -lpvframemetadatautility -lpvplayer_engine"
#define pvjanusplugininterface_m_mk ""
#define pvmedialayernode_plugins_pvasfstreaming "ml_asf.mk"
#define pvvideoencnode_m_lib ""
#define USE_OMX_ENC_NODE 1
#define mp4recognizer_utility_y_lib ""
#define pvm4vencoder_m_lib "-lpvm4vencoder"
#define LIBS_codecs_utilities_shared "-lm4v_config -lpv_config_parser -lcolorconvert"
#define opencore_2way_so_name "opencore_2way"
#define omx_wmvdec_sharedlibrary_so_name ""
#define opencore_rtspreg_so_name "opencore_rtspreg"
#define getactualaacconfig_m_lib "-lgetactualaacconfig"
#define MODS_pvprotocolengine_asfstreaming "-lopencore_net_support -lopencore_player -lopencore_common -lpvasfcommon"
#define pv324m_m_mk "/protocols/systems/3g-324m_pvterminal/build/make/"
#define omx_amrenc_component_y_lib ""
#define pvaacffparsernode_m_mk "/nodes/pvaacffparsernode/build/make"
#define pvm4vencoder_imp_m_lib ""
#define rtppayloadparser_y_lib ""
#define pvrmffparser_y_lib ""
#define pvjanusplugin_m_lib ""
#define wmvdecoder_m_mk ""
#define packetsources_default_m_lib ""
#define omx_amrenc_component_imp_m_lib ""
#define pvaacffrecognizer_y_mk ""
#define pvwmdrmsysclkinterface_m_mk ""
#define LIBS_omxdecimp_static ""
#define pv2wayengine_m_mk "/engines/2way/build/make"
#define MODS_opencore_rtspreg "-lopencore_player -lopencore_common"
#define pvomx_proxy_m_lib "-lpvomx_proxy_lib"
#define SOLIBS_opencore_download "-lpvdownloadmanagernode -lpvdownloadinterface"
#define LIBDIR_nodes_static "                                                           "
#define asfrecognizer_utility_m_mk ""
#define pvframemetadatautility_m_mk "/engines/adapters/player/framemetadatautility/build/make"
#define realaudio_deinterleaver_m_mk ""
#define LIBDIR_baselibs_static "       "
#define pvwav_m_lib "-lpvwav"
#define pvmp4decoder_imp_m_lib ""
#define pv_rtsp_parcom_y_lib ""
#define LIBDIR_static " /oscl/unit_test/build/make                                                                                                                                                                          "
#define config_3gpp_pvr_asf_mk ""
#define pvpvrff_m_mk ""
#define omx_mastercore_m_lib "-lomx_mastercore_lib"
#define pv_config_parser_y_mk ""
#define pvmtpip_so_name ""
#define pvfileoutputnode_m_mk "/nodes/pvfileoutputnode/build/make"
#define wmadecoder_imp_m_lib "n"
#define pvstreamingmanagernode_segments_m_mk "/nodes/streaming/streamingmanager/build/make_segments"
#define oscl_m_mk "/oscl"
#define pvprotocolenginenode_pdl_plugin_in_registry_m_mk "/nodes/pvprotocolenginenode/download_protocols/progressive_download/build/make"
#define pvra8decnode_m_mk ""
#define SOLIBDIRS_pvasflocalpbreg " "
#define pvamrencnode_y_mk ""
#define pvrvdecnode_m_lib ""
#define omx_m4venc_component_imp_m_lib ""
#define pvrmffrecognizer_y_lib ""
#define pvamrffrecognizer_y_lib ""
#define LIBS_omxjointimp_shared "-lomx_mastercore_lib"
#define pvmediaoutputnode_m_mk "/nodes/pvmediaoutputnode/build/make"
#define pvavcencnode_m_lib ""
#define REGISTER_OMX_M4VENC_COMPONENT 1
#define pvpvrff_y_mk ""
#define pvpvrnode_y_mk ""
#define omx_baseclass_y_mk ""
#define pvamrwbdecoder_m_mk "/codecs_v2/audio/gsm_amr/amr_wb/dec/build/make"
#define pvplayer_engine_y_mk ""
#define pvamrwbdecoder_y_lib ""
#define pvsqlite_m_lib ""
#define LIBDIR_tools_v2_shared "/modules/linux_rtsp/core/build/make /modules/linux_rtsp/node_registry/build/make   /modules/linux_download/core/build/make /modules/linux_download/node_registry/build/make /modules/linux_mp4/core/build/make /modules/linux_mp4/node_registry/build/make      "
#define pvamrencnode_y_lib ""
#define wmadecoder_y_mk ""
#define pvmtpip_engine_m_lib ""
#define protocolenginenode_m_mk ""
#define pvmedialayernode_y_lib ""
#define pvdivxffrecognizer_m_mk ""
#define SOLIBDIRS_omx_avcenc_sharedlibrary "/codecs_v2/omx/omx_h264enc/build/make_multithreaded /codecs_v2/video/avc_h264/enc/build/make"
#define pvmp4ffcomposernodeopencore_y_lib ""
#define scsp_y_mk ""
#define pventropysrc_y_lib ""
#define pvvideoencnode_y_mk ""
#define protocolenginenode_segments_m_lib "-lprotocolenginenode_base -lprotocolenginenode_common -lprotocolenginenode_download_common -lprotocolenginenode_pdl -lprotocolenginenode_ps   "
#define pvmp4ffcomposernode_m_lib ""
#define pvloopbacknode_y_mk ""
#define pvwmdrm_m_mk ""
#define SOLIBS_omx_wmvdec_sharedlibrary " "
#define MODS_pvasfstreamingreg "-lopencore_player -lopencore_common"
#define omx_mp3_component_m_mk "/codecs_v2/omx/omx_mp3/build/make_multithreaded"
#define pvmp4ffcomposernode_m_mk ""
#define pv_divxfile_parser_m_lib ""
#define omx_mp3_component_m_lib "-lomx_mp3_component_lib"
#define pvaacparser_m_lib "-lpvaacparser"
#define SOLIBDIRS_pvjanus " "
#define pvwavffparsernode_m_mk "/nodes/pvwavffparsernode/build/make"
#define pvavifileparser_m_lib "-lpvavifileparser"
#define LIBS_omxencimp_static ""
#define MODS_opencore_2way "-lopencore_common"
#define omx_baseclass_y_lib ""
#define LIBDIR_fileformats_shared "/fileformats/common/parser/build/make /fileformats/id3parcom/build/make /fileformats/pvx/parser/build/make /fileformats/wav/parser/build/make  /fileformats/avi/parser/build/make  /fileformats/mp3/parser/build/make /fileformats/rawaac/parser/build/make /fileformats/rawgsmamr/parser/build/make    /fileformats/mp4/parser/utils/mp4recognizer/build/make /fileformats/mp4/parser/build_opencore/make  /fileformats/mp4/composer/build_opencore/make    "
#define pvdummyoutputnode_m_mk "/nodes/pvdummyoutputnode/build/make"
#define SOLIBDIRS_omx_aacdec_sharedlibrary "/codecs_v2/omx/omx_aac/build/make_multithreaded /codecs_v2/audio/aac/dec/build/make"
#define pv_http_parcom_m_mk "/protocols/http_parcom/build/make"
#define LIBDIR_packetsources_shared "n"
#define pvfileparserutils_y_lib ""
#define LIBDIR_omxjoint_static "   "
#define pvasf_streaming_so_name ""
#define pv_amr_nb_common_lib_m_lib "-lpv_amr_nb_common_lib"
#define pvmiofileoutput_y_mk ""
#define MODS_opencore_mp4local "-lopencore_common -lopencore_player"
#define SOLIBS_pvwmdrmdev "n"
#define pvmp3ff_y_lib ""
#define pvmiofileinput_y_lib ""
#define LIBS_fileformats_static "                   "
#define pvmp3ffparsernode_y_mk ""
#define SOLIBDIRS_opencore_author " /fileformats/mp4/composer/build_opencore/make  /nodes/pvmp4ffcomposernode/build_opencore/make /engines/author/build/make"
#define pvavch264enc_y_mk ""
#define LIBS_recognizer_shared "-lpvmfrecognizer -lpvaacffrecognizer -lpvamrffrecognizer   -lpvmp3ffrecognizer -lpvmp4ffrecognizer -lpvwavffrecognizer  "
#define pvstreamingmanagernode_y_lib ""
#define pvstreamingmanagernode_3gpp_m_mk ""
#define pvsqlite_y_lib ""
#define protocolenginenode_base_m_lib "-lprotocolenginenode_base"
#define opencore_player_so_name "opencore_player"
#define rvdecoder_y_mk ""
#define colorconvert_y_mk ""
#define pvoma1ffrecognizer_m_mk ""
#define packetsources_default_y_lib ""
#define pvwmvdecnode_m_mk ""
#define pvdivxffparsernode_m_mk ""
#define SOLIBDIRS_pvasflocalpb " "
#define gsmamrdecnode_m_lib ""
#define pv2wayengine_y_mk ""
#define pvavcdecnode_m_mk ""
#define pvra8decnode_m_lib ""
#define broadcastpvr_support_mk ""
#define pvmp4ffopencore_y_lib ""
#define SOLIBS_opencore_mp4localreg "-lpvmp4reginterface -lpvmp4ffrecognizer"
#define omx_m4venc_component_y_lib ""
#define pvpvrnode_m_lib ""
#define pvpvrff_m_lib ""
#define pvframemetadatautility_y_lib ""
#define MODS_pvjanus "-lopencore_player -lopencore_common -lpvwmdrm"
#define omx_avcenc_component_imp_m_mk ""
#define pvwmvdecnode_y_lib ""
#define pvavch264enc_y_lib ""
#define pvclientserversocketnode_m_mk "/nodes/pvclientserversocketnode/build/make"
#define LIBDIR_nodes_shared "/nodes/pvfileoutputnode/build/make /nodes/pvmediaoutputnode/build/make /nodes/pvsocketnode/build/make  /nodes/pvprotocolenginenode/base/build/make /nodes/pvprotocolenginenode/protocol_common/build/make /nodes/pvprotocolenginenode/download_protocols/common/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_download/build/make /nodes/pvprotocolenginenode/download_protocols/progressive_streaming/build/make      /nodes/pvwavffparsernode/build/make   /nodes/pvomxencnode/build/make /nodes/pvomxbasedecnode/build/make /nodes/pvomxaudiodecnode/build/make /nodes/pvomxvideodecnode/build/make  /nodes/pvaacffparsernode/build/make  /nodes/pvamrffparsernode/build/make   /nodes/pvmp3ffparsernode/build/make  /nodes/pvmp4ffparsernode/build_opencore/make     /nodes/common/build/make   /nodes/pvmediainputnode/build/make_pvauthor  /nodes/pvmp4ffcomposernode/build_opencore/make    /nodes/pvdownloadmanagernode/build/make   /nodes/streaming/streamingmanager/build/make_segments   /modules/linux_rtsp/core/build/make /modules/linux_rtsp/node_registry/build/make /nodes/streaming/medialayernode/build/make  /nodes/streaming/jitterbuffernode/jitterbuffer/common/build/make /nodes/streaming/jitterbuffernode/jitterbuffer/rtp/build/make  /nodes/streaming/jitterbuffernode/build/make /nodes/pvcommsionode/build/make /nodes/pvclientserversocketnode/build/make /nodes/pvloopbacknode/build/make /nodes/pvvideoparsernode/build/make /nodes/pvdummyinputnode/build/make /nodes/pvdummyoutputnode/build/make "
#define pvasfff_y_mk ""
#define getactualaacconfig_for_static_m_lib ""
#define SOLIBS_omx_aacdec_sharedlibrary "-lomx_aac_component_lib -lpv_aac_dec"
#define pvmp3ffrecognizer_y_lib ""
#define omx_amrenc_sharedlibrary_so_name "omx_amrenc_sharedlibrary"
#define SOLIBDIRS_omx_sharedlibrary "/codecs_v2/omx/omx_common/build/make_multithreaded /codecs_v2/omx/omx_queue/build/make /codecs_v2/omx/omx_proxy/build/make /codecs_v2/omx/omx_baseclass/build/make /codecs_v2/omx/omx_sharedlibrary/interface/build/make"
#define pvprotocolenginewmhttpstreamingpluginreginterface_m_mk ""
#define pvclientserversocketnode_m_lib "-lpvclientserversocketnode"
#define pvsqlite_m_mk ""
#define DYNAMIC_LOAD_OMX_MP3_COMPONENT 1
#define pvavcdecoder_y_mk ""
#define getactualaacconfig_y_lib ""
#define pvasf_streamingreg_so_name ""
#define omx_amrenc_component_y_mk ""
#define LIBS_omxjoint_static "   "
#define pvmio_comm_loopback_y_mk ""
#define pvmediaoutputnode_m_lib "-lpvmediaoutputnode"
#define pvra8decoder_m_lib ""
#define pvsdpparser_plugins " sdp_opencore.mk"
#define pvmtpip_engine_m_mk ""
#define pvlatmpayloadparser_y_mk ""
#define pvwavffrecognizer_m_mk "/pvmi/recognizer/plugins/pvwavffrecognizer/build/make"
#define omx_m4v_component_m_mk "/codecs_v2/omx/omx_m4v/build/make_multithreaded"
#define pvvideodecnode_y_lib ""
#define SOLIBDIRS_opencore_rtspreg "/modules/linux_rtsp/node_registry/build/make"
#define passthru_oma1_m_mk "/pvmi/content_policy_manager/plugins/oma1/passthru/build/make"
#define packetsources_default_plugins " "
#define omx_queue_y_lib ""
#define pvjanus_so_name ""
#define pvpvr_m_lib ""
#define pvasxparser_y_mk ""
#define shoutcast_support_flag ""
#define pvwmadecnode_m_lib ""
#define pvmp3ffparsernode_m_mk "/nodes/pvmp3ffparsernode/build/make"
#define pvdivxffrecognizer_y_lib ""
#define pvmedialayernode_opencore_y_mk ""
#define pvdownloadmanagernode_y_mk ""
#define pvdummyoutputnode_y_mk ""
#define omx_sharedlibrary_so_name "omx_sharedlibrary"
#define omx_avc_component_imp_m_mk ""
#define unit_test_m_lib ""
#define MODS_pvasfcommon "-lopencore_player -lopencore_common"
#define pventropysrc_y_mk ""
#define pvprotocolenginenode_shoutcast_plugin_in_registry_m_lib ""
#define pvomx_proxy_y_mk ""
#define pvasfstreamingreginterface_m_lib ""
#define pvmediainputnode_m_mk "/nodes/pvmediainputnode/build/make_pvauthor"
#define pvcommsionode_y_lib ""
#define rtppayloadparser_plugins_pvasfstreaming "asf_payload_parser.mk"
#define SOLIBS_pvmtpip " "
#define pv_avc_common_lib_y_lib ""
#define pvomxaudiodecnode_y_lib ""
#define SOLIBDIRS_pvfasttrack_downloadreg "n"
#define omx_wma_component_y_mk ""
#define omx_baseclass_m_lib "-lomx_baseclass_lib"
#define pvdummyinputnode_m_lib "-lpvdummyinputnode"
#define pvoma1ffrecognizer_y_mk ""
#define pvpvr_y_lib ""
#define pvwav_y_mk ""
#define pvwav_y_lib ""
#define pvra8decnode_y_lib ""
#define pvmiofileoutput_m_lib "-lpvmiofileoutput"
#define pvsdpparser_opencore_y_lib ""
#define protocolenginenode_y_lib ""
#define omx_m4venc_sharedlibrary_so_name "omx_m4venc_sharedlibrary"
#define pvmp3ffparsernode_m_lib "-lpvmp3ffparsernode"
#define pvgeneraltools_m_mk "/protocols/systems/tools/general/build/make"
#define pv_aac_dec_m_mk "/codecs_v2/audio/aac/dec/build/make"
#define SOLIBDIRS_pvwmdrmdev "n"
#define scsp_m_mk ""
#define pv_http_parcom_y_lib ""
#define LIBS_engines_shared "-lpvplayer_engine -lpvauthorengine -lpv2wayengine -lpvframemetadatautility"
#define SOLIBDIRS_pvprotocolengine_asfstreaming " "
#define omx_mp3dec_sharedlibrary_so_name "omx_mp3dec_sharedlibrary"
#define LIBDIR_video_static "      "
#define LIBDIR_omxenc_shared "/codecs_v2/omx/omx_amrenc/build/make_multithreaded /codecs_v2/omx/omx_m4venc/build/make_multithreaded /codecs_v2/omx/omx_h264enc/build/make_multithreaded"
#define opencore_net_support_so_name "opencore_net_support"
#define aacdecnode_y_lib ""
#define LIBS_nodes_static "                                                          "
#define pvavcdecoder_imp_m_mk ""
#define protocolenginenode_download_common_y_lib ""
#define pvomxvideodecnode_y_lib ""
#define SOLIBS_omx_mp3dec_sharedlibrary "-lomx_mp3_component_lib -lpvmp3"
#define realmedia_payload_mk ""
#define pvmtpip_engine_y_mk ""
#define MODS_omx_sharedlibrary "-lopencore_common"
#define pvrtsp_cli_eng_node_opencore_m_mk "/protocols/rtsp_client_engine/build_opencore/make"
#define pv_omx_interface_m_mk "/codecs_v2/omx/omx_sharedlibrary/interface/build/make"
#define pvjitterbuffercommon_m_mk "/nodes/streaming/jitterbuffernode/jitterbuffer/common/build/make"
#define pvavifileparser_y_lib ""
#define pvprotocolenginefasttrackdownloadinterface_m_lib ""
#define pvjitterbuffernode_m_lib "-lpvjitterbuffernode"
#define LIBDIR_omxdec_shared "/codecs_v2/omx/omx_h264/build/make_multithreaded /codecs_v2/omx/omx_m4v/build/make_multithreaded  /codecs_v2/omx/omx_aac/build/make_multithreaded /codecs_v2/omx/omx_amr/build/make_multithreaded /codecs_v2/omx/omx_mp3/build/make_multithreaded "
#define pvvideoparsernode_m_lib "-lpvvideoparsernode"
#define pvfasttrack_download_so_name ""
#define pvprotocolenginenode_ps_plugin_m_mk ""
#define pvmp4decoder_imp_m_mk ""
#define omx_m4v_component_y_mk ""
#define packetsources_default_y_mk ""
#define rdt_parser_m_lib ""
#define pvgsmamrparser_m_lib "-lpvgsmamrparser"
#define pvloopbacknode_y_lib ""
#define realaudio_deinterleaver_m_lib ""
#define cpm_headers_m_mk "/pvmi/content_policy_manager/plugins/common/build/make"
#define LIBS_cpm_static "   "
#define SOLIBDIRS_opencore_common "/oscl  /codecs_v2/omx/omx_mastercore/build/make_multithreaded             /codecs_v2/audio/gsm_amr/common/dec/build/make /codecs_v2/video/avc_h264/common/build/make /codecs_v2/audio/gsm_amr/amr_nb/common/build/make /fileformats/rawgsmamr/parser/build/make /codecs_v2/audio/aac/dec/util/getactualaacconfig/build/make /codecs_v2/utilities/m4v_config_parser/build/make /codecs_v2/utilities/pv_config_parser/build/make /codecs_v2/utilities/colorconvert/build/make /baselibs/threadsafe_callback_ao/build/make /baselibs/media_data_structures/build/make /baselibs/pv_mime_utils/build/make /baselibs/gen_data_structures/build/make /pvmi/pvmf/build/make /nodes/pvfileoutputnode/build/make /nodes/pvmediainputnode/build/make_pvauthor /nodes/pvomxencnode/build/make /pvmi/media_io/pvmi_mio_fileinput/build/make_pvauthor /pvmi/media_io/pvmi_mio_avi_wav_fileinput/build/make /fileformats/avi/parser/build/make /baselibs/thread_messaging/build/make /pvmi/media_io/pvmiofileoutput/build/make /nodes/pvmediaoutputnode/build/make /nodes/pvomxvideodecnode/build/make /nodes/pvomxaudiodecnode/build/make /nodes/pvomxbasedecnode/build/make /protocols/rtp_payload_parser/util/build/latmparser/make /fileformats/wav/parser/build/make /fileformats/common/parser/build/make /nodes/common/build/make /engines/common/build/make /pvmi/content_policy_manager/plugins/common/build/make"
#define rtprtcp_m_lib "-lrtprtcp"
#define pvdbmanager_m_lib ""
#define passthru_oma1_m_lib "-lpassthru_oma1"
#define omx_mastercore_y_mk ""
#define pvmp4ffcomposer_m_mk ""
#define pvasflocalpbreginterface_m_mk ""
#define pvasflocalpbinterface_m_lib ""
#define SOLIBS_omx_amrdec_sharedlibrary "-lomx_amr_component_lib -lpvdecoder_gsmamr -lpvamrwbdecoder"
#define pvprotocolenginenode_ps_plugin_in_registry_y_lib ""
#define optimized_bcast_ps_mk ""
#define pvplayer_engine_m_mk "/engines/player/build/make"
#define LIBDIR_engines_static "    "
#define LIBDIR_pvmi_static "                      "
#define pv324m_common_headers_y_mk ""
#define pvrtppacketsourcenode_m_lib ""
#define pvrtsp_cli_eng_node_3gpp_m_lib ""
#define passthru_oma1_y_lib ""
#define pvprotocolengine_asfstreamingreg_so_name ""
#define oscllib_mk "/oscl/oscl/oscllib/build/make"
#define threadsafe_callback_ao_m_mk "/baselibs/threadsafe_callback_ao/build/make"
#define threadsafe_callback_ao_y_lib ""
#define protocolenginenode_y_mk ""
#define REGISTER_OMX_H263ENC_COMPONENT 1
#define omx_baseclass_m_mk "/codecs_v2/omx/omx_baseclass/build/make"
#define LIBDIR_recognizer_shared "/pvmi/recognizer/build/make /pvmi/recognizer/plugins/pvaacffrecognizer/build/make /pvmi/recognizer/plugins/pvamrffrecognizer/build/make   /pvmi/recognizer/plugins/pvmp3ffrecognizer/build/make /pvmi/recognizer/plugins/pvmp4ffrecognizer/build/make /pvmi/recognizer/plugins/pvwavffrecognizer/build/make  "
#define pvvideoparsernode_y_mk ""
#define LIBS_baselibs_static "       "
#define pvgendatastruct_y_lib ""
#define pv_config_parser_m_lib "-lpv_config_parser"
#define pvmedialayernode_y_mk ""
#define pvdbmanager_y_mk ""
#define pv_divxfile_parser_y_lib ""
#define LIBDIR_extern_libs_static "    "
#define opencore_common_so_name "opencore_common"
#define pvprotocolenginenode_shoutcast_plugin_in_registry_y_mk ""
#define omx_mp3_component_y_mk ""
#define MODS_opencore_mp4localreg "-lopencore_player -lopencore_common"
#define rdt_parser_y_mk ""
#define LIBDIR_packetsources_static "n"
#define REGISTER_OMX_AVCENC_COMPONENT 1
#define pvamrwbdecoder_y_mk ""
#define pvrmffparsernode_y_lib ""
#define pvmp4ffparsernodeopencore_y_mk ""
#define pvmp4ffopencore_y_mk ""
#define colorconvert_y_lib ""
#define protocolenginenode_common_y_lib ""
#define pvencoder_gsmamr_m_lib "-lpvencoder_gsmamr"
#define rfc_2429_mk "rfc_2429.mk"
#define LIBS_protocols_static "            "
#define pvprotocolenginenode_ps_plugin_in_registry_m_lib "-lprotocolenginenode_ps"
#define omx_aacdec_sharedlibrary_so_name "omx_aacdec_sharedlibrary"
#define pv_so_name ""
#define asf_payload_mk ""
#define SOLIBDIRS_pvwmdrmsysclk "n"
#define protocolenginenode_segments_y_mk "       "
#define MODS_pvwmdrmsysclk "-lopencore_player -lopencore_common"
#define pvrmffrecognizer_m_mk ""
#define pvavch264enc_imp_m_mk ""
#define pvgeneraltools_m_lib "-lpvgeneraltools"
#define SOLIBDIRS_pvwmdrmplat "n"
#define pvstreamingmanagernode_plugins_opencore_rtsp "3gpp.mk rtspunicast.mk"
#define LIBS_omxjoint_shared "-lomx_common_lib -lomx_queue_lib -lpvomx_proxy_lib -lomx_baseclass_lib -lomx_mastercore_lib -lpv_omx_interface"
#define cpm_y_lib ""
#define wmadecoder_imp_m_mk "n"
#define MODS_opencore_downloadreg "-lopencore_player -lopencore_common"
#define pvmp3ffrecognizer_m_mk "/pvmi/recognizer/plugins/pvmp3ffrecognizer/build/make"
#define config_3gpp_mk "3gpp.mk"
#define pvmtp_engine_m_lib ""
#define pvrmffparsernode_m_mk ""
#define LIBS_omxjointimp_static "n"
#define SOLIBDIRS_omx_avcdec_sharedlibrary "/codecs_v2/omx/omx_h264/build/make_multithreaded /codecs_v2/video/avc_h264/dec/build/make"
#define omx_m4v_component_imp_m_lib ""
#define pvmp4ffcomposeropencore_y_mk ""
#define pvjanusplugininterface_m_lib ""
#define pvdivxffparsernode_m_lib ""
#define pvwmadecnode_m_mk ""
#define protocolenginenode_common_m_mk "/nodes/pvprotocolenginenode/protocol_common/build/make"
#define engines_common_headers_y_mk ""
#define pvmp4ffparsernode_y_lib ""
#define SOLIBDIRS_omx_mp3dec_sharedlibrary "/codecs_v2/omx/omx_mp3/build/make_multithreaded /codecs_v2/audio/mp3/dec/build/make"
#define pvmioaviwavfileinput_m_mk "/pvmi/media_io/pvmi_mio_avi_wav_fileinput/build/make"
#define pvdb_so_name ""
#define SOLIBDIRS_pvwmdrm "   "
#define pvloopbacknode_m_lib "-lpvloopbacknode"
#define pvdummyinputnode_y_mk ""
#define TARGET_shared " opencore_common opencore_author opencore_player opencore_2way omx_sharedlibrary omx_avcdec_sharedlibrary omx_m4vdec_sharedlibrary  omx_aacdec_sharedlibrary omx_amrdec_sharedlibrary omx_mp3dec_sharedlibrary  omx_avcenc_sharedlibrary omx_m4venc_sharedlibrary omx_amrenc_sharedlibrary opencore_net_support opencore_downloadreg opencore_download   opencore_rtspreg opencore_rtsp        opencore_mp4localreg opencore_mp4local         "
#define pvmf_m_lib "-lpvmf"
#define gsmamrdecnode_y_mk ""
#define pvasfffparsernode_y_lib ""
#define LIBS_recognizer_static "         "
#define pvmp4ffrecognizer_y_lib ""
#define pvmp4ffcomposernode_y_lib ""
#define gsmamrdecnode_y_lib ""
#define LIBS_packetsources_shared "n"
#define pviptransport_y_mk ""
#define pvid3parcom_y_mk ""
#define pvmp3_m_mk "/codecs_v2/audio/mp3/dec/build/make"
#define pvjanusplugin_y_lib ""
#define LIBS_codecs_utilities_static "  "
#define LIBDIR_omxjointimp_shared "/codecs_v2/omx/omx_mastercore/build/make_multithreaded"
#define pvavcdecnode_m_lib ""
#define unit_test_y_lib "-lunit_test"
#define pvrfileplayback_support_mk ""
#define omx_aac_component_imp_m_mk ""
#define omx_m4venc_component_m_mk "/codecs_v2/omx/omx_m4venc/build/make_multithreaded"
#define SOLIBDIRS_omx_m4vdec_sharedlibrary "/codecs_v2/omx/omx_m4v/build/make_multithreaded /codecs_v2/video/m4v_h263/dec/build/make"
#define unit_test_m_mk ""
#define pvomxaudiodecnode_m_mk "/nodes/pvomxaudiodecnode/build/make"
#define SOLIBS_omx_avcenc_sharedlibrary "-lomx_avcenc_component_lib -lpvavch264enc"
#define pvvideodecnode_m_lib ""
#define omx_common_m_mk "/codecs_v2/omx/omx_common/build/make_multithreaded"
#define pvdivxffparsernode_y_mk ""
#define MODS_pvwmdrm "-lopencore_player -lopencore_common"
#define pvavcencnode_y_mk ""
#define mshttp_support_mk ""
#define SOLIBS_omx_sharedlibrary "-lomx_common_lib -lomx_queue_lib -lpvomx_proxy_lib -lomx_baseclass_lib -lpv_omx_interface"
#define pvmediadatastruct_m_mk "/baselibs/media_data_structures/build/make"
#define MODS_omx_amrenc_sharedlibrary "-lomx_sharedlibrary -lopencore_common "
#define MODS_pvmtpip "-lopencore_player -lopencore_common -lpvsqlite -lpvdb -lpvwmdrm -lpvasfcommon -lpvasflocalpbreg"
#define omx_amr_component_y_lib ""
#define pvjanusplugin_y_mk ""
#define pvcrypto_m_lib ""
#define SOLIBS_pvsqlite "n"
#define pvrmff_m_mk ""
//
// That's all, folks!
