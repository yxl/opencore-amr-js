# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := threadsafe_callback_ao

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS := threadsafe_callback_ao.cpp \
        threadsafe_mempool.cpp \
        threadsafe_queue.cpp


HDRS := threadsafe_callback_ao.h \
	threadsafe_mempool.h \
	threadsafe_queue.h

include $(MK)/library.mk
