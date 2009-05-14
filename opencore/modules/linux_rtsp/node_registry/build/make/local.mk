# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvrtspreginterface

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvmfrtspnodereg.cpp

HDRS := pvmfrtspnodereg.h

include $(MK)/library.mk

