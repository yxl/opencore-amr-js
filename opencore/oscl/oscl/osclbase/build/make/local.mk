# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk



TARGET := osclbase

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../src

# compose final src list for actual build
SRCS :=	oscl_time.cpp \
	oscl_assert.cpp \
	oscl_byte_order.cpp \
	oscl_tree.cpp \
	oscl_tagtree.cpp \
	oscl_stdstring.cpp \
	oscl_string_utils.cpp \
	oscl_int64_utils.cpp \
	oscl_base.cpp \
	oscl_tls.cpp \
	oscl_mem_basic_functions.cpp \
	oscl_singleton.cpp \
	oscl_vector.cpp \
	oscl_queue.cpp \
	oscl_linked_list.cpp \
	pvlogger.cpp \
	pvlogger_c.cpp


HDRS :=	oscl_assert.h \
	oscl_base.h \
	oscl_base_alloc.h \
	oscl_base_macros.h \
	oscl_byte_order.h \
	oscl_defalloc.h \
	oscl_dll.h \
	oscl_exclusive_ptr.h \
	oscl_int64_utils.h \
	oscl_mem_inst.h \
	oscl_mem_basic_functions.h \
	oscl_mem_basic_functions.inl \
	oscl_linked_list.h \
	oscl_lock_base.h \
	oscl_map.h \
	oscl_opaque_type.h \
	oscl_queue.h \
	oscl_refcounter.h \
	oscl_refcounter_memfrag.h \
	oscl_shared_ptr.h \
	oscl_stdstring.h \
	oscl_string_utils.h \
	oscl_tagtree.h \
	oscl_time.h \
	oscl_tls.h \
	oscl_tree.h \
	oscl_types.h \
	oscl_uuid.h \
	oscl_vector.h \
	oscl_singleton.h \
	oscl_assert.inl \
	oscl_byte_order.inl \
	oscl_time.inl \
	pvlogger.h \
	pvlogger_c.h \
	pvlogger_registry.h \
	pvlogger_accessories.h


include $(MK)/library.mk
