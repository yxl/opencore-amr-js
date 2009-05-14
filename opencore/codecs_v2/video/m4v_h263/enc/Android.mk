LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/bitstream_io.cpp \
 	src/combined_encode.cpp \
 	src/datapart_encode.cpp \
 	src/dct.cpp \
 	src/findhalfpel.cpp \
 	src/fastcodemb.cpp \
 	src/fastidct.cpp \
 	src/fastquant.cpp \
 	src/pvm4vencoder.cpp \
 	src/me_utils.cpp \
 	src/mp4enc_api.cpp \
 	src/rate_control.cpp \
 	src/motion_est.cpp \
 	src/motion_comp.cpp \
 	src/sad.cpp \
 	src/sad_halfpel.cpp \
 	src/vlc_encode.cpp \
 	src/vop.cpp


LOCAL_MODULE := libpvm4vencoder

LOCAL_CFLAGS := -DBX_RC -DYUV_INPUT -DRGB12_INPUT -DRGB24_INPUT -DYUV420SEMIPLANAR_INPUT $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/video/m4v_h263/enc/src \
 	$(PV_TOP)/codecs_v2/video/m4v_h263/enc/include \
 	$(PV_TOP)/codecs_v2/utilities/colorconvert/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/cvei.h \
 	include/pvm4vencoder.h \
 	include/mp4enc_api.h

include $(BUILD_STATIC_LIBRARY)
