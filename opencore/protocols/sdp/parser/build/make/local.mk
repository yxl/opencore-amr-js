# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvsdpparser




XINCDIRS += ../../../common/include  ../../../../../baselibs/gen_data_structures/src



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := aacmediainfoparser.cpp\
	amrmediainfoparser.cpp\
	sdp_mediaparser_factory.cpp\
	sdpparser.cpp\
	sdp_info.cpp\
	basemediainfoparser.cpp\
	sdp_parsing_utils.cpp\
	sessioninfoparser.cpp\
	h263mediainfoparser.cpp\
	rfc3640mediainfoparser.cpp\
	h264mediainfoparser.cpp\
	still_imagemediainfoparser.cpp\
	m4vmediainfoparser.cpp\
	pcmamediainfoparser.cpp\
	pcmumediainfoparser.cpp

HDRS := aac_media_info_parser.h\
	sdp_mediaparser_factory.h\
	amr_media_info_parser.h\
	sdp_mediaparser_registry.h\
	sdp_parser.h\
	base_media_info_parser.h\
	sdp_parsing_utils.h\
	session_info_parser.h\
	h263_media_info_parser.h\
	rfc3640_media_info_parser.h\
	h264_media_info_parser.h\
	still_image_media_info_parser.h\
	m4v_media_info_parser.h\
	sdp_mediaparser_registry_populator.h

include $(MK)/library.mk
