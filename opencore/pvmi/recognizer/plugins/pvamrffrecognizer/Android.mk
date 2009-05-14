LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvamrffrec_factory.cpp \
 	src/pvamrffrec_plugin.cpp


LOCAL_MODULE := libpvamrffrecognizer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/recognizer/plugins/pvamrffrecognizer/src \
 	$(PV_TOP)/pvmi/recognizer/plugins/pvamrffrecognizer/include \
 	/pvmi/recognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvamrffrec_factory.h

include $(BUILD_STATIC_LIBRARY)
