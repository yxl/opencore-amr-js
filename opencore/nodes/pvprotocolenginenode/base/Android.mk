LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_protocol_engine_node.cpp \
 	src/pvmf_protocol_engine_factory.cpp \
 	src/pvmf_protocol_engine_node_registry.cpp \
 	src/pvmf_protocol_engine_port.cpp


LOCAL_MODULE := libprotocolenginenode_base

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvprotocolenginenode/base/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/include \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/android \
 	$(PV_TOP)/nodes/pvprotocolenginenode/config/linux \
 	$(PV_TOP)/nodes/pvprotocolenginenode/protocol_common/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_download/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/progressive_streaming/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/fasttrack_download/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/shoutcast/src \
 	$(PV_TOP)/nodes/pvprotocolenginenode/wm_http_streaming/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	../include/pvmf_protocol_engine_factory.h \
 	../include/pvmf_protocol_engine_defs.h \
 	../include/pvmf_protocol_engine_node_extension.h \
 	../include/pvmf_protocol_engine_command_format_ids.h \
 	../include/pvmf_protocol_engine_node_events.h \
 	../include/../base/src/pvmf_protocol_engine_node_registry_interface.h \
 	../include/../base/src/pvmf_protocol_engine_node_registry_populator_interface.h \
 	../include/../base/src/pvmf_protocol_engine_node_shared_lib_interface.h

include $(BUILD_STATIC_LIBRARY)
