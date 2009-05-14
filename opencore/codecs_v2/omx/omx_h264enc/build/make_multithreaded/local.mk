# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_avcenc_component_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
  ../../../../../extern_libs_v2/khronos/openmax/include \
  ../../../../video/avc_h264/enc/src \
  ../../../../video/avc_h264/enc/include \
  ../../../../video/avc_h264/common/src \
  ../../../../video/avc_h264/common/include \
  ../../../../utilities/colorconvert/include

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := avc_enc.cpp \
	omx_avcenc_component.cpp


HDRS := avc_enc.h \
	omx_avcenc_component.h
	



include $(MK)/library.mk

