LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvaetest.cpp \
 	src/test_pv_author_engine_testset1.cpp \
 	src/test_pv_author_engine_testset4.cpp \
 	src/./single_core/pvaetestinput.cpp \
 	src/test_pv_author_engine_testset5.cpp \
 	src/test_pv_author_engine_testset6.cpp \
 	src/test_pv_author_engine_testset7.cpp \
 	src/test_pv_mediainput_author_engine.cpp \
 	src/test_pv_author_engine_logger.cpp \
 	src/test_pv_author_engine_testset8.cpp


LOCAL_MODULE := test_pvauthorengine

LOCAL_CFLAGS := -D_IMOTION_SPECIFIC_UT_DISABLE $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := libunit_test  

LOCAL_SHARED_LIBRARIES :=  libopencore_author libopencore_common

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/author/test/src \
 	$(PV_TOP)/engines/author/test/src \
 	$(PV_TOP)/engines/common/include \
 	$(PV_TOP)/engines/author/test/src/single_core \
 	$(PV_TOP)/engines/author/test/config/android \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_EXECUTABLE)
