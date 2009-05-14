LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_medialayer_node.cpp \
 	src/pvmf_medialayer_port.cpp


LOCAL_MODULE := libpvmedialayernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/streaming/medialayernode/src \
 	$(PV_TOP)/nodes/streaming/medialayernode/include \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_TOP)/nodes/streaming/streamingmanager/include \
 	$(PV_TOP)/protocols/sdp/common/include \
 	$(PV_TOP)/baselibs/gen_data_structures/src \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3016/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3984/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3267/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_2429/include \
 	$(PV_TOP)/fileformats/asf/parser/include \
 	$(PV_TOP)/pvmi/content_policy_manager/plugins/common/include \
 	$(PV_TOP)/nodes/streaming/streamingmanager/config/3gpp \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_medialayer_node.h \
 	include/pvmf_medialayer_port.h \
 	include/pvmf_ml_eos_timer.h

include $(BUILD_STATIC_LIBRARY)
