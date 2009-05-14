# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvyuv420semiplnrtoyuv420plnr

SRCDIR := ../../src


SRCS :=	ccyuv420semiplnrtoyuv420plnr.cpp \
        cczoomrotationbase.cpp

include $(MK)/library.mk
