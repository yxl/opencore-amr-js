LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libomx_common_lib \
 	libomx_queue_lib \
 	libpvomx_proxy_lib \
 	libomx_baseclass_lib \
 	libpv_omx_interface

LOCAL_MODULE := libomx_sharedlibrary

LOCAL_PRELINK_MODULE := false

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libopencore_common

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/codecs_v2/omx/omx_common/Android.mk
include   $(PV_TOP)/codecs_v2/omx/omx_queue/Android.mk
include   $(PV_TOP)/codecs_v2/omx/omx_proxy/Android.mk
include   $(PV_TOP)/codecs_v2/omx/omx_baseclass/Android.mk
include   $(PV_TOP)/codecs_v2/omx/omx_sharedlibrary/interface/Android.mk

