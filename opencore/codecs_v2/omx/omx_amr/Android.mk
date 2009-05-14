LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/amr_decode_frame.cpp \
 	src/omx_amr_component.cpp \
 	src/d_homing.cpp


LOCAL_MODULE := libomx_amr_component_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_amr/src \
 	$(PV_TOP)/codecs_v2/omx/omx_amr/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/omx/omx_baseclass/include \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/amr_nb/dec/src \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/amr_nb/dec/include \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/amr_nb/common/include \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/common/dec/include \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/amr_wb/common/include \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/amr_wb/dec/include \
 	$(PV_TOP)/codecs_v2/audio/gsm_amr/amr_wb/dec/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/amr_dec.h \
 	include/omx_amr_component.h \
 	include/d_homing.h

include $(BUILD_STATIC_LIBRARY)
