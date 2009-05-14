LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/mpeg4_enc.cpp \
 	src/omx_mpeg4enc_component.cpp


LOCAL_MODULE := libomx_m4venc_component_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_m4venc/src \
 	$(PV_TOP)/codecs_v2/omx/omx_m4venc/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/video/m4v_h263/enc/src \
 	$(PV_TOP)/codecs_v2/video/m4v_h263/enc/include \
 	$(PV_TOP)/codecs_v2/utilities/colorconvert/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/mpeg4_enc.h \
 	include/omx_mpeg4enc_component.h

include $(BUILD_STATIC_LIBRARY)
