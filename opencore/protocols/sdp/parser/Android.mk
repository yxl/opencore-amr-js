LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/aacmediainfoparser.cpp \
 	src/amrmediainfoparser.cpp \
 	src/sdp_mediaparser_factory.cpp \
 	src/sdpparser.cpp \
 	src/sdp_info.cpp \
 	src/basemediainfoparser.cpp \
 	src/sdp_parsing_utils.cpp \
 	src/sessioninfoparser.cpp \
 	src/h263mediainfoparser.cpp \
 	src/rfc3640mediainfoparser.cpp \
 	src/h264mediainfoparser.cpp \
 	src/still_imagemediainfoparser.cpp \
 	src/m4vmediainfoparser.cpp \
 	src/pcmamediainfoparser.cpp \
 	src/pcmumediainfoparser.cpp \
 	src/sdp_mediaparser_registry_populator.cpp


LOCAL_MODULE := libpvsdpparser

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/sdp/parser/src \
 	$(PV_TOP)/protocols/sdp/parser/include \
 	$(PV_TOP)/protocols/sdp/common/include \
 	$(PV_TOP)/baselibs/gen_data_structures/src \
 	$(PV_TOP)/protocols/sdp/parser/config/opencore \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/aac_media_info_parser.h \
 	include/sdp_mediaparser_factory.h \
 	include/amr_media_info_parser.h \
 	include/sdp_mediaparser_registry.h \
 	include/sdp_parser.h \
 	include/base_media_info_parser.h \
 	include/sdp_parsing_utils.h \
 	include/session_info_parser.h \
 	include/h263_media_info_parser.h \
 	include/rfc3640_media_info_parser.h \
 	include/h264_media_info_parser.h \
 	include/still_image_media_info_parser.h \
 	include/m4v_media_info_parser.h \
 	include/sdp_mediaparser_registry_populator.h

include $(BUILD_STATIC_LIBRARY)
