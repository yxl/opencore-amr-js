LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pv_id3_parcom.cpp


LOCAL_MODULE := libpvid3parcom

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/id3parcom/src \
 	$(PV_TOP)/fileformats/id3parcom/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_id3_parcom.h \
 	include/pv_id3_parcom_types.h \
 	include/pv_id3_parcom_constants.h

include $(BUILD_STATIC_LIBRARY)
