LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_2way_datapath.cpp \
 	src/pv_2way_engine.cpp \
 	src/pv_2way_data_channel_datapath.cpp \
 	src/pv_2way_cmd_control_datapath.cpp \
 	src/pv_2way_dec_data_channel_datapath.cpp \
 	src/pv_2way_enc_data_channel_datapath.cpp \
 	src/pv_2way_mux_datapath.cpp \
 	src/pv_2way_preview_datapath.cpp \
 	src/pv_2way_rec_datapath.cpp \
 	src/pv_2way_engine_factory.cpp \
 	src/pv_2way_proxy_adapter.cpp \
 	src/pv_2way_proxy_factory.cpp


LOCAL_MODULE := libpv2wayengine

LOCAL_CFLAGS := -DPV_DISABLE_VIDRECNODE -DPV_DISABLE_DEVSOUNDNODES -DPV_DISABLE_DEVVIDEOPLAYNODE -DSIP_VOIP_PROJECT=1 -DPV2WAY_USE_OMX $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/2way/src \
 	$(PV_TOP)/engines/2way/include \
 	$(PV_TOP)/engines/2way/src \
 	$(PV_TOP)/engines/2way/include \
 	$(PV_TOP)/engines/common/include \
 	$(PV_TOP)/common/pvdebug/src \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h324/tsc/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/common/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h245/cmn/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h245/per/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h245/se/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h324/srp/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h324/tsc/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h223/include \
 	$(PV_TOP)/protocols/systems/common/include \
 	$(PV_TOP)/protocols/systems/tools/general/common/include \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_TOP)/nodes/pvmediainputnode/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_TOP)/nodes/pvomxvideodecnode/include \
 	$(PV_TOP)/nodes/pvomxbasedecnode/include \
 	$(PV_TOP)/nodes/pvomxaudiodecnode/include \
 	$(PV_TOP)/nodes/pvomxencnode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_2way_interface.h \
 	include/pv_2way_engine_factory.h \
 	include/pv_2way_proxy_factory.h

include $(BUILD_STATIC_LIBRARY)
