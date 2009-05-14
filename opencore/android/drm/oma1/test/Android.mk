LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_DRM_CFLAG = -DKMJ_PLUGIN_UT

ifeq ($(TARGET_ARCH), arm)
LOCAL_DRM_1_PLUGIN_CFLAG = -DDRM_1_PLUGIN_ARCH_ARM
endif

ifeq ($(TARGET_ARCH), x86)
LOCAL_DRM_1_PLUGIN_CFLAG = -DDRM_1_PLUGIN_ARCH_X86
endif

LOCAL_SRC_FILES := \
        test_pv_drm_plugin.cpp \
        test_pv_drm_plugin_ut.cpp \
        test_pv_player_engine_main.cpp \
        test_pv_player_engine_testset_mio_file.cpp
        
LOCAL_MODULE := pv_drm_plugin_test
              
LOCAL_STATIC_LIBRARIES := \
	libpvplayer_engine \
	libpvfileoutputnode \
	libpvpvxparser \
	libpvasfffparsernode \
	libpvmp3ffparsernode \
	libpvwmadecnode \
	libmp3decnode \
	libpvdownloadmanagernode \
	libpvmp4ffparsernode \
	libcpm \
	libpassthru_oma1 \
	libpvaacffparsernode \
	libpvmp3ff \
	libpvasfff \
	libpvmp4ff \
	libpvaacparser \
	libgetactualaacconfig \
	libpvmp3 \
	libwmadecoder \
	libpvwavffparsernode \
	libpvwav \
	libpvomxvideodecnode \
	libpv_video_config_parser \
	libm4v_config \
	libpvlogger \
	libpvvideodecnode \
	libpvavcdecnode \
	libpvwmvdecnode \
	libgsmamrdecnode \
	libaacdecnode \
	libpvstreamingmanagernode_3gpp \
	libpvjitterbuffernode \
	libpvmediaplayernode \
	librtprtcp \
	libpvrtsp_cli_eng_node_3gpp \
	libprotocolenginenode \
	libpv_http_parcom \
	libpvsocketnode \
	librtppayloadparser_3gpp \
	libpvmf \
	libpvsdpparser \
	libpv_rtsp_parcom \
	libomx_common_lib \
	libomx_avc_component_lib \
	libpvavcdecoder \
	libpv_avc_common_lib \
	libomx_m4v_component_lib \
	libpvcrypto \
	libomx_m4v_component_lib \
	libpvmp4decoder \
	libpvdecoder_gsmamr \
	libpv_amr_nb_common_lib \
	libpvamrwbdecoder \
	libpv_aac_dec \
	libfile_mediaio \
	libpvgendatastruct \
	libpvmimeutils \
	libpvmediaoutputnode \
	libpvmediadatastruct \
	libpvamrffparsernode \
	libpvgsmamrparser \
	libpvlatmpayloadparser \
	libpvmfrecognizer \
	libpvamrffrecognizer \
	libpvmp3ffrecognizer \
	libpvasfffrecognizer \
	libpvmp4ffrecognizer \
	libpvwavffrecognizer \
	libpvoma1ffrecognizer \
	libpvfileparserutils \
	libpvid3parcom \
	libpvdbmanager \
	libpvwmdrm \
	libcsprng \
	libpvcrypto \
	libpventropysrc \
	libosclregcli \
	libosclregserv \
	libosclio \
	libosclproc \
	libosclutil \
	libosclmemory \
	libosclerror \
	libosclbase \
	libpvomx_proxy_lib \
	libomx_wmv_component_interface \
	libomx_queue_lib \
	libthreadsafe_callback_ao \
	liboscllib \
	libunit_test \
	libkmj_oma1
	
LOCAL_SHARED_LIBRARIES := libdrm1

LOCAL_C_INCLUDES := $(PV_INCLUDES) \
    $(PV_TOP)/engines/common/include \
    $(PV_TOP)/pvmi/media_io/pvmiofileoutput/include \
    $(PV_TOP)/nodes/pvmediaoutputnode/include \
    $(PV_TOP)/engines/player/include \
    $(PV_TOP)/nodes/common/include \
    $(PV_TOP)/engines/player/src \
    $(PV_TOP)/engines/player/test/config/android \
    $(PV_TOP)/android/drm/oma1/test \
    $(PV_TOP)/oscl/unit_test/src \
    libs/drm/mobile1/include

LOCAL_CFLAGS := $(PV_CFLAGS)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := \

ifeq ($(TARGET_ARCH), x86)
LOCAL_LDLIBS += -lpthread
endif

LOCAL_CFLAGS += $(LOCAL_DRM_CFLAG)
LOCAL_CFLAGS += $(LOCAL_DRM_1_PLUGIN_CFLAG)

include $(BUILD_EXECUTABLE)
