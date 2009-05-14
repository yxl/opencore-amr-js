LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/rtp_payload_parser_base.cpp \
 	src/sequence_gen.cpp \
 	src/../rfc_2429/src/h263_payload_parser.cpp \
 	src/../rfc_2429/src/h263_payload_parser_factory.cpp \
 	src/../rfc_3016/src/m4v_payload_parser.cpp \
 	src/../rfc_3016/src/m4v_payload_parser_factory.cpp \
 	src/../rfc_3016/src/m4v_audio_payload_parser.cpp \
 	src/../rfc_3016/src/m4v_audio_payload_parser_factory.cpp \
 	src/../rfc_3267/src/bit_util.cpp \
 	src/../rfc_3267/src/amr_payload_parser.cpp \
 	src/../rfc_3267/src/amr_payload_parser_factory.cpp \
 	src/../rfc_3640/src/rfc3640_payload_parser_factory.cpp \
 	src/../rfc_3640/src/rfc3640_payload_parser.cpp \
 	src/../rfc_3984/src/h264_payload_parser.cpp \
 	src/../rfc_3984/src/h264_payload_parser_factory.cpp


LOCAL_MODULE := librtppayloadparser

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/rtp_payload_parser/src \
 	$(PV_TOP)/protocols/rtp_payload_parser/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3016/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3267/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3016/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_2429/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3984/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/asf/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3984/src \
 	$(PV_TOP)/protocols/rtp_payload_parser/realmedia/include \
 	$(PV_TOP)/protocols/rtp_payload_parser/rfc_3640/include \
 	$(PV_TOP)/fileformats/asf/parser/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/pvmi/realaudio_deinterleaver/include \
 	$(PV_TOP)/protocols/sdp/common/include \
 	$(PV_TOP)/fileformats/rmff_type_specific_data_parser/src \
 	$(PV_TOP)/baselibs/gen_data_structures/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/bit_util.h \
 	include/payload_parser.h \
 	include/payload_parser_factory.h \
 	include/payload_parser_registry.h \
 	include/sequence_gen.h \
 	include/../rfc_2429/include/h263_payload_parser.h \
 	include/../rfc_2429/include/h263_payload_parser_factory.h \
 	include/../rfc_3016/include/m4v_payload_parser.h \
 	include/../rfc_3016/include/m4v_payload_parser_factory.h \
 	include/../rfc_3016/include/m4v_audio_payload_parser.h \
 	include/../rfc_3016/include/m4v_audio_payload_parser_factory.h \
 	include/../rfc_3267/include/amr_payload_parser.h \
 	include/../rfc_3267/include/amr_payload_parser_factory.h \
 	include/../rfc_3640/include/rfc3640_payload_parser.h \
 	include/../rfc_3640/include/rfc3640_payload_parser_factory.h \
 	include/../rfc_3984/include/h264_payload_parser.h \
 	include/../rfc_3984/include/h264_payload_parser_factory.h

include $(BUILD_STATIC_LIBRARY)
