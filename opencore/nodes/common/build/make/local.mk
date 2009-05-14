# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := 

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := 

HDRS :=  pvmf_common_audio_decnode.h \
	pvmf_composer_size_and_duration.h \
	pvmf_data_source_init_extension.h \
	pvmf_data_source_playback_control.h \
	pvmf_data_source_direction_control.h \
	pvmf_download_data_source.h \
	pvmf_download_progress_interface.h \
	pvmf_ffparsernode_extension.h \
	pvmf_media_presentation_info.h \
	pvmf_meta_data_extension.h \
	pvmf_mp4_progdownload_support_extension.h \
	pvmf_node_shared_lib_interface.h \
	pvmf_node_registry.h \
	pvmf_nodes_sync_control.h \
	pvmf_track_level_info_extension.h \
	pvmf_track_selection_extension.h \
	pvmf_local_data_source.h \
	pvmf_source_context_data.h \
	pv_player_node_registry_interface.h \
	pvmf_streaming_data_source.h \
        pvmf_audio_encnode_extension.h \
	pvmfamrencnode_extension.h \
	pvmp4h263encextension.h \
        pvmf_format_progdownload_support_extension.h

include $(MK)/library.mk
