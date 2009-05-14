LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_avifile.cpp \
 	src/pv_avifile_header.cpp \
 	src/pv_avifile_indx.cpp \
 	src/pv_avifile_parser.cpp \
 	src/pv_avifile_parser_utils.cpp \
 	src/pv_avifile_streamlist.cpp


LOCAL_MODULE := libpvavifileparser

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/avi/parser/src \
 	$(PV_TOP)/fileformats/avi/parser/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_avifile.h \
 	include/pv_avifile_typedefs.h \
 	include/pv_avifile_status.h

include $(BUILD_STATIC_LIBRARY)
