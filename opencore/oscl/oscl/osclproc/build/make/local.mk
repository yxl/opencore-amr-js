# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := osclproc

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := oscl_execschedulerbase.cpp \
        oscl_scheduler.cpp \
        oscl_scheduler_ao.cpp \
        oscl_scheduler_readyq.cpp \
        oscl_scheduler_threadcontext.cpp \
        oscl_double_list.cpp \
        oscl_timer.cpp \
        oscl_timerbase.cpp \
        oscl_mutex.cpp \
        oscl_semaphore.cpp \
        oscl_thread.cpp \
        oscl_init.cpp

HDRS := oscl_aostatus.h \
        oscl_aostatus.inl \
        oscl_scheduler.h \
        oscl_scheduler_aobase.h \
        oscl_scheduler_ao.h \
        oscl_scheduler_readyq.h \
        oscl_scheduler_threadcontext.h \
        oscl_scheduler_types.h \
        oscl_scheduler_tuneables.h \
        oscl_double_list.h \
        oscl_double_list.inl \
        oscl_timer.h \
        oscl_mutex.h \
        oscl_semaphore.h \
        oscl_thread.h \
        oscl_procstatus.h \
        oscl_init.h

include $(MK)/library.mk

