# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmf

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmf_format_type.cpp \
	pvmf_pool_buffer_allocator.cpp \
	pvmf_sync_util_data_queue.cpp \
	pvmf_media_cmd.cpp \
	pvmf_media_data.cpp \
	pvmf_port_base_impl.cpp \
	pvmf_timestamp.cpp \
	pvmf_simple_media_buffer.cpp \
	pvmi_config_and_capability_utils.cpp \
	pvmf_node_interface.cpp \
	pvmf_sync_util.cpp \
	pvmf_basic_errorinfomessage.cpp \
	pvmi_kvp_util.cpp \
	pvmf_mempool.cpp \
	pvmf_media_clock.cpp \
	pvmf_duration_infomessage.cpp \
	pvmf_metadata_infomessage.cpp

HDRS :=  pv_interface.h \
         pvmf_node_interface.h \
         pvmi_config_and_capability.h \
         pvmf_event_handling.h \
         pvmf_node_utils.h \
         pvmi_config_and_capability_observer.h \
         pvmf_fixedsize_buffer_alloc.h \
         pvmf_format_type.h \
         pvmf_pool_buffer_allocator.h \
         pvmf_media_msg_format_ids.h \
         pvmf_media_cmd.h \
         pvmf_port_base_impl.h \
         pvmi_config_and_capability_utils.h \
         pvmf_media_data.h \
         pvmf_port_interface.h \
         pvmi_kvp.h \
         pvmf_media_data_impl.h \
         pvmf_return_codes.h \
         pvmi_media_io_observer.h \
         pvmf_media_frag_group.h \
         pvmf_simple_media_buffer.h \
         pvmi_media_transfer.h \
         pvmf_media_msg.h \
         pvmf_sync_util_data_queue.h \
         pvmi_mio_control.h \
         pvmf_media_msg_header.h \
         pvmf_sync_util.h \
         pvmf_meta_data_types.h \
         pvmf_timestamp.h \
         pv_uuid.h \
         pvmf_node_cmd_msg.h \
         pvmf_video.h \
         pvmf_basic_errorinfomessage.h \
         pvmf_errorinfomessage_extension.h \
         pvmi_media_io_clock_extension.h \
         pvmf_fileformat_events.h \
         pvmi_kvp_util.h \
         pvmf_mempool.h \
         pvmi_port_config_kvp.h \
         pvmi_drm_kvp.h \
         pvmi_data_stream_interface.h \
         pvmi_datastreamuser_interface.h \
         pvmf_resizable_simple_mediamsg.h \
         pvmf_media_clock.h \
         pvmf_duration_infomessage.h \
         pvmf_durationinfomessage_extension.h \
         pvmf_counted_ptr.h \
         pvmf_metadata_infomessage.h \
         pvmf_metadatainfomessage_extension.h \
         pvmi_fileio_kvp.h  \
         pvmf_timedtext.h

include $(MK)/library.mk
