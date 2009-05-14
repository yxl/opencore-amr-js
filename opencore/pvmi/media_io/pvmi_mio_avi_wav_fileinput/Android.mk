LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pvmi_mio_avi_wav_file.cpp


LOCAL_MODULE := libpvmioaviwavfileinput

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/media_io/pvmi_mio_avi_wav_fileinput/src \
 	$(PV_TOP)/pvmi/media_io/pvmi_mio_avi_wav_fileinput/src \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/pvmi/media_io/pvmi_mio_avi_wav_fileinput/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pvmi_mio_avi_wav_file.h \
 	src/../include/pvmi_mio_avi_wav_file_factory.h

include $(BUILD_STATIC_LIBRARY)
