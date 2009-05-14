# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvprotocolengineprogressivedownloadpluginreginterface



XINCDIRS +=  ../../src ../../../../../../../nodes/pvprotocolenginenode/base/src ../../../../../../../nodes/pvprotocolenginenode/protocol_common/src ../../../../../../../nodes/pvprotocolenginenode/download_protocols/progressive_download/src ../../../../../../../nodes/pvprotocolenginenode/include


SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvprotocolenginenode_pdl_plugin_registry.cpp

include $(MK)/library.mk

