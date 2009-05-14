# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := rtppayloadparser


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../rfc_3016/include  ../../rfc_3267/include  ../../rfc_3016/include  ../../rfc_2429/include  ../../rfc_3984/include  ../../asf/include  ../../rfc_3984/src  ../../realmedia/include  ../../rfc_3640/include
XINCDIRS += ../../../../fileformats/asf/parser/include/ ../../../../pvmi/pvmf/include ../../../../pvmi/realaudio_deinterleaver/include ../../../../protocols/sdp/common/include ../../../../fileformats/rmff_type_specific_data_parser/src ../../../../baselibs/gen_data_structures/src



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := rtp_payload_parser_base.cpp \
	sequence_gen.cpp


HDRS := bit_util.h \
	payload_parser.h \
	payload_parser_factory.h \
	payload_parser_registry.h \
	sequence_gen.h

include $(MK)/library.mk

