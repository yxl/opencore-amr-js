LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pvmi_media_io_fileoutput.cpp


LOCAL_MODULE := libpvmiofileoutput

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/media_io/pvmiofileoutput/src \
 	$(PV_TOP)/pvmi/media_io/pvmiofileoutput/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmi_media_io_fileoutput.h \
 	include/pvmi_media_io_fileoutput_registry_factory.h \
 	include/avi_write.h

include $(BUILD_STATIC_LIBRARY)
