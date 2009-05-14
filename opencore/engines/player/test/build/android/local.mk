# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvplayer_engine_test



XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

XCPPFLAGS += -DBUILD_OMX_DEC_NODE 


XINCDIRS +=  ../../../../common/include  ../../../../../pvmi/media_io/pvmiofileoutput/include  ../../../../../nodes/pvmediaoutputnode/include  ../../../include  ../../../../../nodes/common/include  ../../../../../extern_libs_v2/khronos/openmax/include

# This makefile is used by opencore config. So, only the android configuration is required
XINCDIRS +=  ../../config/android





SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := test_pv_player_engine.cpp \
        test_pv_player_engine_testset_mio_file.cpp \
        test_pv_player_engine_testset1.cpp \
        test_pv_player_engine_testset5.cpp \
        test_pv_player_engine_testset6.cpp \
        test_pv_player_engine_testset7.cpp \
        test_pv_player_engine_testset8.cpp \
        test_pv_player_engine_testset9.cpp \
        test_pv_player_engine_testset10.cpp \
        test_pv_player_engine_testset11.cpp \
        test_pv_player_engine_testset12.cpp \
        test_pv_player_engine_testset13.cpp \
        test_pv_player_engine_testset_cpmdlapassthru.cpp


LIBS := unit_test opencore_player opencore_common

ifneq ($(ARCHITECTURE),win32)
  SYSLIBS = -lpthread -ldl
endif

include $(MK)/prog.mk

PE_TEST_DIR = ${BUILD_ROOT}/pe_test
PE_TARGET = pvplayer_engine_test

run_pe_test:: $(REALTARGET) default
	$(quiet) ${RM} -r ${PE_TEST_DIR}
	$(quiet) ${MKDIR} ${PE_TEST_DIR}
	$(quiet) $(CP) $(SRC_ROOT)/tools_v2/build/package/opencore/elem/default/pvplayer.cfg $(PE_TEST_DIR)
	$(quiet) $(CP) -r $(SRC_ROOT)/engines/player/test/data/* $(PE_TEST_DIR)
	$(quiet) export LD_LIBRARY_PATH=${BUILD_ROOT}/installed_lib/${HOST_ARCH}; cd $(PE_TEST_DIR) && ${BUILD_ROOT}/bin/${HOST_ARCH}/$(PE_TARGET) $(TEST_ARGS) $(SOURCE_ARGS)
