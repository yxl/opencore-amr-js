LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	common/src/crccheck.cpp \
 	common/src/layer.cpp \
 	common/src/media_fragment.cpp \
 	common/src/media_packet.cpp \
 	common/src/general_tools.cpp \
 	common/src/pvmf_buffer_data_source.cpp \
 	common/src/pvmf_buffer_data_sink.cpp \
 	common/src/pvmf_file_data_sink.cpp \
 	common/src/pvmf_file_data_source.cpp


LOCAL_MODULE := libpvgeneraltools

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/systems/tools/general/common/src \
 	$(PV_TOP)/protocols/systems/tools/general/common/include \
 	$(PV_TOP)/protocols/systems/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

include $(BUILD_STATIC_LIBRARY)
