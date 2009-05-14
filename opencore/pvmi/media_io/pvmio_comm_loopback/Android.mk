LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmi_mio_comm_loopback.cpp \
 	src/pvmi_mio_comm_data_buffer.cpp


LOCAL_MODULE := libpvmio_comm_loopback

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/media_io/pvmio_comm_loopback/src \
 	$(PV_TOP)/pvmi/media_io/pvmio_comm_loopback/src \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/pvmi/pvmf_nodes/common/include \
 	$(PV_TOP)/pvmi/media_io/pvmio_comm_loopback/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/../include/pvmi_mio_comm_data_buffer.h \
 	src/../include/pvmi_mio_comm_loopback_factory.h

include $(BUILD_STATIC_LIBRARY)
