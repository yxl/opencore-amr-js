# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_mastercore_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
         ../../../omx_common/include \
	 ../../../../../extern_libs_v2/khronos/openmax/include 

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR    := ../../src
INCSRCDIR := ../../include

SRCS := 	\
	pv_omxmastercore.cpp

HDRS :=	omx_interface.h 


include $(MK)/library.mk

