LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_omxcore.cpp \
 	src/pv_omx_config_parser.cpp \
 	src/pv_omxregistry.cpp


LOCAL_MODULE := libomx_common_lib

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_common/src \
 	$(PV_TOP)/codecs_v2/omx/omx_common/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_omxcore.h \
 	include/pv_omx_config_parser.h \
 	include/pv_omxdefs.h

include $(BUILD_STATIC_LIBRARY)
