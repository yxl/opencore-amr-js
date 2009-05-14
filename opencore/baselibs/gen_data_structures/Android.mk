LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/buf_frag_group.cpp \
 	src/pv_string.cpp \
 	src/pv_string_uri.cpp \
 	src/rtsp_range_utils.cpp \
 	src/base64_codec.cpp \
 	src/bitstreamparser.cpp \
 	src/wchar_size_utils.cpp


LOCAL_MODULE := libpvgendatastruct

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/baselibs/gen_data_structures/src \
 	$(PV_TOP)/baselibs/gen_data_structures/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/buf_frag_group.h \
 	src/hash_functions.h \
 	src/hashtable.h \
 	src/linked_list.h \
 	src/pv_string.h \
 	src/pv_string_uri.h \
 	src/rtsp_range_utils.h \
 	src/sorted_list.h \
 	src/bool_array.h \
 	src/rtsp_time_formats.h \
 	src/mime_registry.h \
 	src/base64_codec.h \
 	src/bitstreamparser.h \
 	src/wchar_size_utils.h

include $(BUILD_STATIC_LIBRARY)
