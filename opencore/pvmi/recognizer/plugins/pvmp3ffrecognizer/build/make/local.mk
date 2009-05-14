# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp3ffrecognizer


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)



XINCDIRS =  $(VOB_BASE_DIR)/fileformats/mp3/parser/include  $(VOB_BASE_DIR)/pvmi/recognizer/include




SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmp3ffrec_factory.cpp \
       pvmp3ffrec_plugin.cpp

HDRS := pvmp3ffrec_factory.h
	



include $(MK)/library.mk
