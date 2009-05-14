# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_common_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
  ../../../../../extern_libs_v2/khronos/openmax/include \
  ../../../../../pvmi/pvmf/include

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := \
    pv_omxcore.cpp \
    pv_omx_config_parser.cpp \
    pv_omxregistry.cpp


HDRS := \
    pv_omxcore.h \
    pv_omx_config_parser.h \
    pv_omxdefs.h


include $(MK)/library.mk

