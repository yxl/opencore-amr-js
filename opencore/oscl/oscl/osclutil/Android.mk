LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_bin_stream.cpp \
 	src/oscl_string.cpp \
 	src/oscl_string_containers.cpp \
 	src/oscl_str_ptr_len.cpp \
 	src/oscl_string_utf8.cpp \
 	src/oscl_tickcount.cpp \
 	src/oscl_utf8conv.cpp \
 	src/oscl_snprintf.cpp \
 	src/oscl_math.cpp \
 	src/oscl_rand.cpp \
 	src/oscl_string_uri.cpp \
 	src/oscl_string_xml.cpp \
 	src/oscl_priqueue.cpp


LOCAL_MODULE := libosclutil

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclutil/src \
 	$(PV_TOP)/oscl/oscl/osclutil/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_bin_stream.h \
 	src/oscl_bin_stream.inl \
 	src/oscl_media_data.h \
 	src/oscl_media_status.h \
 	src/oscl_priqueue.h \
 	src/oscl_rand.h \
 	src/oscl_rand.inl \
 	src/oscl_snprintf.h \
 	src/oscl_string.h \
 	src/oscl_string_containers.h \
 	src/oscl_string_rep.h \
 	src/oscl_string_utf8.h \
 	src/oscl_str_ptr_len.h \
 	src/oscl_tickcount.h \
 	src/oscl_tickcount.inl \
 	src/oscl_utf8conv.h \
 	src/oscl_math.h \
 	src/oscl_math.inl \
 	src/oscl_string_uri.h \
 	src/oscl_string_xml.h

include $(BUILD_STATIC_LIBRARY)
