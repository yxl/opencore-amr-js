LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/adaptive_smooth_no_mmx.cpp \
 	src/bitstream.cpp \
 	src/block_idct.cpp \
 	src/cal_dc_scaler.cpp \
 	src/chvr_filter.cpp \
 	src/chv_filter.cpp \
 	src/combined_decode.cpp \
 	src/conceal.cpp \
 	src/datapart_decode.cpp \
 	src/dcac_prediction.cpp \
 	src/dec_pred_intra_dc.cpp \
 	src/deringing_chroma.cpp \
 	src/deringing_luma.cpp \
 	src/find_min_max.cpp \
 	src/get_pred_adv_b_add.cpp \
 	src/get_pred_outside.cpp \
 	src/idct.cpp \
 	src/idct_vca.cpp \
 	src/mb_motion_comp.cpp \
 	src/mb_utils.cpp \
 	src/pvm4vdecoder.cpp \
 	src/pvm4vdecoder_factory.cpp \
 	src/packet_util.cpp \
 	src/post_filter.cpp \
 	src/post_proc_semaphore.cpp \
 	src/pp_semaphore_chroma_inter.cpp \
 	src/pp_semaphore_luma.cpp \
 	src/pvdec_api.cpp \
 	src/scaling_tab.cpp \
 	src/vlc_decode.cpp \
 	src/vlc_dequant.cpp \
 	src/vlc_tab.cpp \
 	src/vop.cpp \
 	src/zigzag_tab.cpp


LOCAL_MODULE := libpvmp4decoder

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/video/m4v_h263/dec/src \
 	$(PV_TOP)/codecs_v2/video/m4v_h263/dec/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvm4vdecoder.h \
 	include/pvm4vdecoder_factory.h \
 	include/pvvideodecoderinterface.h \
 	include/visual_header.h

include $(BUILD_STATIC_LIBRARY)
