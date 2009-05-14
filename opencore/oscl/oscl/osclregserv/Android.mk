LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_registry_serv_impl.cpp \
 	src/oscl_registry_serv_impl_global.cpp \
 	src/oscl_registry_serv_impl_tls.cpp


LOCAL_MODULE := libosclregserv

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclregserv/src \
 	$(PV_TOP)/oscl/oscl/osclregserv/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_registry_serv_impl.h \
 	src/oscl_registry_serv_impl_global.h \
 	src/oscl_registry_serv_impl_tls.h

include $(BUILD_STATIC_LIBRARY)
