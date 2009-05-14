LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_media_output_node_inport.cpp \
 	src/pv_media_output_node.cpp


LOCAL_MODULE := libpvmediaoutputnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvmediaoutputnode/src \
 	$(PV_TOP)/nodes/pvmediaoutputnode/src \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/nodes/pvmediaoutputnode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pv_media_output_node.h \
 	src/../include/pv_media_output_node_factory.h \
 	src/../include/pv_media_output_node_registry_factory.h \
 	src/../include/pv_media_output_node_events.h

include $(BUILD_STATIC_LIBRARY)
