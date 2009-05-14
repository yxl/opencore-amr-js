# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp4ffcomposernode


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += $(SDK_LOCAL)/installed_include ../../src ../../../../nodes/common/include ../../../../pvmi/pvmf/include  ../../include  ../../../../baselibs/pv_mime_utils/src  ../../../../engines/author/include  ../../../../baselibs/media_data_structures/src  ../../../../oscl/oscl/osclio/src  ../../../../fileformats/mp4/composer/config/opencore



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmp4ffcn_port.cpp pvmp4ffcn_node.cpp pvmp4ffcn_node_cap_config.cpp

HDRS := pvmp4ffcn_clipconfig.h pvmp4ffcn_factory.h pvmp4ffcn_trackconfig.h pvmp4ffcn_types.h 


include $(MK)/library.mk
