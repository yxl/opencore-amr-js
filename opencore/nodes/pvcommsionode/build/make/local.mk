# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvcommsionode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += ../../../../pvmi/pvmf/include ../../../../nodes/common/include ../../include 





SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pv_comms_io_node_port.cpp \
	pv_comms_io_node.cpp 

HDRS := pv_comms_io_node.h \
        ../include/pv_comms_io_node_factory.h \
        ../include/pv_comms_io_node_events.h

include $(MK)/library.mk

