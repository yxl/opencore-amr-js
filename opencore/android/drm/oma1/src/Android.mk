LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH), arm)
LOCAL_DRM_1_PLUGIN_CFLAG = -DDRM_1_PLUGIN_ARCH_ARM
endif

ifeq ($(TARGET_ARCH), x86)
LOCAL_DRM_1_PLUGIN_CFLAG = -DDRM_1_PLUGIN_ARCH_X86
endif

LOCAL_SRC_FILES := pvmf_cpmplugin_kmj_oma1.cpp \
                   pvoma1_kmj_recognizer.cpp  \
                   pvmf_cpmplugin_kmj_oma1_factory.cpp \
                   pvmf_cpmplugin_kmj_oma1_data_access.cpp
	
LOCAL_MODULE := libkmj_oma1

LOCAL_CFLAGS := $(PV_CFLAGS)
LOCAL_CFLAGS += $(LOCAL_DRM_1_PLUGIN_CFLAG)


LOCAL_C_INCLUDES := $(PV_TOP)//android/drm/oma1/src \
                    $(PV_INCLUDES) \
                    $(call include-path-for, libdrm1) \
                    $(call include-path-for, system-core)/cutils

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := \
	pvmf_cpmplugin_kmj_oma1.h \
	pvmf_cpmplugin_kmj_oma1_types.h \
	pvmf_cpmplugin_kmj_oma1_factory.h \
	pvoma1_kmj_recognizer.h \
	pvmf_cpmplugin_kmj_oma1_data_access.h

LOCAL_SHARED_LIBRARIES := libdrm1

include $(BUILD_STATIC_LIBRARY)
