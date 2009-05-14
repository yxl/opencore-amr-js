LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/aac_decode_frame.cpp \
 	src/omx_aac_component.cpp \
 	src/aac_timestamp.cpp


LOCAL_MODULE := libomx_aac_component_lib

LOCAL_CFLAGS := -DAAC_PLUS -DHQ_SBR -DPARAMETRICSTEREO $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_aac/src \
 	$(PV_TOP)/codecs_v2/omx/omx_aac/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/omx/omx_baseclass/include \
 	$(PV_TOP)/codecs_v2/audio/aac/dec/src \
 	$(PV_TOP)/codecs_v2/audio/aac/dec/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/aac_dec.h \
 	include/omx_aac_component.h \
 	include/aac_timestamp.h

include $(BUILD_STATIC_LIBRARY)
