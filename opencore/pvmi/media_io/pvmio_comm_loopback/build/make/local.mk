# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmio_comm_loopback


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += ../../../../pvmf/include ../../../../pvmf_nodes/common/include ../../include 





SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvmi_mio_comm_loopback.cpp \
        pvmi_mio_comm_data_buffer.cpp

HDRS := ../include/pvmi_mio_comm_data_buffer.h \
        ../include/pvmi_mio_comm_loopback_factory.h

include $(MK)/library.mk

