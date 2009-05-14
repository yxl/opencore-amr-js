# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvclientserversocketnode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += ../../include 





SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_clientserver_socket_factory.cpp \
	pvmf_clientserver_socket_node.cpp \
	pvmf_clientserver_socket_port.cpp

HDRS := pvmf_clientserver_socket_factory.h \
	pvmf_clientserver_socket_node.h \
	pvmf_clientserver_socket_port.h \
	pvmf_clientserver_socket_tuneables.h \
	pvmf_socket_buffer_allocators.h

include $(MK)/library.mk

