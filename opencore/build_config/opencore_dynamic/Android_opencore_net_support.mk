LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libpvsocketnode \
 	libpv_http_parcom \
 	libprotocolenginenode_base \
 	libprotocolenginenode_common \
 	libprotocolenginenode_download_common \
 	libprotocolenginenode_pdl \
 	libprotocolenginenode_ps

LOCAL_MODULE := libopencore_net_support

LOCAL_PRELINK_MODULE := false

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libopencore_common

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/nodes/pvsocketnode/Android.mk
include   $(PV_TOP)/protocols/http_parcom/Android.mk
include   $(PV_TOP)/nodes/pvprotocolenginenode/base/Android.mk
include   $(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/Android.mk
include   $(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/common/Android.mk
include   $(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_download/Android.mk
include   $(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_streaming/Android.mk

