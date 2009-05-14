# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvomxbasedecnode

XINCDIRS += ../../../../extern_libs_v2/khronos/openmax/include

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS :=	pvmf_omx_basedec_node.cpp \
        pvmf_omx_basedec_port.cpp \
        pvmf_omx_basedec_callbacks.cpp

HDRS := pvmf_omx_basedec_defs.h \
        pvmf_omx_basedec_port.h \
        pvmf_omx_basedec_node.h \
        pvmf_omx_basedec_callbacks.h \
        pvmf_omx_basedec_node_extension_interface.h

include $(MK)/library.mk
