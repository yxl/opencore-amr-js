# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvauthorengine


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)



XINCDIRS += ../../../common/include ../../src/single_core



SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvaenodeutility.cpp pvauthorengine.cpp

HDRS := pvauthorenginefactory.h pvauthorengineinterface.h

## This file is need by PLATFORM_EXTRAS in library.mk
sdkinfo_header_name := pv_author_sdkinfo.h

include $(MK)/library.mk

doc_target := pvauthor_engine
doc_title := "PV Author Engine"
doc_paths := "$(SRC_ROOT)/engines/common/include $(SRC_ROOT)/engines/author/include"
doc_version := $(PVAUTHOR_ENGINE_VERSION)

include $(MK)/doc.mk

sdkinfo_target := $(LOCAL_SRCDIR)/pvauthorengine.cpp
sdkinfo_header_filename := $(LOCAL_SRCDIR)/$(sdkinfo_header_name)
sdkinfo_header_macro := PV_AUTHOR_SDKINFO
sdkinfo_label_macro := PVAUTHOR_ENGINE_SDKINFO_LABEL
sdkinfo_date_macro := PVAUTHOR_ENGINE_SDKINFO_DATE

include $(MK)/sdkinfo_nr.mk
