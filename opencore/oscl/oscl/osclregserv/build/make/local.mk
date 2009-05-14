# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := osclregserv

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := oscl_registry_serv_impl.cpp \
	oscl_registry_serv_impl_global.cpp \
	oscl_registry_serv_impl_tls.cpp


HDRS := oscl_registry_serv_impl.h \
	oscl_registry_serv_impl_global.h \
	oscl_registry_serv_impl_tls.h

include $(MK)/library.mk
