LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libomx_amr_component_lib \
 	libpvdecoder_gsmamr \
 	libpvamrwbdecoder

LOCAL_MODULE := libomx_amrdec_sharedlibrary

LOCAL_PRELINK_MODULE := false

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libomx_sharedlibrary libopencore_common

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/codecs_v2/omx/omx_amr/Android.mk
include   $(PV_TOP)/codecs_v2/audio/gsm_amr/amr_nb/dec/Android.mk
include   $(PV_TOP)/codecs_v2/audio/gsm_amr/amr_wb/dec/Android.mk

