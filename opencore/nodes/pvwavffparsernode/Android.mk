LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_wavffparser_node.cpp \
 	src/pvmf_wavffparser_port.cpp \
 	src/pvmf_wavffparser_factory.cpp


LOCAL_MODULE := libpvwavffparsernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvwavffparsernode/src \
 	$(PV_TOP)/nodes/pvwavffparsernode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_wavffparser_factory.h \
 	include/pvmf_wavffparser_defs.h

include $(BUILD_STATIC_LIBRARY)
