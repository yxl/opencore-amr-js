#
# Automatically generated, don't edit
#
# At: Thu, 26 Feb 2009 19:56:21 +0000
# Linux version 2.6.22-16-generic (buildd@rothera) (gcc version 4.1.3 20070929 (prerelease) (Ubuntu 4.1.2-16ubuntu2)) #1 SMP Mon Nov 24 18:28:27 GMT 2008

#
# PV Code Base Configuration System
#

#
# Menu for selecting supported features
#
module_support=y
shoutcast_support=n


#
# Menu for configuring runtime loadable modules
#
pv_so=n
pvsplit_so=y
streaming_support=y

#
# Menu for selecting streaming features
#
rtsp_support=y
asf_streaming_support=n

download_support=y
fasttrack_download_support=n
mp4local_support=y
asflocal_support=n
janus_support=n
wmdrmplat_support=n
wmdrmdev_support=n
wmdrmsysclk_support=n
mtp_db_support=n


#
# Menu for configuring Baselibs
#
csprng_lib=n
pvcrypto_lib=n
pventropysrc_lib=n
pvgendatastruct_lib=m
pvmediadatastruct_lib=m
pvmimeutils_lib=m
threadsafe_callback_ao_lib=m
pvthreadmessaging_lib=m


#
# Menu for configuring File Formats
#
pvasfff_lib=n
pvmp3ff_lib=m
pvmp4ffcomposer_lib=n
pvmp4ffcomposeropencore_lib=m
pvmp4ff_lib=n
pvmp4ffopencore_lib=m
mp4recognizer_utility_lib=m
pvaacparser_lib=m
pvgsmamrparser_lib=m
pvrmff_lib=n
pvrmffparser_lib=n
pvfileparserutils_lib=m
pvid3parcom_lib=m
pvpvxparser_lib=m
pvwav_lib=m
pvasxparser_lib=n
pvavifileparser_lib=m
pvpvrff_lib=n
asfrecognizer_utility_lib=n
pv_divxfile_parser_lib=n


#
# Menu for configuring Codecs
#

#
# Menu for configuring OMX Support
#
omx_mastercore_lib=m
MAX_NUMBER_OF_OMX_CORES=10
MAX_NUMBER_OF_OMX_COMPONENTS=50
pv_omx=y
omx_avc_component_lib=m
omx_common_lib=m
omx_m4v_component_lib=m
omx_queue_lib=m
omx_wmv_component_lib=n
pvomx_proxy_lib=m
omx_aac_component_lib=m
omx_amr_component_lib=m
omx_mp3_component_lib=m
omx_wma_component_lib=n
omx_amrenc_component_lib=m
omx_m4venc_component_lib=m
omx_avcenc_component_lib=m
omx_baseclass_lib=m


#
# Menu for configuring audio codecs
#
pv_aac_dec_lib=m
getactualaacconfig_lib=m
pv_amr_nb_common_lib=m
pvdecoder_gsmamr_lib=m
pvencoder_gsmamr_lib=m
pvamrwbdecoder_lib=m
gsm_amr_headers_lib=m
pvmp3_lib=m
pvra8decoder_lib=n
wmadecoder_lib=n


#
# Menu for configuring video codecs
#
pv_avc_common_lib=m
pvavcdecoder_lib=m
wmvdecoder_lib=n
pvmp4decoder_lib=m
rvdecoder_lib=n
pvm4vencoder_lib=m
pvavch264enc_lib=m


#
# Menu for configuring codecs utilities
#
m4v_config_lib=m
pv_config_parser_lib=m
colorconvert_lib=m



#
# Menu for configuring Nodes
#

#
# Menu for configuring Streaming
#
pvstreamingmanagernode_segments_lib=m

#
# Menu for configuring Streaming Features
#
mshttp_support=n
rtspunicast_support=y
unicastpvr_support=n
broadcastpvr_support=n
pvrfileplayback_support=n

pvstreamingmanagernode_lib=n
pvstreamingmanagernode_3gpp_lib=n
pvmedialayernode_lib=m
pvmedialayernode_opencore_lib=n
pvjitterbuffernode_lib=m

#
# Menu for configuring Jitter Buffer Node
#
rtpjitterbuffer_support=m
asfjitterbuffer_support=n



#
# Menu for configuring Download
#
pvdownloadmanagernode_lib=m

#
# Menu for configuring downloadmanager features
#
PVMF_DOWNLOADMANAGER_SUPPORT_PPB=y
PVMF_DOWNLOADMANAGER_MIN_TCP_BUFFERS_FOR_PPB=39
PVMF_DOWNLOADMANAGER_CACHE_SIZE_FOR_SC_IN_SECONDS=6
PVMF_DOWNLOADMANAGER_MAX_BITRATE_FOR_SC=128



#
# Menu for adding NON-OMX Nodes
#
aacdecnode_lib=n
mp3decnode_lib=n
pvavcdecnode_lib=n
pvwmadecnode_lib=n
pvwmvdecnode_lib=n
gsmamrdecnode_lib=n
pvvideodecnode_lib=n
pvvideoencnode_lib=n
pvamrencnode_lib=n
pvavcencnode_lib=n


#
# Menu for configuring ProtocolEngine
#
pvprotocolenginenode_segments_lib=m
pvprotocolenginenode_lib=n
pvprotocolenginenode_pdl_plugin_lib=n
pvprotocolenginenode_ps_plugin_lib=n
pvprotocolenginenode_ftdl_plugin_lib=n
pvprotocolenginenode_wmhttpstreaming_plugin_lib=n
pvprotocolenginenode_shoutcast_plugin_lib=n

#
# Menu for configuring protocol engine node plugin registry
#
BUILD_PROGRESSIVE_DOWNLOAD_PLUGIN=y
BUILD_PROGRESSIVE_STREAMING_PLUGIN=y
BUILD_FASTTRACK_DOWNLOAD_PLUGIN=n
BUILD_WMHTTPSTREAMING_PLUGIN=n
BUILD_SHOUTCAST_PLUGIN=n


pvfileoutputnode_lib=m
pvmediaoutputnode_lib=m
pvsocketnode_lib=m
pvwavffparsernode_lib=m
pvomxencnode_lib=m
pvomxaudiodecnode_lib=m
pvomxbasedecnode_lib=m
pvomxvideodecnode_lib=m
pvaacffparsernode_lib=m
pvamrffparsernode_lib=m
pvasfffparsernode_lib=n
pvmp3ffparsernode_lib=m
pvmp4ffparsernode_lib=n
pvmp4ffparsernodeopencore_lib=m
pvrvdecnode_lib=n
pvra8decnode_lib=n
pvrmffparsernode_lib=n
pvrtppacketsourcenode_lib=n
nodes_common_headers_lib=m
pvmediainputnode_lib=m
pvmp4ffcomposernode_lib=n
pvmp4ffcomposernodeopencore_lib=m
pvpvr_lib=n
pvpvrnode_lib=n
pvcommsionode_lib=m
pvclientserversocketnode_lib=m
pvloopbacknode_lib=m
pvvideoparsernode_lib=m
pvdummyinputnode_lib=m
pvdummyoutputnode_lib=m
pvdivxffparsernode_lib=n


#
# Menu for configuring Oscl
#
build_oscl=m
unit_test_lib=y


#
# Menu for configuring Protocols
#

#
# Menu for configuring Value Adds for 2way
#
twoway_value_add_config=y
PV_2WAY_VALUE_ADD_NONE=y

pv_http_parcom_lib=m
pvlatmpayloadparser_lib=m
sdp_common=m

#
# Menu for configuring SDPParser
#
sdp_default=n
sdp_opencore=m

rdt_parser_lib=n
pv_rtsp_parcom_lib=m
pvrtsp_cli_eng_node_lib=n
pvrtsp_cli_eng_node_3gpp_lib=n
pvrtsp_cli_eng_node_opencore_lib=m
rtppayloadparser_lib=m

#
# Menu for rtppayload parser plugins
#
rfc_2429=y
rfc_3016=y
rfc_3267=y
rfc_3640=y
rfc_3984=y
asf_payload=n
realmedia_payload=n

rtprtcp_lib=m
pv324m_lib=m
pv324m_common_headers_lib=m
pvgeneraltools_lib=m


#
# Menu for configuring Pvmi
#

#
# Menu for configuring Recognizers
#
pvmfrecognizer_lib=m
pvaacffrecognizer_lib=m
pvamrffrecognizer_lib=m
pvoma1ffrecognizer_lib=n
pvasfffrecognizer_lib=n
pvmp3ffrecognizer_lib=m
pvmp4ffrecognizer_lib=m
pvwavffrecognizer_lib=m
pvrmffrecognizer_lib=n
pvdivxffrecognizer_lib=n


#
# Menu for configuring Content Policy Manager
#
cpm_lib=m
passthru_oma1_lib=m
pvjanusplugin_lib=n
cpm_headers_lib=m
pvoma1lockstream_lib=n


#
# Menu for configuring Media IO
#
pvmiofileinput_lib=m
pvmiofileoutput_lib=m
pvmioaviwavfileinput_lib=m
pvmio_comm_loopback_lib=m


#
# Menu for configuring PacketSources
#
packetsources_default_lib=n

#
# Menu for configuring PacketSource Plugins
#
optimized_bcast_ps_support=n
standard_bcast_ps_support=n


pvmf_lib=m
realaudio_deinterleaver_lib=n
pvdbmanager_lib=n


#
# Menu for configuring Engines
#

#
# Menu for configuring Player
#
pvplayer_engine_lib=m

#
# Menu for player engine tunables
#
PVPLAYERENGINE_CONFIG_SKIPTOREQUESTEDPOS_DEF=y
PVPLAYERENGINE_CONFIG_SYNCMARGIN_EARLY_DEF=-200
PVPLAYERENGINE_CONFIG_SYNCMARGIN_LATE_DEF=200
VIDEO_DEC_NODE_LOW_PRIORITY=y


#
# Menu for configuring player registry
#
BUILD_OMX_VIDEO_DEC_NODE=y
BUILD_OMX_AUDIO_DEC_NODE=y
BUILD_VIDEO_DEC_NODE=n
BUILD_AVC_DEC_NODE=n
BUILD_WMV_DEC_NODE=n
BUILD_RV_DEC_NODE=n
BUILD_WMA_DEC_NODE=n
BUILD_G726_DEC_NODE=n
BUILD_GSMAMR_DEC_NODE=n
BUILD_AAC_DEC_NODE=n
BUILD_MP3_DEC_NODE=n
BUILD_RA8_DEC_NODE=n
BUILD_MP4_FF_PARSER_NODE=n
BUILD_AMR_FF_PARSER_NODE=y
BUILD_AAC_FF_PARSER_NODE=y
BUILD_MP3_FF_PARSER_NODE=y
BUILD_WAV_FF_PARSER_NODE=y
BUILD_ASF_FF_PARSER_NODE=n
BUILD_RM_FF_PARSER_NODE=n
BUILD_STREAMING_MANAGER_NODE=n
BUILD_DOWNLOAD_MANAGER_NODE=n
BUILD_STILL_IMAGE_NODE=n
BUILD_MP4_FF_REC=n
BUILD_ASF_FF_REC=n
BUILD_OMA1_FF_REC=n
BUILD_AAC_FF_REC=y
BUILD_RM_FF_REC=n
BUILD_MP3_FF_REC=y
BUILD_WAV_FF_REC=y
BUILD_AMR_FF_REC=y
BUILD_DIVX_FF_PARSER_NODE=n
BUILD_DIVX_FF_REC=n



#
# Menu for configuring Author
#
pvauthorengine_lib=m


#
# Menu for configuring pv2way
#
pv2wayengine_lib=m

engines_common_headers_lib=m
pvframemetadatautility_lib=m


#
# Menu for configuring Extern_libs
#
pvmtp_engine_lib=n
pvmtpip_engine_lib=n
pviptransport_lib=n
pvsqlite_lib=n
pvwmdrm_lib=n
wmdrm_config=n


#
# That's all, folks!
