# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvgeneraltools


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../../../common/include




SRCDIR := ../../common/src
INCSRCDIR := ../../common/include

SRCS := crccheck.cpp \
	layer.cpp \
	media_fragment.cpp \
	media_packet.cpp \
	general_tools.cpp \
	pvmf_buffer_data_source.cpp \
	pvmf_buffer_data_sink.cpp \
	pvmf_file_data_sink.cpp \
	pvmf_file_data_source.cpp

HDRS := 

include $(MK)/library.mk
