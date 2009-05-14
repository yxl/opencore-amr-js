LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvwavffrec_factory.cpp \
 	src/pvwavffrec_plugin.cpp


LOCAL_MODULE := libpvwavffrecognizer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/recognizer/plugins/pvwavffrecognizer/src \
 	$(PV_TOP)/pvmi/recognizer/plugins/pvwavffrecognizer/include \
 	/pvmi/recognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvwavffrec_factory.h

include $(BUILD_STATIC_LIBRARY)
