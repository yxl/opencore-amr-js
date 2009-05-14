# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


TARGET := oscllib

XINCDIRS := ../../../osclio/src

SRCDIR := ../../src
INCSRCDIR := ../../src

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

# compose final src list for actual build
SRCS :=	oscl_shared_library.cpp \
	oscl_configfile_list.cpp \
	oscl_library_list.cpp

HDRS :=	oscl_shared_lib_interface.h \
	oscl_shared_library.h \
	oscl_library_common.h \
	oscl_configfile_list.h \
	oscl_library_list.h


include $(MK)/library.mk
