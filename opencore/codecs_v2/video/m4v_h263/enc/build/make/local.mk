# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvm4vencoder

XINCDIRS +=  ../../../../../utilities/colorconvert/include

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XCPPFLAGS += -DBX_RC -DYUV_INPUT -DRGB12_INPUT -DRGB24_INPUT -DYUV420SEMIPLANAR_INPUT

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := bitstream_io.cpp \
        combined_encode.cpp \
	datapart_encode.cpp \
	dct.cpp \
        findhalfpel.cpp \
	fastcodemb.cpp \
	fastidct.cpp \
	fastquant.cpp \
	pvm4vencoder.cpp \
	me_utils.cpp \
	mp4enc_api.cpp \
	rate_control.cpp \
	motion_est.cpp \
	motion_comp.cpp \
	sad.cpp \
	sad_halfpel.cpp \
	vlc_encode.cpp \
	vop.cpp 

HDRS := cvei.h \
	pvm4vencoder.h \
	mp4enc_api.h



include $(MK)/library.mk
 
