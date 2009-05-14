# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := 

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)





SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := 
HDRS := pvmf_cpmplugin_access_interface.h \
	pvmf_cpmplugin_access_interface_factory.h \
	pvmf_cpmplugin_authentication_interface.h \
	pvmf_cpmplugin_authorization_interface.h \
	pvmf_cpmplugin_contentmanagement_interface.h \
	pvmf_cpmplugin_license_interface.h \
	pvmf_cpmplugin_license_interface_types.h \
	pvmf_cpmplugin_interface.h \
	pvmf_cpmplugin_decryption_context.h
 
include $(MK)/library.mk
install:: headers-install

 
