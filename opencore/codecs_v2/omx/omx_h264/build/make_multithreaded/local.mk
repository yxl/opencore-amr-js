# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_avc_component_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += ../../../../../extern_libs_v2/khronos/openmax/include \
            ../../../../omx/omx_baseclass/include \
            ../../../../video/avc_h264/dec/src \
            ../../../../video/avc_h264/dec/include

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := avc_dec.cpp \
	omx_avc_component.cpp 


HDRS := avc_dec.h \
	omx_avc_component.h


include $(MK)/library.mk

