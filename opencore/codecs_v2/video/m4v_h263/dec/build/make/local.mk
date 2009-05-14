# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4decoder

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := adaptive_smooth_no_mmx.cpp \
	bitstream.cpp \
	block_idct.cpp \
	cal_dc_scaler.cpp \
	chvr_filter.cpp \
	chv_filter.cpp \
	combined_decode.cpp \
	conceal.cpp \
	datapart_decode.cpp \
	dcac_prediction.cpp \
	dec_pred_intra_dc.cpp \
	deringing_chroma.cpp \
	deringing_luma.cpp \
	find_min_max.cpp \
	get_pred_adv_b_add.cpp \
	get_pred_outside.cpp \
	idct.cpp \
	idct_vca.cpp \
	mb_motion_comp.cpp \
	mb_utils.cpp \
	pvm4vdecoder.cpp \
	pvm4vdecoder_factory.cpp \
	packet_util.cpp \
	post_filter.cpp \
	post_proc_semaphore.cpp \
	pp_semaphore_chroma_inter.cpp \
	pp_semaphore_luma.cpp \
	pvdec_api.cpp \
	scaling_tab.cpp \
	vlc_decode.cpp \
	vlc_dequant.cpp \
	vlc_tab.cpp \
	vop.cpp \
	zigzag_tab.cpp

HDRS := pvm4vdecoder.h \
	pvm4vdecoder_factory.h \
	pvvideodecoderinterface.h \
	visual_header.h


include $(MK)/library.mk
