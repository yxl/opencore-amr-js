# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_queue_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
  ../../../../../extern_libs_v2/khronos/openmax/include \
  ../../../../../pvmi/pvmf/include 

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pv_omx_queue.cpp 


HDRS := pv_omx_queue.h 


include $(MK)/library.mk

