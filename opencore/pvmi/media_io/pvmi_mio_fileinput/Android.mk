LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pvmi_mio_fileinput.cpp


LOCAL_MODULE := libpvmiofileinput

LOCAL_CFLAGS := -DTEXT_TRACK_DESC_INFO $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/media_io/pvmi_mio_fileinput/src \
 	$(PV_TOP)/pvmi/media_io/pvmi_mio_fileinput/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/../src/pvmi_mio_fileinput.h \
 	include/pvmi_mio_fileinput_factory.h

include $(BUILD_STATIC_LIBRARY)
