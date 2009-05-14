LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_mp3ffparser_node.cpp \
 	src/pvmf_mp3ffparser_outport.cpp \
 	src/pvmf_mp3ffparser_factory.cpp


LOCAL_MODULE := libpvmp3ffparsernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvmp3ffparsernode/src \
 	$(PV_TOP)/nodes/pvmp3ffparsernode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_mp3ffparser_factory.h \
 	include/pvmf_mp3ffparser_defs.h

include $(BUILD_STATIC_LIBRARY)
