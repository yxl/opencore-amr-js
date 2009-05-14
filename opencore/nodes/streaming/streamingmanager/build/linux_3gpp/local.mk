# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvstreamingmanagernode_3gpp

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += ../../../common/include ../../../jitterbuffernode/include ../../../jitterbuffernode/include ../../../medialayernode/include ../../../../../protocols/rtp/src ../../../../common/include ../../../../../protocols/sdp/common/include ../../../../../fileformats/asf/parser/include ../../../../../fileformats/rmff_type_specific_parser/src ../../../../../protocols/rdt_parser/realchallenge/include ../../../../../pvmi/content_policy_manager/include ../../../../../pvmi/content_policy_manager/plugins/common/include ../../../../../nodes/pvrtppacketsourcenode/include ../../../../../pvmi/pvmf/include ../../../../../protocols/rtp_payload_parser/rfc_3640/include ../../../../../nodes/pvpvr/include ../../../../../nodes/pvpvr/src ../../../../../nodes/pvpvrnode/include ../../../../../nodes/pvpvrnode/src ../../../../../fileformats/pvr/composer/include
# For picking the configurations of JB and ML in the pvmf_sm_config.h
XINCDIRS += ../../config/3gpp
XINCDIRS += ../../plugins/common/include

XINCDIRS += ../../../../../protocols/rtsp_client_engine/src
XINCDIRS += ../../plugins/rtspunicast/include



SRCDIR := ../../src
INCSRCDIR := ../../include

# compose final src list for actual build
#BCH

SRCS :=  pvmf_sm_node_factory.cpp pvmf_streaming_manager_node.cpp 		

#FSP Base
SRCS +=	../config/3gpp/pvmf_sm_fsp_registry.cpp 
SRCS +=	../plugins/common/src/pvmf_sm_fsp_base_impl.cpp 
SRCS +=	../plugins/common/src/pvmf_sm_fsp_base_cpm_support.cpp 

#RTSP Unicast Plugin
SRCS +=	../plugins/rtspunicast/src/pvmf_sm_rtsp_unicast_node_factory.cpp 
SRCS +=	../plugins/rtspunicast/src/pvmf_sm_fsp_rtsp_unicast.cpp 

HDRS :=  pvmf_sm_node_events.h \
         pvmf_sm_node_factory.h

include $(MK)/library.mk

