LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/http_composer.cpp \
 	src/http_parser.cpp \
 	src/string_keyvalue_store.cpp


LOCAL_MODULE := libpv_http_parcom

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/http_parcom/src \
 	$(PV_TOP)/protocols/http_parcom/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/http_composer.h \
 	include/http_parser.h \
 	include/http_parser_external.h

include $(BUILD_STATIC_LIBRARY)
