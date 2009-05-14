# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvfileoutputnode




XINCDIRS += ../../include ../../../../pvmi/pvmf/include ../../../common/include 





SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_fileoutput_inport.cpp \
	pvmf_fileoutput_node.cpp \
	pvmf_fileoutput_factory.cpp \
	pvmf_fileoutput_node_cap_config.cpp

HDRS := pvmf_fileoutput_config.h\
	pvmf_fileoutput_factory.h


include $(MK)/library.mk

