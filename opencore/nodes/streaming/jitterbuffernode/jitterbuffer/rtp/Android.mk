LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_rtp_jitter_buffer_factory.cpp \
 	src/pvmf_rtp_jitter_buffer_impl.cpp


LOCAL_MODULE := libpvjitterbufferrtp

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/rtp/src \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/rtp/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/nodes/streaming/streamingmanager/plugins/mshttp/config \
 	$(PV_TOP)/nodes/streaming/streamingmanager/include \
 	$(PV_TOP)/protocols/rtp/src \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvmf_rtp_jitter_buffer_factory.h

include $(BUILD_STATIC_LIBRARY)
