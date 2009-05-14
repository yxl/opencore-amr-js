LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_socket_node.cpp \
 	src/pvmf_socket_node_extension_interface.cpp \
 	src/pvmf_socket_factory.cpp \
 	src/pvmf_socket_port.cpp


LOCAL_MODULE := libpvsocketnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvsocketnode/src \
 	$(PV_TOP)/nodes/pvsocketnode/include \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_TOP)/nodes/pvsocketnode/config/common \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_socket_node_extension_interface.h \
 	include/pvmf_socket_factory.h \
 	include/pvmf_socket_node.h \
 	include/pvmf_socket_port.h \
 	include/pvmf_socket_node_events.h

include $(BUILD_STATIC_LIBRARY)
