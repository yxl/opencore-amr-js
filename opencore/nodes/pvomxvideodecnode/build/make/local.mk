# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvomxvideodecnode

XINCDIRS += ../../../../extern_libs_v2/khronos/openmax/include \
            ../../../../codecs_v2/video/wmv_vc1/dec/src \
            ../../../../baselibs/threadsafe_callback_ao/src \
            ../../../pvomxbasedecnode/include \
            ../../../pvomxbasedecnode/src

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_omx_videodec_factory.cpp \
	pvmf_omx_videodec_node.cpp

HDRS := pvmf_omx_videodec_factory.h

include $(MK)/library.mk

