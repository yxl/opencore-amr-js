LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libpvrtspinterface \
 	libpvstreamingmanagernode \
 	libpv_rtsp_parcom \
 	libpvrtsp_cli_eng_node \
 	librtppayloadparser \
 	librtprtcp \
 	libpvjitterbuffer \
 	libpvjitterbufferrtp \
 	libpvjitterbuffernode \
 	libpvmedialayernode \
 	libpvsdpparser

LOCAL_MODULE := libopencore_rtsp

LOCAL_PRELINK_MODULE := false

-include $(PV_TOP)/Android_platform_extras.mk

-include $(PV_TOP)/Android_system_extras.mk

LOCAL_SHARED_LIBRARIES +=   libopencore_net_support libopencore_player libopencore_common

include $(BUILD_SHARED_LIBRARY)
include   $(PV_TOP)/modules/linux_rtsp/core/Android.mk
include   $(PV_TOP)/nodes/streaming/streamingmanager/Android.mk
include   $(PV_TOP)/protocols/rtsp_parcom/Android.mk
include   $(PV_TOP)/protocols/rtsp_client_engine/Android.mk
include   $(PV_TOP)/protocols/rtp_payload_parser/Android.mk
include   $(PV_TOP)/protocols/rtp/Android.mk
include   $(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/common/Android.mk
include   $(PV_TOP)/nodes/streaming/jitterbuffernode/jitterbuffer/rtp/Android.mk
include   $(PV_TOP)/nodes/streaming/jitterbuffernode/Android.mk
include   $(PV_TOP)/nodes/streaming/medialayernode/Android.mk
include   $(PV_TOP)/protocols/sdp/parser/Android.mk
include   $(PV_TOP)/protocols/sdp/common/Android.mk

