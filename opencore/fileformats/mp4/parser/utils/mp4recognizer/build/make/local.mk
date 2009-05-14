# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := mp4recognizer_utility

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmp4ffrec.cpp \
	atomutils.cpp

HDRS := pvmp4ffrec.h \
	atomdefs.h



include $(MK)/library.mk

