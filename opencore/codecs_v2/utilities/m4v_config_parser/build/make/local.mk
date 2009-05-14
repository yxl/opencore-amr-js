# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := m4v_config


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := \
	m4v_config_parser.cpp

HDRS := \
	m4v_config_parser.h

include $(MK)/library.mk
