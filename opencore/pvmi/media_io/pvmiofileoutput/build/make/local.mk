# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmiofileoutput 


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../../../../nodes/common/include  ../../../../pvmf/include



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmi_media_io_fileoutput.cpp

HDRS := pvmi_media_io_fileoutput.h \
   pvmi_media_io_fileoutput_registry_factory.h \
   avi_write.h

include $(MK)/library.mk
