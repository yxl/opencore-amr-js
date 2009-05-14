LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_protocol_engine_common.cpp \
 	src/pvmf_protocol_engine_node_common.cpp


LOCAL_MODULE := libprotocolenginenode_common

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/android \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/linux \
 	$(PV_TOP)/nodes/pvprotocolenginenode/include \
 	$(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/base/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pvmf_protocol_engine_node_common.h \
 	src/pvmf_protocol_engine_common.h \
 	src/pvmf_protocol_engine_internal.h

include $(BUILD_STATIC_LIBRARY)
