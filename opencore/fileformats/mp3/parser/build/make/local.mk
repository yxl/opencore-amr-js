# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp3ff


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := \
	imp3ff.cpp \
	mp3fileio.cpp \
	mp3parser.cpp \
	mp3utils.cpp 

HDRS := \
	imp3ff.h \
	mp3fileio.h \
	mp3parser.h \
	mp3utils.h


include $(MK)/library.mk
