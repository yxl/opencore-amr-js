# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvthreadmessaging

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS := pv_interface_proxy.cpp \
        pv_interface_proxy_handler.cpp \
        pv_interface_proxy_notifier.cpp

HDRS := pv_interface_proxy.h \
        pv_interface_proxy_handler.h \
        pv_interface_proxy_notifier.h \
        pv_proxied_interface.h \

include $(MK)/library.mk
