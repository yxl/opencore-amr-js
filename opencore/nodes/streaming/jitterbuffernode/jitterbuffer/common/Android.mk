LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_jb_event_notifier.cpp \
 	src/pvmf_jb_firewall_pkts_impl.cpp \
 	src/pvmf_jb_jitterbuffermisc.cpp \
 	src/pvmf_jb_session_duration_timer.cpp \
 	src/pvmf_jitter_buffer_impl.cpp \
 	src/pvmf_rtcp_proto_impl.cpp \
 	src/pvmf_rtcp_timer.cpp


LOCAL_MODULE := libpvjitterbuffer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/common/src \
 	$(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/common/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/nodes/streaming/streamingmanager/plugins/mshttp/config \
 	$(PV_TOP)/nodes/streaming/streamingmanager/include \
 	$(PV_TOP)/protocols/rtp/src \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_jb_event_notifier.h \
 	include/pvmf_jb_jitterbuffermisc.h \
 	include/pvmf_jitter_buffer.h \
 	include/pvmf_jitter_buffer_common_types.h \
 	include/pvmf_jitter_buffer_factory.h

include $(BUILD_STATIC_LIBRARY)
