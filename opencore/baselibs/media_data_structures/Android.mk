LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/media_clock_converter.cpp \
 	src/pv_gau.cpp \
 	src/access_unit.cpp \
 	src/access_unit_impl.cpp \
 	src/time_comparison_utils.cpp \
 	src/au_utils.cpp


LOCAL_MODULE := libpvmediadatastruct

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/baselibs/media_data_structures/src \
 	$(PV_TOP)/baselibs/media_data_structures/src \
 	$(PV_TOP)/fileformats/mp4/parser/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/access_unit.h \
 	src/access_unit_impl.h \
 	src/au_utils.h \
 	src/gau_object.h \
 	src/media_clock_converter.h \
 	src/time_comparison_utils.h \
 	src/pv_gau.h

include $(BUILD_STATIC_LIBRARY)
