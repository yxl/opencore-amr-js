LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_format_type.cpp \
 	src/pvmf_pool_buffer_allocator.cpp \
 	src/pvmf_sync_util_data_queue.cpp \
 	src/pvmf_media_cmd.cpp \
 	src/pvmf_media_data.cpp \
 	src/pvmf_port_base_impl.cpp \
 	src/pvmf_timestamp.cpp \
 	src/pvmf_simple_media_buffer.cpp \
 	src/pvmi_config_and_capability_utils.cpp \
 	src/pvmf_node_interface.cpp \
 	src/pvmf_sync_util.cpp \
 	src/pvmf_basic_errorinfomessage.cpp \
 	src/pvmi_kvp_util.cpp \
 	src/pvmf_mempool.cpp \
 	src/pvmf_media_clock.cpp \
 	src/pvmf_duration_infomessage.cpp \
 	src/pvmf_metadata_infomessage.cpp


LOCAL_MODULE := libpvmf

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/pvmi/pvmf/src \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_interface.h \
 	include/pvmf_node_interface.h \
 	include/pvmi_config_and_capability.h \
 	include/pvmf_event_handling.h \
 	include/pvmf_node_utils.h \
 	include/pvmi_config_and_capability_observer.h \
 	include/pvmf_fixedsize_buffer_alloc.h \
 	include/pvmf_format_type.h \
 	include/pvmf_pool_buffer_allocator.h \
 	include/pvmf_media_msg_format_ids.h \
 	include/pvmf_media_cmd.h \
 	include/pvmf_port_base_impl.h \
 	include/pvmi_config_and_capability_utils.h \
 	include/pvmf_media_data.h \
 	include/pvmf_port_interface.h \
 	include/pvmi_kvp.h \
 	include/pvmf_media_data_impl.h \
 	include/pvmf_return_codes.h \
 	include/pvmi_media_io_observer.h \
 	include/pvmf_media_frag_group.h \
 	include/pvmf_simple_media_buffer.h \
 	include/pvmi_media_transfer.h \
 	include/pvmf_media_msg.h \
 	include/pvmf_sync_util_data_queue.h \
 	include/pvmi_mio_control.h \
 	include/pvmf_media_msg_header.h \
 	include/pvmf_sync_util.h \
 	include/pvmf_meta_data_types.h \
 	include/pvmf_timestamp.h \
 	include/pv_uuid.h \
 	include/pvmf_node_cmd_msg.h \
 	include/pvmf_video.h \
 	include/pvmf_basic_errorinfomessage.h \
 	include/pvmf_errorinfomessage_extension.h \
 	include/pvmi_media_io_clock_extension.h \
 	include/pvmf_fileformat_events.h \
 	include/pvmi_kvp_util.h \
 	include/pvmf_mempool.h \
 	include/pvmi_port_config_kvp.h \
 	include/pvmi_drm_kvp.h \
 	include/pvmi_data_stream_interface.h \
 	include/pvmi_datastreamuser_interface.h \
 	include/pvmf_resizable_simple_mediamsg.h \
 	include/pvmf_media_clock.h \
 	include/pvmf_duration_infomessage.h \
 	include/pvmf_durationinfomessage_extension.h \
 	include/pvmf_counted_ptr.h \
 	include/pvmf_metadata_infomessage.h \
 	include/pvmf_metadatainfomessage_extension.h \
 	include/pvmi_fileio_kvp.h \
 	include/pvmf_timedtext.h

include $(BUILD_STATIC_LIBRARY)
