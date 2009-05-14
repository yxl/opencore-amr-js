# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvdummyinputnode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_dummy_fileinput_node.cpp \
	pvmf_fileinput_port.cpp



HDRS := pvmf_fileinput_node_extension.h \
        pvmf_dummy_fileinput_node_factory.h \
	pvmf_dummy_fileinput_node.h \
	pvmf_fileinput_node_internal.h \
	pvmf_fileinput_port.h \
	pvmf_fileinput_settings.h


include $(MK)/library.mk

