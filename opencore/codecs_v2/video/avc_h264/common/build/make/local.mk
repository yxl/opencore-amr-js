# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pv_avc_common_lib

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := deblock.cpp \
	dpb.cpp \
	fmo.cpp \
	mb_access.cpp \
	reflist.cpp

HDRS :=  avcapi_common.h avcint_common.h avclib_common.h

include $(MK)/library.mk

