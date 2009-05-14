# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := rtppayloadparser_3gpp




XINCDIRS +=  ../../rfc_3016/include  ../../rfc_3267/include  ../../rfc_3016/include  ../../rfc_2429/include  ../../rfc_3984/include  ../../rfc_3984/src  ../../rfc_3640/include 
XINCDIRS += ../../../../pvmi/pvmf/include ../../../../protocols/sdp/common/include  ../../../../baselibs/gen_data_structures/src 



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := ../rfc_2429/src/h263_payload_parser.cpp \
	../rfc_2429/src/h263_payload_parser_factory.cpp \
	../rfc_3016/src/m4v_payload_parser.cpp \
	../rfc_3016/src/m4v_payload_parser_factory.cpp \
	../rfc_3016/src/m4v_audio_payload_parser.cpp \
	../rfc_3016/src/m4v_audio_payload_parser_factory.cpp \
	../rfc_3267/src/bit_util.cpp \
	../rfc_3267/src/amr_payload_parser.cpp \
	../rfc_3267/src/amr_payload_parser_factory.cpp \
	../rfc_3984/src/h264_payload_parser.cpp \
	../rfc_3984/src/h264_payload_parser_factory.cpp \
	../rfc_3640/src/rfc3640_payload_parser_factory.cpp \
	../rfc_3640/src/rfc3640_payload_parser.cpp \
	 sequence_gen.cpp

HDRS := bit_util.h \
	payload_parser.h \
	payload_parser_factory.h \
	payload_parser_registry.h \
	../rfc_2429/include/h263_payload_parser.h \
	../rfc_2429/include/h263_payload_parser_factory.h \
	../rfc_3016/include/m4v_payload_parser.h \
	../rfc_3016/include/m4v_payload_parser_factory.h \
	../rfc_3016/include/m4v_audio_payload_parser.h \
	../rfc_3016/include/m4v_audio_payload_parser_factory.h \
	../rfc_3267/include/amr_payload_parser.h \
	../rfc_3267/include/amr_payload_parser_factory.h \
	../rfc_3984/include/h264_payload_parser.h \
	../rfc_3984/include/h264_payload_parser_factory.h \
	../rfc_3640/include/rfc3640_payload_parser.h \
	../rfc_3640/include/rfc3640_payload_parser_factory.h \
	sequence_gen.h

include $(MK)/library.mk

