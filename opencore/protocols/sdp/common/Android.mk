LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	




LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/sdp/common/build/make \
 	$(PV_TOP)/protocols/sdp/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/aac_media_info.h \
 	include/aac_payload_info.h \
 	include/amr_media_info.h \
 	include/amr_payload_info.h \
 	include/asf_media_info.h \
 	include/common_info.h \
 	include/h263_media_info.h \
 	include/h263_payload_info.h \
 	include/h264_media_info.h \
 	include/h264_payload_info.h \
 	include/m4v_media_info.h \
 	include/m4v_payload_info.h \
 	include/media_info.h \
 	include/payload_info.h \
 	include/pcma_media_info.h \
 	include/pcma_payload_info.h \
 	include/pcmu_media_info.h \
 	include/pcmu_payload_info.h \
 	include/pv_oscl.h \
 	include/rfc3640_media_info.h \
 	include/rfc3640_payload_info.h \
 	include/rm_media_info.h \
 	include/rm_payload_info.h \
 	include/sdp_error.h \
 	include/sdp_info_comparison.h \
 	include/sdp_info.h \
 	include/sdp_memory.h \
 	include/sdp_return_code.h \
 	include/session_info.h \
 	include/still_image_media_info.h

include $(BUILD_COPY_HEADERS)
