LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pv_mime_string_utils.cpp


LOCAL_MODULE := libpvmimeutils

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/baselibs/pv_mime_utils/src \
 	$(PV_TOP)/baselibs/pv_mime_utils/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	src/pv_mime_string_utils.h

include $(BUILD_STATIC_LIBRARY)
