# -*- makefile -*-
#
# This makefile template should be included by makefiles in
# library directories.
#

# Set the directories for the local sources and headers
LOCAL_SRCDIR :=  $(abspath $(LOCAL_PATH)/$(SRCDIR))
TMP_SRCDIR := $(abspath $(LOCAL_PATH)/$(SRCDIR))
LOCAL_INCSRCDIR :=  $(abspath $(LOCAL_PATH)/$(INCSRCDIR))

#$(info LOCAL_PATH = $(LOCAL_PATH))
#$(info LOCAL_SRCDIR = $(LOCAL_SRCDIR))
#$(info TMP_SRCDIR = $(TMP_SRCDIR))
#$(info LOCAL_INCSRCDIR = $(LOCAL_INCSRCDIR))


###########################################################
#
# See if there are any plugin files to include
#
MY_TARGET_PLUGINS := $($(TARGET)_plugins)
SOLIB_OBJDIR_COMP :=
SOLIB_TARGET_COMP :=
ifeq ($(DEFAULT_LIBTYPE),shared-archive)
  ifneq ($(strip $(SOLIB)),)
    SOLIB_TARGET_COMP :=_$(SOLIB)
    ifneq ($(strip $($(TARGET)_plugins_$(SOLIB))),)
      MY_TARGET_PLUGINS := $($(TARGET)_plugins_$(SOLIB))
      SOLIB_OBJDIR_COMP := /$(SOLIB)
    endif
  endif
endif

# $(info plugins to include $(TARGET) = $(MY_TARGET_PLUGINS))
ifneq ($(strip $(MY_TARGET_PLUGINS)),)
include $(call process_include_list,$(LOCAL_PATH),$(MY_TARGET_PLUGINS))
endif


#
# See if there are any additional src files listed. E.g. assembly src files for codecs
#
-include $(call process_include_list,$(LOCAL_PATH),$(TOOLSET)_$(PROCESSOR).mk)
###########################################################
# Rules for installing the header files

HDR_FILES := $(notdir $(HDRS))
LOCAL_INSTALLED_HDRS := $(HDR_FILES:%=$(INCDESTDIR)/%)

$(foreach hdr, $(strip $(HDRS)), $(eval $(call INST_TEMPLATE,$(notdir $(hdr)),$(patsubst %/,%,$(LOCAL_INCSRCDIR)/$(dir $(hdr))),$(INCDESTDIR))))

$(INCDESTDIR)/ALL_HDRS_INSTALLED: $(LOCAL_INSTALLED_HDRS)

###################################################################
# 
#                   Set up the target

# remove any leading / trailing whitespace
TARGET := $(strip $(TARGET))


$(TARGET)_libmode ?= $(DEFAULT_LIBMODE)
$(TARGET)_libtype ?= $(DEFAULT_LIBTYPE)

$(TARGET)_asm_flags ?= $(DEFAULT_CPP_ASM_FLAGS)
XCPPFLAGS += $($(TARGET)_asm_flags) 

# $(info target = $(TARGET), libtype = $($(TARGET)_libtype))

ifeq ($($(TARGET)_libtype),shared-archive)
  CUMULATIVE_TARGET_LIST := $(CUMULATIVE_TARGET_LIST) $(TARGET)
  LIB_EXT:=$(SHARED_ARCHIVE_LIB_EXT)
  OBJSUBDIR:=shared
  XCXXFLAGS+=$(SHARED_CXXFLAGS)
  TMPDEPS = $(patsubst %,$$(%_fullname),$(LIBS))
  $(eval $(TARGET)_LIBDEPS = $(TMPDEPS))
else 
  ifeq ($($(TARGET)_libtype),shared)
    LIB_EXT:=$(SHARED_LIB_EXT)
    OBJSUBDIR:=shared
    XCXXFLAGS+=$(SHARED_CXXFLAGS)
    TMPDEPS = $(patsubst %,$$(%_fullname),$(LIBS))
    $(eval $(TARGET)_LIBDEPS = $(TMPDEPS))
  else
    LIB_EXT:=$(STAT_LIB_EXT)
    OBJSUBDIR:=static
    $(TARGET)_LIBDEPS =
  endif
endif

ifeq ($($(TARGET)_libmode),debug)
  TARGET_NAME_SUFFIX:=_debug
  OBJSUBDIR:=$(OBJSUBDIR)-dbg
  XCPPFLAGS+=$(DEBUG_CPPFLAGS)
  XCXXFLAGS+=$(DEBUG_CXXFLAGS)
else
  TARGET_NAME_SUFFIX:=
  OBJSUBDIR:=$(OBJSUBDIR)-rel
  XCXXFLAGS+=$(OPT_CXXFLAG)
  XCXXFLAGS+=$(RELEASE_CXXFLAGS)
  XCPPFLAGS+=$(RELEASE_CPPFLAGS)
endif

ifneq ($(strip $(OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE)),true)
  XCXXFLAGS += $(OPTIMIZE_FOR_SIZE)
endif

OBJDIR := $(subst $(SRC_ROOT),$(BUILD_ROOT),$(abspath $(LOCAL_PATH)/$(OUTPUT_DIR_COMPONENT)$(SOLIB_OBJDIR_COMP)/$(OBJSUBDIR)))

ifeq ($($(TARGET)_libtype),shared-archive)
  LIBTARGET := $(TARGET:%=$(OBJDIR)/lib%$(TARGET_NAME_SUFFIX).$(LIB_EXT))
else
  LIBTARGET := $(TARGET:%=$(DESTDIR)/lib%$(TARGET_NAME_SUFFIX).$(LIB_EXT))
endif

$(TARGET)$(SOLIB_TARGET_COMP)_fullname := $(LIBTARGET)

#$(info src_root = $(abspath $(SRC_ROOT)))
#$(info build_root = $(abspath $(BUILD_ROOT)))
#$(info objdir = $(OBJDIR))

$(eval $(call set-src-and-obj-names,$(SRCS),$(LOCAL_SRCDIR)))

TMPOBJS := $(patsubst %,$$(%_compiled_objs),$(COMPONENT_LIBS))
$(eval COMPILED_OBJS += $(TMPOBJS))


# save compiled objects in a macro
$(TARGET)_compiled_objs := $(COMPILED_OBJS)

ifneq ($(strip $(FORCED_OBJS)),)
 # The point of this dependency is to force object rebuilds when the 
 # corresponding dependency files are missing (even if the object file exists).
 $(FORCED_OBJS): FORCE
endif

# $(info DEPS = $(DEPS))

ifneq "$(MAKECMDGOALS)" "clean"
  ifneq ($(strip $(FOUND_DEPS)),)
  -include $(FOUND_DEPS)
  endif
endif


# LOCAL_XINCDIRS := $(abspath $(patsubst %,$(LOCAL_PATH)/%,$(filter ../%,$(XINCDIRS))))

# Currently remove -I until the old makefiles are obsoleted
LOCAL_XINCDIRS := $(abspath $(patsubst ../%,$(LOCAL_PATH)/../%,$(patsubst -I%,%,$(XINCDIRS))))


LOCAL_TOTAL_INCDIRS := $(LOCAL_SRCDIR) $(LOCAL_INCSRCDIR) $(LOCAL_XINCDIRS)

# $(info  LOCAL_TOTAL_INCDIRS = $(LOCAL_TOTAL_INCDIRS), XCXXFLAGS = $(XCXXFLAGS))

$(COMPILED_OBJS): XFLAGS := $(XCPPFLAGS) $(patsubst %,-I%,$(LOCAL_TOTAL_INCDIRS)) $(XCXXFLAGS)

#$(info remote_dirs = $(REMOTE_DIRS))

ifneq ($(strip $(REMOTE_DIRS)),)
# $(info remote dirs = $(REMOTE_DIRS))
$(foreach srcdir, $(strip $(REMOTE_DIRS)), $(eval $(call OBJ_TEMPLATE,$(srcdir),$(OBJDIR))))
endif


$(OBJDIR)/%.$(OBJ_EXT): $(LOCAL_SRCDIR)/%.cpp 
	$(call make-cpp-obj-and-depend,$<,$@,$(subst .$(OBJ_EXT),.d,$@),$(XFLAGS))

$(OBJDIR)/%.$(OBJ_EXT): $(LOCAL_SRCDIR)/%.c
	$(call make-c-obj-and-depend,$<,$@,$(subst .$(OBJ_EXT),.d,$@),$(XFLAGS))

# $(info target = $(TARGET), LIBDEPS = $($(TARGET)_LIBDEPS))

$(LIBTARGET): LIBTYPE := $($(TARGET)_libtype)

$(LIBTARGET): $(COMPILED_OBJS) $($(TARGET)_LIBDEPS)
	@echo Build $@
	$(call create_objdir,$(@D))
	$(call generate_$(LIBTYPE)_lib,$@,$^)
	@echo Done

ALL_LIBS_INSTALLED: $(LIBTARGET)

# Pseudo-targets for libraries. With this, we can use "make lib$(TARGET)" instead of "make $(DESTDIR)/lib%$(TARGET_NAME_SUFFIX).$(LIB_EXT)"
# E.g., make libunit_test
lib$(TARGET): $(LIBTARGET)

-include $(MK)/depgraph.mk

TARGET_TYPE := library

-include $(PLATFORM_EXTRAS)


.PRECIOUS:: $(DEPS) $(COMPILED_OBJS)


