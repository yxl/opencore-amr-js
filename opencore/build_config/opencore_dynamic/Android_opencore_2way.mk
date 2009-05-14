LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libpv2wayengine \
 	libpv324m \
 	libpvvideoparsernode \
 	libpvcommsionode \
 	libpvmio_comm_loopback \
 	libpvgeneraltools

LOCAL_MODULE := libopencore_2way

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libopencore_common

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/engines/2way/Android.mk
include   $(PV_TOP)/protocols/systems/3g-324m_pvterminal/Android.mk
include   $(PV_TOP)/nodes/pvvideoparsernode/Android.mk
include   $(PV_TOP)/nodes/pvcommsionode/Android.mk
include   $(PV_TOP)/pvmi/media_io/pvmio_comm_loopback/Android.mk
include   $(PV_TOP)/protocols/systems/common/Android.mk
include   $(PV_TOP)/protocols/systems/tools/general/Android.mk

