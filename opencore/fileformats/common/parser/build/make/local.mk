# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvfileparserutils







SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvfile.cpp \
	pvmi_datastreamsyncinterface_ref_factory.cpp \
	pvmi_datastreamsyncinterface_ref_impl.cpp


HDRS := pvfile.h \
	pvmi_datastreamsyncinterface_ref_factory.h \
	virtual_buffer.h 

 
include $(MK)/library.mk
install:: headers-install

 
