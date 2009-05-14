# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvomxaudiodecnode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += \
  ../../../../extern_libs_v2/khronos/openmax/include \

XINCDIRS += ../../../../extern_libs_v2/khronos/openmax/include \
            ../../../../baselibs/threadsafe_callback_ao/src \
            ../../../pvomxbasedecnode/include \
            ../../../pvomxbasedecnode/src

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_omx_audiodec_factory.cpp \
	pvmf_omx_audiodec_node.cpp

HDRS :=	pvmf_omx_audiodec_factory.h 

include $(MK)/library.mk

