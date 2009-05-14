LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/mp3_dec.cpp \
 	src/omx_mp3_component.cpp \
 	src/mp3_timestamp.cpp


LOCAL_MODULE := libomx_mp3_component_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_mp3/src \
 	$(PV_TOP)/codecs_v2/omx/omx_mp3/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/omx/omx_baseclass/include \
 	$(PV_TOP)/codecs_v2/audio/mp3/dec/src \
 	$(PV_TOP)/codecs_v2/audio/mp3/dec/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/mp3_dec.h \
 	include/omx_mp3_component.h \
 	include/mp3_timestamp.h

include $(BUILD_STATIC_LIBRARY)
