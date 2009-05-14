# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := protocolenginenode_base

XINCDIRS += ../../../config/$(BUILD_ARCH)  ../../../config/linux
XINCDIRS += ../../../protocol_common/src ../../../download_protocols/progressive_download/src ../../../download_protocols/progressive_streaming/src ../../../download_protocols/fasttrack_download/src ../../../download_protocols/shoutcast/src ../../../wm_http_streaming/src

SRCDIR := ../../src
INCSRCDIR := ../../../include

SRCS :=	pvmf_protocol_engine_node.cpp \
	pvmf_protocol_engine_factory.cpp \
	pvmf_protocol_engine_node_registry.cpp \
	pvmf_protocol_engine_port.cpp 	

HDRS := pvmf_protocol_engine_factory.h \
	pvmf_protocol_engine_defs.h \
	pvmf_protocol_engine_node_extension.h \
	pvmf_protocol_engine_command_format_ids.h \
	pvmf_protocol_engine_node_events.h \
	../base/src/pvmf_protocol_engine_node_registry_interface.h \
	../base/src/pvmf_protocol_engine_node_registry_populator_interface.h \
	../base/src/pvmf_protocol_engine_node_shared_lib_interface.h
	
# include $(call process_include_list,$(LOCAL_PATH),$(BASE_PROTOCOL_PLUGINS_SUPPORT))

include $(MK)/library.mk
