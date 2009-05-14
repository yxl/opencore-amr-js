# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvavcdecoder

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := avcdec_api.cpp \
	avc_bitstream.cpp \
	header.cpp \
	itrans.cpp \
	pred_inter.cpp \
	pred_intra.cpp \
	pvavcdecoder.cpp \
	pvavcdecoder_factory.cpp \
	residual.cpp \
	slice.cpp \
	vlc.cpp

HDRS := pvavcdecoder.h \
	pvavcdecoder_factory.h \
	pvavcdecoderinterface.h

include $(MK)/library.mk
