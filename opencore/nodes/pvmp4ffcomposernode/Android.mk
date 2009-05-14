LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmp4ffcn_port.cpp \
 	src/pvmp4ffcn_node.cpp \
 	src/pvmp4ffcn_node_cap_config.cpp


LOCAL_MODULE := libpvmp4ffcomposernode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvmp4ffcomposernode/src \
 	$(PV_TOP)/nodes/pvmp4ffcomposernode/include \
 	/installed_include \
 	$(PV_TOP)/nodes/pvmp4ffcomposernode/src \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/pvmp4ffcomposernode/include \
 	$(PV_TOP)/baselibs/pv_mime_utils/src \
 	$(PV_TOP)/engines/author/include \
 	$(PV_TOP)/baselibs/media_data_structures/src \
 	$(PV_TOP)/oscl/oscl/osclio/src \
 	$(PV_TOP)/fileformats/mp4/composer/config/opencore \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmp4ffcn_clipconfig.h \
 	include/pvmp4ffcn_factory.h \
 	include/pvmp4ffcn_trackconfig.h \
 	include/pvmp4ffcn_types.h

include $(BUILD_STATIC_LIBRARY)
