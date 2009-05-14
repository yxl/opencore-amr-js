# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvomxencnode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += \
  ../../../../extern_libs_v2/khronos/openmax/include



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_omx_enc_factory.cpp \
	pvmf_omx_enc_node.cpp \
	pvmf_omx_enc_port.cpp \
	pvmf_omx_enc_callbacks.cpp

HDRS := pvmf_omx_enc_defs.h \
	pvmf_omx_enc_factory.h \
	pvmf_omx_enc_port.h 

include $(MK)/library.mk

