# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp3

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include


SRCS := pvmp3_normalize.cpp \
	pvmp3_alias_reduction.cpp \
	pvmp3_crc.cpp \
	pvmp3_decode_header.cpp \
	pvmp3_decode_huff_cw.cpp \
	pvmp3_decoder.cpp \
	pvmp3_getbits.cpp \
	pvmp3_dequantize_sample.cpp \
	pvmp3_framedecoder.cpp \
	pvmp3_get_main_data_size.cpp \
	pvmp3_get_side_info.cpp \
	pvmp3_get_scale_factors.cpp \
	pvmp3_mpeg2_get_scale_data.cpp \
	pvmp3_mpeg2_get_scale_factors.cpp \
	pvmp3_mpeg2_stereo_proc.cpp \
	pvmp3_huffman_decoding.cpp \
	pvmp3_huffman_parsing.cpp \
	pvmp3_tables.cpp \
	pvmp3_imdct_synth.cpp \
	pvmp3_mdct_6.cpp \
	pvmp3_dct_6.cpp \
	pvmp3_poly_phase_synthesis.cpp \
	pvmp3_equalizer.cpp \
	pvmp3_seek_synch.cpp \
	pvmp3_stereo_proc.cpp \
	pvmp3_reorder.cpp \
	pvmp3_polyphase_filter_window.cpp \
	pvmp3_mdct_18.cpp \
	pvmp3_dct_9.cpp \
	pvmp3_dct_16.cpp 


HDRS := pvmp3_decoder.h pvmp3decoder_api.h pvmp3_audio_type_defs.h

include $(MK)/library.mk


