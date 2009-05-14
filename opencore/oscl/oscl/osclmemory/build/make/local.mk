# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


TARGET := osclmemory

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS :=  oscl_mem.cpp \
	oscl_mem_imp.cpp \
	oscl_mem_audit.cpp \
	oscl_mem_mempool.cpp

HDRS :=  oscl_mem.h \
	oscl_mem_audit.h \
	oscl_mem_auto_ptr.h \
	oscl_mem_mempool.h \
	oscl_mem.inl 

include $(MK)/library.mk

