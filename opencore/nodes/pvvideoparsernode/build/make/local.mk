# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvvideoparsernode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)







SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_videoparser_port.cpp \
	pvmf_videoparser_node.cpp


HDRS := pvmf_videoparser_port.h \
	pvmf_videoparser_node.h



include $(MK)/library.mk

