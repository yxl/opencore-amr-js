# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmfrecognizer


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)



XINCDIRS +=  $(VOB_BASE_DIR)/pvmi/pvmf/include \
  $(VOB_BASE_DIR)/baselibs/pv_mime_utils/src



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_recognizer_registry.cpp \
	pvmf_recognizer_registry_impl.cpp

HDRS := pvmf_recognizer_registry.h \
	pvmf_recognizer_types.h \
    pvmf_recognizer_plugin.h

include $(MK)/library.mk



