LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_omx_basedec_node.cpp \
 	src/pvmf_omx_basedec_port.cpp \
 	src/pvmf_omx_basedec_callbacks.cpp


LOCAL_MODULE := libpvomxbasedecnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvomxbasedecnode/src \
 	$(PV_TOP)/nodes/pvomxbasedecnode/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_omx_basedec_defs.h \
 	include/pvmf_omx_basedec_port.h \
 	include/pvmf_omx_basedec_node.h \
 	include/pvmf_omx_basedec_callbacks.h \
 	include/pvmf_omx_basedec_node_extension_interface.h

include $(BUILD_STATIC_LIBRARY)
