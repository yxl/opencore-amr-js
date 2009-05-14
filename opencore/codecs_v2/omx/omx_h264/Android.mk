LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/avc_dec.cpp \
 	src/omx_avc_component.cpp


LOCAL_MODULE := libomx_avc_component_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_h264/src \
 	$(PV_TOP)/codecs_v2/omx/omx_h264/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/omx/omx_baseclass/include \
 	$(PV_TOP)/codecs_v2/video/avc_h264/dec/src \
 	$(PV_TOP)/codecs_v2/video/avc_h264/dec/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/avc_dec.h \
 	include/omx_avc_component.h

include $(BUILD_STATIC_LIBRARY)
