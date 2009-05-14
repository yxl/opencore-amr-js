# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvstreamingmanagernode




XINCDIRS += ../../../common/include ../../../jitterbuffernode/include ../../../jitterbuffernode/include ../../../medialayernode/include ../../../../../protocols/rtp/src ../../../../common/include ../../../../../protocols/sdp/common/include ../../../../../fileformats/asf/parser/include ../../../../../fileformats/rmff_type_specific_parser/src ../../../../../protocols/rdt_parser/realchallenge/include ../../../../../pvmi/content_policy_manager/include ../../../../../pvmi/content_policy_manager/plugins/common/include ../../../../../nodes/pvrtppacketsourcenode/include ../../../../../pvmi/pvmf/include ../../../../../protocols/rtp_payload_parser/rfc_3640/include ../../../../../nodes/pvpvr/include ../../../../../nodes/pvpvr/src ../../../../../nodes/pvpvrnode/include ../../../../../nodes/pvpvrnode/src ../../../../../fileformats/pvr/composer/include
XINCDIRS += ../../plugins/common/include
XINCDIRS += -I../../../../streaming/jitterbuffernode/jitterbuffer/common/include



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_sm_node_factory.cpp 
SRCS +=	pvmf_streaming_manager_node.cpp 		

HDRS := pvmf_sm_node_events.h \
	pvmf_sm_node_factory.h


# include $(call process_include_list,$(LOCAL_PATH),$(STREAMING_PLUGINS))

include $(MK)/library.mk

