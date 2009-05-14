LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/omx_proxy_interface.cpp \
 	src/pv_omx_interface_proxy.cpp \
 	src/pv_omx_interface_proxy_handler.cpp \
 	src/pv_omx_interface_proxy_notifier.cpp


LOCAL_MODULE := libpvomx_proxy_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_proxy/src \
 	$(PV_TOP)/codecs_v2/omx/omx_proxy/src \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/engines/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/omx_proxy_interface.h \
 	src/omx_proxy_vector.h \
 	src/pv_omx_interface_proxy.h \
 	src/pv_omx_interface_proxy_handler.h \
 	src/pv_omx_interface_proxy_notifier.h \
 	src/pv_omx_proxied_interface.h

include $(BUILD_STATIC_LIBRARY)
