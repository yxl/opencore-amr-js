# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pv324m


XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)


XINCDIRS +=  ../../h223/include  ../../h245/cmn/include  ../../h245/per/include  ../../h245/se/include  ../../h324/include  ../../h324/srp/include  ../../h324/tsc/include  ../../common/include  ../../../common/include  ../../../../../engines/common/include  ../../../tools/general/common/include  ../../../../../nodes/streaming/common/include



SRCDIR := ../../.
INCSRCDIR := ../../.


SRCS += common/src/pvt_common.cpp \
	common/src/h324utils.cpp \
	h223/src/adaptationlayer.cpp \
	h223/src/cpvh223multiplex.cpp \
	h223/src/h223api.cpp \
	h223/src/h223types.cpp \
	h223/src/level0.cpp \
	h223/src/level1.cpp \
	h223/src/level2.cpp \
	h223/src/logicalchannel.cpp \
	h223/src/lowerlayer.cpp \
	h223/src/muxtbl.cpp \
	h324/tsc/src/tscmain.cpp \
	h324/tsc/src/tsc_blc.cpp \
	h324/tsc/src/tsc_capability.cpp \
	h324/tsc/src/tsc_ce.cpp \
	h324/tsc/src/tsc_channelcontrol.cpp \
	h324/tsc/src/tsc_clc.cpp \
	h324/tsc/src/tsc_command.cpp \
	h324/tsc/src/tsc_component.cpp \
	h324/tsc/src/tsc_eventreceive.cpp \
	h324/tsc/src/tsc_h324m_config.cpp \
	h324/tsc/src/tsc_indication.cpp \
	h324/tsc/src/tsc_lc.cpp \
	h324/tsc/src/tsc_msd.cpp \
	h324/tsc/src/tsc_mt.cpp \
	h324/tsc/src/tsc_node_interface.cpp \
	h324/tsc/src/tsc_rme.cpp \
	h324/tsc/src/tsc_statemanager.cpp \
	h324/tsc/src/tsc_sub.cpp \
	h324/tsc/src/tschandling.cpp \
	h324/tsc/src/tscsrpbuffer.cpp \
	h324/srp/src/srp.cpp \
	h245/per/src/per.cpp \
	h245/per/src/genericper.cpp \
	h245/per/src/analyzeper.cpp \
	h245/per/src/h245_analysis.cpp \
	h245/per/src/h245_encoder.cpp \
	h245/per/src/h245_decoder.cpp \
	h245/per/src/h245_deleter.cpp \
	h245/per/src/h245_copier.cpp \
	h245/se/src/annex.cpp \
	h245/se/src/blc.cpp \
	h245/se/src/clc.cpp \
	h245/se/src/lc.cpp \
	h245/se/src/lcblccmn.cpp \
	h245/se/src/ml.cpp \
	h245/se/src/mr.cpp \
	h245/se/src/mt.cpp \
	h245/se/src/rme.cpp \
	h245/se/src/rtd.cpp \
	h245/se/src/se.cpp \
	h245/se/src/sebase.cpp


HDRS := h324/tsc/include/tsc_h324m_config_interface.h \
	common/include/pv_2way_h324m_interface.h \
	common/include/pv_2way_h324m_types.h \
	common/include/pvt_common.h \
        h245/cmn/include/h245def.h \
        h245/per/include/per_common.h \
        h245/per/include/analyzeper.h \
        h245/per/include/genericper.h 

include $(MK)/library.mk

