# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := getactualaacconfig

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XCPPFLAGS := -DAAC_PLUS -DHQ_SBR -DPARAMETRICSTEREO 

XINCDIRS := ../../../../include ../../../../src


SRCDIR := ../../src
INCSRCDIR := ../../include
SRCS := getactualaacconfig.cpp\
	byte_align.cpp\
	get_audio_specific_config.cpp\
	get_ele_list.cpp\
	get_ga_specific_config.cpp\
	get_ics_info.cpp\
	get_prog_config.cpp\
        getgroup.cpp\
        lt_decode.cpp\
	infoinit.cpp\
	pvmp4audiodecodergetmemrequirements.cpp\
	pvmp4audiodecoderinitlibrary.cpp\
	set_mc_info.cpp\
	sfb.cpp 


HDRS := getactualaacconfig.h

include $(MK)/library.mk

