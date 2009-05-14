# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvomx_proxy_lib

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XCPPFLAGS +=

XINCDIRS += \
  ../../../../../extern_libs_v2/khronos/openmax/include \
  ../../../../../pvmi/pvmf/include \
  ../../../../../engines/common/include

SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := omx_proxy_interface.cpp \
	pv_omx_interface_proxy.cpp \
	pv_omx_interface_proxy_handler.cpp \
	pv_omx_interface_proxy_notifier.cpp 


HDRS := omx_proxy_interface.h \
	omx_proxy_vector.h \
	pv_omx_interface_proxy.h \
	pv_omx_interface_proxy_handler.h \
	pv_omx_interface_proxy_notifier.h \
	pv_omx_proxied_interface.h




include $(MK)/library.mk

