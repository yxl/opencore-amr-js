LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/oscl_time.cpp \
 	src/oscl_assert.cpp \
 	src/oscl_byte_order.cpp \
 	src/oscl_tree.cpp \
 	src/oscl_tagtree.cpp \
 	src/oscl_stdstring.cpp \
 	src/oscl_string_utils.cpp \
 	src/oscl_int64_utils.cpp \
 	src/oscl_base.cpp \
 	src/oscl_tls.cpp \
 	src/oscl_mem_basic_functions.cpp \
 	src/oscl_singleton.cpp \
 	src/oscl_vector.cpp \
 	src/oscl_queue.cpp \
 	src/oscl_linked_list.cpp \
 	src/pvlogger.cpp \
 	src/pvlogger_c.cpp


LOCAL_MODULE := libosclbase

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/oscl/oscl/osclbase/src \
 	$(PV_TOP)/oscl/oscl/osclbase/src \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	src/oscl_assert.h \
 	src/oscl_base.h \
 	src/oscl_base_alloc.h \
 	src/oscl_base_macros.h \
 	src/oscl_byte_order.h \
 	src/oscl_defalloc.h \
 	src/oscl_dll.h \
 	src/oscl_exclusive_ptr.h \
 	src/oscl_int64_utils.h \
 	src/oscl_mem_inst.h \
 	src/oscl_mem_basic_functions.h \
 	src/oscl_mem_basic_functions.inl \
 	src/oscl_linked_list.h \
 	src/oscl_lock_base.h \
 	src/oscl_map.h \
 	src/oscl_opaque_type.h \
 	src/oscl_queue.h \
 	src/oscl_refcounter.h \
 	src/oscl_refcounter_memfrag.h \
 	src/oscl_shared_ptr.h \
 	src/oscl_stdstring.h \
 	src/oscl_string_utils.h \
 	src/oscl_tagtree.h \
 	src/oscl_time.h \
 	src/oscl_tls.h \
 	src/oscl_tree.h \
 	src/oscl_types.h \
 	src/oscl_uuid.h \
 	src/oscl_vector.h \
 	src/oscl_singleton.h \
 	src/oscl_assert.inl \
 	src/oscl_byte_order.inl \
 	src/oscl_time.inl \
 	src/pvlogger.h \
 	src/pvlogger_c.h \
 	src/pvlogger_registry.h \
 	src/pvlogger_accessories.h

include $(BUILD_STATIC_LIBRARY)
