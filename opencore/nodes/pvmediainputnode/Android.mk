LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_media_input_node_outport.cpp \
 	src/pvmf_media_input_node.cpp \
 	src/pvmf_media_input_data_buffer.cpp \
 	src/pvmf_media_input_node_cap_config.cpp


LOCAL_MODULE := libpvmediainputnode

LOCAL_CFLAGS := -DTEXT_TRACK_DESC_INFO -D_TEST_AE_ERROR_HANDLING $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvmediainputnode/src \
 	$(PV_TOP)/nodes/pvmediainputnode/src \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/nodes/pvmediainputnode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pvmf_media_input_data_buffer.h \
 	src/pvmf_media_input_node.h \
 	src/../include/pvmf_media_input_node_factory.h \
 	src/../include/pvmf_media_input_node_events.h

include $(BUILD_STATIC_LIBRARY)
