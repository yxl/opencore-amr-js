# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvid3parcom

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pv_id3_parcom.cpp

HDRS := pv_id3_parcom.h \
       pv_id3_parcom_types.h \
       pv_id3_parcom_constants.h


include $(MK)/library.mk
