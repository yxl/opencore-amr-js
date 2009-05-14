# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


TARGET = osclerror

# Use XCFLAGS to add extra compiler directives for the C compiler
# if you add any please use "XCFLAGS += ..." not "XCFLAGS = " 
# unless you're sure you want to override the project defaults.
# Please include a comment if you do this.
#
# XCFLAGS += 

# Use XCXXFLAGS to add extra compiler directives for the C++ compiler.
# See notes above regarding overriding project defaults.
#
# XCXXFLAGS += -DUSE_UNICODE  -D_POSIX_PTHREAD_SEMANTICS   #-D_POSIX_SOURCE
XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

# Use XCPPFLAGS to add extra preprocessor directives. Use XINCDIRS
# for other include directories, but please only do this if absolutely
# necessary -- let the makefile system find the include directories if 
# possible.
# See notes above regarding overriding project defaults.
#
# XCPPFLAGS +=  

# Use XINCDIRS for extra include directories. These should be proceeded 
# with "-I" just as they would be when passing to the compiler.
#
# XINCDIRS += 


# Use XLIBDIRS for extra library directories. These should be proceeded 
# with "-L" just as they would be when passing to the linker. 
# Used for building executables.
#
# XLIBDIRS += 

# Use XLDFLAGS for extra linker directives.
# Used for building executables.
#
# XLDFLAGS += 

SRCDIR = ../../src
INCSRCDIR = ../../src

SRCS =  oscl_errno.cpp \
	oscl_error.cpp \
	oscl_error_imp_jumps.cpp \
	oscl_mempool_allocator.cpp \
	oscl_error_trapcleanup.cpp \
	oscl_heapbase.cpp

HDRS =  oscl_errno.h \
	oscl_error_allocator.h \
	oscl_error_codes.h \
	oscl_error.h \
	oscl_error_imp_cppexceptions.h \
	oscl_error_imp_fatalerror.h \
	oscl_error_imp.h \
	oscl_error_imp_jumps.h \
	oscl_mempool_allocator.h \
	oscl_error_trapcleanup.h \
	oscl_exception.h \
	oscl_heapbase.h \
	oscl_namestring.h \
	oscl_errno.inl \
	oscl_heapbase.inl


include $(MK)/library.mk

