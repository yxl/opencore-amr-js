LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_interface_proxy.cpp \
 	src/pv_interface_proxy_handler.cpp \
 	src/pv_interface_proxy_notifier.cpp


LOCAL_MODULE := libpvthreadmessaging

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/baselibs/thread_messaging/src \
 	$(PV_TOP)/baselibs/thread_messaging/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pv_interface_proxy.h \
 	src/pv_interface_proxy_handler.h \
 	src/pv_interface_proxy_notifier.h \
 	src/pv_proxied_interface.h

include $(BUILD_STATIC_LIBRARY)
