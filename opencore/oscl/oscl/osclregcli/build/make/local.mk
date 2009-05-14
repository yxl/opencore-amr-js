# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := osclregcli

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := oscl_registry_access_client.cpp \
	oscl_registry_client.cpp \
	oscl_registry_client_impl.cpp


HDRS := oscl_registry_access_client.h \
	oscl_registry_client.h \
	oscl_registry_types.h

include $(MK)/library.mk
