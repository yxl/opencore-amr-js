# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := protocolenginenode_ps



XINCDIRS += ../../../../config/$(BUILD_ARCH)  ../../../../config/linux
XINCDIRS += ../../../../include ../../src ../../../progressive_download/src ../../../common/src ../../../../protocol_common/src ../../../../base/src

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS :=	pvmf_protocol_engine_node_progressive_streaming.cpp \
	pvmf_protocol_engine_node_progressive_streaming_container_factory.cpp
	
HDRS := pvmf_protocol_engine_node_progressive_streaming.h \
	pvmf_protocol_engine_node_progressive_streaming_container_factory.h	

include $(MK)/library.mk
