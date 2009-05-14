# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvlatmpayloadparser


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../../../../../codecs_v2/audio/aac/dec/include  ../../../../../../codecs_v2/audio/aac/dec/src  ../../../../../../baselibs/gen_data_structures/src  ../../../../../sdp/common/include  ../../../../../../pvmi/pvmf/include






SRCDIR := ../../../src
INCSRCDIR := ../../../include

SRCS := latmpayloadparser.cpp

HDRS := latmpayloadparser.h



include $(MK)/library.mk
