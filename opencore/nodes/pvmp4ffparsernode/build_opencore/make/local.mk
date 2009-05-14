# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4ffparsernode




XINCDIRS += ../../src/default 
XINCDIRS += ../../../../fileformats/mp4/parser/utils/mp4recognizer/include




SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_mp4ffparser_node.cpp pvmf_mp4ffparser_factory.cpp pvmf_mp4ffparser_outport.cpp pvmf_mp4ffparser_node_metadata.cpp pvmf_mp4ffparser_node_cap_config.cpp

HDRS := pvmf_mp4ffparser_factory.h \
	pvmf_mp4ffparser_events.h



include $(MK)/library.mk
