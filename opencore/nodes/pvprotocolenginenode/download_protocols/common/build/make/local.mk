# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := protocolenginenode_download_common



XINCDIRS +=  ../../../../config/$(BUILD_ARCH)  ../../../../config/linux
XINCDIRS +=  ../../../../include  ../../src  ../../../../protocol_common/src  ../../../../base/src 

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS :=	pvdl_config_file.cpp \
	pvmf_protocol_engine_download_common.cpp \
	pvmf_protocol_engine_node_download_common.cpp
	
HDRS := pvdl_config_file.h \
	pvmf_protocol_engine_download_common.h \
	pvmf_protocol_engine_node_download_common.h
	

include $(MK)/library.mk
