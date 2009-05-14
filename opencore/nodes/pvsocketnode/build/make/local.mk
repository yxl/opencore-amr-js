# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvsocketnode


XINCDIRS +=  ../../../streaming/common/include  ../../config/common

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_socket_node.cpp \
        pvmf_socket_node_extension_interface.cpp \
	pvmf_socket_factory.cpp \
	pvmf_socket_port.cpp

HDRS := pvmf_socket_node_extension_interface.h \
	pvmf_socket_factory.h \
	pvmf_socket_node.h \
	pvmf_socket_port.h \
	pvmf_socket_node_events.h


include $(MK)/library.mk

