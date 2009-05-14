LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_protocol_engine_progressive_download.cpp \
 	src/pvmf_protocol_engine_node_progressive_download.cpp \
 	src/pvmf_protocol_engine_node_progressive_download_container_factory.cpp


LOCAL_MODULE := libprotocolenginenode_pdl

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_download/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_download/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/android \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/linux \
 	$(PV_TOP)/nodes/pvprotocolenginenode/include \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_download/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/base/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pvmf_protocol_engine_progressive_download.h \
 	src/pvmf_protocol_engine_node_progressive_download.h \
 	src/pvmf_protocol_engine_node_progressive_download_container_factory.h

include $(BUILD_STATIC_LIBRARY)
