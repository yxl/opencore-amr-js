# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := passthru_oma1


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../include
XINCDIRS +=  ../../../../common/include
XINCDIRS +=  ../../../../../include
XINCDIRS +=  ../../../../../../../nodes/common/include



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_cpmplugin_passthru_oma1.cpp

HDRS := pvmf_cpmplugin_passthru_oma1.h \
	pvmf_cpmplugin_passthru_oma1_types.h \
	pvmf_cpmplugin_passthru_oma1_factory.h


include $(MK)/library.mk
