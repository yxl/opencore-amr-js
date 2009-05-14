# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmedialayernode





XINCDIRS += ../../../common/include ../../../streamingmanager/include ../../../../../protocols/sdp/common/include ../../../../../baselibs/gen_data_structures/src ../../../../../protocols/rtp_payload_parser/rfc_3016/include ../../../../../protocols/rtp_payload_parser/rfc_3984/include ../../../../../protocols/rtp_payload_parser/rfc_3267/include ../../../../../protocols/rtp_payload_parser/rfc_2429/include ../../../../../fileformats/asf/parser/include ../../../../../pvmi/content_policy_manager/plugins/common/include




SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_medialayer_node.cpp \
       pvmf_medialayer_port.cpp

HDRS := pvmf_medialayer_node.h \
		pvmf_medialayer_port.h \
		pvmf_ml_eos_timer.h

include $(MK)/library.mk

