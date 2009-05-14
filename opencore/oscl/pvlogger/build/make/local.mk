# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := 

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS :=	

HDRS := pvlogger_stderr_appender.h \
	pvlogger_file_appender.h \
	pvlogger_mem_appender.h \
	pvlogger_time_and_id_layout.h


include $(MK)/library.mk

