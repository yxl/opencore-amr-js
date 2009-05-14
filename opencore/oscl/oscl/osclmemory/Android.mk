LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_mem.cpp \
 	src/oscl_mem_imp.cpp \
 	src/oscl_mem_audit.cpp \
 	src/oscl_mem_mempool.cpp


LOCAL_MODULE := libosclmemory

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclmemory/src \
 	$(PV_TOP)/oscl/oscl/osclmemory/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_mem.h \
 	src/oscl_mem_audit.h \
 	src/oscl_mem_auto_ptr.h \
 	src/oscl_mem_mempool.h \
 	src/oscl_mem.inl

include $(BUILD_STATIC_LIBRARY)
