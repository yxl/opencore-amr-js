LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_omx_enc_factory.cpp \
 	src/pvmf_omx_enc_node.cpp \
 	src/pvmf_omx_enc_port.cpp \
 	src/pvmf_omx_enc_callbacks.cpp


LOCAL_MODULE := libpvomxencnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvomxencnode/src \
 	$(PV_TOP)/nodes/pvomxencnode/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_omx_enc_defs.h \
 	include/pvmf_omx_enc_factory.h \
 	include/pvmf_omx_enc_port.h

include $(BUILD_STATIC_LIBRARY)
