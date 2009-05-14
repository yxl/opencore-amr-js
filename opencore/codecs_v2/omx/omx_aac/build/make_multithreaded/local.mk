# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_aac_component_lib

XCPPFLAGS += -DAAC_PLUS -DHQ_SBR -DPARAMETRICSTEREO

XINCDIRS += \
    ../../../../../extern_libs_v2/khronos/openmax/include \
    ../../../../omx/omx_baseclass/include \
    ../../../../audio/aac/dec/src \
    ../../../../audio/aac/dec/include

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := aac_decode_frame.cpp \
        omx_aac_component.cpp \
        aac_timestamp.cpp  


HDRS := aac_dec.h \
        omx_aac_component.h \
        aac_timestamp.h


include $(MK)/library.mk

