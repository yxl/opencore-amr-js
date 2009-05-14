LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/avc_enc.cpp \
 	src/omx_avcenc_component.cpp


LOCAL_MODULE := libomx_avcenc_component_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_h264enc/src \
 	$(PV_TOP)/codecs_v2/omx/omx_h264enc/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/video/avc_h264/enc/src \
 	$(PV_TOP)/codecs_v2/video/avc_h264/enc/include \
 	$(PV_TOP)/codecs_v2/video/avc_h264/common/src \
 	$(PV_TOP)/codecs_v2/video/avc_h264/common/include \
 	$(PV_TOP)/codecs_v2/utilities/colorconvert/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/avc_enc.h \
 	include/omx_avcenc_component.h

include $(BUILD_STATIC_LIBRARY)
