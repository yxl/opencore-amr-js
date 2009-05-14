LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmp3ffrec_factory.cpp \
 	src/pvmp3ffrec_plugin.cpp


LOCAL_MODULE := libpvmp3ffrecognizer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/recognizer/plugins/pvmp3ffrecognizer/src \
 	$(PV_TOP)/pvmi/recognizer/plugins/pvmp3ffrecognizer/include \
 	/fileformats/mp3/parser/include \
 	/pvmi/recognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvmp3ffrec_factory.h

include $(BUILD_STATIC_LIBRARY)
