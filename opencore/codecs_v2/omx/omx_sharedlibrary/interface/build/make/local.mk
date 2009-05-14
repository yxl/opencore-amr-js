# Get the current local path as the first operation 
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


TARGET := pv_omx_interface

XINCDIRS += \
        ../../../../../../extern_libs_v2/khronos/openmax/include

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src

# compose final src list for actual build
SRCS :=	pv_omx_interface.cpp
HDRS := 


include $(MK)/library.mk

