# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := test_pvauthorengine

# Comment this out until all warnings are fixed
#XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)
XCPPFLAGS += -D_IMOTION_SPECIFIC_UT_DISABLE

XINCDIRS +=  ../../../../common/include ../../src/single_core ../../config/android ../../../../../pvmi/pvmf/include ../../../../../nodes/common/include  ../../../../../extern_libs_v2/khronos/openmax/include




SRCDIR := ../../src
INCSRCDIR := ../../src

SRCS := pvaetest.cpp test_pv_author_engine_testset1.cpp test_pv_author_engine_testset4.cpp ./single_core/pvaetestinput.cpp test_pv_author_engine_testset5.cpp test_pv_author_engine_testset6.cpp test_pv_author_engine_testset7.cpp test_pv_mediainput_author_engine.cpp test_pv_author_engine_logger.cpp test_pv_author_engine_testset8.cpp


LIBS := unit_test opencore_author opencore_common



ifneq ($(ARCHITECTURE),win32)
  SYSLIBS = -lpthread -ldl
endif

include $(MK)/prog.mk

AE_TEST_DIR = ${BUILD_ROOT}/ae_test
AE_TARGET = test_pvauthorengine

run_ae_test:: $(REALTARGET) default
	$(quiet) ${RM} -r ${AE_TEST_DIR}
	$(quiet) ${MKDIR} ${AE_TEST_DIR}
	$(quiet) $(CP) $(SRC_ROOT)/tools_v2/build/package/opencore/elem/default/pvplayer.cfg $(AE_TEST_DIR)
	$(quiet) $(CP) $(SRC_ROOT)/engines/author/test/test_input/* $(AE_TEST_DIR)
	$(quiet) export LD_LIBRARY_PATH=${BUILD_ROOT}/installed_lib/${HOST_ARCH}; cd $(AE_TEST_DIR) && ${BUILD_ROOT}/bin/${HOST_ARCH}/$(AE_TARGET) $(TEST_ARGS) $(SOURCE_ARGS)

