# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pv_frame_metadata_utility_test

XINCDIRS += ../../config/common

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := test_pv_frame_metadata_utility.cpp \
        test_pv_frame_metadata_utility_testset1.cpp \
        test_pv_frame_metadata_utility_testset_januscpm.cpp

LIBS :=  pvframemetadatautility \
        colorconvert \
pvplayer_engine \
        pvfileoutputnode \
        pvpvxparser \
        pvmp3ffparsernode \
        mp3decnode \
        pvdownloadmanagernode \
        pvmp4ffparsernode \
        cpm \
        passthru_oma1 \
        pvaacffparsernode \
        pvwavffparsernode \
        pvstillimagenode \
        pvmp4ff \
        pvmp3ff \
        pvwav \
        pvaacparser \
        getactualaacconfig \
        pvomxvideodecnode \
        pvomxaudiodecnode \
        pvomxbasedecnode \
        m4v_config \
        pvvideodecnode \
        pvavcdecnode \
        gsmamrdecnode \
        aacdecnode \
        pvstreamingmanagernode \
        pvjitterbuffernode \
        pvjitterbufferasf \
        pvjitterbufferrtp \
        pvjitterbuffer \
        pvmedialayernode \
        rtprtcp \
        pvrtsp_cli_eng_node \
        protocolenginenode \
        pv_http_parcom \
        pvsocketnode \
        pvrtppacketsourcenode \
        rtppayloadparser \
        pvpvrnode \
        pvpvr \
        pvpvrff \
        pvmf \
        pvsdpparser \
        pv_rtsp_parcom \
        pvwmadecnode \
        omx_common_lib \
        pv_config_parser \
        omx_mp3_component_lib \
        pvmp3 \
        omx_avc_component_lib \
        pvavcdecoder \
        pv_avc_common_lib \
        omx_amr_component_lib \
        pvdecoder_gsmamr \
        pv_amr_nb_common_lib \
        pvamrwbdecoder \
        omx_m4v_component_lib \
        pvmp4decoder \
        omx_wmv_component_lib \
        omx_aac_component_lib \
        pv_aac_dec \
        pvmiofileoutput \
        pvmimeutils \
        pvmediaoutputnode \
        pvmediadatastruct \
        pvamrffparsernode \
        pvgsmamrparser \
        pvlatmpayloadparser \
        rdt_parser \
        pvwmvdecnode \
        g726decnode \
        pvg726decoder \
        wmvdecoder \
        omx_wma_component_lib \
        wmadecoder \
        pvasfffparsernode \
        pvasfff \
        pvjanusplugin \
        pvmfrecognizer \
        pvasfffrecognizer \
        pvmp4ffrecognizer \
        mp4recognizer_utility \
        pvmp3ffrecognizer \
        pvrmffrecognizer \
        pvoma1ffrecognizer \
        pvaacffrecognizer \
        pvfileparserutils \
        pvrvdecnode \
        rvdecoder \
        pvra8decnode \
        pvra8decoder \
        pvrmffparsernode \
        pvrmffparser \
        pvrmff \
        realaudio_deinterleaver \
        pvid3parcom \
        pvgendatastruct \
        pvwmdrm \
        csprng\
        pvcrypto \
        pventropysrc\
        osclregcli \
        osclregserv \
        osclio \
        osclproc \
        osclutil \
        osclmemory \
        osclerror \
        osclbase \
        omx_baseclass_lib \
        pvomx_proxy_lib \
        omx_queue_lib \
        threadsafe_callback_ao \
        packetsources_default \
        unit_test \
	asfrecognizer_utility \
	scsp \
        pvdivxffrecognizer \
        pvdivxffparsernode \
        pv_divxfile_parser
SYSLIBS += $(SYS_THREAD_LIB)

include $(MK)/prog.mk

