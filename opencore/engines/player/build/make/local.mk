# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvplayer_engine

XINCDIRS += ../../../common/include

XINCDIRS +=  ../../config/core 


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pv_player_datapath.cpp \
	pv_player_engine.cpp \
	pv_player_factory.cpp \
        pv_player_node_registry.cpp \
        ../config/core/pv_player_node_registry_populator.cpp

HDRS := pv_player_datasinkfilename.h \
	pv_player_datasource.h \
        pv_player_events.h \
	pv_player_factory.h \
	pv_player_datasink.h \
	pv_player_datasourcepvmfnode.h \
	pv_player_interface.h \
	pv_player_datasinkpvmfnode.h \
	pv_player_datasourceurl.h \
	pv_player_types.h \
	pv_player_license_acquisition_interface.h \
	pv_player_track_selection_interface.h \
	pv_player_registry_interface.h

## This file is need by PLATFORM_EXTRAS in library.mk
sdkinfo_header_name := pv_player_sdkinfo.h

include $(MK)/library.mk

doc_target := pvplayer_engine
doc_title := "PV Player Engine"
doc_paths := "$(SRC_ROOT)/engines/player/include/pv_player_interface.h"
doc_version := $(PVPLAYER_ENGINE_VERSION)

include $(MK)/doc.mk

sdkinfo_target := $(LOCAL_SRCDIR)/pv_player_engine.cpp
sdkinfo_header_filename := $(LOCAL_SRCDIR)/$(sdkinfo_header_name)
sdkinfo_header_macro := PV_PLAYER_SDKINFO
sdkinfo_label_macro := PVPLAYER_ENGINE_SDKINFO_LABEL
sdkinfo_date_macro := PVPLAYER_ENGINE_SDKINFO_DATE

include $(MK)/sdkinfo_nr.mk
