LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_recognizer_registry.cpp \
 	src/pvmf_recognizer_registry_impl.cpp


LOCAL_MODULE := libpvmfrecognizer

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/recognizer/src \
 	$(PV_TOP)/pvmi/recognizer/include \
 	/pvmi/pvmf/include \
 	/baselibs/pv_mime_utils/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_recognizer_registry.h \
 	include/pvmf_recognizer_types.h \
 	include/pvmf_recognizer_plugin.h

include $(BUILD_STATIC_LIBRARY)
