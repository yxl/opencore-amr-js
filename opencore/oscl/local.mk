LOCAL_PATH := $(call get_makefile_dir)

# save off the local_path because it will be overwritten 
# by other includes
#SRC_ROOT := $(LOCAL_PATH)


LOCAL_MAKEFILES_LIST := \
$(SRC_ROOT)/oscl/oscl/osclbase/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclerror/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclmemory/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclutil/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclproc/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclio/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclregserv/build/make/local.mk \
$(SRC_ROOT)/oscl/oscl/osclregcli/build/make/local.mk \
$(SRC_ROOT)/oscl/pvlogger/build/make/local.mk

ifeq ($(USE_LOADABLE_MODULES),y)
  LOCAL_MAKEFILES_LIST += $(SRC_ROOT)/oscl/oscl/oscllib/build/make/local.mk 
endif

include $(MK)/intermediate_level.mk

doc_target := oscl
doc_title := "OSCL API"
doc_paths := "$(SRC_ROOT)/oscl/oscl/config/shared $(SRC_ROOT)/oscl/oscl/config/$(BUILD_ARCH) $(SRC_ROOT)/oscl/oscl/osclbase $(SRC_ROOT)/oscl/oscl/osclerror $(SRC_ROOT)/oscl/oscl/osclio $(SRC_ROOT)/oscl/oscl/osclmemory $(SRC_ROOT)/oscl/oscl/osclproc $(SRC_ROOT)/oscl/oscl/osclregcli $(SRC_ROOT)/oscl/oscl/osclregserv $(SRC_ROOT)/oscl/oscl/osclutil" 
doc_version := $(OSCL_VERSION)

include $(MK)/doc.mk
