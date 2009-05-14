LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/avcdec_api.cpp \
 	src/avc_bitstream.cpp \
 	src/header.cpp \
 	src/itrans.cpp \
 	src/pred_inter.cpp \
 	src/pred_intra.cpp \
 	src/pvavcdecoder.cpp \
 	src/pvavcdecoder_factory.cpp \
 	src/residual.cpp \
 	src/slice.cpp \
 	src/vlc.cpp


LOCAL_MODULE := libpvavcdecoder

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/video/avc_h264/dec/src \
 	$(PV_TOP)/codecs_v2/video/avc_h264/dec/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvavcdecoder.h \
 	include/pvavcdecoder_factory.h \
 	include/pvavcdecoderinterface.h

include $(BUILD_STATIC_LIBRARY)
