LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_player_datapath.cpp \
 	src/pv_player_engine.cpp \
 	src/pv_player_factory.cpp \
 	src/pv_player_node_registry.cpp \
 	src/../config/core/pv_player_node_registry_populator.cpp


LOCAL_MODULE := libpvplayer_engine

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/player/src \
 	$(PV_TOP)/engines/player/include \
 	$(PV_TOP)/engines/common/include \
 	$(PV_TOP)/engines/player/config/core \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_player_datasinkfilename.h \
 	include/pv_player_datasource.h \
 	include/pv_player_events.h \
 	include/pv_player_factory.h \
 	include/pv_player_datasink.h \
 	include/pv_player_datasourcepvmfnode.h \
 	include/pv_player_interface.h \
 	include/pv_player_datasinkpvmfnode.h \
 	include/pv_player_datasourceurl.h \
 	include/pv_player_types.h \
 	include/pv_player_license_acquisition_interface.h \
 	include/pv_player_track_selection_interface.h \
 	include/pv_player_registry_interface.h

include $(BUILD_STATIC_LIBRARY)
