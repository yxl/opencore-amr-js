# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvloopbacknode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_loopback_ioport.cpp  pvmf_loopback_node.cpp


HDRS := pvmf_loopback_ioport.h  pvmf_loopback_node.h



include $(MK)/library.mk

