# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := cpm




XINCDIRS +=  ../../include
XINCDIRS +=  ../../plugins/common/include
XINCDIRS +=  ../../plugins/oma1/r75/include
XINCDIRS +=  ../../../../nodes/common/include



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := cpm.cpp \
		pvmf_cpmplugin_factory_registry.cpp

HDRS := cpm.h \
		cpm_types.h \
		cpm_plugin_registry.h \
		pvmf_cpmplugin_factory_registry.h


include $(MK)/library.mk
