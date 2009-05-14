LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libpvdownloadmanagernode \
 	libpvdownloadinterface

LOCAL_MODULE := libopencore_download

LOCAL_PRELINK_MODULE := false

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libopencore_net_support libopencore_player libopencore_common

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/nodes/pvdownloadmanagernode/Android.mk
include   $(PV_TOP)/modules/linux_download/core/Android.mk

