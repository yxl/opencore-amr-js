LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_omx_videodec_factory.cpp \
 	src/pvmf_omx_videodec_node.cpp


LOCAL_MODULE := libpvomxvideodecnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvomxvideodecnode/src \
 	$(PV_TOP)/nodes/pvomxvideodecnode/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/codecs_v2/video/wmv_vc1/dec/src \
 	$(PV_TOP)/baselibs/threadsafe_callback_ao/src \
 	$(PV_TOP)/nodes/pvomxbasedecnode/include \
 	$(PV_TOP)/nodes/pvomxbasedecnode/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvmf_omx_videodec_factory.h

include $(BUILD_STATIC_LIBRARY)
