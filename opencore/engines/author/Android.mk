LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvaenodeutility.cpp \
 	src/pvauthorengine.cpp


LOCAL_MODULE := libpvauthorengine

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/author/src \
 	$(PV_TOP)/engines/author/include \
 	$(PV_TOP)/engines/common/include \
 	$(PV_TOP)/engines/author/src/single_core \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvauthorenginefactory.h \
 	include/pvauthorengineinterface.h

include $(BUILD_STATIC_LIBRARY)
