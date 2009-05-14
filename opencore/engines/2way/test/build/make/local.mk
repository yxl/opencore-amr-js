# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


ifeq ($(USING_MONA),1)
TARGET := pv2way_mona_engine_test
else
ifeq ($(USING_OMX),1)
TARGET := pv2way_omx_engine_test
else
TARGET := pv2way_engine_test
endif
endif


ifeq ($(USING_MONA),1)
MONA_FLAGS = -DUSING_MONA
XCPPFLAGS += -DPV_USE_AMR_CODECS $(SIPCPPFLAGS) $(MONA_FLAGS) $(OMX_FLAGS)
else
XCPPFLAGS += -DPV_USE_AMR_CODECS $(SIPCPPFLAGS) $(OMX_FLAGS)
endif

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

ifeq ($(USING_MONA),1)
XINCDIRS +=  ../../h223/mona/include  ../../h324/tsc/mona/include 
endif

SRCDIR := ../../src
INCSRCDIR := ../../include
XINCDIRS +=  ../../../pvlogger/src 

SRCS := alloc_dealloc_test.cpp \
	av_duplicate_test.cpp \
	test_engine.cpp \
	init_cancel_test.cpp \
	init_test.cpp \
	test_base.cpp \
	../../pvlogger/src/pv_logger_impl.cpp

SRCS_324 = av_test.cpp \
	connect_cancel_test.cpp \
	connect_test.cpp \
	audio_only_test.cpp \
	video_only_test.cpp \
	user_input_test.cpp

SRCS += $(SRCS_324)



ifeq ($(USING_OMX),1)
ifeq ($(pv2wayengine_lib),y)
FULL_LIBS =  pv2wayengine \
	     pv324m \
	     pvomxvideodecnode \
	     pvomxaudiodecnode \
	     pvomxencnode \
	     pvomxbasedecnode \
	     omx_common_lib \
	     omx_m4v_component_lib \
	     pvmp4decoder \
	     omx_amr_component_lib \
	     pvdecoder_gsmamr \
             pvamrwbdecoder \
	     omx_amrenc_component_lib \
	     pvencoder_gsmamr \
	     pv_amr_nb_common_lib \
	     omx_m4venc_component_lib \
	     pvm4vencoder \
	     omx_baseclass_lib \
	     pvomx_proxy_lib \
	     omx_queue_lib \
	     pvvideoparsernode \
	     pvgeneraltools \
	     pvcommsionode \
	     pvmediaoutputnode \
	     pvmediainputnode \
	     colorconvert \
	     pvmio_comm_loopback \
	     pvmiofileinput \
	     pvmiofileoutput\
	     pvmf \
	     pvlatmpayloadparser \
	     pvgendatastruct \
	     pvmediadatastruct \
	     pvthreadmessaging \
	     pv_config_parser \
	     m4v_config \
	     getactualaacconfig \
	     pvmimeutils \
	     osclregcli \
	     osclregserv \
             osclio \
             osclproc \
             osclutil \
             osclmemory \
	     osclerror \
	     osclbase \
             unit_test \
             threadsafe_callback_ao
else
ifeq ($(pv2wayengine_lib),m)
FULL_LIBS =  opencore_2way \
pvomxvideodecnode pvomxaudiodecnode pvomxencnode pvomxbasedecnode \
omx_common_lib omx_m4v_component_lib omx_amr_component_lib \
omx_amrenc_component_lib omx_m4venc_component_lib \
omx_baseclass_lib pvomx_proxy_lib omx_queue_lib \
pvvideoparsernode \
unit_test opencore_common
endif
endif

else
# NON-OMX
FULL_LIBS =  pv2wayengine pv324m \
pvvideodecnode pvamrencnode gsmamrdecnode \
pvvideoencnode pvvideoparsernode pvamrwbdecoder pvmp4decoder pvm4vencoder \
pvencoder_gsmamr pvdecoder_gsmamr pv_amr_nb_common_lib \
pvgeneraltools pvcommsionode pvmediaoutputnode pvmediainputnode \
pvmio_comm_loopback pvmiofileinput pvmiofileoutput \
pvmf pvlatmpayloadparser pvgendatastruct pvmediadatastruct \
colorconvert pvthreadmessaging pvmimeutils \
osclio osclproc osclutil osclmemory osclerror osclbase unit_test 

endif
LIBS := $(FULL_LIBS)

ifneq ($(HOST_ARCH),win32)
SYSLIBS += $(SYS_THREAD_LIB)
endif

include $(MK)/prog.mk



TWOWAY_TARGET = ${TARGET}


ifeq ($(HOST_ARCH),win32)
TWOWAY_TARGET = ${TARGET}.exe
TWOWAYFULL_TARGET = ${TWOWAY_TARGET}
TWOWAY_TEST_DIR = build\2way_test
RUNPREF = 
else
TWOWAY_TEST_DIR = ${BUILD_ROOT}/2way_test
TWOWAYFULL_TARGET = ./${TWOWAY_TARGET}
endif
 

run_2way_test:: $(REALTARGET) default
	$(quiet) ${RM} -r $(TWOWAY_TEST_DIR)
	$(quiet) ${MKDIR} -p $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) $(SRC_ROOT)/tools_v2/build/package/opencore/elem/default/pvplayer.cfg $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) $(SRC_ROOT)/engines/2way/pvlogger/config/pvlogger.ini $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) ${BUILD_ROOT}/bin/${HOST_ARCH}/$(TWOWAY_TARGET) $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) -r $(SRC_ROOT)/engines/2way/test/test_data/* $(TWOWAY_TEST_DIR)
	$(quiet) export LD_LIBRARY_PATH=${BUILD_ROOT}/installed_lib/${HOST_ARCH} && \
	cd $(TWOWAY_TEST_DIR) && $(TWOWAYFULL_TARGET) $(TEST_ARGS) $(SOURCE_ARGS)
