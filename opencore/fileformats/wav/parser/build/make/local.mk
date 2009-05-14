# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvwav


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)








SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvwavfileparser.cpp

HDRS := pvwavfileparser.h



include $(MK)/library.mk
