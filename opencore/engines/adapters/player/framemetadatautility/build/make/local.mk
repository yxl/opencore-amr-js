# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvframemetadatautility

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

XINCDIRS += ../../config/$(BUILD_ARCH)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pv_frame_metadata_utility.cpp \
        pv_frame_metadata_factory.cpp \
        pv_frame_metadata_mio_video.cpp \
        pv_frame_metadata_mio_audio.cpp \
        ../config/common/pv_frame_metadata_mio_video_config.cpp

HDRS := pv_frame_metadata_factory.h \
        pv_frame_metadata_interface.h

include $(MK)/library.mk
