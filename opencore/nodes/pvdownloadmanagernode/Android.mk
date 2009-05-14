LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_downloadmanager_factory.cpp \
 	src/pvmf_downloadmanager_node.cpp \
 	src/pvmf_filebufferdatastream_factory.cpp \
 	src/pvmf_memorybufferdatastream_factory.cpp


LOCAL_MODULE := libpvdownloadmanagernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvdownloadmanagernode/src \
 	$(PV_TOP)/nodes/pvdownloadmanagernode/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_downloadmanager_defs.h \
 	include/pvmf_downloadmanager_factory.h \
 	include/pvmf_filebufferdatastream_factory.h \
 	include/pvmf_memorybufferdatastream_factory.h

include $(BUILD_STATIC_LIBRARY)
