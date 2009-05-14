# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvjitterbuffer

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

# Added this to for osclconfig.h
XINCDIRS += -I ../../../../../../common/include -I ../../../../../streamingmanager/plugins/mshttp/config -I ../../../../../streamingmanager/include -I ../../../../../../../protocols/rtp/src
XINCDIRS += -I ../../../../../common/include

SRCDIR := ../../src
INCSRCDIR := ../../include

# compose final src list for actual build
SRCS = pvmf_jb_event_notifier.cpp \
	 pvmf_jb_firewall_pkts_impl.cpp \
	 pvmf_jb_jitterbuffermisc.cpp \
	 pvmf_jb_session_duration_timer.cpp \
	 pvmf_jitter_buffer_impl.cpp \
	 pvmf_rtcp_proto_impl.cpp \
	 pvmf_rtcp_timer.cpp

HDRS = pvmf_jb_event_notifier.h \
	pvmf_jb_jitterbuffermisc.h \
	pvmf_jitter_buffer.h \
	pvmf_jitter_buffer_common_types.h \
	pvmf_jitter_buffer_factory.h 

include $(MK)/library.mk

