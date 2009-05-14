# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4ffrecognizer


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmp4ffrec_factory.cpp \
       pvmp4ffrec_plugin.cpp

HDRS := pvmp4ffrec_factory.h
    



include $(MK)/library.mk
