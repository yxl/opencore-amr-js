LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_comms_io_node_port.cpp \
 	src/pv_comms_io_node.cpp


LOCAL_MODULE := libpvcommsionode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvcommsionode/src \
 	$(PV_TOP)/nodes/pvcommsionode/src \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/nodes/pvcommsionode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pv_comms_io_node.h \
 	src/../include/pv_comms_io_node_factory.h \
 	src/../include/pv_comms_io_node_events.h

include $(BUILD_STATIC_LIBRARY)
