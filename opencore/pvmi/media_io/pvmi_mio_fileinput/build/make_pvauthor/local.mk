# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmiofileinput 


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

XCPPFLAGS += -DTEXT_TRACK_DESC_INFO

XINCDIRS += ../../../../pvmf/include




SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmi_mio_fileinput.cpp 


HDRS := ../src/pvmi_mio_fileinput.h \
       pvmi_mio_fileinput_factory.h

	




















































include $(MK)/library.mk
