LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	./common/src/pvt_common.cpp \
 	./common/src/h324utils.cpp \
 	./h223/src/adaptationlayer.cpp \
 	./h223/src/cpvh223multiplex.cpp \
 	./h223/src/h223api.cpp \
 	./h223/src/h223types.cpp \
 	./h223/src/level0.cpp \
 	./h223/src/level1.cpp \
 	./h223/src/level2.cpp \
 	./h223/src/logicalchannel.cpp \
 	./h223/src/lowerlayer.cpp \
 	./h223/src/muxtbl.cpp \
 	./h324/tsc/src/tscmain.cpp \
 	./h324/tsc/src/tsc_blc.cpp \
 	./h324/tsc/src/tsc_capability.cpp \
 	./h324/tsc/src/tsc_ce.cpp \
 	./h324/tsc/src/tsc_channelcontrol.cpp \
 	./h324/tsc/src/tsc_clc.cpp \
 	./h324/tsc/src/tsc_command.cpp \
 	./h324/tsc/src/tsc_component.cpp \
 	./h324/tsc/src/tsc_eventreceive.cpp \
 	./h324/tsc/src/tsc_h324m_config.cpp \
 	./h324/tsc/src/tsc_indication.cpp \
 	./h324/tsc/src/tsc_lc.cpp \
 	./h324/tsc/src/tsc_msd.cpp \
 	./h324/tsc/src/tsc_mt.cpp \
 	./h324/tsc/src/tsc_node_interface.cpp \
 	./h324/tsc/src/tsc_rme.cpp \
 	./h324/tsc/src/tsc_statemanager.cpp \
 	./h324/tsc/src/tsc_sub.cpp \
 	./h324/tsc/src/tschandling.cpp \
 	./h324/tsc/src/tscsrpbuffer.cpp \
 	./h324/srp/src/srp.cpp \
 	./h245/per/src/per.cpp \
 	./h245/per/src/genericper.cpp \
 	./h245/per/src/analyzeper.cpp \
 	./h245/per/src/h245_analysis.cpp \
 	./h245/per/src/h245_encoder.cpp \
 	./h245/per/src/h245_decoder.cpp \
 	./h245/per/src/h245_deleter.cpp \
 	./h245/per/src/h245_copier.cpp \
 	./h245/se/src/annex.cpp \
 	./h245/se/src/blc.cpp \
 	./h245/se/src/clc.cpp \
 	./h245/se/src/lc.cpp \
 	./h245/se/src/lcblccmn.cpp \
 	./h245/se/src/ml.cpp \
 	./h245/se/src/mr.cpp \
 	./h245/se/src/mt.cpp \
 	./h245/se/src/rme.cpp \
 	./h245/se/src/rtd.cpp \
 	./h245/se/src/se.cpp \
 	./h245/se/src/sebase.cpp \
 	./h324/tsc/src/tsc_componentregistry.cpp


LOCAL_MODULE := libpv324m

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/protocols/systems/3g-324m_pvterminal \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h223/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h245/cmn/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h245/per/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h245/se/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h324/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h324/srp/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/h324/tsc/include \
 	$(PV_TOP)/protocols/systems/3g-324m_pvterminal/common/include \
 	$(PV_TOP)/protocols/systems/common/include \
 	$(PV_TOP)/engines/common/include \
 	$(PV_TOP)/protocols/systems/tools/general/common/include \
 	$(PV_TOP)/nodes/streaming/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	./h324/tsc/include/tsc_h324m_config_interface.h \
 	./common/include/pv_2way_h324m_interface.h \
 	./common/include/pv_2way_h324m_types.h \
 	./common/include/pvt_common.h \
 	./h245/cmn/include/h245def.h \
 	./h245/per/include/per_common.h \
 	./h245/per/include/analyzeper.h \
 	./h245/per/include/genericper.h

include $(BUILD_STATIC_LIBRARY)
