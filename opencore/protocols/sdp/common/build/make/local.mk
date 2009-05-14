# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


TARGET := 
INCSRCDIR := ../../include

HDRS := \
	aac_media_info.h \
	aac_payload_info.h \
	amr_media_info.h \
	amr_payload_info.h \
	asf_media_info.h \
	common_info.h \
	h263_media_info.h \
	h263_payload_info.h \
	h264_media_info.h \
	h264_payload_info.h \
	m4v_media_info.h \
	m4v_payload_info.h \
	media_info.h \
	payload_info.h \
	pcma_media_info.h \
	pcma_payload_info.h \
	pcmu_media_info.h \
	pcmu_payload_info.h \
	pv_oscl.h \
        rfc3640_media_info.h \
        rfc3640_payload_info.h \
	rm_media_info.h \
	rm_payload_info.h \
	sdp_error.h \
	sdp_info_comparison.h \
	sdp_info.h \
	sdp_memory.h \
	sdp_return_code.h \
	session_info.h \
	still_image_media_info.h



include $(MK)/library.mk
