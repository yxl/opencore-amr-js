LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_registry_access_client.cpp \
 	src/oscl_registry_client.cpp \
 	src/oscl_registry_client_impl.cpp


LOCAL_MODULE := libosclregcli

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclregcli/src \
 	$(PV_TOP)/oscl/oscl/osclregcli/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_registry_access_client.h \
 	src/oscl_registry_client.h \
 	src/oscl_registry_types.h

include $(BUILD_STATIC_LIBRARY)
