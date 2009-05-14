# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := protocolenginenode

XCPPFLAGS += -DBUILD_PROGRESSIVE_DOWNLOAD_PLUGIN -DBUILD_PROGRESSIVE_STREAMING_PLUGIN -DBUILD_FASTTRACK_DOWNLOAD_PLUGIN -DBUILD_WMHTTPSTREAMING_PLUGIN -DBUILD_SHOUTCAST_PLUGIN 

XINCDIRS +=  ../../config/$(BUILD_ARCH)  ../../config/linux
XINCDIRS +=  ../../include ../../base/src ../../protocol_common/src ../../download_protocols/common/src ../../download_protocols/progressive_download/src ../../download_protocols/progressive_streaming/src ../../download_protocols/shoutcast/src ../../download_protocols/fasttrack_download/src ../../wm_http_streaming/src 

SRCDIR := ../../base/src 
INCSRCDIR := ../../include

SRCS := pvmf_protocol_engine_node.cpp \
	pvmf_protocol_engine_factory.cpp \
	pvmf_protocol_engine_port.cpp \
	pvmf_protocol_engine_node_registry.cpp \
	../../protocol_common/src/pvmf_protocol_engine_common.cpp \
	../../protocol_common/src/pvmf_protocol_engine_node_common.cpp \
	../../download_protocols/common/src/pvdl_config_file.cpp \
	../../download_protocols/common/src/pvmf_protocol_engine_download_common.cpp \
	../../download_protocols/common/src/pvmf_protocol_engine_node_download_common.cpp \
	../../download_protocols/progressive_download/src/pvmf_protocol_engine_progressive_download.cpp \
	../../download_protocols/progressive_download/src/pvmf_protocol_engine_node_progressive_download.cpp \
	../../download_protocols/progressive_download/src/pvmf_protocol_engine_node_progressive_download_container_factory.cpp \
	../../download_protocols/progressive_streaming/src/pvmf_protocol_engine_node_progressive_streaming.cpp \
	../../download_protocols/progressive_streaming/src/pvmf_protocol_engine_node_progressive_streaming_container_factory.cpp \
	../../download_protocols/shoutcast/src/pvmf_protocol_engine_shoutcast.cpp \
	../../download_protocols/shoutcast/src/pvmf_protocol_engine_node_shoutcast.cpp \
	../../download_protocols/shoutcast/src/pvmf_protocol_engine_node_shoutcast_container_factory.cpp \
	../../download_protocols/fasttrack_download/src/pvmf_protocol_engine_fasttrack_download.cpp \
	../../download_protocols/fasttrack_download/src/pvmf_protocol_engine_node_fasttrack_download.cpp \
	../../download_protocols/fasttrack_download/src/pvmf_protocol_engine_node_fasttrack_download_container_factory.cpp \
	../../wm_http_streaming/src/pvmf_protocol_engine_wmhttpstreaming.cpp \
	../../wm_http_streaming/src/pvmf_protocol_engine_node_wmhttpstreaming.cpp \
	../../wm_http_streaming/src/pvms_http_streaming_parser.cpp \
	../../wm_http_streaming/src/xml_composer.cpp \
	../../wm_http_streaming/src/pvmf_protocol_engine_node_wm_http_streaming_container_factory.cpp
	

HDRS := pvmf_protocol_engine_factory.h \
	pvmf_protocol_engine_defs.h \
	pvmf_protocol_engine_node_extension.h \
	pvmf_protocol_engine_command_format_ids.h \
	pvmf_protocol_engine_node_events.h

include $(MK)/library.mk
