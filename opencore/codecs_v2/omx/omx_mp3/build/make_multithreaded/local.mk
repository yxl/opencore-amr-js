# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_mp3_component_lib


XINCDIRS += \
        ../../../../../extern_libs_v2/khronos/openmax/include \
        ../../../../omx/omx_baseclass/include \
        ../../../../audio/mp3/dec/src \
        ../../../../audio/mp3/dec/include

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := mp3_dec.cpp \
	omx_mp3_component.cpp \
	mp3_timestamp.cpp  


HDRS := mp3_dec.h \
	omx_mp3_component.h \
	mp3_timestamp.h


include $(MK)/library.mk

