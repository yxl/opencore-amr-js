LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/rtcp.cpp \
 	src/rtcp_decoder.cpp \
 	src/rtcp_encoder.cpp


LOCAL_MODULE := librtprtcp

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/rtp/src \
 	$(PV_TOP)/protocols/rtp/src \
 	$(PV_TOP)/protocols/rtp/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/rtprtcp.h \
 	src/rtcp.h \
 	src/rtcp_decoder.h \
 	src/rtcp_encoder.h \
 	src/rtcp_constants.h

include $(BUILD_STATIC_LIBRARY)
