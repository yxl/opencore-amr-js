LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmp4ffrec.cpp \
 	src/atomutils.cpp


LOCAL_MODULE := libmp4recognizer_utility

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/mp4/parser/utils/mp4recognizer/src \
 	$(PV_TOP)/fileformats/mp4/parser/utils/mp4recognizer/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmp4ffrec.h \
 	include/atomdefs.h

include $(BUILD_STATIC_LIBRARY)
