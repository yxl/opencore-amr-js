# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pv_rtsp_parcom


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../../rtp/src





SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := rtsp_par_com_outgoing_message.cpp \
	rtsp_parser.cpp \
	rtsp_par_com_incoming_message.cpp \
	rtsp_par_com_message.cpp 

HDRS := rtsp_par_com_constants.h \
	rtsp_par_com_tunable.h \
	rtsp_par_com.h \
	rtsp_parser.h \
	rtsp_par_com_basic_ds.h \
	rtsp_session_types.h \
	rtsp_par_com_message.h \
	rtsp_transport.h

include $(MK)/library.mk

