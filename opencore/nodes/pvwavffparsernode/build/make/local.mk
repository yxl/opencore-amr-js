# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvwavffparsernode

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_wavffparser_node.cpp pvmf_wavffparser_port.cpp pvmf_wavffparser_factory.cpp

HDRS := pvmf_wavffparser_factory.h pvmf_wavffparser_defs.h 



include $(MK)/library.mk
