LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_videoparser_port.cpp \
 	src/pvmf_videoparser_node.cpp


LOCAL_MODULE := libpvvideoparsernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvvideoparsernode/src \
 	$(PV_TOP)/nodes/pvvideoparsernode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_videoparser_port.h \
 	include/pvmf_videoparser_node.h

include $(BUILD_STATIC_LIBRARY)
