LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_errno.cpp \
 	src/oscl_error.cpp \
 	src/oscl_error_imp_jumps.cpp \
 	src/oscl_mempool_allocator.cpp \
 	src/oscl_error_trapcleanup.cpp \
 	src/oscl_heapbase.cpp


LOCAL_MODULE := libosclerror

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclerror/src \
 	$(PV_TOP)/oscl/oscl/osclerror/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_errno.h \
 	src/oscl_error_allocator.h \
 	src/oscl_error_codes.h \
 	src/oscl_error.h \
 	src/oscl_error_imp_cppexceptions.h \
 	src/oscl_error_imp_fatalerror.h \
 	src/oscl_error_imp.h \
 	src/oscl_error_imp_jumps.h \
 	src/oscl_mempool_allocator.h \
 	src/oscl_error_trapcleanup.h \
 	src/oscl_exception.h \
 	src/oscl_heapbase.h \
 	src/oscl_namestring.h \
 	src/oscl_errno.inl \
 	src/oscl_heapbase.inl

include $(BUILD_STATIC_LIBRARY)
