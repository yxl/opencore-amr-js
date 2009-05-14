# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := 


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := 

HDRS := \
	pv_common_types.h \
	pv_config_interface.h \
	pv_engine_observer.h \
	pv_engine_observer_message.h \
	pv_engine_types.h \
	pv_interface_cmd_message.h



include $(MK)/library.mk
install:: headers-install

 
