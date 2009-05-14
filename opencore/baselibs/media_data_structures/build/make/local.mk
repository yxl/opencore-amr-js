# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmediadatastruct

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

XINCDIRS += ../../../../fileformats/mp4/parser/include
XINCDIRS += ../../../../pvmi/pvmf/include


SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS :=	media_clock_converter.cpp \
	pv_gau.cpp \
	access_unit.cpp \
	access_unit_impl.cpp \
	time_comparison_utils.cpp \
	au_utils.cpp


HDRS := access_unit.h \
	access_unit_impl.h \
	au_utils.h \
	gau_object.h \
	media_clock_converter.h \
	time_comparison_utils.h \
	pv_gau.h


include $(MK)/library.mk

