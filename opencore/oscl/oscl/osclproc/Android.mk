LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_execschedulerbase.cpp \
 	src/oscl_scheduler.cpp \
 	src/oscl_scheduler_ao.cpp \
 	src/oscl_scheduler_readyq.cpp \
 	src/oscl_scheduler_threadcontext.cpp \
 	src/oscl_double_list.cpp \
 	src/oscl_timer.cpp \
 	src/oscl_timerbase.cpp \
 	src/oscl_mutex.cpp \
 	src/oscl_semaphore.cpp \
 	src/oscl_thread.cpp \
 	src/oscl_init.cpp


LOCAL_MODULE := libosclproc

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclproc/src \
 	$(PV_TOP)/oscl/oscl/osclproc/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_aostatus.h \
 	src/oscl_aostatus.inl \
 	src/oscl_scheduler.h \
 	src/oscl_scheduler_aobase.h \
 	src/oscl_scheduler_ao.h \
 	src/oscl_scheduler_readyq.h \
 	src/oscl_scheduler_threadcontext.h \
 	src/oscl_scheduler_types.h \
 	src/oscl_scheduler_tuneables.h \
 	src/oscl_double_list.h \
 	src/oscl_double_list.inl \
 	src/oscl_timer.h \
 	src/oscl_mutex.h \
 	src/oscl_semaphore.h \
 	src/oscl_thread.h \
 	src/oscl_procstatus.h \
 	src/oscl_init.h

include $(BUILD_STATIC_LIBRARY)
