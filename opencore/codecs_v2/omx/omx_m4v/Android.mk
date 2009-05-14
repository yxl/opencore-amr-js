LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/omx_mpeg4_component.cpp \
 	src/mpeg4_dec.cpp


LOCAL_MODULE := libomx_m4v_component_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_m4v/src \
 	$(PV_TOP)/codecs_v2/omx/omx_m4v/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/omx/omx_baseclass/include \
 	$(PV_TOP)/codecs_v2/video/m4v_h263/dec/src \
 	$(PV_TOP)/codecs_v2/video/m4v_h263/dec/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/omx_mpeg4_component.h \
 	include/mpeg4_dec.h \
 	include/mpeg4video_frame.h

include $(BUILD_STATIC_LIBRARY)
