LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/rtsp_par_com_outgoing_message.cpp \
 	src/rtsp_parser.cpp \
 	src/rtsp_par_com_incoming_message.cpp \
 	src/rtsp_par_com_message.cpp


LOCAL_MODULE := libpv_rtsp_parcom

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/rtsp_parcom/src \
 	$(PV_TOP)/protocols/rtsp_parcom/src \
 	$(PV_TOP)/protocols/rtp/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/rtsp_par_com_constants.h \
 	src/rtsp_par_com_tunable.h \
 	src/rtsp_par_com.h \
 	src/rtsp_parser.h \
 	src/rtsp_par_com_basic_ds.h \
 	src/rtsp_session_types.h \
 	src/rtsp_par_com_message.h \
 	src/rtsp_transport.h

include $(BUILD_STATIC_LIBRARY)
