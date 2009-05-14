LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/getactualaacconfig.cpp \
 	src/byte_align.cpp \
 	src/get_audio_specific_config.cpp \
 	src/get_ele_list.cpp \
 	src/get_ga_specific_config.cpp \
 	src/get_ics_info.cpp \
 	src/get_prog_config.cpp \
 	src/getgroup.cpp \
 	src/lt_decode.cpp \
 	src/infoinit.cpp \
 	src/pvmp4audiodecodergetmemrequirements.cpp \
 	src/pvmp4audiodecoderinitlibrary.cpp \
 	src/set_mc_info.cpp \
 	src/sfb.cpp


LOCAL_MODULE := libgetactualaacconfig

LOCAL_CFLAGS := -DAAC_PLUS -DHQ_SBR -DPARAMETRICSTEREO $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/audio/aac/dec/util/getactualaacconfig/src \
 	$(PV_TOP)/codecs_v2/audio/aac/dec/util/getactualaacconfig/include \
 	$(PV_TOP)/codecs_v2/audio/aac/dec/include \
 	$(PV_TOP)/codecs_v2/audio/aac/dec/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/getactualaacconfig.h

include $(BUILD_STATIC_LIBRARY)
