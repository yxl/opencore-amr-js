# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvavifileparser


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pv_avifile.cpp\
	pv_avifile_header.cpp\
	pv_avifile_indx.cpp\
	pv_avifile_parser.cpp\
	pv_avifile_parser_utils.cpp\
	pv_avifile_streamlist.cpp\

HDRS := pv_avifile.h\
	pv_avifile_typedefs.h\
	pv_avifile_status.h\


include $(MK)/library.mk
