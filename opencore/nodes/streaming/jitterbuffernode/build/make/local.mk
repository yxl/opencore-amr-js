# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvjitterbuffernode

XINCDIRS += ../../../common/include ../../../streamingmanager/plugins/mshttp/config ../../../streamingmanager/include ../../../../../protocols/rtp/src
XINCDIRS += -I ../../../common/include
XINCDIRS += -I ../../jitterbuffer/common/include

SRCDIR := ../../src
INCSRCDIR := ../../include

# compose final src list for actual build
SRCS = pvmf_jitter_buffer_extension_interface.cpp \
	pvmf_jitter_buffer_port.cpp \
	pvmf_jitter_buffer_node.cpp 

HDRS = pvmf_jitter_buffer_ext_interface.h \
	pvmf_jitter_buffer_internal.h \
	pvmf_jitter_buffer_port.h \
	pvmf_jitter_buffer_node.h 

include $(MK)/library.mk

