# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


TARGET := osclutil

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src


SRCS :=  oscl_bin_stream.cpp \
	oscl_string.cpp \
	oscl_string_containers.cpp \
	oscl_str_ptr_len.cpp \
	oscl_string_utf8.cpp \
	oscl_tickcount.cpp \
	oscl_utf8conv.cpp \
	oscl_snprintf.cpp \
	oscl_math.cpp \
	oscl_rand.cpp \
	oscl_string_uri.cpp \
	oscl_string_xml.cpp \
	oscl_priqueue.cpp


HDRS :=  oscl_bin_stream.h \
	oscl_bin_stream.inl \
	oscl_media_data.h \
	oscl_media_status.h \
	oscl_priqueue.h \
	oscl_rand.h \
	oscl_rand.inl \
	oscl_snprintf.h \
	oscl_string.h \
	oscl_string_containers.h \
	oscl_string_rep.h \
	oscl_string_utf8.h \
	oscl_str_ptr_len.h \
	oscl_tickcount.h \
	oscl_tickcount.inl \
	oscl_utf8conv.h \
	oscl_math.h \
	oscl_math.inl \
	oscl_string_uri.h \
	oscl_string_xml.h

include $(MK)/library.mk


