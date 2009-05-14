LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmp4ffrec_factory.cpp \
 	src/pvmp4ffrec_plugin.cpp


LOCAL_MODULE := libpvmp4ffrecognizer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/recognizer/plugins/pvmp4ffrecognizer/src \
 	$(PV_TOP)/pvmi/recognizer/plugins/pvmp4ffrecognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvmp4ffrec_factory.h

include $(BUILD_STATIC_LIBRARY)
