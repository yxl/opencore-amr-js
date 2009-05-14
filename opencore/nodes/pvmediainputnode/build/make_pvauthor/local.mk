# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmediainputnode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

XCPPFLAGS += -DTEXT_TRACK_DESC_INFO -D_TEST_AE_ERROR_HANDLING

XINCDIRS += ../../../../pvmi/pvmf/include ../../../../nodes/common/include ../../include 





SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvmf_media_input_node_outport.cpp \
		pvmf_media_input_node.cpp \
		pvmf_media_input_data_buffer.cpp \
		pvmf_media_input_node_cap_config.cpp

HDRS := pvmf_media_input_data_buffer.h \
		pvmf_media_input_node.h \
        ../include/pvmf_media_input_node_factory.h \
        ../include/pvmf_media_input_node_events.h

include $(MK)/library.mk

