# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_m4v_component_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
        ../../../../../extern_libs_v2/khronos/openmax/include \
        ../../../../omx/omx_baseclass/include \
        ../../../../video/m4v_h263/dec/src \
        ../../../../video/m4v_h263/dec/include

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS :=	omx_mpeg4_component.cpp \
	mpeg4_dec.cpp 


HDRS :=	omx_mpeg4_component.h \
	mpeg4_dec.h \
	mpeg4video_frame.h



include $(MK)/library.mk

