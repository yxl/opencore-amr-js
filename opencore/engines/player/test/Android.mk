LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/test_pv_player_engine.cpp \
 	src/test_pv_player_engine_testset_mio_file.cpp \
 	src/test_pv_player_engine_testset1.cpp \
 	src/test_pv_player_engine_testset5.cpp \
 	src/test_pv_player_engine_testset6.cpp \
 	src/test_pv_player_engine_testset7.cpp \
 	src/test_pv_player_engine_testset8.cpp \
 	src/test_pv_player_engine_testset9.cpp \
 	src/test_pv_player_engine_testset10.cpp \
 	src/test_pv_player_engine_testset11.cpp \
 	src/test_pv_player_engine_testset12.cpp \
 	src/test_pv_player_engine_testset13.cpp \
 	src/test_pv_player_engine_testset_cpmdlapassthru.cpp


LOCAL_MODULE := pvplayer_engine_test

LOCAL_CFLAGS := -DBUILD_OMX_DEC_NODE $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := libunit_test  

LOCAL_SHARED_LIBRARIES :=  libopencore_player libopencore_common

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/player/test/src \
 	$(PV_TOP)/engines/player/test/src \
 	$(PV_TOP)/engines/common/include \
 	$(PV_TOP)/pvmi/media_io/pvmiofileoutput/include \
 	$(PV_TOP)/nodes/pvmediaoutputnode/include \
 	$(PV_TOP)/engines/player/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/engines/player/test/config/android \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_EXECUTABLE)
