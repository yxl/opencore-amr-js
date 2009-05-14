# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_m4venc_component_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
  ../../../../../extern_libs_v2/khronos/openmax/include \
  ../../../../video/m4v_h263/enc/src \
  ../../../../video/m4v_h263/enc/include \
  ../../../../utilities/colorconvert/include

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := mpeg4_enc.cpp \
	omx_mpeg4enc_component.cpp


HDRS := mpeg4_enc.h \
	omx_mpeg4enc_component.h
	



include $(MK)/library.mk

