# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk



TARGET := pvavch264enc

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XCPPFLAGS += -D__arm__ -DYUV_INPUT -DRGB24_INPUT -DRGB12_INPUT -DYUV420SEMIPLANAR_INPUT

XINCDIRS += ../../../common/include  ../../../../../utilities/colorconvert/include

XLIBDIRS += 


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := avcenc_api.cpp \
	bitstream_io.cpp \
	block.cpp \
	findhalfpel.cpp \
	header.cpp \
	init.cpp \
	intra_est.cpp \
	motion_comp.cpp \
	motion_est.cpp \
	pvavcencoder.cpp \
	pvavcencoder_factory.cpp \
	rate_control.cpp \
	residual.cpp \
	sad.cpp \
	sad_halfpel.cpp \
	slice.cpp \
	vlc_encode.cpp

HDRS := pvavcencoder.h \
       pvavcencoder_factory.h \
       pvavcencoderinterface.h


include $(MK)/library.mk

