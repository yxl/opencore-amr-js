# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvamrffparsernode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)







SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_amrffparser_node.cpp pvmf_amrffparser_port.cpp pvmf_amrffparser_factory.cpp

HDRS := pvmf_amrffparser_factory.h \
	pvmf_amrffparser_registry_factory.h \
	pvmf_amrffparser_events.h



include $(MK)/library.mk
