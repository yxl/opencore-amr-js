# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvaacffparsernode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_aacffparser_node.cpp pvmf_aacffparser_factory.cpp pvmf_aacffparser_outport.cpp

HDRS := pvmf_aacffparser_factory.h \
        pvmf_aacffparser_defs.h \



include $(MK)/library.mk
