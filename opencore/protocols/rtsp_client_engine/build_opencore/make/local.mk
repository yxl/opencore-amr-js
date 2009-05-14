# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvrtsp_cli_eng_node


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../../../nodes/streaming/streamingmanager/config/3gpp ../../../rtsp_parcom/src ../../../rtp/src ../../../../baselibs/gen_data_structures/src  ../../../sdp/common/include ../../../../nodes/streaming/streamingmanager/include ../../../../baselibs/pvcrypto/src ../../../rdt_parser/include ../../../../nodes/streaming/common/include



SRCDIR := ../../src
INCSRCDIR := ../../inc

SRCS := pvrtsp_client_engine_node.cpp\
	pvrtspenginenodeextensioninterface_impl.cpp \
	pvrtsp_client_engine_utils.cpp\
	pvrtsp_client_engine_factory.cpp\
	pvrtsp_client_engine_port.cpp

HDRS := pvrtsp_client_engine_node.h\
	pvrtspenginenodeextensioninterface.h\
	pvrtsp_client_engine_utils.h\
	pvrtsp_client_engine_factory.h\
	pvrtsp_client_engine_error_code.h



include $(MK)/library.mk
