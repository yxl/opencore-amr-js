LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/deblock.cpp \
 	src/dpb.cpp \
 	src/fmo.cpp \
 	src/mb_access.cpp \
 	src/reflist.cpp


LOCAL_MODULE := libpv_avc_common_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/video/avc_h264/common/src \
 	$(PV_TOP)/codecs_v2/video/avc_h264/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/avcapi_common.h \
 	include/avcint_common.h \
 	include/avclib_common.h

include $(BUILD_STATIC_LIBRARY)
