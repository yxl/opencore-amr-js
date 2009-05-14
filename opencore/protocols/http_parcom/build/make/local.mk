# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pv_http_parcom









SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := http_composer.cpp \
	http_parser.cpp \
	string_keyvalue_store.cpp

HDRS := http_composer.h \
	http_parser.h \
	http_parser_external.h

include $(MK)/library.mk



