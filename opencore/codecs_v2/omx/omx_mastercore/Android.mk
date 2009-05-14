LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pv_omxmastercore.cpp


LOCAL_MODULE := libomx_mastercore_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_mastercore/src \
 	$(PV_TOP)/codecs_v2/omx/omx_mastercore/include \
 	$(PV_TOP)/codecs_v2/omx/omx_common/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/omx_interface.h

include $(BUILD_STATIC_LIBRARY)
