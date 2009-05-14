LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pvmfrtspnodereg.cpp


LOCAL_MODULE := libpvrtspreginterface

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/modules/linux_rtsp/node_registry/src \
 	$(PV_TOP)/modules/linux_rtsp/node_registry/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	src/pvmfrtspnodereg.h

include $(BUILD_STATIC_LIBRARY)
