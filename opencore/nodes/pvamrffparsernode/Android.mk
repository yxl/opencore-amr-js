LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_amrffparser_node.cpp \
 	src/pvmf_amrffparser_port.cpp \
 	src/pvmf_amrffparser_factory.cpp


LOCAL_MODULE := libpvamrffparsernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvamrffparsernode/src \
 	$(PV_TOP)/nodes/pvamrffparsernode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_amrffparser_factory.h \
 	include/pvmf_amrffparser_registry_factory.h \
 	include/pvmf_amrffparser_events.h

include $(BUILD_STATIC_LIBRARY)
