# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_amr_component_lib

XINCDIRS += \
        ../../../../../extern_libs_v2/khronos/openmax/include \
        ../../../../omx/omx_baseclass/include \
        ../../../../audio/gsm_amr/amr_nb/dec/src \
        ../../../../audio/gsm_amr/amr_nb/dec/include \
        ../../../../audio/gsm_amr/amr_nb/common/include \
        ../../../../audio/gsm_amr/common/dec/include \
        ../../../../audio/gsm_amr/amr_wb/common/include \
        ../../../../audio/gsm_amr/amr_wb/dec/include \
        ../../../../audio/gsm_amr/amr_wb/dec/src

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := amr_decode_frame.cpp \
	omx_amr_component.cpp \
	d_homing.cpp


HDRS := amr_dec.h \
	omx_amr_component.h \
	d_homing.h


include $(MK)/library.mk

