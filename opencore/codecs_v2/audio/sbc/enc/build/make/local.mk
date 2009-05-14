# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk



TARGET := pv_sbc_enc




XINCDIRS +=  ../../include


XLIBDIRS += 

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := sbcenc_allocation.cpp \
	sbcenc_bitstream.cpp \
	sbcenc_crc8.cpp \
	sbcenc_filter.cpp \
	sbc_encoder.cpp \
	scalefactors.cpp \
	pvsbcencoder.cpp \
	pvsbcencoder_factory.cpp
	
HDRS := pvsbcencoder_factory.h pvsbcencoderinterface.h
LIBS =


include $(MK)/library.mk

