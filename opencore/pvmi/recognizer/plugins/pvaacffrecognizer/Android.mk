LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvaacffrec_factory.cpp \
 	src/pvaacffrec_plugin.cpp


LOCAL_MODULE := libpvaacffrecognizer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/recognizer/plugins/pvaacffrecognizer/src \
 	$(PV_TOP)/pvmi/recognizer/plugins/pvaacffrecognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvaacffrec_factory.h

include $(BUILD_STATIC_LIBRARY)
