LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvdl_config_file.cpp \
 	src/pvmf_protocol_engine_download_common.cpp \
 	src/pvmf_protocol_engine_node_download_common.cpp


LOCAL_MODULE := libprotocolenginenode_download_common

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/android \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/linux \
 	$(PV_TOP)/nodes/pvprotocolenginenode/include \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/base/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/pvdl_config_file.h \
 	src/pvmf_protocol_engine_download_common.h \
 	src/pvmf_protocol_engine_node_download_common.h

include $(BUILD_STATIC_LIBRARY)
