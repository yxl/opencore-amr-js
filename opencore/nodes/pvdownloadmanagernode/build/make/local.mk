# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvdownloadmanagernode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)



XINCDIRS += \
  ../../../common/include \
  ../../../../pvmi/pvmf/include \
  ../../../streaming/common/include




SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := \
	pvmf_downloadmanager_factory.cpp \
	pvmf_downloadmanager_node.cpp \
	pvmf_filebufferdatastream_factory.cpp \
	pvmf_memorybufferdatastream_factory.cpp

HDRS := \
	pvmf_downloadmanager_defs.h \
	pvmf_downloadmanager_factory.h \
	pvmf_filebufferdatastream_factory.h \
	pvmf_memorybufferdatastream_factory.h


include $(MK)/library.mk
