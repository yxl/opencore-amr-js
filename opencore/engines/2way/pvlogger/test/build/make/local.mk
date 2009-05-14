# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvlogger_test


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../../../pvlogger
INCSRCDIR := ../../../../pvlogger/src

# compose final src list for actual build
SRCS :=	test/src/test.cpp src/pv_logger_impl.cpp

LIBS :=  osclio osclproc osclutil osclmemory osclerror osclbase 

SYSLIBS += $(SYS_THREAD_LIB)

include $(MK)/prog.mk

