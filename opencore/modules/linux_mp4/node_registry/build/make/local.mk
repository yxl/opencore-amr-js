# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4reginterface

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvmfmp4nodereg.cpp

HDRS := pvmfmp4nodereg.h

include $(MK)/library.mk

