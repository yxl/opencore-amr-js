LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	




LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/common/src \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_common_audio_decnode.h \
 	include/pvmf_composer_size_and_duration.h \
 	include/pvmf_data_source_init_extension.h \
 	include/pvmf_data_source_playback_control.h \
 	include/pvmf_data_source_direction_control.h \
 	include/pvmf_download_data_source.h \
 	include/pvmf_download_progress_interface.h \
 	include/pvmf_ffparsernode_extension.h \
 	include/pvmf_media_presentation_info.h \
 	include/pvmf_meta_data_extension.h \
 	include/pvmf_mp4_progdownload_support_extension.h \
 	include/pvmf_node_shared_lib_interface.h \
 	include/pvmf_node_registry.h \
 	include/pvmf_nodes_sync_control.h \
 	include/pvmf_track_level_info_extension.h \
 	include/pvmf_track_selection_extension.h \
 	include/pvmf_local_data_source.h \
 	include/pvmf_source_context_data.h \
 	include/pv_player_node_registry_interface.h \
 	include/pvmf_streaming_data_source.h \
 	include/pvmf_audio_encnode_extension.h \
 	include/pvmfamrencnode_extension.h \
 	include/pvmp4h263encextension.h \
 	include/pvmf_format_progdownload_support_extension.h

include $(BUILD_COPY_HEADERS)
