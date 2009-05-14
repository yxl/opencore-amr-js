LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/avcenc_api.cpp \
 	src/bitstream_io.cpp \
 	src/block.cpp \
 	src/findhalfpel.cpp \
 	src/header.cpp \
 	src/init.cpp \
 	src/intra_est.cpp \
 	src/motion_comp.cpp \
 	src/motion_est.cpp \
 	src/pvavcencoder.cpp \
 	src/pvavcencoder_factory.cpp \
 	src/rate_control.cpp \
 	src/residual.cpp \
 	src/sad.cpp \
 	src/sad_halfpel.cpp \
 	src/slice.cpp \
 	src/vlc_encode.cpp


LOCAL_MODULE := libpvavch264enc

LOCAL_CFLAGS := -D__arm__ -DYUV_INPUT -DRGB24_INPUT -DRGB12_INPUT -DYUV420SEMIPLANAR_INPUT $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/video/avc_h264/enc/src \
 	$(PV_TOP)/codecs_v2/video/avc_h264/enc/include \
 	$(PV_TOP)/codecs_v2/video/avc_h264/common/include \
 	$(PV_TOP)/codecs_v2/utilities/colorconvert/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvavcencoder.h \
 	include/pvavcencoder_factory.h \
 	include/pvavcencoderinterface.h

include $(BUILD_STATIC_LIBRARY)
