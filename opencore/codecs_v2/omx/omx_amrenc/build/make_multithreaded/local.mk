# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := omx_amrenc_component_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XINCDIRS += \
  ../../../../../extern_libs_v2/khronos/openmax/include \
  ../../../../audio/gsm_amr/amr_nb/common/src \
  ../../../../audio/gsm_amr/amr_nb/common/include \
  ../../../../audio/gsm_amr/amr_nb/enc/src \
  ../../../../audio/gsm_amr/amr_nb/enc/include 


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := amr_enc.cpp \
	omx_amrenc_component.cpp


HDRS := amr_enc.h \
	omx_amrenc_component.h
	



include $(MK)/library.mk

