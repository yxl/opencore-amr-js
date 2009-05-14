# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmediaoutputnode




XINCDIRS += ../../../../pvmi/pvmf/include ../../../../nodes/common/include ../../include 





SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pv_media_output_node_inport.cpp \
	pv_media_output_node.cpp 

HDRS := pv_media_output_node.h \
        ../include/pv_media_output_node_factory.h \
        ../include/pv_media_output_node_registry_factory.h \
        ../include/pv_media_output_node_events.h

include $(MK)/library.mk

