# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmioaviwavfileinput 


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS += ../../../../pvmf/include ../../include


 XLIBDIRS += -Lpvavifileparser -Lpvwav 


SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvmi_mio_avi_wav_file.cpp 


HDRS := pvmi_mio_avi_wav_file.h \
       ../include/pvmi_mio_avi_wav_file_factory.h



	




















































include $(MK)/library.mk
