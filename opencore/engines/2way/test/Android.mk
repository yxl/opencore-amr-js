LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/alloc_dealloc_test.cpp \
 	src/av_duplicate_test.cpp \
 	src/test_engine.cpp \
 	src/init_cancel_test.cpp \
 	src/init_test.cpp \
 	src/test_base.cpp \
 	src/../../pvlogger/src/pv_logger_impl.cpp \
 	src/av_test.cpp \
 	src/connect_cancel_test.cpp \
 	src/connect_test.cpp \
 	src/audio_only_test.cpp \
 	src/video_only_test.cpp \
 	src/user_input_test.cpp


LOCAL_MODULE := pv2way_omx_engine_test

LOCAL_CFLAGS := -DPV_USE_AMR_CODECS $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES :=               libunit_test 

LOCAL_SHARED_LIBRARIES := libopencore_2way               libopencore_common

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/2way/test/src \
 	$(PV_TOP)/engines/2way/test/include \
 	$(PV_TOP)/engines/2way/pvlogger/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_EXECUTABLE)
