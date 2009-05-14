# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvprotocolengineprogressivedownloadinterface

XINCDIRS +=  -I ../../../../../../../nodes/pvprotocolenginenode/base/src -I ../../../../../../../nodes/pvprotocolenginenode/download_protocols/progressive_download/src

SRCDIR := ../../src

SRCS := pvmfprotocolenginenode_progressive_download_plugin.cpp

include $(MK)/library.mk

