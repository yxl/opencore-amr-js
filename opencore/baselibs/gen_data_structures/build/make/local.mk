# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvgendatastruct

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS := buf_frag_group.cpp \
	pv_string.cpp \
	pv_string_uri.cpp \
	rtsp_range_utils.cpp \
	base64_codec.cpp \
	bitstreamparser.cpp \
	wchar_size_utils.cpp

HDRS := buf_frag_group.h \
	hash_functions.h \
	hashtable.h \
	linked_list.h \
	pv_string.h \
	pv_string_uri.h \
	rtsp_range_utils.h \
	sorted_list.h \
	bool_array.h \
	rtsp_time_formats.h \
	mime_registry.h \
	base64_codec.h \
	bitstreamparser.h \
	wchar_size_utils.h

include $(MK)/library.mk

