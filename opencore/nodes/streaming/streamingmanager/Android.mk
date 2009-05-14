LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_sm_node_factory.cpp \
 	src/pvmf_streaming_manager_node.cpp \
 	src/../config/3gpp/pvmf_sm_fsp_registry.cpp \
 	src/../plugins/common/src/pvmf_sm_fsp_base_impl.cpp \
 	src/../plugins/common/src/pvmf_sm_fsp_base_cpm_support.cpp \
 	src/../plugins/rtspunicast/src/pvmf_sm_rtsp_unicast_node_factory.cpp \
 	src/../plugins/rtspunicast/src/pvmf_sm_fsp_rtsp_unicast.cpp


LOCAL_MODULE := libpvstreamingmanagernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/streaming/streamingmanager/src \
 	$(PV_TOP)/nodes/streaming/streamingmanager/include \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/include \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/include \
 	$(PV_TOP)/nodes/streaming/medialayernode/include \
 	$(PV_TOP)/protocols/rtp/src \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/protocols/sdp/common/include \
 	$(PV_TOP)/fileformats/asf/parser/include \
 	$(PV_TOP)/fileformats/rmff_type_specific_parser/src \
 	$(PV_TOP)/protocols/rdt_parser/realchallenge/include \
 	$(PV_TOP)/pvmi/content_policy_manager/include \
 	$(PV_TOP)/pvmi/content_policy_manager/plugins/common/include \
 	$(PV_TOP)/nodes/pvrtppacketsourcenode/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3640/include \
 	$(PV_TOP)/nodes/pvpvr/include \
 	$(PV_TOP)/nodes/pvpvr/src \
 	$(PV_TOP)/nodes/pvpvrnode/include \
 	$(PV_TOP)/nodes/pvpvrnode/src \
 	$(PV_TOP)/fileformats/pvr/composer/include \
 	$(PV_TOP)/nodes/streaming/streamingmanager/plugins/common/include \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/common/include \
 	$(PV_TOP)/nodes/streaming/streamingmanager/config/3gpp \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/rtp/include \
 	$(PV_TOP)/protocols/rtsp_client_engine/src \
 	$(PV_TOP)/nodes/streaming/streamingmanager/plugins/rtspunicast/include \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/rtp/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_sm_node_events.h \
 	include/pvmf_sm_node_factory.h

include $(BUILD_STATIC_LIBRARY)
