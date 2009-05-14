LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/cpm.cpp \
 	src/pvmf_cpmplugin_factory_registry.cpp


LOCAL_MODULE := libcpm

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/content_policy_manager/src \
 	$(PV_TOP)/pvmi/content_policy_manager/include \
 	$(PV_TOP)/pvmi/content_policy_manager/include \
 	$(PV_TOP)/pvmi/content_policy_manager/plugins/common/include \
 	$(PV_TOP)/pvmi/content_policy_manager/plugins/oma1/r75/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/cpm.h \
 	include/cpm_types.h \
 	include/cpm_plugin_registry.h \
 	include/pvmf_cpmplugin_factory_registry.h

include $(BUILD_STATIC_LIBRARY)
