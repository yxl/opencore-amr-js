# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := colorconvert

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XCPPFLAGS += -DFALSE=false

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS :=	ccrgb16toyuv420.cpp \
	ccrgb24torgb16.cpp \
	ccyuv422toyuv420.cpp \
	cczoomrotation12.cpp \
	cczoomrotation16.cpp \
	cczoomrotation24.cpp \
	cczoomrotation32.cpp \
	cczoomrotationbase.cpp \
	cpvvideoblend.cpp \
	ccrgb24toyuv420.cpp \
	ccrgb12toyuv420.cpp \
	ccyuv420semiplnrtoyuv420plnr.cpp \
	ccyuv420toyuv420semi.cpp

HDRS :=  cczoomrotationbase.h \
	cczoomrotation12.h \
	cczoomrotation16.h \
	cczoomrotation24.h \
	cczoomrotation32.h \
	ccrgb16toyuv420.h \
	ccrgb24torgb16.h \
	ccyuv422toyuv420.h \
	colorconv_config.h \
	pvvideoblend.h \
	ccrgb24toyuv420.h \
	ccrgb12toyuv420.h \
	ccyuv420semitoyuv420.h \
	ccyuv420toyuv420semi.h

include $(MK)/library.mk
